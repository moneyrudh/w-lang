// operator_utils.h
#ifndef OPERATOR_UTILS_H
#define OPERATOR_UTILS_H

#include <stdbool.h>
#include "types.h"

typedef enum {
    OP_CAT_ARITHMETIC,   // + - * /
    OP_CAT_COMPARISON,   // == != < > <= >= (and eq ne lt gt le ge is)
    OP_CAT_LOGICAL       // and or not
} OperatorCategory;

// map an operator token (symbol or word form) to its OperatorType
// returns false if the token is not an operator
bool token_to_operator(TokenType token, OperatorType* out);

OperatorCategory get_operator_category(OperatorType op);

#endif
