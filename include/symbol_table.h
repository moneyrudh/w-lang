#ifndef SYMBOL_TABLE_H
#define SYMBOL_TABLE_H

#include <stdbool.h>

#include "types.h"
#include "ast.h"

typedef struct Symbol {
    char* name;
    DataType type;
    struct Symbol* next;
} Symbol;

#define MAX_SCOPE_DEPTH 128

// symbols form a stack; each scope remembers where the stack stood when it opened
typedef struct SymbolTable {
    Symbol* head;
    Symbol* scope_starts[MAX_SCOPE_DEPTH];
    int scope_depth;
} SymbolTable;

typedef struct FunctionSymbol {
    char* name;
    DataType return_type;
    struct FunctionSymbol* next;
} FunctionSymbol;

typedef struct FunctionTable {
    FunctionSymbol* head;
} FunctionTable;

SymbolTable* getSymbolTable(void);
void create_symbol_table(void);
void free_symbol_table(void);
bool add_symbol(SymbolTable* table, const char* name, DataType type);
Symbol* lookup_symbol(SymbolTable* table, const char* name);
void push_scope(SymbolTable* table);
void pop_scope(SymbolTable* table);
DataType get_expression_type(ASTNode* node, SymbolTable* table);

FunctionTable* getFunctionTable(void);
void create_function_table(void);
void free_function_table(void);
bool add_function(FunctionTable* table, const char* name, DataType return_type);
FunctionSymbol* lookup_function(FunctionTable* table, const char* name);

bool compare_types(DataType left, DataType right);
DataType get_operation_type(DataType left, DataType right, OperatorType op);
bool can_convert_type(DataType from, DataType to);

extern SymbolTable* symbol_table;

#endif