#include "runtime/wlang_runtime.h"
#include <stdlib.h>
#include <string.h>

// ==================== map creation ====================

Map* wlang_map_create_num_num(void) {
    MapConfig config = {
        .hash = hash_int,
        .key_equal = key_equal_int,
        .key_copy = NULL,        // integers stored as values
        .value_copy = NULL,      // integers stored as values
        .key_free = NULL,
        .value_free = NULL
    };
    return map_create(16, config);
}

Map* wlang_map_create_num_str(void) {
    MapConfig config = {
        .hash = hash_int,
        .key_equal = key_equal_int,
        .key_copy = NULL,        // integers stored as values
        .value_copy = value_copy_string,  // deep copy strings
        .key_free = NULL,
        .value_free = value_free_string
    };
    return map_create(16, config);
}

Map* wlang_map_create_str_num(void) {
    MapConfig config = {
        .hash = hash_string,
        .key_equal = key_equal_string,
        .key_copy = key_copy_string,  // deep copy strings
        .value_copy = NULL,           // integers stored as values
        .key_free = key_free_string,
        .value_free = NULL
    };
    return map_create(16, config);
}

Map* wlang_map_create_str_str(void) {
    MapConfig config = {
        .hash = hash_string,
        .key_equal = key_equal_string,
        .key_copy = key_copy_string,   // deep copy strings
        .value_copy = value_copy_string, // deep copy strings
        .key_free = key_free_string,
        .value_free = value_free_string
    };
    return map_create(16, config);
}

Map* wlang_map_create_real_real(void) {
    MapConfig config = {
        .hash = hash_float,
        .key_equal = key_equal_float,
        .key_copy = NULL,        // floats stored as pointers to heap-allocated values
        .value_copy = NULL,
        .key_free = NULL,
        .value_free = NULL
    };
    return map_create(16, config);
}

Map* wlang_map_create_num_real(void) {
    MapConfig config = {
        .hash = hash_int,
        .key_equal = key_equal_int,
        .key_copy = NULL,
        .value_copy = NULL,
        .key_free = NULL,
        .value_free = NULL
    };
    return map_create(16, config);
}

Map* wlang_map_create_chr_num(void) {
    MapConfig config = {
        .hash = hash_char,
        .key_equal = key_equal_char,
        .key_copy = NULL,
        .value_copy = NULL,
        .key_free = NULL,
        .value_free = NULL
    };
    return map_create(16, config);
}

// ==================== put operations ====================

bool wlang_map_put_num_num(Map* map, int key, int value) {
    return map_put(map, (void*)(intptr_t)key, (void*)(intptr_t)value);
}

bool wlang_map_put_num_str(Map* map, int key, const char* value) {
    return map_put(map, (void*)(intptr_t)key, (void*)value);
}

bool wlang_map_put_str_num(Map* map, const char* key, int value) {
    return map_put(map, (void*)key, (void*)(intptr_t)value);
}

bool wlang_map_put_str_str(Map* map, const char* key, const char* value) {
    return map_put(map, (void*)key, (void*)value);
}

bool wlang_map_put_real_real(Map* map, float key, float value) {
    // allocate heap memory for float keys and values
    float* key_ptr = malloc(sizeof(float));
    float* value_ptr = malloc(sizeof(float));
    if (!key_ptr || !value_ptr) {
        free(key_ptr);
        free(value_ptr);
        return false;
    }
    *key_ptr = key;
    *value_ptr = value;
    return map_put(map, key_ptr, value_ptr);
}

bool wlang_map_put_num_real(Map* map, int key, float value) {
    float* value_ptr = malloc(sizeof(float));
    if (!value_ptr) return false;
    *value_ptr = value;
    return map_put(map, (void*)(intptr_t)key, value_ptr);
}

bool wlang_map_put_chr_num(Map* map, char key, int value) {
    return map_put(map, (void*)(intptr_t)key, (void*)(intptr_t)value);
}

// ==================== get operations ====================

int wlang_map_get_num_num(Map* map, int key, int default_value) {
    void* result = map_get(map, (void*)(intptr_t)key);
    return result ? (int)(intptr_t)result : default_value;
}

const char* wlang_map_get_num_str(Map* map, int key, const char* default_value) {
    void* result = map_get(map, (void*)(intptr_t)key);
    return result ? (const char*)result : default_value;
}

int wlang_map_get_str_num(Map* map, const char* key, int default_value) {
    void* result = map_get(map, (void*)key);
    return result ? (int)(intptr_t)result : default_value;
}

const char* wlang_map_get_str_str(Map* map, const char* key, const char* default_value) {
    void* result = map_get(map, (void*)key);
    return result ? (const char*)result : default_value;
}

float wlang_map_get_real_real(Map* map, float key, float default_value) {
    void* result = map_get(map, &key);
    return result ? *(float*)result : default_value;
}

float wlang_map_get_num_real(Map* map, int key, float default_value) {
    void* result = map_get(map, (void*)(intptr_t)key);
    return result ? *(float*)result : default_value;
}

int wlang_map_get_chr_num(Map* map, char key, int default_value) {
    void* result = map_get(map, (void*)(intptr_t)key);
    return result ? (int)(intptr_t)result : default_value;
}

// ==================== contains operations ====================

bool wlang_map_contains_num(Map* map, int key) {
    return map_contains(map, (void*)(intptr_t)key);
}

bool wlang_map_contains_str(Map* map, const char* key) {
    return map_contains(map, (void*)key);
}

bool wlang_map_contains_real(Map* map, float key) {
    return map_contains(map, &key);
}

bool wlang_map_contains_chr(Map* map, char key) {
    return map_contains(map, (void*)(intptr_t)key);
}

// ==================== remove operations ====================

bool wlang_map_remove_num(Map* map, int key) {
    return map_remove(map, (void*)(intptr_t)key);
}

bool wlang_map_remove_str(Map* map, const char* key) {
    return map_remove(map, (void*)key);
}

bool wlang_map_remove_real(Map* map, float key) {
    return map_remove(map, &key);
}

bool wlang_map_remove_chr(Map* map, char key) {
    return map_remove(map, (void*)(intptr_t)key);
}

// ==================== generic map operations ====================

size_t wlang_map_size(Map* map) {
    return map_size(map);
}

bool wlang_map_is_empty(Map* map) {
    return map_is_empty(map);
}

void wlang_map_clear(Map* map) {
    map_clear(map);
}

void wlang_map_destroy(Map* map) {
    map_destroy(map);
}
