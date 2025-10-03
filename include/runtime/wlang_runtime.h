#ifndef WLANG_RUNTIME_H
#define WLANG_RUNTIME_H

#include "data_structures/map.h"
#include <stdbool.h>

// ==================== Runtime Map Helpers ====================
// These functions are used in the transpiled C code when W Lang users
// write map(T, U) in their W Lang programs.

// ==================== Map Creation ====================

// Create map with integer keys and integer values: map(num, num)
Map* wlang_map_create_num_num(void);

// Create map with integer keys and string values: map(num, str)
Map* wlang_map_create_num_str(void);

// Create map with string keys and integer values: map(str, num)
Map* wlang_map_create_str_num(void);

// Create map with string keys and string values: map(str, str)
Map* wlang_map_create_str_str(void);

// Create map with float keys and float values: map(real, real)
Map* wlang_map_create_real_real(void);

// Create map with integer keys and float values: map(num, real)
Map* wlang_map_create_num_real(void);

// Create map with char keys and integer values: map(chr, num)
Map* wlang_map_create_chr_num(void);

// ==================== Map Operations (Type-Safe Wrappers) ====================

// Put operations for common type combinations
bool wlang_map_put_num_num(Map* map, int key, int value);
bool wlang_map_put_num_str(Map* map, int key, const char* value);
bool wlang_map_put_str_num(Map* map, const char* key, int value);
bool wlang_map_put_str_str(Map* map, const char* key, const char* value);
bool wlang_map_put_real_real(Map* map, float key, float value);
bool wlang_map_put_num_real(Map* map, int key, float value);
bool wlang_map_put_chr_num(Map* map, char key, int value);

// Get operations for common type combinations
int wlang_map_get_num_num(Map* map, int key, int default_value);
const char* wlang_map_get_num_str(Map* map, int key, const char* default_value);
int wlang_map_get_str_num(Map* map, const char* key, int default_value);
const char* wlang_map_get_str_str(Map* map, const char* key, const char* default_value);
float wlang_map_get_real_real(Map* map, float key, float default_value);
float wlang_map_get_num_real(Map* map, int key, float default_value);
int wlang_map_get_chr_num(Map* map, char key, int default_value);

// Contains operations for common key types
bool wlang_map_contains_num(Map* map, int key);
bool wlang_map_contains_str(Map* map, const char* key);
bool wlang_map_contains_real(Map* map, float key);
bool wlang_map_contains_chr(Map* map, char key);

// Remove operations for common key types
bool wlang_map_remove_num(Map* map, int key);
bool wlang_map_remove_str(Map* map, const char* key);
bool wlang_map_remove_real(Map* map, float key);
bool wlang_map_remove_chr(Map* map, char key);

// ==================== Generic Map Operations ====================
// These work with any map type

// Get the number of entries in the map
size_t wlang_map_size(Map* map);

// Check if map is empty
bool wlang_map_is_empty(Map* map);

// Clear all entries
void wlang_map_clear(Map* map);

// Destroy the map
void wlang_map_destroy(Map* map);

#endif // WLANG_RUNTIME_H
