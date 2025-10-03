#include "transpiler/type_registry.h"
#include <stdlib.h>
#include <string.h>

// Global type registry maps
static Map* enum_to_mapping = NULL;     // DataType -> TypeMapping*
static Map* token_to_mapping = NULL;    // TokenType -> TypeMapping*
static Map* wlang_to_mapping = NULL;    // const char* -> TypeMapping*
static Map* c_to_mapping = NULL;        // const char* -> TypeMapping*

// Static type mapping table
static TypeMapping type_mappings[] = {
    // enum_value,  token_value, w_lang_name, c_equivalent, format_spec, default_value
    {TYPE_NUM,      NUM,         "num",       "int",        "%d",        "0"},
    {TYPE_REAL,     REAL,        "real",      "float",      "%f",        "0.0f"},
    {TYPE_CHR,      CHR,         "chr",       "char",       "%c",        "'\\0'"},
    {TYPE_BOOL,     BOOL,        "bool",      "bool",       "%d",        "false"},
    {TYPE_STR,      STR,         "str",       "char*",      "%s",        "NULL"},
    {TYPE_ZIL,      ZIL,         "zil",       "void",       "",          ""},
};

static const size_t num_type_mappings = sizeof(type_mappings) / sizeof(type_mappings[0]);

// ==================== Initialization & Cleanup ====================

void type_registry_init(void) {
    // Avoid double initialization
    if (enum_to_mapping != NULL) {
        return;
    }

    // Create maps with appropriate configurations
    MapConfig enum_config = {
        .hash = hash_int,
        .key_equal = key_equal_int,
        .key_copy = NULL,           // Store enum values directly
        .value_copy = NULL,         // Store pointers to static data
        .key_free = NULL,
        .value_free = NULL
    };

    MapConfig token_config = {
        .hash = hash_int,
        .key_equal = key_equal_int,
        .key_copy = NULL,
        .value_copy = NULL,
        .key_free = NULL,
        .value_free = NULL
    };

    MapConfig string_config = {
        .hash = hash_string,
        .key_equal = key_equal_string,
        .key_copy = NULL,           // Store pointers to static strings
        .value_copy = NULL,         // Store pointers to static data
        .key_free = NULL,
        .value_free = NULL
    };

    enum_to_mapping = map_create(16, enum_config);
    token_to_mapping = map_create(16, token_config);
    wlang_to_mapping = map_create(16, string_config);
    c_to_mapping = map_create(16, string_config);

    // Populate all maps
    for (size_t i = 0; i < num_type_mappings; i++) {
        TypeMapping* mapping = &type_mappings[i];

        map_put(enum_to_mapping,
                (void*)(intptr_t)mapping->enum_value,
                mapping);

        map_put(token_to_mapping,
                (void*)(intptr_t)mapping->token_value,
                mapping);

        map_put(wlang_to_mapping,
                (void*)mapping->w_lang_name,
                mapping);

        map_put(c_to_mapping,
                (void*)mapping->c_equivalent,
                mapping);
    }
}

void type_registry_cleanup(void) {
    if (enum_to_mapping) {
        map_destroy(enum_to_mapping);
        enum_to_mapping = NULL;
    }
    if (token_to_mapping) {
        map_destroy(token_to_mapping);
        token_to_mapping = NULL;
    }
    if (wlang_to_mapping) {
        map_destroy(wlang_to_mapping);
        wlang_to_mapping = NULL;
    }
    if (c_to_mapping) {
        map_destroy(c_to_mapping);
        c_to_mapping = NULL;
    }
}

// ==================== Lookup Functions ====================

const TypeMapping* type_registry_get_by_enum(DataType type) {
    if (!enum_to_mapping) return NULL;
    return (const TypeMapping*)map_get(enum_to_mapping, (void*)(intptr_t)type);
}

const TypeMapping* type_registry_get_by_token(TokenType token) {
    if (!token_to_mapping) return NULL;
    return (const TypeMapping*)map_get(token_to_mapping, (void*)(intptr_t)token);
}

const TypeMapping* type_registry_get_by_wlang_name(const char* name) {
    if (!wlang_to_mapping || !name) return NULL;
    return (const TypeMapping*)map_get(wlang_to_mapping, (void*)name);
}

const TypeMapping* type_registry_get_by_c_name(const char* name) {
    if (!c_to_mapping || !name) return NULL;
    return (const TypeMapping*)map_get(c_to_mapping, (void*)name);
}

// ==================== Convenience Functions ====================

const char* get_c_type_from_enum(DataType type) {
    const TypeMapping* mapping = type_registry_get_by_enum(type);
    return mapping ? mapping->c_equivalent : "void";
}

const char* get_wlang_type_from_enum(DataType type) {
    const TypeMapping* mapping = type_registry_get_by_enum(type);
    return mapping ? mapping->w_lang_name : "unknown";
}

const char* get_format_spec_from_enum(DataType type) {
    const TypeMapping* mapping = type_registry_get_by_enum(type);
    return mapping ? mapping->format_spec : "";
}

const char* get_default_value_from_enum(DataType type) {
    const TypeMapping* mapping = type_registry_get_by_enum(type);
    return mapping ? mapping->default_value : "";
}

DataType convert_token_to_data_type(TokenType token) {
    const TypeMapping* mapping = type_registry_get_by_token(token);
    return mapping ? mapping->enum_value : TYPE_ZIL;
}

TokenType convert_data_type_to_token(DataType type) {
    const TypeMapping* mapping = type_registry_get_by_enum(type);
    return mapping ? mapping->token_value : ZIL;
}
