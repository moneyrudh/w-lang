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

// The result type of `left op right`, or TYPE_ERROR with *error set to what's wrong (the
// checker prefixes the expression, e.g. "num + bool: "). Every pair of types is covered by
// tests/unit/type_rules.c.
DataType operation_type(DataType left, DataType right, OperatorType op, const char** error) {
    if (left == TYPE_ZIL || right == TYPE_ZIL) {
        *error = "zil has no value (it's the result of a call that returns nothing)";
        return TYPE_ERROR;
    }

    OperatorCategory category = get_operator_category(op);

    // and / or: anything but str (numbers are true when they're not 0, as in C)
    if (category == OP_CAT_LOGICAL) {
        if (left == TYPE_STR || right == TYPE_STR) {
            *error = "and / or can't be used on str";
            return TYPE_ERROR;
        }
        return TYPE_BOOL;
    }

    // comparisons: str only compares with str (emitted via strcmp)
    if (category == OP_CAT_COMPARISON) {
        if ((left == TYPE_STR) != (right == TYPE_STR)) {
            *error = "can't compare str with a non-str value";
            return TYPE_ERROR;
        }
        return TYPE_BOOL;
    }

    // arithmetic. + joins two strings (D8: nothing else joins with a str)
    if (left == TYPE_STR || right == TYPE_STR) {
        if (op == OP_ADD && left == TYPE_STR && right == TYPE_STR) {
            return TYPE_STR;
        }
        *error = (op == OP_ADD)
            ? "can't join str with a non-str value (to print them together, use log(\"text\", value))"
            : "can't do arithmetic on str";
        return TYPE_ERROR;
    }

    // D9: true + 1 is almost always a mistake
    if (left == TYPE_BOOL || right == TYPE_BOOL) {
        *error = "can't do arithmetic on bool";
        return TYPE_ERROR;
    }

    if (left == TYPE_REAL || right == TYPE_REAL) {
        return TYPE_REAL;
    }

    // num and chr ('a' + 1 is 98)
    return TYPE_NUM;
}

DataType unary_operation_type(DataType operand, OperatorType op, const char** error) {
    (void)op;   // OP_NOT is the only unary operator so far
    if (operand == TYPE_ZIL) {
        *error = "zil has no value (it's the result of a call that returns nothing)";
        return TYPE_ERROR;
    }
    if (operand == TYPE_STR) {
        *error = "can't be used on str";
        return TYPE_ERROR;
    }
    return TYPE_BOOL;
}
