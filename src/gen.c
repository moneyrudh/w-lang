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

const char* get_c_type_string(DataType type) {
    return get_c_type_from_enum(type);
}

void generate_log_statement(FILE* output, LogElement* elements, int indent_level) {
    char indent[256] = {0};
    for (int i=0; i < indent_level; i++) {
        strcat(indent, "    ");
    }

    fprintf(output, "%sprintf(\"", indent);

    LogElement* current = elements;
    int arg_count = 0;
    while (current != NULL) {
        switch (current->type) {
            case NODE_STRING:
                fprintf(output, "%s", current->value.string);
                break;
            case NODE_NUMBER:
                fprintf(output, "%%d");
                arg_count++;
                break;
            case NODE_VARIABLE: {
                Symbol* symbol = lookup_symbol(getSymbolTable(), current->value.string);
                if (symbol == NULL) {
                    parser_error("Undefined variable in log statement");
                } else if (symbol->type == TYPE_NUM) {
                    fprintf(output, "%%d");
                } else if (symbol->type == TYPE_REAL) {
                    fprintf(output, "%%f");
                } else if (symbol->type == TYPE_CHR) {
                    fprintf(output, "%%c");
                } else if (symbol->type == TYPE_STR) {
                    fprintf(output, "%%s");
                } else if (symbol->type == TYPE_BOOL) {
                    fprintf(output, "%%d");
                } else {
                    parser_error("Invalid type in log statement");
                }
                arg_count++;
                break;
            }
            default:
                parser_error("Unknown node type in log statement");
        }
        current = current->next;
    }

    fprintf(output, "\\n\"");

    current = elements;
    while (current != NULL) {
        if (current->type == NODE_NUMBER) {
            fprintf(output, ", %d", current->value.number);
        } else if (current->type == NODE_VARIABLE) {
            fprintf(output, ", %s", current->value.string);
        }
        current = current->next;
    }

    fprintf(output, ");\n");
}

void generate_expression_with_cast(FILE* output, ASTNode* expr, DataType target_type) {
    if (!expr) return;

    DataType expr_type = get_expression_type(expr, getSymbolTable());

    if (expr_type != target_type && compare_types(target_type, expr_type)) {
        fprintf(output, "(%s)(", get_c_type_string(target_type));
        generate(output, expr, 0);
        fprintf(output, ")");
    } else {
        generate(output, expr, 0);
    }
}

static void generate_cast_if_needed(FILE* output, DataType from, DataType to) {
    if (from == to) return;
    
    if (can_convert_type(from, to)) {
        fprintf(output, "(%s)", get_c_type_string(to));
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
        fprintf(output, "(");
        generate_cast_if_needed(output, left_type, result_type);
        generate(output, node->data.binary_expr.left, indent_level);
        fprintf(output, ")");
    } else {
        generate_cast_if_needed(output, left_type, result_type);
        generate(output, node->data.binary_expr.left, indent_level);
    }
    
    fprintf(output, " %c ", node->data.binary_expr.operator);
    
    generate_cast_if_needed(output, right_type, result_type);
    generate(output, node->data.binary_expr.right, indent_level);
}

static void generate_assignment(FILE* output, ASTNode* node, int indent_level) {
    if (!node || node->type != NODE_ASSIGNMENT) return;

    char indent[256] = {0};
    for (int i = 0; i < indent_level; i++) {
        strcat(indent, "    ");
    }

    Symbol* target_symbol = lookup_symbol(getSymbolTable(), node->data.assignment.target);
    DataType target_type = target_symbol->type;
    DataType value_type = get_expression_type(node->data.assignment.value, getSymbolTable());

    fprintf(output, "%s%s = ", indent, node->data.assignment.target);

    generate_cast_if_needed(output, value_type, target_type);

    generate(output, node->data.assignment.value, 0);
    fprintf(output, ";\n");
}

