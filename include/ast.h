#ifndef AST_H
#define AST_H

#include "types.h"

typedef struct {
    int line;
    int column;
    const char* filename;
} SourceLocation;

typedef struct {
    NodeType type;
    SourceLocation location;
    DataType expr_type;
} Expression;

typedef struct ASTNode {
    NodeType type;
    SourceLocation location;
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
            Expression base;
            char* target;
            struct ASTNode* value;
        } assignment;
        struct {
            struct ASTNode* expression;
        } return_statement;
        struct {
            LogElement* elements;
        } log;
        struct {
            Expression base;
            struct ASTNode* left;
            struct ASTNode* right;
            char operator;
        } binary_expr;
        struct {
            Expression base;
            struct ASTNode* operand;
            char operator;
        } unary_expr;
        struct {
            Expression base;
            char* name;
            struct ASTNode** args;    // Array of argument expressions
            int arg_count;
        } function_call;
        struct {
            Expression base;
            int value;
        } number;
        struct {
            Expression base;
            char* value;
        } string;
        struct {
            Expression base;
            char* name;
        } variable;
        struct {
            Expression base;
            double value;
        } float_val;
        struct {
            Expression base;
            char value;
        } char_val;
        struct {
            Expression base;
            bool value;
        } bool_val;
        struct {
            char* name;
            DataType type;
            struct ASTNode* init_expr;
        } var_declaration;
    } data;
    struct ASTNode* next;
} ASTNode;

ASTNode* create_unary_expr_node(char operator, ASTNode* operand, SourceLocation loc);
ASTNode* create_function_call_node(char* name, ASTNode** args, int arg_count, SourceLocation loc);

void set_node_location(ASTNode* node, SourceLocation loc);

ASTNode* create_program_node(SourceLocation loc);
ASTNode* create_function_node(char* return_type, char* name, ASTNode* body, int has_return, SourceLocation loc);
ASTNode* create_log_node(LogElement* elements);
ASTNode* create_assignment_node(char* name, ASTNode* value, SourceLocation loc);
ASTNode* create_binary_expr_node(ASTNode* left, ASTNode* right, char operator, SourceLocation loc);
ASTNode* create_number_node(int value, SourceLocation loc);
ASTNode* create_string_node(char* value, SourceLocation loc);
ASTNode* create_variable_node(char* name, SourceLocation loc);
ASTNode* create_float_node(double value, SourceLocation loc);
ASTNode* create_char_node(char value, SourceLocation loc);
ASTNode* create_bool_node(bool value, SourceLocation loc);
ASTNode* create_var_declaration_node(char* name, DataType type, ASTNode* init_expr, SourceLocation loc);

void free_log_elements(LogElement* elements);
void free_ast(ASTNode* node);

extern ASTNode* ast;

#endif