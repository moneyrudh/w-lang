#include <stdio.h>

#include "checker.h"
#include "codegen/internal.h"
#include "codegen/c_syntax.h"
#include "language/operators.h"
#include "language/type_registry.h"

// ==================== literals & names ====================

void generate_literal(FILE* output, ASTNode* node) {
    switch (node->type) {
        case NODE_NUMBER:
            fprintf(output, "%d", node->data.number.value);
            break;
        case NODE_STRING:
            fprintf(output, C_STRING_QUOTE);
            for (const char* p = node->data.string.value; *p; p++) {
                switch (*p) {
                    case '\n': fprintf(output, C_ESC_NEWLINE); break;
                    case '\t': fprintf(output, C_ESC_TAB); break;
                    case '\"': fprintf(output, C_ESC_QUOTE); break;
                    case '\\': fprintf(output, C_ESC_BACKSLASH); break;
                    default: fputc(*p, output);
                }
            }
            fprintf(output, C_STRING_QUOTE);
            break;
        case NODE_FLOAT:
            fprintf(output, "%ff", node->data.float_val.value);
            break;
        case NODE_CHAR:
            fprintf(output, C_CHAR_QUOTE);
            if (node->data.char_val.value == '\n') {
                fprintf(output, C_ESC_NEWLINE);
            } else if (node->data.char_val.value == '\t') {
                fprintf(output, C_ESC_TAB);
            } else if (node->data.char_val.value == '\'') {
                fprintf(output, C_ESC_SINGLE_QUOTE);
            } else if (node->data.char_val.value == '\\') {
                fprintf(output, C_ESC_BACKSLASH);
            } else {
                fprintf(output, "%c", node->data.char_val.value);
            }
            fprintf(output, C_CHAR_QUOTE);
            break;
        case NODE_BOOL:
            fprintf(output, "%s", node->data.bool_val.value ? C_TRUE : C_FALSE);
            break;
        default:
            break;
    }
}

void generate_variable(FILE* output, ASTNode* node) {
    fprintf(output, "%s", mangle_identifier(node->data.variable.name, false));
}

// ==================== casts ====================

void generate_expression_with_cast(FILE* output, ASTNode* expr, DataType target_type) {
    if (!expr) return;

    DataType expr_type = get_expression_type(expr, getSymbolTable());

    if (expr_type != target_type && compare_types(target_type, expr_type)) {
        fprintf(output, C_LPAREN "%s" C_RPAREN C_LPAREN, get_c_type_from_enum(target_type));
        generate(output, expr, 0);
        fprintf(output, C_RPAREN);
    } else {
        generate(output, expr, 0);
    }
}

void generate_cast_if_needed(FILE* output, DataType from, DataType to) {
    if (from == to) return;

    if (can_convert_type(from, to)) {
        emit_cast(output, from, to);
    }
}

// ==================== operators ====================

// emits an operand, wrapped in parens if it is itself a binary expression so
// that grouping from the source (e.g. a * (b + c)) is preserved
static void generate_operand(FILE* output, ASTNode* operand, int indent_level) {
    bool needs_parens = operand->type == NODE_BINARY_EXPR;
    if (needs_parens) fprintf(output, C_LPAREN);
    generate(output, operand, indent_level);
    if (needs_parens) fprintf(output, C_RPAREN);
}

void generate_binary_expr(FILE* output, ASTNode* node, int indent_level) {
    if (!node || node->type != NODE_BINARY_EXPR) return;

    ASTNode* left = node->data.binary_expr.left;
    ASTNode* right = node->data.binary_expr.right;
    OperatorType op = node->data.binary_expr.operator;
    const char* op_string = get_binary_operator_string(op);

    DataType left_type = get_expression_type(left, getSymbolTable());
    DataType right_type = get_expression_type(right, getSymbolTable());
    OperatorCategory category = get_operator_category(op);

    // str comparisons: strcmp(a, b) <op> 0
    if (category == OP_CAT_COMPARISON && left_type == TYPE_STR && right_type == TYPE_STR) {
        fprintf(output, C_STRCMP C_LPAREN);
        generate(output, left, indent_level);
        fprintf(output, C_COMMA);
        generate(output, right, indent_level);
        fprintf(output, C_RPAREN "%s" C_ZERO, op_string);
        return;
    }

    // arithmetic operands are cast to the result type; comparisons and logic
    // operate on the operands as-is (casting x > y to bool operands would be wrong)
    if (category == OP_CAT_ARITHMETIC) {
        DataType result_type = get_operation_type(left_type, right_type, op);
        generate_cast_if_needed(output, left_type, result_type);
        generate_operand(output, left, indent_level);
        fprintf(output, "%s", op_string);
        generate_cast_if_needed(output, right_type, result_type);
        generate_operand(output, right, indent_level);
        return;
    }

    generate_operand(output, left, indent_level);
    fprintf(output, "%s", op_string);
    generate_operand(output, right, indent_level);
}

void generate_unary_expr(FILE* output, ASTNode* node, int indent_level) {
    if (!node || node->type != NODE_UNARY_EXPR) return;

    // OP_NOT is currently the only unary operator
    fprintf(output, C_OP_NOT);
    generate_operand(output, node->data.unary_expr.operand, indent_level);
}

// ==================== calls ====================

void generate_function_call(FILE* output, ASTNode* node, int indent_level) {
    // if this is at indent_level > 0, it's a statement, so add indent and semicolon
    // if indent_level == 0, it's an expression within another expression
    if (indent_level > 0) {
        emit_indent(output, indent_level);
    }

    fprintf(output, "%s" C_LPAREN, mangle_identifier(node->data.function_call.name, true));
    for (int i = 0; i < node->data.function_call.arg_count; i++) {
        if (i > 0) {
            fprintf(output, C_COMMA);
        }
        generate(output, node->data.function_call.args[i], 0);
    }
    fprintf(output, C_RPAREN);

    if (indent_level > 0) {
        fprintf(output, C_SEMICOLON_NL);
    }
}
