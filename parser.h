#ifndef W_PARSER_H
#define W_PARSER_H

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

typedef enum {
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

typedef struct ASTNode {
    NodeType type;
    union {
        struct {
            char* return_type;
            char* name;
            struct ASTNode* body;
            int has_return;
        } function;
        struct {
            struct ASTNode* expression;
        } return_statement;
        struct {
            LogElement* elements;
        } log;
        struct {
            struct ASTNode* left;
            struct ASTNode* right;
            char operator;
        } binary_expr;
        struct {
            int value;
        } number;
        struct {
            char* value;
        } string;
        struct {
            char* name;
        } variable;
    } data;
    struct ASTNode* next;
} ASTNode;

extern int yylex();
extern char* yytext;
extern FILE* yyin;

#endif