void generate(FILE* output, ASTNode* node, int indent_level) {
    if (!node) return;

    // printf("Generating node of type %d\n", node->type);
    char indent[256] = {0};
    for (int i=0; i < indent_level; i++) {
        strcat(indent, "    ");
    }

    switch (node->type) {
        case NODE_PROGRAM: {
            fprintf(output, "#include <stdio.h>\n\n");
            fprintf(output, "#include <stdbool.h>\n\n");
            fprintf(output, "#include <string.h>\n\n");

            if (node->data.program.globals) {
                ASTNode* global = node->data.program.globals;
                while (global) {
                    generate(output, global, 0);
                    fprintf(output, "\n");
                    global = global->next;
                }
                fprintf(output, "\n");
            }

            ASTNode* function = node->data.program.functions;
            while (function != NULL) {
                generate(output, function, indent_level);
                fprintf(output, "\n");
                function = function->next;
            }
            break;
        }
        case NODE_FUNCTION: {
            // Convert W Lang return type to C type
            const TypeMapping* mapping = type_registry_get_by_wlang_name(node->data.function.return_type);
            const char* c_return_type = mapping ? mapping->c_equivalent : node->data.function.return_type;
            fprintf(output, "%s %s() {\n", c_return_type, node->data.function.name);
            ASTNode* statement = node->data.function.body;
            while (statement != NULL) {
                generate(output, statement, indent_level + 1);
                statement = statement->next;
            }
            fprintf(output, "}\n");
            break;
        }
        case NODE_LOG:
            generate_log_statement(output, node->data.log.elements, indent_level);
            break;
        case NODE_VAR_DECLARATION: {
            fprintf(
                output, "%s%s %s",
                indent,
                get_c_type_string(node->data.var_declaration.type),
                node->data.var_declaration.name
            );

            if (node->data.var_declaration.init_expr) {
                fprintf(output, " = ");
                generate_expression_with_cast(
                    output,
                    node->data.var_declaration.init_expr,
                    node->data.var_declaration.type
                );
            } else {
                const char* default_val = get_default_value_from_enum(node->data.var_declaration.type);
                if (default_val && strlen(default_val) > 0) {
                    fprintf(output, " = %s", default_val);
                }
            }
            fprintf(output, ";\n");
            break;
        }
        case NODE_BINARY_EXPR:
            generate_binary_expr(output, node, indent_level);
            break;
        case NODE_NUMBER:
            fprintf(output, "%d", node->data.number.value);
            break;
        case NODE_STRING:
            fprintf(output, "\"");
            for (const char* p = node->data.string.value; *p; p++) {
                switch (*p) {
                    case '\n': fprintf(output, "\\n"); break;
                    case '\t': fprintf(output, "\\t"); break;
                    case '\"': fprintf(output, "\\\""); break;
                    case '\\': fprintf(output, "\\\\"); break;
                    default: fputc(*p, output);
                }
            }
            fprintf(output, "\"");
            break;
        case NODE_FLOAT:
            fprintf(output, "%ff", node->data.float_val.value);
            break;
        case NODE_CHAR:
            if (node->data.char_val.value == '\n') {
                fprintf(output, "'\\n'");
            } else if (node->data.char_val.value == '\t') {
                fprintf(output, "'\\t'");
            } else if (node->data.char_val.value == '\'') {
                fprintf(output, "'\\''");
            } else if (node->data.char_val.value == '\\') {
                fprintf(output, "'\\\\'");
            } else {
                fprintf(output, "'%c'", node->data.char_val.value);
            }
            break;
        case NODE_BOOL:
            fprintf(output, "%s", node->data.bool_val.value ? "true" : "false");
            break;
        case NODE_ASSIGNMENT: {
            generate_assignment(output, node, indent_level);
            break;
        }
        case NODE_VARIABLE:
            fprintf(output, "%s", node->data.variable.name);
            break;
        case NODE_RETURN: {
            fprintf(output, "%sreturn ", indent);
            if (node->data.return_statement.expression) {
                // DataType type = get_expression_type(node->data.return_statement.expression, getSymbolTable());
                generate(output, node->data.return_statement.expression, 0);
            }
            fprintf(output, ";\n");
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
