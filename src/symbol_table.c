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
    symbol_table->scope_depth = 0;
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

    // check if function already exists
    FunctionSymbol* current = table->head;
    while (current != NULL) {
        if (strcmp(current->name, name) == 0) {
            return false;  // Function already declared
        }
        current = current->next;
    }

    // create new function symbol
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

static void free_symbol(Symbol* symbol) {
    free(symbol->name);
    free(symbol);
}

void free_symbol_table(void) {
    Symbol* current = symbol_table->head;
    while (current != NULL) {
        Symbol* next = current->next;
        free_symbol(current);
        current = next;
    }
    free(symbol_table);
}

void push_scope(SymbolTable* table) {
    if (table->scope_depth >= MAX_SCOPE_DEPTH) {
        parser_error("Blocks nested too deeply");
        return;
    }
    table->scope_starts[table->scope_depth++] = table->head;
}

void pop_scope(SymbolTable* table) {
    if (table->scope_depth == 0) return;
    Symbol* scope_start = table->scope_starts[--table->scope_depth];
    while (table->head != scope_start) {
        Symbol* next = table->head->next;
        free_symbol(table->head);
        table->head = next;
    }
}

// a name may shadow an outer scope, but not repeat within its own scope
bool add_symbol(SymbolTable* table, const char* name, DataType type) {
    Symbol* scope_start = table->scope_depth > 0 ? table->scope_starts[table->scope_depth - 1] : NULL;
    for (Symbol* s = table->head; s != scope_start; s = s->next) {
        if (strcmp(s->name, name) == 0) {
            return false;
        }
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
        case NODE_VARIABLE:
            // resolved when the node was created (TYPE_ZIL if undefined)
            return node->data.variable.base.expr_type;
        case NODE_BINARY_EXPR: {
            DataType left = get_expression_type(node->data.binary_expr.left, table);
            DataType right = get_expression_type(node->data.binary_expr.right, table);
            return get_operation_type(left, right, node->data.binary_expr.operator);
        }
        case NODE_UNARY_EXPR: {
            DataType operand = get_expression_type(node->data.unary_expr.operand, table);
            if (operand == TYPE_STR || operand == TYPE_ZIL) {
                char error_msg[100];
                snprintf(error_msg, sizeof(error_msg), "Cannot apply 'not' to %s", type_to_string(operand));
                parser_error(error_msg);
                return TYPE_ZIL;
            }
            return TYPE_BOOL;
        }
        case NODE_ASSIGNMENT:
            // resolved and type-checked when the node was created
            return node->data.assignment.target_type;
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

    OperatorCategory category = get_operator_category(op);

    // and / or: any non-str operand is truthy in C
    if (category == OP_CAT_LOGICAL) {
        if (left == TYPE_STR || right == TYPE_STR) {
            parser_error("Logical operators (and, or) cannot be used on str");
            return TYPE_ZIL;
        }
        return TYPE_BOOL;
    }

    // comparisons: str only compares with str (emitted via strcmp)
    if (category == OP_CAT_COMPARISON) {
        if ((left == TYPE_STR) != (right == TYPE_STR)) {
            parser_error("Cannot compare str with a non-str value");
            return TYPE_ZIL;
        }
        return TYPE_BOOL;
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
