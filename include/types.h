#ifndef W_TYPES_H
#define W_TYPES_H

#include <stdio.h>
#include <stdbool.h>

typedef enum {
    NUM = 258,
    ZIL,
    REAL,
    CHR,
    STR,
    BOOL,

    VEC,
    MAP,
    SET,
    REF,
    HEAP,
    STACK,
    QUE,
    LINK,
    TREE,
    POD,

    DEC,
    FUN,
    USE,
    MAIN,
    RETURN,
    LOG,

    IDENTIFIER,
    INT_LITERAL,
    FLOAT_LITERAL,
    STRING_LITERAL,
    CHAR_LITERAL,
    BOOL_LITERAL,

    COLON,
    INFER_ASSIGN,
    ASSIGNMENT,
    LBRACKET,
    RBRACKET,
    LPAREN,
    RPAREN,
    LBRACE,
    RBRACE,
    COMMA,
    SEMICOLON,

    PLUS,
    MINUS,
    MULTIPLY,
    DIVIDE,

    EQUAL,
    NOT_EQUAL,
    LESS,
    GREATER,
    LESS_EQUAL,
    GREATER_EQUAL,

    EQ,
    NE,
    GT,
    LT,
    GE,
    LE,

    AND,
    OR,
    NOT,
    BANG,

    IS
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
    TYPE_NUM,
    TYPE_REAL,
    TYPE_CHR,
    TYPE_BOOL,
    TYPE_STR,
    TYPE_ZIL,

    TYPE_VEC,
    TYPE_MAP,
    TYPE_SET,
    TYPE_REF,
    TYPE_HEAP,
    TYPE_STACK,
    TYPE_QUE,
    TYPE_LINK,
    TYPE_TREE,
    TYPE_POD
} DataType;

typedef enum {
    OP_ADD,
    OP_SUB,
    OP_MUL,
    OP_DIV
} OperatorType;

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
