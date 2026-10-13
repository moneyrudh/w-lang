#include "checker/internal.h"
#include "diagnostics.h"
#include "language/type_registry.h"

static DataType check_variable(ASTNode* node) {
    Symbol* symbol = lookup_variable(node->data.variable.name);
    if (!symbol) {
        report_error(node->location.line, "Undefined variable: '%s'", node->data.variable.name);
        return TYPE_ERROR;
    }
    return symbol->type;
}

static DataType check_binary_expr(ASTNode* node) {
    DataType left = check_expression(node->data.binary_expr.left);
    DataType right = check_expression(node->data.binary_expr.right);
    if (left == TYPE_ERROR || right == TYPE_ERROR) {
        return TYPE_ERROR;
    }

    const char* error = NULL;
    DataType result = operation_type(left, right, node->data.binary_expr.operator, &error);
    if (error) {
        report_error(node->location.line, "%s", error);
    }
    return result;
}

static DataType check_unary_expr(ASTNode* node) {
    // OP_NOT is currently the only unary operator
    DataType operand = check_expression(node->data.unary_expr.operand);
    if (operand == TYPE_ERROR) {
        return TYPE_ERROR;
    }
    if (operand == TYPE_STR || operand == TYPE_ZIL) {
        report_error(node->location.line, "Cannot apply 'not' to %s", get_wlang_type_from_enum(operand));
        return TYPE_ERROR;
    }
    return TYPE_BOOL;
}

static DataType expression_type(ASTNode* node) {
    switch (node->type) {
        case NODE_NUMBER:        return TYPE_NUM;
        case NODE_STRING:        return TYPE_STR;
        case NODE_FLOAT:         return TYPE_REAL;
        case NODE_CHAR:          return TYPE_CHR;
        case NODE_BOOL:          return TYPE_BOOL;
        case NODE_VARIABLE:      return check_variable(node);
        case NODE_BINARY_EXPR:   return check_binary_expr(node);
        case NODE_UNARY_EXPR:    return check_unary_expr(node);
        case NODE_FUNCTION_CALL: return check_call(node);
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
