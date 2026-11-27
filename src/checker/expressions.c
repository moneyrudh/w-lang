#include <limits.h>
#include <math.h>

#include "checker/internal.h"
#include "diagnostics.h"
#include "language/type_registry.h"
#include "language/operators.h"

static DataType check_variable(ASTNode* node) {
    Symbol* symbol = lookup_variable(node->data.variable.name);
    if (!symbol) {
        report_error(node->location.line, "Undefined variable: '%s'", node->data.variable.name);
        return TYPE_ERROR;
    }
    node->data.variable.is_global = symbol->is_global;
    node->data.variable.is_ref = symbol->is_ref;
    return symbol->type;
}

static DataType check_binary_expr(ASTNode* node) {
    DataType left = check_expression(node->data.binary_expr.left);
    DataType right = check_expression(node->data.binary_expr.right);
    if (left == TYPE_ERROR || right == TYPE_ERROR) {
        return TYPE_ERROR;
    }

    const char* error = NULL;
    OperatorType op = node->data.binary_expr.operator;
    DataType result = operation_type(left, right, op, &error);
    if (error) {
        report_error(node->location.line, "%s %s %s: %s", get_wlang_type_from_enum(left),
                     operator_symbol(op), get_wlang_type_from_enum(right), error);
    }
    return result;
}

static DataType check_unary_expr(ASTNode* node) {
    DataType operand = check_expression(node->data.unary_expr.operand);
    if (operand == TYPE_ERROR) {
        return TYPE_ERROR;
    }

    const char* error = NULL;
    OperatorType op = node->data.unary_expr.operator;
    DataType result = unary_operation_type(operand, op, &error);
    if (error) {
        report_error(node->location.line, "%s %s: %s", operator_symbol(op),
                     get_wlang_type_from_enum(operand), error);
    }
    return result;
}

// a number literal must fit a num (the parser keeps it as written, minus sign included)
static DataType check_number(ASTNode* node) {
    long long value = node->data.number.value;
    if (value > INT_MAX) {
        report_error(node->location.line, "Number too large for num: the largest is 2147483647");
        return TYPE_ERROR;
    }
    if (value < INT_MIN) {
        report_error(node->location.line, "Number too small for num: the smallest is -2147483648");
        return TYPE_ERROR;
    }
    return TYPE_NUM;
}

// a real literal is read as a C float; one too large for it became infinity
static DataType check_real(ASTNode* node) {
    if (isinf(node->data.float_val.value)) {
        report_error(node->location.line, "Number too large for real: the largest is about 3.4e38");
        return TYPE_ERROR;
    }
    return TYPE_REAL;
}

static DataType expression_type(ASTNode* node) {
    switch (node->type) {
        case NODE_NUMBER:        return check_number(node);
        case NODE_STRING:        return TYPE_STR;
        case NODE_FLOAT:         return check_real(node);
        case NODE_CHAR:          return TYPE_CHR;
        case NODE_BOOL:          return TYPE_BOOL;
        case NODE_VARIABLE:      return check_variable(node);
        case NODE_BINARY_EXPR:   return check_binary_expr(node);
        case NODE_UNARY_EXPR:    return check_unary_expr(node);
        case NODE_FUNCTION_CALL: return check_call(node);
        case NODE_FIELD_ACCESS:  return check_field_access(node);
        default:
            report_error(node->location.line, "Unknown expression type");
            return TYPE_ERROR;
    }
}

DataType check_expression(ASTNode* node) {
    // a missing expression was already reported by the parser
    if (!node) return TYPE_ERROR;

    node->expr_type = expression_type(node);
    return node->expr_type;
}
