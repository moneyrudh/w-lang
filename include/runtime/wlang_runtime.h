#ifndef WLANG_RUNTIME_H
#define WLANG_RUNTIME_H

#include "data_structures/map.h"
#include <stdbool.h>

// ==================== runtime map helpers ====================
// these functions are used in the transpiled C code when W Lang users
// write map(T, U) in their W Lang programs.

// ==================== map creation ====================

// create map with integer keys and integer values: map(num, num)
Map* wlang_map_create_num_num(void);

// create map with integer keys and string values: map(num, str)
Map* wlang_map_create_num_str(void);

// create map with string keys and integer values: map(str, num)
Map* wlang_map_create_str_num(void);

// create map with string keys and string values: map(str, str)
Map* wlang_map_create_str_str(void);

// create map with float keys and float values: map(real, real)
Map* wlang_map_create_real_real(void);

// create map with integer keys and float values: map(num, real)
Map* wlang_map_create_num_real(void);

// create map with char keys and integer values: map(chr, num)
Map* wlang_map_create_chr_num(void);

// ==================== map operations (type-safe wrappers) ====================

// put operations for common type combinations
bool wlang_map_put_num_num(Map* map, int key, int value);
bool wlang_map_put_num_str(Map* map, int key, const char* value);
bool wlang_map_put_str_num(Map* map, const char* key, int value);
bool wlang_map_put_str_str(Map* map, const char* key, const char* value);
bool wlang_map_put_real_real(Map* map, float key, float value);
bool wlang_map_put_num_real(Map* map, int key, float value);
bool wlang_map_put_chr_num(Map* map, char key, int value);

// get operations for common type combinations
int wlang_map_get_num_num(Map* map, int key, int default_value);
const char* wlang_map_get_num_str(Map* map, int key, const char* default_value);
int wlang_map_get_str_num(Map* map, const char* key, int default_value);
const char* wlang_map_get_str_str(Map* map, const char* key, const char* default_value);
float wlang_map_get_real_real(Map* map, float key, float default_value);
float wlang_map_get_num_real(Map* map, int key, float default_value);
int wlang_map_get_chr_num(Map* map, char key, int default_value);

// contains operations for common key types
bool wlang_map_contains_num(Map* map, int key);
bool wlang_map_contains_str(Map* map, const char* key);
bool wlang_map_contains_real(Map* map, float key);
bool wlang_map_contains_chr(Map* map, char key);

// remove operations for common key types
bool wlang_map_remove_num(Map* map, int key);
bool wlang_map_remove_str(Map* map, const char* key);
bool wlang_map_remove_real(Map* map, float key);
bool wlang_map_remove_chr(Map* map, char key);

// ==================== generic map operations ====================
// these work with any map type

// get the number of entries in the map
size_t wlang_map_size(Map* map);

// check if map is empty
bool wlang_map_is_empty(Map* map);

// clear all entries
void wlang_map_clear(Map* map);

// destroy the map
void wlang_map_destroy(Map* map);

#endif // WLANG_RUNTIME_H
