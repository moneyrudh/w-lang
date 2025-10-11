#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "gen.h"
#include "symbol_table.h"
#include "types.h"
#include "ast.h"
#include "parser.h"
#include "operator_utils.h"
#include "transpiler/type_registry.h"
#include "codegen/c_syntax.h"
#include "codegen/formatters.h"

const char* get_c_type_string(DataType type) {
    return get_c_type_from_enum(type);
}

void generate_log_statement(FILE* output, LogElement* elements, int indent_level) {
    emit_indent(output, indent_level);
    fprintf(output, C_PRINTF C_LPAREN C_STRING_QUOTE);

    LogElement* current = elements;
    int arg_count = 0;
    while (current != NULL) {
        switch (current->type) {
            case NODE_STRING:
                fprintf(output, "%s", current->value.string);
                break;
            case NODE_NUMBER:
                fputs(C_FMT_INT, output);
                arg_count++;
                break;
            case NODE_VARIABLE: {
                Symbol* symbol = lookup_symbol(getSymbolTable(), current->value.string);
                if (symbol == NULL) {
                    parser_error("Undefined variable in log statement");
                } else {
                    const char* fmt = get_format_spec_from_enum(symbol->type);
                    fputs(fmt, output);
                    arg_count++;
                }
                break;
            }
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
            fprintf(output, C_COMMA "%s", current->value.string);
        }
        current = current->next;
    }

    fprintf(output, C_RPAREN C_SEMICOLON_NL);
}

void generate_expression_with_cast(FILE* output, ASTNode* expr, DataType target_type) {
    if (!expr) return;

    DataType expr_type = get_expression_type(expr, getSymbolTable());

    if (expr_type != target_type && compare_types(target_type, expr_type)) {
        fprintf(output, C_LPAREN "%s" C_RPAREN C_LPAREN, get_c_type_string(target_type));
        generate(output, expr, 0);
        fprintf(output, C_RPAREN);
    } else {
        generate(output, expr, 0);
    }
}

static void generate_cast_if_needed(FILE* output, DataType from, DataType to) {
    if (from == to) return;

    if (can_convert_type(from, to)) {
        emit_cast(output, from, to);
    }
}

static void generate_binary_expr(FILE* output, ASTNode* node, int indent_level) {
    if (!node || node->type != NODE_BINARY_EXPR) return;

    DataType left_type = get_expression_type(node->data.binary_expr.left, getSymbolTable());
    DataType right_type = get_expression_type(node->data.binary_expr.right, getSymbolTable());
    DataType result_type = get_operation_type(left_type, right_type, char_to_operator(node->data.binary_expr.operator));

    bool needs_parens = node->data.binary_expr.left->type == NODE_BINARY_EXPR ||
                        node->data.binary_expr.right->type == NODE_BINARY_EXPR;

    if (needs_parens && node->data.binary_expr.left->type == NODE_BINARY_EXPR) {
        fprintf(output, C_LPAREN);
        generate_cast_if_needed(output, left_type, result_type);
        generate(output, node->data.binary_expr.left, indent_level);
        fprintf(output, C_RPAREN);
    } else {
        generate_cast_if_needed(output, left_type, result_type);
        generate(output, node->data.binary_expr.left, indent_level);
    }

    fprintf(output, "%s", get_binary_operator_string(node->data.binary_expr.operator));

    generate_cast_if_needed(output, right_type, result_type);
    generate(output, node->data.binary_expr.right, indent_level);
}

static void generate_assignment(FILE* output, ASTNode* node, int indent_level) {
    if (!node || node->type != NODE_ASSIGNMENT) return;

    emit_indent(output, indent_level);

    Symbol* target_symbol = lookup_symbol(getSymbolTable(), node->data.assignment.target);
    DataType target_type = target_symbol->type;
    DataType value_type = get_expression_type(node->data.assignment.value, getSymbolTable());

    fprintf(output, "%s" C_ASSIGN, node->data.assignment.target);

    generate_cast_if_needed(output, value_type, target_type);

    generate(output, node->data.assignment.value, 0);
    fprintf(output, C_SEMICOLON_NL);
}

void generate(FILE* output, ASTNode* node, int indent_level) {
    if (!node) return;


    switch (node->type) {
        case NODE_PROGRAM: {
            emit_c_includes(output);

            if (node->data.program.globals) {
                ASTNode* global = node->data.program.globals;
                while (global) {
                    generate(output, global, 0);
                    fprintf(output, C_NEWLINE);
                    global = global->next;
                }
                fprintf(output, C_NEWLINE);
            }

            ASTNode* function = node->data.program.functions;
            while (function != NULL) {
                generate(output, function, indent_level);
                fprintf(output, C_NEWLINE);
                function = function->next;
            }
            break;
        }
        case NODE_FUNCTION: {
            // convert W Lang return type to C type
            const TypeMapping* mapping = type_registry_get_by_wlang_name(node->data.function.return_type);
            const char* c_return_type = mapping ? mapping->c_equivalent : node->data.function.return_type;

            // generate function signature using formatter
            emit_function_signature(output, c_return_type, node->data.function.name,
                                   node->data.function.parameters, node->data.function.param_count);

            // generate function body
            ASTNode* statement = node->data.function.body;
            while (statement != NULL) {
                generate(output, statement, indent_level + 1);
                statement = statement->next;
            }
            fprintf(output, C_RBRACE);
            break;
        }
        case NODE_LOG:
            generate_log_statement(output, node->data.log.elements, indent_level);
            break;
        case NODE_VAR_DECLARATION: {
            emit_indent(output, indent_level);
            fprintf(
                output, "%s %s",
                get_c_type_string(node->data.var_declaration.type),
                node->data.var_declaration.name
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
            break;
        }
        case NODE_BINARY_EXPR:
            generate_binary_expr(output, node, indent_level);
            break;
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
        case NODE_ASSIGNMENT: {
            generate_assignment(output, node, indent_level);
            break;
        }
        case NODE_VARIABLE:
            fprintf(output, "%s", node->data.variable.name);
            break;
        case NODE_RETURN: {
            emit_indent(output, indent_level);
            fprintf(output, C_RETURN C_SPACE);
            if (node->data.return_statement.expression) {
                // DataType type = get_expression_type(node->data.return_statement.expression, getSymbolTable());
                generate(output, node->data.return_statement.expression, 0);
            }
            fprintf(output, C_SEMICOLON_NL);
            break;
        }
        case NODE_FUNCTION_CALL: {
            // if this is at indent_level > 0, it's a statement, so add indent and semicolon
            // if indent_level == 0, it's an expression within another expression
            if (indent_level > 0) {
                emit_indent(output, indent_level);
            }

            fprintf(output, "%s" C_LPAREN, node->data.function_call.name);
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
            break;
        }
        default:
            fprintf(stderr, "Unknown node type in AST\n");
            exit(1);
    }
}

void generate_code(FILE* output, ASTNode* node) {
    if (!node) return;

    if (node->type != NODE_PROGRAM) {
        fprintf(stderr, "Expected program node at root.\n");
        return;
    }

    generate(output, node, 0);
}
