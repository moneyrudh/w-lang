#include <stdio.h>
#include <string.h>

#include "codegen/internal.h"
#include "codegen/c_syntax.h"
#include "language/type_registry.h"

void generate_var_declaration(FILE* output, ASTNode* node, int indent_level) {
    const char* c_type = get_c_type_from_enum(node->data.var_declaration.type);
    const char* name = node->data.var_declaration.name;

    // shadowing `dec n := n + 1`: the value is computed while the outer n is still in scope
    //   int W__n_init = W__n_v + 1;
    //   int W__n_v = W__n_init;
    if (node->data.var_declaration.shadows_outer && node->data.var_declaration.init_expr) {
        emit_indent(output, indent_level);
        fprintf(output, "%s %s" C_ASSIGN, c_type, mangle_init_temp(name));
        generate_expression_with_cast(output, node->data.var_declaration.init_expr, node->data.var_declaration.type);
        fprintf(output, C_SEMICOLON_NL);
        emit_indent(output, indent_level);
        fprintf(output, "%s %s" C_ASSIGN, c_type, mangle_identifier(name, false));
        fprintf(output, "%s" C_SEMICOLON_NL, mangle_init_temp(name));
        return;
    }

    emit_indent(output, indent_level);
    fprintf(
        output, "%s %s",
        get_c_type_from_enum(node->data.var_declaration.type),
        mangle_identifier(node->data.var_declaration.name, false)
    );

    if (node->data.var_declaration.init_expr) {
        fprintf(output, C_ASSIGN);
        generate_expression_with_cast(
            output,
            node->data.var_declaration.init_expr,
            node->data.var_declaration.type
        );
    } else {
        const char* default_val = get_default_value_from_enum(node->data.var_declaration.type);
        if (default_val && strlen(default_val) > 0) {
            fprintf(output, C_ASSIGN "%s", default_val);
        }
    }
    fprintf(output, C_SEMICOLON_NL);
}

void generate_assignment(FILE* output, ASTNode* node, int indent_level) {
    if (!node || node->type != NODE_ASSIGNMENT) return;

    emit_indent(output, indent_level);

    DataType target_type = node->data.assignment.target_type;
    DataType value_type = node->data.assignment.value->expr_type;

    fprintf(output, "%s" C_ASSIGN, mangle_identifier(node->data.assignment.target, false));

    generate_cast_if_needed(output, value_type, target_type);

    generate(output, node->data.assignment.value, 0);
    fprintf(output, C_SEMICOLON_NL);
}

// a str + str inside log is printed as its pieces side by side ("%s%s"), so it needs no
// string memory (joining strings elsewhere is roadmap task 2.6)
static bool is_string_join(ASTNode* node) {
    return node->type == NODE_BINARY_EXPR && node->expr_type == TYPE_STR &&
           node->data.binary_expr.operator == OP_ADD;
}

static void generate_log_format(FILE* output, ASTNode* value) {
    if (is_string_join(value)) {
        generate_log_format(output, value->data.binary_expr.left);
        generate_log_format(output, value->data.binary_expr.right);
        return;
    }
    fputs(get_format_spec_from_enum(value->expr_type), output);
}

static void generate_log_values(FILE* output, ASTNode* value) {
    if (is_string_join(value)) {
        generate_log_values(output, value->data.binary_expr.left);
        generate_log_values(output, value->data.binary_expr.right);
        return;
    }
    fputs(C_COMMA, output);
    if (value->expr_type == TYPE_BOOL) {
        fputs(C_BOOL_TO_TEXT_OPEN, output);
        generate(output, value, 0);
        fputs(C_BOOL_TO_TEXT_CLOSE, output);
    } else {
        generate(output, value, 0);
    }
}

// log(a, b, ...) -> printf("<spec a> <spec b> ...\n", a, b, ...). Every value, strings included,
// is a printf argument, so a '%' in W text can't be read as a format directive
void generate_log_statement(FILE* output, ASTNode* node, int indent_level) {
    ASTNode** args = node->data.log.args;
    int arg_count = node->data.log.arg_count;

    emit_indent(output, indent_level);
    fprintf(output, C_PRINTF C_LPAREN C_STRING_QUOTE);
    for (int i = 0; i < arg_count; i++) {
        if (i > 0) fputs(C_SPACE, output);
        generate_log_format(output, args[i]);
    }
    fprintf(output, C_ESC_NEWLINE C_STRING_QUOTE);

    for (int i = 0; i < arg_count; i++) {
        generate_log_values(output, args[i]);
    }

    fprintf(output, C_RPAREN C_SEMICOLON_NL);
}

void generate_return(FILE* output, ASTNode* node, int indent_level) {
    emit_indent(output, indent_level);
    fprintf(output, C_RETURN C_SPACE);
    if (node->data.return_statement.expression) {
        generate(output, node->data.return_statement.expression, 0);
    }
    fprintf(output, C_SEMICOLON_NL);
}
