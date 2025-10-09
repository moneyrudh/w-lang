#include "data_structures/map.h"
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <math.h>

#define DEFAULT_INITIAL_CAPACITY 16
#define DEFAULT_LOAD_FACTOR 0.75f
#define MIN_CAPACITY 8

// ==================== internal helper functions ====================

static size_t get_bucket_index(const Map* map, const void* key) {
    unsigned long hash = map->config.hash(key);
    return hash % map->bucket_count;
}

static void map_resize(Map* map, size_t new_capacity);

// ==================== core API implementation ====================

Map* map_create(size_t initial_capacity, MapConfig config) {
    if (initial_capacity < MIN_CAPACITY) {
        initial_capacity = MIN_CAPACITY;
    }

    // must have hash and equality functions
    assert(config.hash != NULL);
    assert(config.key_equal != NULL);

    Map* map = malloc(sizeof(Map));
    if (!map) return NULL;

    map->buckets = calloc(initial_capacity, sizeof(MapEntry*));
    if (!map->buckets) {
        free(map);
        return NULL;
    }

    map->bucket_count = initial_capacity;
    map->size = 0;
    map->load_factor = DEFAULT_LOAD_FACTOR;
    map->config = config;

    return map;
}

bool map_put(Map* map, void* key, void* value) {
    if (!map) return false;

    // check if resize needed
    if ((float)map->size / map->bucket_count > map->load_factor) {
        map_resize(map, map->bucket_count * 2);
    }

    size_t bucket_idx = get_bucket_index(map, key);
    MapEntry* entry = map->buckets[bucket_idx];

    // check if key exists (update case)
    while (entry) {
        if (map->config.key_equal(entry->key, key)) {
            // update existing entry
            void* new_value = map->config.value_copy
                ? map->config.value_copy(value)
                : value;

            if (map->config.value_free && entry->value != new_value) {
                map->config.value_free(entry->value);
            }
            entry->value = new_value;
            return false;  // Updated existing
        }
        entry = entry->next;
    }

    // create new entry
    MapEntry* new_entry = malloc(sizeof(MapEntry));
    if (!new_entry) return false;

    new_entry->key = map->config.key_copy
        ? map->config.key_copy(key)
        : key;
    new_entry->value = map->config.value_copy
        ? map->config.value_copy(value)
        : value;
    new_entry->next = map->buckets[bucket_idx];
    map->buckets[bucket_idx] = new_entry;
    map->size++;

    return true;  // New entry
}

void* map_get(const Map* map, const void* key) {
    if (!map) return NULL;

    size_t bucket_idx = get_bucket_index(map, key);
    MapEntry* entry = map->buckets[bucket_idx];

    while (entry) {
        if (map->config.key_equal(entry->key, key)) {
            return entry->value;
        }
        entry = entry->next;
    }

    return NULL;
}

bool map_contains(const Map* map, const void* key) {
    return map_get(map, key) != NULL;
}

bool map_remove(Map* map, const void* key) {
    if (!map) return false;

    size_t bucket_idx = get_bucket_index(map, key);
    MapEntry* entry = map->buckets[bucket_idx];
    MapEntry* prev = NULL;

    while (entry) {
        if (map->config.key_equal(entry->key, key)) {
            // found it, remove it
            if (prev) {
                prev->next = entry->next;
            } else {
                map->buckets[bucket_idx] = entry->next;
            }

            // free key and value if free functions provided
            if (map->config.key_free) {
                map->config.key_free(entry->key);
            }
            if (map->config.value_free) {
                map->config.value_free(entry->value);
            }

            free(entry);
            map->size--;
            return true;
        }
        prev = entry;
        entry = entry->next;
    }

    return false;
}

size_t map_size(const Map* map) {
    return map ? map->size : 0;
}

bool map_is_empty(const Map* map) {
    return map_size(map) == 0;
}

void map_clear(Map* map) {
    if (!map) return;

    for (size_t i = 0; i < map->bucket_count; i++) {
        MapEntry* entry = map->buckets[i];
        while (entry) {
            MapEntry* next = entry->next;

            if (map->config.key_free) {
                map->config.key_free(entry->key);
            }
            if (map->config.value_free) {
                map->config.value_free(entry->value);
            }

            free(entry);
            entry = next;
        }
        map->buckets[i] = NULL;
    }

    map->size = 0;
}

void map_destroy(Map* map) {
    if (!map) return;

    map_clear(map);
    free(map->buckets);
    free(map);
}

