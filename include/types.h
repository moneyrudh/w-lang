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

    // control flow
    SAY,
    WHEN,
    LEST,
    ELF,
    NOR,
    ELSE,
    LOOP,
    WHILE,
    UNTIL,
    FOREVER,
    SKIP,
    STOP,

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

    // layout tokens produced from newlines and indentation
    NEWLINE,
    INDENT,
    DEDENT,

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

typedef enum {
    TYPE_NUM,
    TYPE_REAL,
    TYPE_CHR,
    TYPE_BOOL,
    TYPE_STR,
    TYPE_ZIL,
    TYPE_ERROR,     // internal: an error was already reported for this expression

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
    OP_DIV,

    OP_EQ,
    OP_NE,
    OP_LT,
    OP_GT,
    OP_LE,
    OP_GE,

    OP_AND,
    OP_OR,
    OP_NOT
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
    NODE_RETURN,
    NODE_IF,
    NODE_LOOP,
    NODE_SKIP,
    NODE_STOP
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
    DataType var_type;   // for NODE_VARIABLE elements, filled in by the checker
    struct LogElement* next;
} LogElement;

#endif
