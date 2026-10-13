#include <stdio.h>
#include <string.h>

#include "parser.h"
#include "checker.h"
#include "codegen/internal.h"
#include "codegen/c_syntax.h"
#include "language/type_registry.h"

void generate_var_declaration(FILE* output, ASTNode* node, int indent_level) {
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
    DataType value_type = get_expression_type(node->data.assignment.value, getSymbolTable());

    fprintf(output, "%s" C_ASSIGN, mangle_identifier(node->data.assignment.target, false));

    generate_cast_if_needed(output, value_type, target_type);

    generate(output, node->data.assignment.value, 0);
    fprintf(output, C_SEMICOLON_NL);
}

void generate_log_statement(FILE* output, LogElement* elements, int indent_level) {
    emit_indent(output, indent_level);
    fprintf(output, C_PRINTF C_LPAREN C_STRING_QUOTE);

    LogElement* current = elements;
    while (current != NULL) {
        switch (current->type) {
            case NODE_STRING:
                fprintf(output, "%s", current->value.string);
                break;
            case NODE_NUMBER:
                fputs(C_FMT_INT, output);
                break;
            case NODE_VARIABLE:
                fputs(get_format_spec_from_enum(current->var_type), output);
                break;
            default:
                parser_error("Unknown node type in log statement");
        }
        current = current->next;
    }

    fprintf(output, C_ESC_NEWLINE C_STRING_QUOTE);

    current = elements;
    while (current != NULL) {
        if (current->type == NODE_NUMBER) {
            fprintf(output, C_COMMA "%d", current->value.number);
        } else if (current->type == NODE_VARIABLE) {
            fprintf(output, C_COMMA "%s", mangle_identifier(current->value.string, false));
        }
        current = current->next;
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
