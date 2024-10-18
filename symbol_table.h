#ifndef SYMBOL_TABLE_H
#define SYMBOL_TABLE_H

#include <stdbool.h>
#include "parser.h"

typedef enum {
    TYPE_INT,
    TYPE_STRING,
    TYPE_VOID
} DataType;

typedef struct Symbol {
    char* name;
    DataType type;
    struct Symbol* next;
} Symbol;

typedef struct SymbolTable {
    Symbol* head;
} SymbolTable;

SymbolTable* create_symbol_table();
void free_symbol_table(SymbolTable* table);
bool add_symbol(SymbolTable* table, const char* name, DataType type);
Symbol* lookup_symbol(SymbolTable* table, const char* name);
DataType get_expression_type(ASTNode* node, SymbolTable* table);
bool compare_types(DataType left, DataType right);

#endif