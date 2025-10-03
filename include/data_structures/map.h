#ifndef WLANG_MAP_H
#define WLANG_MAP_H

#include <stddef.h>
#include <stdbool.h>

// Forward declarations
typedef struct MapEntry MapEntry;
typedef struct Map Map;

// Function pointer types for generic operations
typedef unsigned long (*HashFunc)(const void* key);
typedef bool (*KeyEqualFunc)(const void* key1, const void* key2);
typedef void (*KeyFreeFunc)(void* key);
typedef void (*ValueFreeFunc)(void* value);
typedef void* (*KeyCopyFunc)(const void* key);
typedef void* (*ValueCopyFunc)(const void* value);

// Entry in the hash map (linked list node for collision resolution)
struct MapEntry {
    void* key;
    void* value;
    MapEntry* next;  // For collision chaining
};

// Hash map configuration
typedef struct {
    HashFunc hash;              // Required: hash function
    KeyEqualFunc key_equal;     // Required: key equality function
    KeyCopyFunc key_copy;       // Optional: if NULL, stores pointer directly
    ValueCopyFunc value_copy;   // Optional: if NULL, stores pointer directly
    KeyFreeFunc key_free;       // Optional: cleanup function for keys
    ValueFreeFunc value_free;   // Optional: cleanup function for values
} MapConfig;

// Main hash map structure
struct Map {
    MapEntry** buckets;         // Array of bucket heads
    size_t bucket_count;        // Number of buckets
    size_t size;                // Number of entries
    float load_factor;          // Threshold for resizing (default 0.75)
    MapConfig config;           // Configuration with function pointers
};

// ==================== Core API ====================

// Create a new map with specified configuration
Map* map_create(size_t initial_capacity, MapConfig config);

// Insert or update a key-value pair
// Returns: true if new entry, false if updated existing
bool map_put(Map* map, void* key, void* value);

// Retrieve a value by key
// Returns: value pointer or NULL if not found
void* map_get(const Map* map, const void* key);

// Check if key exists
bool map_contains(const Map* map, const void* key);

// Remove a key-value pair
// Returns: true if found and removed, false otherwise
bool map_remove(Map* map, const void* key);

// Get the number of entries
size_t map_size(const Map* map);

// Check if map is empty
bool map_is_empty(const Map* map);

// Clear all entries (calls free functions if provided)
void map_clear(Map* map);

// Destroy the map and free all resources
void map_destroy(Map* map);

// ==================== Iteration API ====================

typedef struct {
    const Map* map;
    size_t bucket_index;
    MapEntry* current_entry;
} MapIterator;

// Initialize iterator
MapIterator map_iterator(const Map* map);

// Check if iterator has more entries
bool map_iterator_has_next(MapIterator* iter);

// Get next entry (returns false if no more entries)
bool map_iterator_next(MapIterator* iter, void** key_out, void** value_out);

// ==================== Built-in Hash Functions ====================

// Hash function for integer keys (num type)
unsigned long hash_int(const void* key);

// Hash function for float keys (real type)
unsigned long hash_float(const void* key);

// Hash function for char keys (chr type)
unsigned long hash_char(const void* key);

// Hash function for string keys (str type) - FNV-1a algorithm
unsigned long hash_string(const void* key);

// Hash function for pointer addresses
unsigned long hash_pointer(const void* key);

// ==================== Built-in Equality Functions ====================

bool key_equal_int(const void* k1, const void* k2);
bool key_equal_float(const void* k1, const void* k2);
bool key_equal_char(const void* k1, const void* k2);
bool key_equal_string(const void* k1, const void* k2);
bool key_equal_pointer(const void* k1, const void* k2);

// ==================== Built-in Copy Functions ====================

// Copy function for strings (allocates new string)
void* key_copy_string(const void* key);
void* value_copy_string(const void* value);

// ==================== Built-in Free Functions ====================

// Free function for strings
void key_free_string(void* key);
void value_free_string(void* value);

// ==================== Type-Safe Wrapper Macros ====================

// Convenience macros for common types
#define MAP_GET_INT(map, key) ((int)(intptr_t)map_get(map, (void*)(intptr_t)(key)))
#define MAP_PUT_INT(map, key, value) map_put(map, (void*)(intptr_t)(key), (void*)(intptr_t)(value))

#define MAP_GET_PTR(map, key) map_get(map, key)
#define MAP_PUT_PTR(map, key, value) map_put(map, key, value)

#endif // WLANG_MAP_H
