#ifndef TOKEN_REGISTRY_H
#define TOKEN_REGISTRY_H

#include "types.h"
#include <stdbool.h>

typedef enum {
    TOKEN_CAT_TYPE,        // num, real, chr, str, bool, zil
    TOKEN_CAT_KEYWORD,     // fun, dec, use, ret, log, w
    TOKEN_CAT_OPERATOR,    // +, -, *, /
    TOKEN_CAT_PUNCTUATION, // (, ), {, }, ;, :, ,, [, ]
    TOKEN_CAT_LITERAL,     // INT_LITERAL, STRING_LITERAL, etc.
    TOKEN_CAT_IDENTIFIER,  // IDENTIFIER
    TOKEN_CAT_ASSIGNMENT   // =, :=
} TokenCategory;

typedef struct {
    TokenType token;
    const char* display_name;  // for error messages: "NUM", "IDENTIFIER", etc.
    const char* lexeme;        // actual text: "num", "+", "(", etc. (NULL for non-literals)
    TokenCategory category;
} TokenMetadata;

// ==================== initialization & cleanup ====================

// initialize token registry (call at transpiler startup)
void token_registry_init(void);

// cleanup token registry (call at transpiler shutdown)
void token_registry_cleanup(void);

// ==================== lookup functions ====================

// lookup token metadata by TokenType
const TokenMetadata* token_registry_get(TokenType token);

// get display name for error messages (replaces tokenToString in parser.c)
const char* token_registry_get_display_name(TokenType token);

// get lexeme (actual source text) for a token
const char* token_registry_get_lexeme(TokenType token);

// get token category
TokenCategory token_registry_get_category(TokenType token);

// ==================== utility functions ====================

// check if token is a type keyword (replaces is_type_token in parser.c)
bool token_registry_is_type(TokenType token);

// check if token is an operator
bool token_registry_is_operator(TokenType token);

#endif // TOKEN_REGISTRY_H
