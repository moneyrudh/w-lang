#include <limits.h>
#include <stdio.h>

#include "checker.h"
#include "codegen/internal.h"
#include "codegen/c_syntax.h"
#include "language/operators.h"
#include "language/type_registry.h"

// while emitting globals' values: those are C constants, so no helper calls
static bool in_global_value = false;

void set_in_global_value(bool in_global) {
    in_global_value = in_global;
}

bool in_global_value_now(void) {
    return in_global_value;
}

// ==================== literals & names ====================

void generate_literal(FILE* output, ASTNode* node) {
    switch (node->type) {
        case NODE_NUMBER:
            if (node->data.number.value == INT_MIN) {
                fputs(C_INT_MIN, output);
            } else {
                fprintf(output, "%lld", node->data.number.value);   // the checker made sure it fits a num
            }
            break;
        case NODE_STRING:
            fprintf(output, C_STRING_QUOTE);
            for (const char* p = node->data.string.value; *p; p++) {
                switch (*p) {
                    case '\n': fprintf(output, C_ESC_NEWLINE); break;
                    case '\t': fprintf(output, C_ESC_TAB); break;
                    case '\"': fprintf(output, C_ESC_QUOTE); break;
                    case '\\': fprintf(output, C_ESC_BACKSLASH); break;
                    case '\r': fprintf(output, C_ESC_CARRIAGE); break;
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
    // a global str is read through a temporary copy: a call in the same expression may replace
    // (and free) the global's text while this expression still uses it. So is a ref str, which
    // may be a global's slot (f(ref g)), or reached by a call through another ref
    bool is_ref = node->data.variable.is_ref;
    if ((node->data.variable.is_global || is_ref) && node->expr_type == TYPE_STR) {
        fprintf(output, C_TEMP C_LPAREN C_STR_COPY C_LPAREN "%s" C_RPAREN C_RPAREN,
                variable_c_name(node->data.variable.name, is_ref));
        return;
    }
    fprintf(output, "%s", variable_c_name(node->data.variable.name, node->data.variable.is_ref));
}

// a `ref` argument's place, for its address: the variable itself (a global str too, not a
// copy of it), then its fields
static void generate_place(FILE* output, ASTNode* node) {
    if (node->type == NODE_FIELD_ACCESS) {
        generate_place(output, node->data.field_access.object);
        fprintf(output, C_FIELD_SELECT, mangle_field(node->data.field_access.field));
        return;
    }
    fprintf(output, "%s", variable_c_name(node->data.variable.name, node->data.variable.is_ref));
}

// ==================== casts ====================

void generate_expression_with_cast(FILE* output, ASTNode* expr, DataType target_type) {
    if (!expr) return;

    DataType expr_type = expr->expr_type;

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

    DataType left_type = left->expr_type;
    DataType right_type = right->expr_type;
    OperatorCategory category = get_operator_category(op);

    // str + str: a new string, freed after the statement (src/codegen/strings.c)
    if (op == OP_ADD && node->expr_type == TYPE_STR) {
        fprintf(output, C_STR_JOIN C_LPAREN);
        generate(output, left, indent_level);
        fprintf(output, C_COMMA);
        generate(output, right, indent_level);
        fprintf(output, C_RPAREN);
        return;
    }

    // str comparisons: strcmp(a, b) <op> 0
    if (category == OP_CAT_COMPARISON && left_type == TYPE_STR && right_type == TYPE_STR) {
        fprintf(output, C_STRCMP C_LPAREN);
        generate(output, left, indent_level);
        fprintf(output, C_COMMA);
        generate(output, right, indent_level);
        fprintf(output, C_RPAREN "%s" C_ZERO, op_string);
        return;
    }

    // num + - * / %: wrapping, and checked division (arithmetic.c)
    if (category == OP_CAT_ARITHMETIC && generate_num_arithmetic(output, node, in_global_value)) {
        return;
    }

    // arithmetic operands are cast to the result type; comparisons and logic
    // operate on the operands as-is (casting x > y to bool operands would be wrong)
    if (category == OP_CAT_ARITHMETIC) {
        DataType result_type = node->expr_type;
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

    if (node->data.unary_expr.operator == OP_NEG && node->expr_type == TYPE_NUM) {
        generate_num_negation(output, node->data.unary_expr.operand);
        return;
    }
    if (node->data.unary_expr.operator == OP_NEG) {
        fprintf(output, C_NEG_OPEN);
        generate(output, node->data.unary_expr.operand, indent_level);
        fprintf(output, C_RPAREN);
        return;
    }
    fprintf(output, C_OP_NOT);
    generate_operand(output, node->data.unary_expr.operand, indent_level);
}

// ==================== calls ====================

void generate_function_call(FILE* output, ASTNode* node, int indent_level) {
    if (node->data.function_call.is_pod_literal) {
        generate_pod_literal(output, node, in_global_value);
        return;
    }

    // if this is at indent_level > 0, it's a statement, so add indent and semicolon
    // if indent_level == 0, it's an expression within another expression
    if (indent_level > 0) {
        emit_indent(output, indent_level);
    }

    // the call's line goes first, for traces (formatters.c: every W function takes it)
    fprintf(output, "%s" C_LPAREN "%d", mangle_identifier(node->data.function_call.name, true),
            node->location.line);
    bool* refs = node->data.function_call.arg_is_ref;
    for (int i = 0; i < node->data.function_call.arg_count; i++) {
        fprintf(output, C_COMMA);
        if (refs && refs[i]) {
            // ref: the caller's variable's address, &(W__b_v.W__pos_m)
            fprintf(output, C_ADDRESS_OF C_LPAREN);
            generate_place(output, node->data.function_call.args[i]);
            fprintf(output, C_RPAREN);
        } else {
            generate(output, node->data.function_call.args[i], 0);
        }
    }
    fprintf(output, C_RPAREN);

    if (indent_level > 0) {
        fprintf(output, C_SEMICOLON_NL);
    }
}
