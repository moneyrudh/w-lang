#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "symbol_table.h"
#include "parser.h"

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
        case NODE_ASSIGNMENT:
            return get_expression_type(node->data.assignment.value, table);
        case NODE_VARIABLE: {
            Symbol* symbol = lookup_symbol(table, node->data.variable.name);
            if (symbol == NULL) {
                fprintf(stderr, "Error: Undefined variable '%s'\n", node->data.variable.name);
                exit(1);
            }
            return symbol->type;
        }
        case NODE_BINARY_EXPR: {
            DataType left = get_expression_type(node->data.binary_expr.left, table);
            DataType right = get_expression_type(node->data.binary_expr.right, table);
            if (!compare_types(left, right)) {
                fprintf(stderr, "Error: Type mismatch in binary expression\n");
                exit(1);
            }
            return left;
        }
        default:
            fprintf(stderr, "Error: Unknown node type in AST\n");
            exit(1);
    }
}

bool compare_types(DataType left, DataType right) {
    if (left == right) {
        return true;
    }

    if (left == TYPE_FLOAT && right == INT) {
        return true;
    }

    if (left == INT && right == TYPE_CHAR) {
        return true;
    }

    if (left == TYPE_CHAR && right == INT) {
        return true;
    }
    // if (left == INT && right == STRING) {
    //     return true;
    // }
    // if (left == STRING && right == INT) {
    //     return true;
    // }
    return false;
}