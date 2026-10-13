#ifndef W_CHECKER_INTERNAL_H
#define W_CHECKER_INTERNAL_H

// shared between the checker's files only; the public API is include/checker.h

#include <stdbool.h>

#include "types.h"
#include "ast.h"

// what the checker is inside of right now
typedef struct {
    ASTNode* function;   // function whose body is being checked
    bool has_return;     // a 'ret' was seen in it (not per path)
    int loop_depth;      // > 0 inside a loop body: skip / stop are valid
} CheckerContext;

extern CheckerContext checker_context;

// ==================== scopes.c ====================

typedef struct Symbol {
    char* name;
    DataType type;
    struct Symbol* next;
} Symbol;

typedef struct FunctionSymbol {
    ASTNode* function;   // the NODE_FUNCTION: name, return type, parameters
    struct FunctionSymbol* next;
} FunctionSymbol;

void init_scopes(void);
void free_scopes(void);

// line is for the "nested too deeply" error
void push_scope(int line);
void pop_scope(void);

// false if the name is already declared in the innermost scope (outer ones may be shadowed)
bool declare_variable(const char* name, DataType type);
Symbol* lookup_variable(const char* name);

// false if a function with this name already exists
bool declare_function(ASTNode* function);
FunctionSymbol* lookup_function(const char* name);

// ==================== types.c ====================

// result type of `left op right`; on a type error returns TYPE_ERROR and sets *error
DataType operation_type(DataType left, DataType right, OperatorType op, const char** error);

// ==================== expressions.c ====================

// checks an expression, stores its type in node->expr_type and returns it.
// TYPE_ERROR means an error was already reported: don't report another for it
DataType check_expression(ASTNode* node);

// ==================== statements.c ====================

// a block: its own scope
void check_block(ASTNode* statements);

// a list of statements in the current scope
void check_statements(ASTNode* statements);

void check_statement(ASTNode* node);

// ==================== functions.c ====================

// declares every function, so calls can come before definitions
void declare_functions(ASTNode* functions);

void check_function(ASTNode* function);
void check_return(ASTNode* node);
DataType check_call(ASTNode* node);

// ==================== control_flow.c ====================

void check_if(ASTNode* node);
void check_loop(ASTNode* node);
void check_loop_jump(ASTNode* node);

#endif
