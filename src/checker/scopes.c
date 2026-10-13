#include <stdlib.h>
#include <string.h>

#include "checker/internal.h"
#include "diagnostics.h"

#define MAX_SCOPE_DEPTH 128

// variables form a stack; each scope remembers where the stack stood when it opened
static Symbol* variables = NULL;
static Symbol* scope_starts[MAX_SCOPE_DEPTH];
static int scope_depth = 0;

static FunctionSymbol* functions = NULL;

static void free_symbol(Symbol* symbol) {
    free(symbol->name);
    free(symbol);
}

void init_scopes(void) {
    variables = NULL;
    scope_depth = 0;
    functions = NULL;
}

void free_scopes(void) {
    while (variables != NULL) {
        Symbol* next = variables->next;
        free_symbol(variables);
        variables = next;
    }
    scope_depth = 0;

    while (functions != NULL) {
        FunctionSymbol* next = functions->next;
        free(functions);
        functions = next;
    }
}

// ==================== variables ====================

void push_scope(int line) {
    if (scope_depth >= MAX_SCOPE_DEPTH) {
        report_error(line, "Blocks nested too deeply");
        return;
    }
    scope_starts[scope_depth++] = variables;
}

void pop_scope(void) {
    if (scope_depth == 0) return;
    Symbol* scope_start = scope_starts[--scope_depth];
    while (variables != scope_start) {
        Symbol* next = variables->next;
        free_symbol(variables);
        variables = next;
    }
}

bool declare_variable(const char* name, DataType type) {
    Symbol* scope_start = scope_depth > 0 ? scope_starts[scope_depth - 1] : NULL;
    for (Symbol* s = variables; s != scope_start; s = s->next) {
        if (strcmp(s->name, name) == 0) {
            return false;
        }
    }

    Symbol* symbol = malloc(sizeof(Symbol));
    symbol->name = strdup(name);
    symbol->type = type;
    symbol->next = variables;
    variables = symbol;
    return true;
}

Symbol* lookup_variable(const char* name) {
    for (Symbol* s = variables; s != NULL; s = s->next) {
        if (strcmp(s->name, name) == 0) {
            return s;
        }
    }
    return NULL;
}

// ==================== functions ====================

bool declare_function(ASTNode* function) {
    if (lookup_function(function->data.function.name)) {
        return false;
    }

    FunctionSymbol* symbol = malloc(sizeof(FunctionSymbol));
    symbol->function = function;
    symbol->next = functions;
    functions = symbol;
    return true;
}

FunctionSymbol* lookup_function(const char* name) {
    for (FunctionSymbol* f = functions; f != NULL; f = f->next) {
        if (strcmp(f->function->data.function.name, name) == 0) {
            return f;
        }
    }
    return NULL;
}
