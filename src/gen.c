#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "gen.h"
#include "symbol_table.h"
#include "types.h"
#include "ast.h"
#include "parser.h"

const char* get_c_type_string(DataType type) {
    switch (type) {
        case TYPE_INT:
            return "int";
        case TYPE_FLOAT:
            return "float";
        case TYPE_CHAR:
            return "char";
        case TYPE_BOOL:
            return "bool";
        case TYPE_STRING:
            return "char*";
        case TYPE_VOID:
            return "void";
        default:
            fprintf(stderr, "Uknown type in code generation\n");
            exit(1);
    }
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
                } else if (symbol->type == TYPE_INT) {
                    fprintf(output, "%%d");
                } else if (symbol->type == TYPE_STRING) {
                    fprintf(output, "%%s");
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

void generate(FILE* output, ASTNode* node, int indent_level) {
    if (!node) return;

    printf("Generating node of type %d\n", node->type);
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
        case NODE_FUNCTION:
            fprintf(output, "%s %s() {\n", node->data.function.return_type, node->data.function.name);
            ASTNode* statement = node->data.function.body;
            while (statement != NULL) {
                generate(output, statement, indent_level + 1);
                statement = statement->next;
            }
            fprintf(output, "}\n");
            break;
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
                switch (node->data.var_declaration.type) {
                    case TYPE_INT:
                    case TYPE_BOOL:
                        fprintf(output, " = 0");
                        break;
                    case TYPE_FLOAT:
                        fprintf(output, " = 0.0f");
                        break;
                    case TYPE_CHAR:
                        fprintf(output, " = '\\0'");
                        break;
                    case TYPE_STRING:
                        fprintf(output, " = NULL");
                        break;
                    default:
                        break;
                }
            }
            fprintf(output, ";\n");
            break;
        }
        case NODE_BINARY_EXPR:
            fprintf(output, "%s", indent);
            generate(output, node->data.binary_expr.left, 0);
            fprintf(output, " %c ", node->data.binary_expr.operator);
            generate(output, node->data.binary_expr.right, 0);
            fprintf(output, ";\n");
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
            fprintf(output, "%s%s = ", indent, node->data.assignment.target);
            generate_expression_with_cast(
                output,
                node->data.assignment.value,
                node->data.assignment.base.expr_type
            );
            fprintf(output, ";\n");
            break;
        }
        case NODE_VARIABLE:
            fprintf(output, "%s", node->data.variable.name);
            break;
        case NODE_RETURN: {
            fprintf(output, "%sreturn ", indent);
            if (node->data.return_statement.expression) {
                DataType type = get_expression_type(node->data.return_statement.expression, getSymbolTable());
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
