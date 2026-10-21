#include "language/token_registry.h"
#include "util/map.h"
#include <stdio.h>
#include <stdlib.h>

// global token registry map
static Map* token_map = NULL;

// static token metadata table. display_name is how error messages name the token:
// the quoted source text when it has one, otherwise words
static TokenMetadata token_metadata[] = {
    // types
    {NUM,           "'num'",         "num",   TOKEN_CAT_TYPE},
    {REAL,          "'real'",        "real",  TOKEN_CAT_TYPE},
    {CHR,           "'chr'",         "chr",   TOKEN_CAT_TYPE},
    {STR,           "'str'",         "str",   TOKEN_CAT_TYPE},
    {BOOL,          "'bool'",        "bool",  TOKEN_CAT_TYPE},
    {ZIL,           "'zil'",         "zil",   TOKEN_CAT_TYPE},

    // future types
    {VEC,           "'vec'",         "vec",   TOKEN_CAT_TYPE},
    {MAP,           "'map'",         "map",   TOKEN_CAT_TYPE},
    {SET,           "'set'",         "set",   TOKEN_CAT_TYPE},
    {REF,           "'ref'",         "ref",   TOKEN_CAT_TYPE},
    {HEAP,          "'heap'",        "heap",  TOKEN_CAT_TYPE},
    {STACK,         "'stack'",       "stack", TOKEN_CAT_TYPE},
    {QUE,           "'que'",         "que",   TOKEN_CAT_TYPE},
    {LINK,          "'link'",        "link",  TOKEN_CAT_TYPE},
    {TREE,          "'tree'",        "tree",  TOKEN_CAT_TYPE},
    {POD,           "'pod'",         "pod",   TOKEN_CAT_TYPE},

    // keywords
    {FUN,           "'fun'",         "fun",   TOKEN_CAT_KEYWORD},
    {DEC,           "'dec'",         "dec",   TOKEN_CAT_KEYWORD},
    {USE,           "'use'",         "use",   TOKEN_CAT_KEYWORD},
    {MAIN,          "'w'",           "w",     TOKEN_CAT_KEYWORD},
    {RETURN,        "'ret'",         "ret",   TOKEN_CAT_KEYWORD},
    {LOG,           "'log'",         "log",   TOKEN_CAT_KEYWORD},

    // control flow
    {SAY,           "'say'",         "say",   TOKEN_CAT_KEYWORD},
    {WHEN,          "'when'",        "when",  TOKEN_CAT_KEYWORD},
    {LEST,          "'lest'",        "lest",  TOKEN_CAT_KEYWORD},
    {ELF,           "'elf'",         "elf",   TOKEN_CAT_KEYWORD},
    {NOR,           "'nor'",         "nor",   TOKEN_CAT_KEYWORD},
    {ELSE,          "'else'",        "else",  TOKEN_CAT_KEYWORD},
    {LOOP,          "'loop'",        "loop",  TOKEN_CAT_KEYWORD},
    {WHILE,         "'while'",       "while", TOKEN_CAT_KEYWORD},
    {UNTIL,         "'until'",       "until", TOKEN_CAT_KEYWORD},
    {FOREVER,       "'forever'",     "forever", TOKEN_CAT_KEYWORD},
    {SKIP,          "'skip'",        "skip",  TOKEN_CAT_KEYWORD},
    {STOP,          "'stop'",        "stop",  TOKEN_CAT_KEYWORD},

    // operators
    {PLUS,          "'+'",           "+",     TOKEN_CAT_OPERATOR},
    {MINUS,         "'-'",           "-",     TOKEN_CAT_OPERATOR},
    {MULTIPLY,      "'*'",           "*",     TOKEN_CAT_OPERATOR},
    {DIVIDE,        "'/'",           "/",     TOKEN_CAT_OPERATOR},

    // comparison operators (symbol form)
    {EQUAL,         "'=='",          "==",    TOKEN_CAT_OPERATOR},
    {NOT_EQUAL,     "'!='",          "!=",    TOKEN_CAT_OPERATOR},
    {LESS,          "'<'",           "<",     TOKEN_CAT_OPERATOR},
    {GREATER,       "'>'",           ">",     TOKEN_CAT_OPERATOR},
    {LESS_EQUAL,    "'<='",          "<=",    TOKEN_CAT_OPERATOR},
    {GREATER_EQUAL, "'>='",          ">=",    TOKEN_CAT_OPERATOR},

    // comparison operators (word form)
    {EQ,            "'eq'",          "eq",    TOKEN_CAT_OPERATOR},
    {NE,            "'ne'",          "ne",    TOKEN_CAT_OPERATOR},
    {GT,            "'gt'",          "gt",    TOKEN_CAT_OPERATOR},
    {LT,            "'lt'",          "lt",    TOKEN_CAT_OPERATOR},
    {GE,            "'ge'",          "ge",    TOKEN_CAT_OPERATOR},
    {LE,            "'le'",          "le",    TOKEN_CAT_OPERATOR},
    {IS,            "'is'",          "is",    TOKEN_CAT_OPERATOR},

    // logical operators ('&&' and '||' lex to AND / OR)
    {AND,           "'and'",         "and",   TOKEN_CAT_OPERATOR},
    {OR,            "'or'",          "or",    TOKEN_CAT_OPERATOR},
    {NOT,           "'not'",         "not",   TOKEN_CAT_OPERATOR},
    {BANG,          "'!'",           "!",     TOKEN_CAT_OPERATOR},

    // punctuation
    {LPAREN,        "'('",           "(",     TOKEN_CAT_PUNCTUATION},
    {RPAREN,        "')'",           ")",     TOKEN_CAT_PUNCTUATION},
    {LBRACE,        "'{'",           "{",     TOKEN_CAT_PUNCTUATION},
    {RBRACE,        "'}'",           "}",     TOKEN_CAT_PUNCTUATION},
    {SEMICOLON,     "';'",           ";",     TOKEN_CAT_PUNCTUATION},
    {COLON,         "':'",           ":",     TOKEN_CAT_PUNCTUATION},
    {COMMA,         "','",           ",",     TOKEN_CAT_PUNCTUATION},
    {LBRACKET,      "'['",           "[",     TOKEN_CAT_PUNCTUATION},
    {RBRACKET,      "']'",           "]",     TOKEN_CAT_PUNCTUATION},

    // layout
    {NEWLINE,       "end of line",           NULL,  TOKEN_CAT_LAYOUT},
    {INDENT,        "indented block",        NULL,  TOKEN_CAT_LAYOUT},
    {DEDENT,        "end of indented block", NULL,  TOKEN_CAT_LAYOUT},

    // assignment
    {ASSIGNMENT,    "'='",           "=",     TOKEN_CAT_ASSIGNMENT},
    {INFER_ASSIGN,  "':='",          ":=",    TOKEN_CAT_ASSIGNMENT},

    // literals
    {INT_LITERAL,   "number",                NULL,  TOKEN_CAT_LITERAL},
    {FLOAT_LITERAL, "decimal number",        NULL,  TOKEN_CAT_LITERAL},
    {STRING_LITERAL, "string",               NULL,  TOKEN_CAT_LITERAL},
    {CHAR_LITERAL,  "character",             NULL,  TOKEN_CAT_LITERAL},
    {BOOL_LITERAL,  "true or false",         NULL,  TOKEN_CAT_LITERAL},
    {IDENTIFIER,    "name",                  NULL,  TOKEN_CAT_IDENTIFIER},
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

const char* token_registry_describe(TokenType token) {
    if (token == 0) {
        return "end of file";
    }
    // a character the lexer didn't recognize comes back as itself
    if (token < 256) {
        static char unknown_char[4];
        snprintf(unknown_char, sizeof(unknown_char), "'%c'", (char)token);
        return unknown_char;
    }

    const TokenMetadata* meta = token_registry_get(token);
    return meta ? meta->display_name : "unknown token";
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
