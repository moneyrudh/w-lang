#ifndef W_TYPES_H
#define W_TYPES_H

#include <stdio.h>

typedef enum {
    INT = 258,
    VOID,
    IDENTIFIER,
    LOG,
    LPAREN,
    RPAREN,
    LBRACE,
    RBRACE,
    SEMICOLON,
    STRING,
    NUMBER,
    PLUS,
    MINUS,
    MULTIPLY,
    DIVIDE,
    MAIN,
    RETURN
} TokenType;

typedef struct {
    union {
        char* string;
        int number;
    };
} YYSTYPE;

extern YYSTYPE yylval;
extern int yylineno;
extern TokenType token;

typedef enum {
    NODE_PROGRAM,
    NODE_FUNCTION,
    NODE_LOG,
    NODE_BINARY_EXPR,
    NODE_NUMBER,
    NODE_STRING,
    NODE_VARIABLE,
    NODE_RETURN
} NodeType;

typedef struct LogElement {
    NodeType type;
    union {
        char* string;
        int number;
        char* variable;
    } value;
    struct LogElement* next;
} LogElement;

typedef struct {
    char* current_name;
    char* current_return_type;
    int has_return;
    int return_value_required;
} FunctionContext;

typedef struct {
    int brace_depth;
    int* block_lines;
    int block_capacity;
    int error_count;
    int in_function_body;
    FunctionContext* function_context;
} ParserState;

extern int yylex();
extern char* yytext;
extern FILE* yyin;

#endif