#include "checker.h"
#include "checker/internal.h"
#include "language/operators.h"

// Type rules only: no AST, no errors printed.

bool compare_types(DataType target, DataType value) {
    if (target == value) {
        return true;
    }

    if (target == TYPE_REAL && value == TYPE_NUM) {
        return true;
    }

    if (target == TYPE_NUM && value == TYPE_CHR) {
        return true;
    }

    if (
        (target == TYPE_BOOL && value == TYPE_NUM)
        ||
        (target == TYPE_NUM && value == TYPE_BOOL)
    ) {
        return true;
    }

    return false;
}

bool can_convert_type(DataType from, DataType to) {
    if (from == to) return true;

    switch (to) {
        case TYPE_REAL:
            return from == TYPE_NUM || from == TYPE_CHR || from == TYPE_BOOL;
        case TYPE_NUM:
            return from == TYPE_CHR || from == TYPE_BOOL;
        case TYPE_BOOL:
            return from == TYPE_NUM || from == TYPE_REAL || from == TYPE_CHR;
        case TYPE_CHR:
            return from == TYPE_NUM;
        default:
            return false;
    }
}

DataType operation_type(DataType left, DataType right, OperatorType op, const char** error) {
    if (left == TYPE_ZIL || right == TYPE_ZIL) {
        *error = "Cannot perform operations on zil type";
        return TYPE_ERROR;
    }

    OperatorCategory category = get_operator_category(op);

    // and / or: any non-str operand is truthy in C
    if (category == OP_CAT_LOGICAL) {
        if (left == TYPE_STR || right == TYPE_STR) {
            *error = "Logical operators (and, or) cannot be used on str";
            return TYPE_ERROR;
        }
        return TYPE_BOOL;
    }

    // comparisons: str only compares with str (emitted via strcmp)
    if (category == OP_CAT_COMPARISON) {
        if ((left == TYPE_STR) != (right == TYPE_STR)) {
            *error = "Cannot compare str with a non-str value";
            return TYPE_ERROR;
        }
        return TYPE_BOOL;
    }

    // string concat between string 1 and string 2
    if (op == OP_ADD && (left == TYPE_STR || right == TYPE_STR)) {
        if (left == TYPE_STR && right == TYPE_STR) {
            return TYPE_STR;
        }
        *error = "String concatenation only works betweens strings";
        return TYPE_ERROR;
    }

    // invalid operation on string (can only do +)
    if (left == TYPE_STR || right == TYPE_STR) {
        *error = "Invalid arithmetic operation on str type";
        return TYPE_ERROR;
    }

    if (left == TYPE_REAL || right == TYPE_REAL) {
        return TYPE_REAL;
    }

    // converts num, chr, and bool into num
    return TYPE_NUM;
}
