#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "checker.h"
#include "parser.h"

static SymbolTable* symbol_table;
static FunctionTable* function_table;

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
