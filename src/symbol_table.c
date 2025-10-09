#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "symbol_table.h"
#include "parser.h"
#include "operator_utils.h"

SymbolTable* symbol_table;
FunctionTable* function_table;

SymbolTable* getSymbolTable(void) {
    return symbol_table;
}

void create_symbol_table() {
    symbol_table = malloc(sizeof(SymbolTable));
    symbol_table->head = NULL;
    return;
}

FunctionTable* getFunctionTable(void) {
    return function_table;
}

void create_function_table() {
    function_table = malloc(sizeof(FunctionTable));
    function_table->head = NULL;
}

bool add_function(FunctionTable* table, const char* name, DataType return_type) {
    if (!table) return false;

    // Check if function already exists
    FunctionSymbol* current = table->head;
    while (current != NULL) {
        if (strcmp(current->name, name) == 0) {
            return false;  // Function already declared
        }
        current = current->next;
    }

    // Create new function symbol
    FunctionSymbol* func = malloc(sizeof(FunctionSymbol));
    if (!func) return false;

    func->name = strdup(name);
    func->return_type = return_type;
    func->next = table->head;
    table->head = func;
    return true;
}

FunctionSymbol* lookup_function(FunctionTable* table, const char* name) {
    if (!table) return NULL;

    FunctionSymbol* current = table->head;
    while (current != NULL) {
        if (strcmp(current->name, name) == 0) {
            return current;
        }
        current = current->next;
    }
    return NULL;
}

void free_function_table(void) {
    if (!function_table) return;

    FunctionSymbol* current = function_table->head;
    while (current != NULL) {
        FunctionSymbol* next = current->next;
        free(current->name);
        free(current);
        current = next;
    }
    free(function_table);
    function_table = NULL;
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
    if (!node) return TYPE_ZIL;

    switch (node->type) {
        case NODE_NUMBER:
            return TYPE_NUM;
        case NODE_STRING:
            return TYPE_STR;
        case NODE_FLOAT:
            return TYPE_REAL;
        case NODE_CHAR:
            return TYPE_CHR;
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
                return TYPE_ZIL;
            }
            return symbol->type;
        }
        case NODE_BINARY_EXPR: {
            DataType left = get_expression_type(node->data.binary_expr.left, table);
            DataType right = get_expression_type(node->data.binary_expr.right, table);
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
                return TYPE_ZIL;
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
                return TYPE_ZIL;
            }

            return symbol->type;
        }
        case NODE_FUNCTION_CALL: {
            FunctionSymbol* func = lookup_function(getFunctionTable(), node->data.function_call.name);
            if (!func) {
                char error_msg[100];
                snprintf(
                    error_msg,
                    sizeof(error_msg),
                    "Undefined function: '%s'",
                    node->data.function_call.name
                );
                parser_error(error_msg);
                return TYPE_ZIL;
            }
            return func->return_type;
        }
        default:
            parser_error("Unknown expression type");
            return TYPE_ZIL;
    }
}

bool compare_types(DataType left, DataType right) {
    if (left == right) {
        return true;
    }

    if (left == TYPE_REAL && right == TYPE_NUM) {
        return true;
    }

    if (left == TYPE_NUM && right == TYPE_CHR) {
        return true;
    }

    if (
        (left == TYPE_BOOL && right == TYPE_NUM)
        ||
        (left == TYPE_NUM && right == TYPE_BOOL)
    ) {
        return true;
    }

    return false;
}

bool can_convert_type(DataType from, DataType to) {
    if (from == to) return true;

    switch (to) {
        case TYPE_REAL:
            return from == TYPE_NUM || from == TYPE_CHR || from == TYPE_BOOL;
        case TYPE_NUM:
            return from == TYPE_CHR || from == TYPE_BOOL;
        case TYPE_BOOL:
            return from == TYPE_NUM || from == TYPE_REAL || from == TYPE_CHR;
        case TYPE_CHR:
            return from == TYPE_NUM;
        case TYPE_STR:
            return false;
        case TYPE_ZIL:
            return false;
        default:
            return false;
    }
}

DataType get_operation_type(DataType left, DataType right, OperatorType op) {
    if (left == TYPE_ZIL || right == TYPE_ZIL) {
        parser_error("Cannot perform operations on zil type");
        return TYPE_ZIL;
    }

    // string concat between string 1 and string 2
    if (op == OP_ADD && (left == TYPE_STR || right == TYPE_STR)) {
        if (left == TYPE_STR && right == TYPE_STR) {
            return TYPE_STR;
        }
        parser_error("String concatenation only works betweens strings");
        return TYPE_ZIL;
    }

    // invalid operation on string (can only do +)
    if (left == TYPE_STR || right == TYPE_STR) {
        parser_error("Invalid arithmetic operation on str type");
        return TYPE_ZIL;
    }

    if (left == TYPE_REAL || right == TYPE_REAL) {
        return TYPE_REAL;
    }

    // converts num, chr, and bool into num
    return TYPE_NUM;
}
