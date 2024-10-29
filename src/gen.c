#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "gen.h"
#include "symbol_table.h"
#include "types.h"
#include "ast.h"
#include "parser.h"

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
            ASTNode* function = node->data.program.functions;
            while (function != NULL) {
                generate(output, function, indent_level);
                function = function->next;
            }
        } break;
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
            fprintf(output, "\"%s\"", node->data.string.value);
            break;
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
