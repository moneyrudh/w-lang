#ifndef AST_H
#define AST_H

#include "types.h"

typedef struct ASTNode {
    NodeType type;
    union {
        struct {
            struct ASTNode* functions;
            struct ASTNode* globals;
        } program;
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

ASTNode* create_program_node();
ASTNode* create_function_node(char* return_type, char* name, ASTNode* body, int has_return);
ASTNode* create_log_node(LogElement* elements);
ASTNode* create_binary_expr_node(ASTNode* left, ASTNode* right, char operator);
ASTNode* create_number_node(int value);
ASTNode* create_string_node(char* value);
ASTNode* create_variable_node(char* name);

void free_log_elements(LogElement* elements);
void free_ast(ASTNode* node);

extern ASTNode* ast;

#endif