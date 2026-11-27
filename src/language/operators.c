#include <stddef.h>
#include "language/operators.h"

// symbol and word forms map to the same operator (e.g. '==', 'eq', 'is' -> OP_EQ)
static const struct {
    TokenType token;
    OperatorType op;
} token_operators[] = {
    {PLUS,          OP_ADD},
    {MINUS,         OP_SUB},
    {MULTIPLY,      OP_MUL},
    {DIVIDE,        OP_DIV},
    {MODULO,        OP_MOD},
    {MOD,           OP_MOD},

    {EQUAL,         OP_EQ},
    {EQ,            OP_EQ},
    {IS,            OP_EQ},
    {NOT_EQUAL,     OP_NE},
    {NE,            OP_NE},
    {LESS,          OP_LT},
    {LT,            OP_LT},
    {GREATER,       OP_GT},
    {GT,            OP_GT},
    {LESS_EQUAL,    OP_LE},
    {LE,            OP_LE},
    {GREATER_EQUAL, OP_GE},
    {GE,            OP_GE},

    {AND,           OP_AND},
    {OR,            OP_OR},
    {NOT,           OP_NOT},
    {BANG,          OP_NOT},
};

static const OperatorCategory operator_categories[] = {
    [OP_ADD] = OP_CAT_ARITHMETIC,
    [OP_SUB] = OP_CAT_ARITHMETIC,
    [OP_MUL] = OP_CAT_ARITHMETIC,
    [OP_DIV] = OP_CAT_ARITHMETIC,
    [OP_MOD] = OP_CAT_ARITHMETIC,
    [OP_EQ]  = OP_CAT_COMPARISON,
    [OP_NE]  = OP_CAT_COMPARISON,
    [OP_LT]  = OP_CAT_COMPARISON,
    [OP_GT]  = OP_CAT_COMPARISON,
    [OP_LE]  = OP_CAT_COMPARISON,
    [OP_GE]  = OP_CAT_COMPARISON,
    [OP_AND] = OP_CAT_LOGICAL,
    [OP_OR]  = OP_CAT_LOGICAL,
    [OP_NOT] = OP_CAT_LOGICAL,
    [OP_NEG] = OP_CAT_ARITHMETIC,
};

bool token_to_operator(TokenType token, OperatorType* out) {
    size_t count = sizeof(token_operators) / sizeof(token_operators[0]);
    for (size_t i = 0; i < count; i++) {
        if (token_operators[i].token == token) {
            *out = token_operators[i].op;
            return true;
        }
    }
    return false;
}

OperatorCategory get_operator_category(OperatorType op) {
    return operator_categories[op];
}

// how error messages show each operator (W's spelling)
static const char* const operator_symbols[] = {
    [OP_ADD] = "+",  [OP_SUB] = "-",  [OP_MUL] = "*",  [OP_DIV] = "/",  [OP_MOD] = "%",
    [OP_EQ]  = "==", [OP_NE]  = "!=", [OP_LT]  = "<",  [OP_GT]  = ">",
    [OP_LE]  = "<=", [OP_GE]  = ">=",
    [OP_AND] = "and", [OP_OR] = "or", [OP_NOT] = "not", [OP_NEG] = "-",
};

const char* operator_symbol(OperatorType op) {
    return operator_symbols[op];
}
