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

typedef struct SymbolTable {
    Symbol* head;
} SymbolTable;

SymbolTable* getSymbolTable(void);
void create_symbol_table(void);
void free_symbol_table(void);
bool add_symbol(SymbolTable* table, const char* name, DataType type);
Symbol* lookup_symbol(SymbolTable* table, const char* name);
DataType get_expression_type(ASTNode* node, SymbolTable* table);
bool compare_types(DataType left, DataType right);

extern SymbolTable* symbol_table;

#endif