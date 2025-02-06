#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "symbol_table.h"
#include "parser.h"
#include "operator_utils.h"

SymbolTable* symbol_table;

SymbolTable* getSymbolTable(void) {
    return symbol_table;
}

void create_symbol_table() {
    symbol_table = malloc(sizeof(SymbolTable));
    symbol_table->head = NULL;
    return;
}

void free_symbol_table(void) {
    Symbol* current = symbol_table->head;
    while (current != NULL) {
        Symbol* next = current->next;
        free(current->name);
        free(current);
        current = next;
    }
    free(symbol_table);
}

bool add_symbol(SymbolTable* table, const char* name, DataType type) {
    if (lookup_symbol(table, name) != NULL) {
        return false;
    }

    Symbol* symbol = malloc(sizeof(Symbol));
    symbol->name = strdup(name);
    symbol->type = type;
    symbol->next = table->head;
    table->head = symbol;
    return true;
}

Symbol* lookup_symbol(SymbolTable* table, const char* name) {
    Symbol* current = table->head;
    while (current != NULL) {
        if (strcmp(current->name, name) == 0) {
            return current;
        }
        current = current->next;
    }
    return NULL;
}

DataType get_expression_type(ASTNode* node, SymbolTable* table) {
    if (!node) return TYPE_VOID;

    switch (node->type) {
        case NODE_NUMBER:
            return TYPE_INT;
        case NODE_STRING:
            return TYPE_STRING;
        case NODE_FLOAT:
            return TYPE_FLOAT;
        case NODE_CHAR:
            return TYPE_CHAR;
        case NODE_BOOL:
            return TYPE_BOOL;
        case NODE_VARIABLE: {
            Symbol* symbol = lookup_symbol(table, node->data.variable.name);
            if (!symbol) {
                char error_msg[100];
                snprintf(
                    error_msg, 
                    sizeof(error_msg),
                    "Undefined variable: '%s'",
                    node->data.variable.name
                );
                parser_error(error_msg);
                return TYPE_VOID;
            }
            return symbol->type;
        }
        case NODE_BINARY_EXPR: {
            DataType left = get_expression_type(node->data.binary_expr.left, table);
            DataType right = get_expression_type(node->data.binary_expr.right, table);
            // if (!compare_types(left, right)) {
            //     fprintf(stderr, "Error: Type mismatch in binary expression\n");
            //     exit(1);
            // }
            // return left;
            return get_operation_type(
                left, 
                right,
                char_to_operator(node->data.binary_expr.operator)
            );
        }
        case NODE_ASSIGNMENT: {
            // for x = (y + z) * f, the AST would look like:
            // NODE_ASSIGNMENT
            // ├── target: "x"
            // └── value: NODE_BINARY_EXPR (*)
            //     ├── left: NODE_BINARY_EXPR (+)
            //     │   ├── left: NODE_VARIABLE ("y")
            //     │   └── right: NODE_VARIABLE ("z")
            //     └── right: NODE_VARIABLE ("f")

            Symbol* symbol = lookup_symbol(table, node->data.assignment.target);
            if (!symbol) {
                char error_msg[100];
                snprintf(
                    error_msg,
                    sizeof(error_msg),
                    "Assignment to undefined variable '%s'",
                    node->data.assignment.target
                );
                parser_error(error_msg);
                return TYPE_VOID;
            }

            DataType value_type = get_expression_type(node->data.assignment.value, table);
            if (!can_convert_type(value_type, symbol->type)) {
                char error_msg[100];
                snprintf(
                    error_msg, 
                    sizeof(error_msg),
                    "Cannot assign %s to %s",
                    type_to_string(value_type),
                    type_to_string(symbol->type)
                );
                parser_error(error_msg);
                return TYPE_VOID;
            }

            return symbol->type;
        }
        default:
            parser_error("Unknown expression type");
            return TYPE_VOID;
    }
}

bool compare_types(DataType left, DataType right) {
    if (left == right) {
        return true;
    }

    if (left == TYPE_FLOAT && right == TYPE_INT) {
        return true;
    }

    if (left == TYPE_INT && right == TYPE_CHAR) {
        return true;
    }

    if (left == TYPE_CHAR && right == TYPE_INT) {
        return true;
    }
    // if (left == TYPE_INT && right == TYPE_STRING) {
    //     return true;
    // }
    // if (left == TYPE_STRING && right == TYPE_INT) {
    //     return true;
    // }
    return false;
}

bool can_convert_type(DataType from, DataType to) {
    if (from == to) return true;

    switch (to) {
        case TYPE_FLOAT:
            return from == TYPE_INT || from == TYPE_CHAR || from == TYPE_BOOL;
        case TYPE_INT:
            return from == TYPE_CHAR || from == TYPE_BOOL;
        case TYPE_BOOL:
            return from == TYPE_INT || from == TYPE_FLOAT || from == TYPE_CHAR;
        default:
            return false;
    }
}

DataType get_operation_type(DataType left, DataType right, OperatorType op) {
    if (left == TYPE_VOID || right == TYPE_VOID) {
        parser_error("Cannot perform operations on void type");
        return TYPE_VOID;
    }

    if (op == OP_ADD && (left == TYPE_STRING || right == TYPE_STRING)) {
        if (left == TYPE_STRING && right == TYPE_STRING) {
            return TYPE_STRING;
        }
        parser_error("String concatenation only works between strings");
        return TYPE_VOID;
    }

    if (left == TYPE_STRING || right == TYPE_STRING) {
        parser_error("Invalid operation on string type");
        return TYPE_VOID;
    }

    if (left == TYPE_FLOAT || right == TYPE_FLOAT) {
        return TYPE_FLOAT;
    }

    return TYPE_INT;
}