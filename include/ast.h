#ifndef W_AST_H
#define W_AST_H

#include "types.h"

typedef struct {
    int line;
    int column;
    const char* filename;
} SourceLocation;

// a function's parameter, or a pod's field
typedef struct Parameter {
    char* name;
    DataType type;
    int line;                  // where it's written, for errors about it
    bool is_ref;               // `b: ref Ball`: the function changes the caller's variable
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
            struct ASTNode* pods;       // NODE_POD_DECL list
        } program;
        struct {
            char* name;
            DataType type;              // the pod's type (user_types.h)
            Parameter* fields;          // name + type of each field, in order
            int field_count;
        } pod;
        struct {
            struct ASTNode* object;
            char* field;
        } field_access;
        struct {
            DataType return_type;
            char* name;
            Parameter* parameters;
            int param_count;
            struct ASTNode* body;
        } function;
        struct {
            char* target;             // the variable assigned, or the variable a field belongs to
            struct ASTNode* target_field;   // NODE_FIELD_ACCESS for `a.b = value`, else NULL
            bool target_is_ref;       // set by the checker: the target is a ref parameter
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
            struct ASTNode** args;   // ordinary expressions, printed separated by spaces
            int arg_count;
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
            char** arg_names;         // `name: value` arguments (NULL entries for plain ones), or NULL
            bool* arg_is_ref;         // `ref x` arguments (true entries), or NULL when there are none
            int arg_count;
            bool is_pod_literal;      // set by the checker: Name(field: value, ...) builds a pod
        } function_call;
        struct {
            long long value;   // as written; the checker reports it if it doesn't fit a num
        } number;
        struct {
            char* value;
        } string;
        struct {
            char* name;
            bool is_global;   // set by the checker: the name is a global variable
            bool is_ref;      // set by the checker: the name is a ref parameter (a pointer in C)
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
            bool shadows_outer;   // set by the checker: an enclosing scope has the same name
        } var_declaration;
    } data;
    struct ASTNode* next;
} ASTNode;

ASTNode* create_unary_expr_node(OperatorType operator, ASTNode* operand, SourceLocation loc);
ASTNode* create_function_call_node(char* name, ASTNode** args, int arg_count, SourceLocation loc);

ASTNode* create_program_node(SourceLocation loc);
ASTNode* create_function_node(DataType return_type, char* name, Parameter* parameters, int param_count, ASTNode* body, SourceLocation loc);
ASTNode* create_log_node(ASTNode** args, int arg_count, SourceLocation loc);
ASTNode* create_return_node(ASTNode* expression, SourceLocation loc);
ASTNode* create_assignment_node(char* name, ASTNode* value, SourceLocation loc);
ASTNode* create_binary_expr_node(ASTNode* left, ASTNode* right, OperatorType operator, SourceLocation loc);
ASTNode* create_number_node(long long value, SourceLocation loc);
ASTNode* create_if_node(ASTNode* condition, bool negated, ASTNode* body, SourceLocation loc);
ASTNode* create_loop_node(ASTNode* condition, bool negated, ASTNode* body, SourceLocation loc);
ASTNode* create_jump_node(NodeType type, SourceLocation loc);   // NODE_SKIP or NODE_STOP
ASTNode* create_string_node(char* value, SourceLocation loc);
ASTNode* create_variable_node(char* name, SourceLocation loc);
ASTNode* create_float_node(double value, SourceLocation loc);
ASTNode* create_char_node(char value, SourceLocation loc);
ASTNode* create_bool_node(bool value, SourceLocation loc);
ASTNode* create_var_declaration_node(char* name, DataType type, ASTNode* init_expr, SourceLocation loc);
// takes ownership of name and fields
ASTNode* create_pod_node(char* name, DataType type, Parameter* fields, int field_count, SourceLocation loc);
// takes ownership of object; copies field
ASTNode* create_field_access_node(ASTNode* object, const char* field, SourceLocation loc);

void free_parameters(Parameter* parameters);
void free_ast(ASTNode* node);

extern ASTNode* ast;

#endif