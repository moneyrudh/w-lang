#include "transpiler/token_registry.h"
#include "data_structures/map.h"
#include <stdlib.h>

// global token registry map
static Map* token_map = NULL;

// static token metadata table
static TokenMetadata token_metadata[] = {
    // types
    {NUM,       "NUM",          "num",      TOKEN_CAT_TYPE},
    {REAL,      "REAL",         "real",     TOKEN_CAT_TYPE},
    {CHR,       "CHR",          "chr",      TOKEN_CAT_TYPE},
    {STR,       "STR",          "str",      TOKEN_CAT_TYPE},
    {BOOL,      "BOOL",         "bool",     TOKEN_CAT_TYPE},
    {ZIL,       "ZIL",          "zil",      TOKEN_CAT_TYPE},

    // future types
    {VEC,       "VEC",          "vec",      TOKEN_CAT_TYPE},
    {MAP,       "MAP",          "map",      TOKEN_CAT_TYPE},
    {SET,       "SET",          "set",      TOKEN_CAT_TYPE},
    {REF,       "REF",          "ref",      TOKEN_CAT_TYPE},
    {HEAP,      "HEAP",         "heap",     TOKEN_CAT_TYPE},
    {STACK,     "STACK",        "stack",    TOKEN_CAT_TYPE},
    {QUE,       "QUE",          "que",      TOKEN_CAT_TYPE},
    {LINK,      "LINK",         "link",     TOKEN_CAT_TYPE},
    {TREE,      "TREE",         "tree",     TOKEN_CAT_TYPE},
    {POD,       "POD",          "pod",      TOKEN_CAT_TYPE},

    // keywords
    {FUN,       "FUN",          "fun",      TOKEN_CAT_KEYWORD},
    {DEC,       "DEC",          "dec",      TOKEN_CAT_KEYWORD},
    {USE,       "USE",          "use",      TOKEN_CAT_KEYWORD},
    {MAIN,      "MAIN",         "w",        TOKEN_CAT_KEYWORD},
    {RETURN,    "RETURN",       "ret",      TOKEN_CAT_KEYWORD},
    {LOG,       "LOG",          "log",      TOKEN_CAT_KEYWORD},

    // operators
    {PLUS,      "PLUS",         "+",        TOKEN_CAT_OPERATOR},
    {MINUS,     "MINUS",        "-",        TOKEN_CAT_OPERATOR},
    {MULTIPLY,  "MULTIPLY",     "*",        TOKEN_CAT_OPERATOR},
    {DIVIDE,    "DIVIDE",       "/",        TOKEN_CAT_OPERATOR},

    // punctuation
    {LPAREN,    "LPAREN",       "(",        TOKEN_CAT_PUNCTUATION},
    {RPAREN,    "RPAREN",       ")",        TOKEN_CAT_PUNCTUATION},
    {LBRACE,    "LBRACE",       "{",        TOKEN_CAT_PUNCTUATION},
    {RBRACE,    "RBRACE",       "}",        TOKEN_CAT_PUNCTUATION},
    {SEMICOLON, "SEMICOLON",    ";",        TOKEN_CAT_PUNCTUATION},
    {COLON,     "COLON",        ":",        TOKEN_CAT_PUNCTUATION},
    {COMMA,     "COMMA",        ",",        TOKEN_CAT_PUNCTUATION},
    {LBRACKET,  "LBRACKET",     "[",        TOKEN_CAT_PUNCTUATION},
    {RBRACKET,  "RBRACKET",     "]",        TOKEN_CAT_PUNCTUATION},

    // assignment
    {ASSIGNMENT,      "ASSIGNMENT",    "=",   TOKEN_CAT_ASSIGNMENT},
    {INFER_ASSIGN,    "INFER_ASSIGN",  ":=",  TOKEN_CAT_ASSIGNMENT},

    // literals
    {INT_LITERAL,     "INT_LITERAL",     NULL, TOKEN_CAT_LITERAL},
    {FLOAT_LITERAL,   "FLOAT_LITERAL",   NULL, TOKEN_CAT_LITERAL},
    {STRING_LITERAL,  "STRING_LITERAL",  NULL, TOKEN_CAT_LITERAL},
    {CHAR_LITERAL,    "CHAR_LITERAL",    NULL, TOKEN_CAT_LITERAL},
    {BOOL_LITERAL,    "BOOL_LITERAL",    NULL, TOKEN_CAT_LITERAL},
    {IDENTIFIER,      "IDENTIFIER",      NULL, TOKEN_CAT_IDENTIFIER},
};

static const size_t num_tokens = sizeof(token_metadata) / sizeof(token_metadata[0]);

// ==================== initialization & cleanup ====================

void token_registry_init(void) {
    // avoid double initialization
    if (token_map != NULL) {
        return;
    }

    MapConfig config = {
        .hash = hash_int,
        .key_equal = key_equal_int,
        .key_copy = NULL,           // store token values directly
        .value_copy = NULL,         // store pointers to static data
        .key_free = NULL,
        .value_free = NULL
    };

    token_map = map_create(64, config);

    // populate map
    for (size_t i = 0; i < num_tokens; i++) {
        map_put(token_map,
                (void*)(intptr_t)token_metadata[i].token,
                &token_metadata[i]);
    }
}

void token_registry_cleanup(void) {
    if (token_map) {
        map_destroy(token_map);
        token_map = NULL;
    }
}

// ==================== lookup functions ====================

const TokenMetadata* token_registry_get(TokenType token) {
    if (!token_map) return NULL;
    return (const TokenMetadata*)map_get(token_map, (void*)(intptr_t)token);
}

const char* token_registry_get_display_name(TokenType token) {
    // handle single character tokens (< 256)
    static char single_char[2] = {0, 0};

    if (token < 256) {
        single_char[0] = (char)token;
        return single_char;
    }

    const TokenMetadata* meta = token_registry_get(token);
    return meta ? meta->display_name : "UNKNOWN";
}

const char* token_registry_get_lexeme(TokenType token) {
    const TokenMetadata* meta = token_registry_get(token);
    return meta ? meta->lexeme : NULL;
}

TokenCategory token_registry_get_category(TokenType token) {
    const TokenMetadata* meta = token_registry_get(token);
    return meta ? meta->category : TOKEN_CAT_IDENTIFIER;
}

// ==================== utility functions ====================

bool token_registry_is_type(TokenType token) {
    return token_registry_get_category(token) == TOKEN_CAT_TYPE;
}

bool token_registry_is_operator(TokenType token) {
    return token_registry_get_category(token) == TOKEN_CAT_OPERATOR;
}
