#ifndef TYPE_REGISTRY_H
#define TYPE_REGISTRY_H

#include "types.h"
#include "data_structures/map.h"

// type mapping structure - contains all information about a W Lang type
typedef struct {
    DataType enum_value;        // TYPE_NUM, TYPE_REAL, etc.
    TokenType token_value;      // NUM, REAL, etc.
    const char* w_lang_name;    // "num", "real", etc.
    const char* c_equivalent;   // "int", "float", etc.
    const char* format_spec;    // "%d", "%f", etc.
    const char* default_value;  // "0", "0.0f", etc.
} TypeMapping;

// ==================== initialization & cleanup ====================

// initialize the type registry (must be called at transpiler startup)
void type_registry_init(void);

// cleanup the type registry (call at transpiler shutdown)
void type_registry_cleanup(void);

// ==================== lookup functions ====================

// lookup by DataType enum value (e.g., TYPE_NUM)
const TypeMapping* type_registry_get_by_enum(DataType type);

// lookup by TokenType value (e.g., NUM)
const TypeMapping* type_registry_get_by_token(TokenType token);

// lookup by W Lang type name (e.g., "num")
const TypeMapping* type_registry_get_by_wlang_name(const char* name);

// lookup by C type name (e.g., "int")
const TypeMapping* type_registry_get_by_c_name(const char* name);

// ==================== convenience functions ====================
// these replace scattered switch statements throughout the codebase

// get C type string from DataType enum (replaces get_c_type_string in gen.c)
const char* get_c_type_from_enum(DataType type);

// get W Lang type string from DataType enum (replaces type_to_string in parser.c)
const char* get_wlang_type_from_enum(DataType type);

// get format specifier for printf/scanf (e.g., "%d" for num)
const char* get_format_spec_from_enum(DataType type);

// get default value for uninitialized variables (e.g., "0" for num)
const char* get_default_value_from_enum(DataType type);

// convert TokenType to DataType (replaces token_to_data_type in parser.c)
DataType convert_token_to_data_type(TokenType token);

// convert DataType to TokenType
TokenType convert_data_type_to_token(DataType type);

// convert W Lang type string to DataType enum (e.g., "num" -> TYPE_NUM)
// replaces manual string->enum conversions in parser.c
DataType type_registry_string_to_enum(const char* type_str);

#endif // TYPE_REGISTRY_H
