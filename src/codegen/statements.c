#include <stdio.h>
#include <string.h>

#include "codegen/internal.h"
#include "codegen/c_syntax.h"
#include "language/type_registry.h"

// a str variable owns a copy of its value: char* W__s_v = W__str_copy(value);
static void generate_str_declaration(FILE* output, ASTNode* node, int indent_level) {
    const char* c_type = get_c_type_from_enum(TYPE_STR);
    const char* name = node->data.var_declaration.name;
    ASTNode* init = node->data.var_declaration.init_expr;
    bool via_temp = node->data.var_declaration.shadows_outer && init;

    emit_indent(output, indent_level);
    fprintf(output, "%s %s" C_ASSIGN C_STR_COPY C_LPAREN, c_type,
            via_temp ? mangle_init_temp(name) : mangle_identifier(name, false));
    if (init) {
        generate(output, init, 0);
    } else {
        fputs(C_EMPTY_STRING, output);
    }
    fprintf(output, C_RPAREN C_SEMICOLON_NL);

    if (via_temp) {
        emit_indent(output, indent_level);
        fprintf(output, "%s %s" C_ASSIGN, c_type, mangle_identifier(name, false));
        fprintf(output, "%s" C_SEMICOLON_NL, mangle_init_temp(name));
    }
    strings_own(mangle_identifier(name, false));
}

void generate_var_declaration(FILE* output, ASTNode* node, int indent_level) {
    const char* c_type = get_c_type_from_enum(node->data.var_declaration.type);
    const char* name = node->data.var_declaration.name;

    // shadowing a str this function owns (with a variable of any type): the outer str is kept
    // under an alias, so a ret / stop / skip from the inner scope still frees the right thing
    if (node->data.var_declaration.shadows_outer) {
        strings_shadow(output, indent_level, mangle_identifier(name, false));
    }

    if (node->data.var_declaration.type == TYPE_STR) {
        generate_str_declaration(output, node, indent_level);
        return;
    }

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

    // a str variable frees its old text: W__str_set(&W__s_v, value);
    if (target_type == TYPE_STR) {
        fprintf(output, C_STR_SET C_LPAREN C_ADDRESS_OF "%s" C_COMMA,
                mangle_identifier(node->data.assignment.target, false));
        generate(output, node->data.assignment.value, 0);
        fprintf(output, C_RPAREN C_SEMICOLON_NL);
        return;
    }

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

// ret with strings to clean up: the value is computed first (it may use them), then the
// function's temporaries and str variables are freed. A str result is a copy, handed to the
// caller as one of its temporaries:
//   { char* W__ret = W__str_copy(value); W__temps_free(W__fmark); free(W__s_v); return W__temp(W__ret); }
void generate_return(FILE* output, ASTNode* node, int indent_level) {
    ASTNode* value = node->data.return_statement.expression;

    if (!strings_return_needs_cleanup()) {
        emit_indent(output, indent_level);
        fprintf(output, C_RETURN C_SPACE);
        if (value) {
            generate(output, value, 0);
        }
        fprintf(output, C_SEMICOLON_NL);
        return;
    }

    DataType return_type = current_return_type();
    emit_indent(output, indent_level);
    fprintf(output, C_BLOCK_OPEN);
    if (value) {
        emit_indent(output, indent_level + 1);
        fprintf(output, "%s " C_RETURN_VALUE C_ASSIGN, get_c_type_from_enum(return_type));
        if (return_type == TYPE_STR) {
            fprintf(output, C_STR_COPY C_LPAREN);
            generate(output, value, 0);
            fprintf(output, C_RPAREN);
        } else {
            generate_expression_with_cast(output, value, return_type);
        }
        fprintf(output, C_SEMICOLON_NL);
    }
    strings_free_for_return(output, indent_level + 1);
    emit_indent(output, indent_level + 1);
    if (!value) {
        fprintf(output, C_RETURN C_SEMICOLON_NL);
    } else if (return_type == TYPE_STR) {
        fprintf(output, C_RETURN C_SPACE C_TEMP C_LPAREN C_RETURN_VALUE C_RPAREN C_SEMICOLON_NL);
    } else {
        fprintf(output, C_RETURN C_SPACE C_RETURN_VALUE C_SEMICOLON_NL);
    }
    emit_indent(output, indent_level);
    fprintf(output, C_RBRACE);
}