static void map_resize(Map* map, size_t new_capacity) {
    if (!map || new_capacity < MIN_CAPACITY) return;

    MapEntry** old_buckets = map->buckets;
    size_t old_capacity = map->bucket_count;

    // allocate new buckets
    map->buckets = calloc(new_capacity, sizeof(MapEntry*));
    if (!map->buckets) {
        map->buckets = old_buckets;  // Restore on failure
        return;
    }

    map->bucket_count = new_capacity;
    map->size = 0;

    // rehash all entries
    for (size_t i = 0; i < old_capacity; i++) {
        MapEntry* entry = old_buckets[i];
        while (entry) {
            MapEntry* next = entry->next;

            // reinsert into new buckets
            size_t new_idx = get_bucket_index(map, entry->key);
            entry->next = map->buckets[new_idx];
            map->buckets[new_idx] = entry;
            map->size++;

            entry = next;
        }
    }

    free(old_buckets);
}

// ==================== iteration API implementation ====================

MapIterator map_iterator(const Map* map) {
    MapIterator iter = {
        .map = map,
        .bucket_index = 0,
        .current_entry = NULL
    };

    if (map && map->bucket_count > 0) {
        // find first non-empty bucket
        for (size_t i = 0; i < map->bucket_count; i++) {
            if (map->buckets[i]) {
                iter.bucket_index = i;
                iter.current_entry = map->buckets[i];
                break;
            }
        }
    }

    return iter;
}

bool map_iterator_has_next(MapIterator* iter) {
    return iter && iter->current_entry != NULL;
}

bool map_iterator_next(MapIterator* iter, void** key_out, void** value_out) {
    if (!iter || !iter->current_entry) return false;

    // return current entry
    if (key_out) *key_out = iter->current_entry->key;
    if (value_out) *value_out = iter->current_entry->value;

    // advance to next entry
    if (iter->current_entry->next) {
        iter->current_entry = iter->current_entry->next;
    } else {
        // find next non-empty bucket
        iter->current_entry = NULL;
        for (size_t i = iter->bucket_index + 1; i < iter->map->bucket_count; i++) {
            if (iter->map->buckets[i]) {
                iter->bucket_index = i;
                iter->current_entry = iter->map->buckets[i];
                break;
            }
        }
    }

    return true;
}

// ==================== built-in hash functions ====================

unsigned long hash_int(const void* key) {
    unsigned long k = (unsigned long)(intptr_t)key;
    // wang's integer hash
    k = (k ^ 61) ^ (k >> 16);
    k = k + (k << 3);
    k = k ^ (k >> 4);
    k = k * 0x27d4eb2d;
    k = k ^ (k >> 15);
    return k;
}

unsigned long hash_float(const void* key) {
    float f = *(float*)key;
    // hash the bit pattern of the float
    unsigned int bits;
    memcpy(&bits, &f, sizeof(float));
    return hash_int((void*)(intptr_t)bits);
}

unsigned long hash_char(const void* key) {
    char c = (char)(intptr_t)key;
    return hash_int((void*)(intptr_t)c);
}

unsigned long hash_string(const void* key) {
    const char* str = (const char*)key;
    unsigned long hash = 2166136261u;  // FNV-1a offset basis

    while (*str) {
        hash ^= (unsigned char)(*str++);
        hash *= 16777619;  // FNV-1a prime
    }

    return hash;
}

unsigned long hash_pointer(const void* key) {
    return (unsigned long)(uintptr_t)key;
}

// ==================== built-in equality functions ====================

bool key_equal_int(const void* k1, const void* k2) {
    return (intptr_t)k1 == (intptr_t)k2;
}

bool key_equal_float(const void* k1, const void* k2) {
    float f1 = *(float*)k1;
    float f2 = *(float*)k2;
    // use small epsilon for floating point comparison
    return fabsf(f1 - f2) < 1e-6f;
}

bool key_equal_char(const void* k1, const void* k2) {
    return (char)(intptr_t)k1 == (char)(intptr_t)k2;
}

bool key_equal_string(const void* k1, const void* k2) {
    return strcmp((const char*)k1, (const char*)k2) == 0;
}

bool key_equal_pointer(const void* k1, const void* k2) {
    return k1 == k2;
}

// ==================== built-in copy functions ====================

void* key_copy_string(const void* key) {
    if (!key) return NULL;
    return strdup((const char*)key);
}

void* value_copy_string(const void* value) {
    if (!value) return NULL;
    return strdup((const char*)value);
}

// ==================== built-in free functions ====================

void key_free_string(void* key) {
    free(key);
}

void value_free_string(void* value) {
    free(value);
}
