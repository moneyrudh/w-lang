#ifndef W_AST_H
#define W_AST_H

#include "types.h"

typedef struct {
    int line;
    int column;
    const char* filename;
} SourceLocation;

typedef struct Parameter {
    char* name;
    DataType type;
    struct Parameter* next;
} Parameter;

typedef struct ASTNode {
    NodeType type;
    SourceLocation location;   // line the construct starts on
    DataType expr_type;        // expressions only: filled in by the checker
    union {
        struct {
            struct ASTNode* functions;
            struct ASTNode* globals;
        } program;
        struct {
            DataType return_type;
            char* name;
            Parameter* parameters;
            int param_count;
            struct ASTNode* body;
        } function;
        struct {
            char* target;
            DataType target_type;     // filled in by the checker
            struct ASTNode* value;
        } assignment;
        struct {
            struct ASTNode* expression;
        } return_statement;
        struct {
            struct ASTNode* condition;    // NULL for a plain 'else'
            bool negated;                 // lest / elf lest / nor
            struct ASTNode* body;
            struct ASTNode* else_branch;  // next elf / nor / else in the chain
        } if_statement;
        struct {
            struct ASTNode* condition;    // NULL for 'loop forever'
            bool negated;                 // loop until
            struct ASTNode* body;
        } loop_statement;
        struct {
            LogElement* elements;
        } log;
        struct {
            struct ASTNode* left;
            struct ASTNode* right;
            OperatorType operator;
        } binary_expr;
        struct {
            struct ASTNode* operand;
            OperatorType operator;
        } unary_expr;
        struct {
            char* name;
            struct ASTNode** args;    // array of argument expressions
            int arg_count;
        } function_call;
        struct {
            int value;
        } number;
        struct {
            char* value;
        } string;
        struct {
            char* name;
        } variable;
        struct {
            double value;
        } float_val;
        struct {
            char value;
        } char_val;
        struct {
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

ASTNode* create_unary_expr_node(OperatorType operator, ASTNode* operand, SourceLocation loc);
ASTNode* create_function_call_node(char* name, ASTNode** args, int arg_count, SourceLocation loc);

ASTNode* create_program_node(SourceLocation loc);
ASTNode* create_function_node(DataType return_type, char* name, Parameter* parameters, int param_count, ASTNode* body, SourceLocation loc);
ASTNode* create_log_node(LogElement* elements, SourceLocation loc);
ASTNode* create_return_node(ASTNode* expression, SourceLocation loc);
ASTNode* create_assignment_node(char* name, ASTNode* value, SourceLocation loc);
ASTNode* create_binary_expr_node(ASTNode* left, ASTNode* right, OperatorType operator, SourceLocation loc);
ASTNode* create_number_node(int value, SourceLocation loc);
ASTNode* create_if_node(ASTNode* condition, bool negated, ASTNode* body, SourceLocation loc);
ASTNode* create_loop_node(ASTNode* condition, bool negated, ASTNode* body, SourceLocation loc);
ASTNode* create_jump_node(NodeType type, SourceLocation loc);   // NODE_SKIP or NODE_STOP
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