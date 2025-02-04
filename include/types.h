#ifndef W_TYPES_H
#define W_TYPES_H

#include <stdio.h>
#include <stdbool.h>

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
    ASSIGNMENT,
    RETURN,
    FLOAT_TYPE,
    FLOAT_LITERAL,
    CHAR_TYPE,
    STRING_TYPE,
    CHAR_LITERAL,
    BOOL_TYPE,
    BOOL_LITERAL
} TokenType;

typedef struct {
    union {
        char* string;
        int number;
        double float_val;
        char char_val;
        bool bool_val;
    };
} YYSTYPE;

extern YYSTYPE yylval;
extern int yylineno;
extern TokenType token;

typedef enum {
    TYPE_INT,
    TYPE_FLOAT,
    TYPE_CHAR,
    TYPE_BOOL,
    TYPE_STRING,
    TYPE_VOID
} DataType;

typedef enum {
    NODE_PROGRAM,
    NODE_FUNCTION,
    NODE_FUNCTION_CALL,
    NODE_LOG,
    NODE_BINARY_EXPR,
    NODE_UNARY_EXPR,
    NODE_NUMBER,
    NODE_STRING,
    NODE_FLOAT,
    NODE_CHAR,
    NODE_BOOL,
    NODE_VARIABLE,
    NODE_VAR_DECLARATION,
    NODE_ASSIGNMENT,
    NODE_RETURN
} NodeType;

typedef struct LogElement {
    NodeType type;
    union {
        char* string;
        int number;
        double float_val;
        char char_val;
        bool bool_val;
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