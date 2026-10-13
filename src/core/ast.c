#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "ast.h"

// Constructors only build nodes: names are resolved and types filled in by the checker.

ASTNode* ast = NULL;

// a node with every common field set; the caller fills in node->data
static ASTNode* new_node(NodeType type, SourceLocation loc) {
    ASTNode* node = malloc(sizeof(ASTNode));
    if (!node) {
        fprintf(stderr, "Out of memory\n");
        exit(1);
    }
    node->type = type;
    node->location = loc;
    node->expr_type = TYPE_ERROR;
    node->next = NULL;
    return node;
}

ASTNode* create_program_node(SourceLocation loc) {
    ASTNode* node = new_node(NODE_PROGRAM, loc);
    node->data.program.functions = NULL;
    node->data.program.globals = NULL;
    return node;
}

// takes ownership of name and parameters
ASTNode* create_function_node(DataType return_type, char* name, Parameter* parameters, int param_count, ASTNode* body, SourceLocation loc) {
    ASTNode* node = new_node(NODE_FUNCTION, loc);
    node->data.function.return_type = return_type;
    node->data.function.name = name;
    node->data.function.parameters = parameters;
    node->data.function.param_count = param_count;
    node->data.function.body = body;
    return node;
}

// copies name and the args array (the argument nodes themselves are owned by the call)
ASTNode* create_function_call_node(char* name, ASTNode** args, int arg_count, SourceLocation loc) {
    ASTNode* node = new_node(NODE_FUNCTION_CALL, loc);
    node->data.function_call.name = strdup(name);
    node->data.function_call.args = NULL;
    if (arg_count > 0) {
        node->data.function_call.args = malloc(sizeof(ASTNode*) * arg_count);
        memcpy(node->data.function_call.args, args, sizeof(ASTNode*) * arg_count);
    }
    node->data.function_call.arg_count = arg_count;
    return node;
}

ASTNode* create_log_node(LogElement* elements, SourceLocation loc) {
    ASTNode* node = new_node(NODE_LOG, loc);
    node->data.log.elements = elements;
    return node;
}

ASTNode* create_return_node(ASTNode* expression, SourceLocation loc) {
    ASTNode* node = new_node(NODE_RETURN, loc);
    node->data.return_statement.expression = expression;
    return node;
}

ASTNode* create_assignment_node(char* target, ASTNode* value, SourceLocation loc) {
    ASTNode* node = new_node(NODE_ASSIGNMENT, loc);
    node->data.assignment.target = strdup(target);
    node->data.assignment.target_type = TYPE_ERROR;
    node->data.assignment.value = value;
    return node;
}

ASTNode* create_binary_expr_node(ASTNode* left, ASTNode* right, OperatorType operator, SourceLocation loc) {
    ASTNode* node = new_node(NODE_BINARY_EXPR, loc);
    node->data.binary_expr.left = left;
    node->data.binary_expr.right = right;
    node->data.binary_expr.operator = operator;
    return node;
}

ASTNode* create_unary_expr_node(OperatorType operator, ASTNode* operand, SourceLocation loc) {
    ASTNode* node = new_node(NODE_UNARY_EXPR, loc);
    node->data.unary_expr.operator = operator;
    node->data.unary_expr.operand = operand;
    return node;
}

ASTNode* create_if_node(ASTNode* condition, bool negated, ASTNode* body, SourceLocation loc) {
    ASTNode* node = new_node(NODE_IF, loc);
    node->data.if_statement.condition = condition;
    node->data.if_statement.negated = negated;
    node->data.if_statement.body = body;
    node->data.if_statement.else_branch = NULL;
    return node;
}

ASTNode* create_loop_node(ASTNode* condition, bool negated, ASTNode* body, SourceLocation loc) {
    ASTNode* node = new_node(NODE_LOOP, loc);
    node->data.loop_statement.condition = condition;
    node->data.loop_statement.negated = negated;
    node->data.loop_statement.body = body;
    return node;
}

ASTNode* create_jump_node(NodeType type, SourceLocation loc) {
    return new_node(type, loc);
}

ASTNode* create_number_node(int value, SourceLocation loc) {
    ASTNode* node = new_node(NODE_NUMBER, loc);
    node->data.number.value = value;
    return node;
}

ASTNode* create_string_node(char* value, SourceLocation loc) {
    ASTNode* node = new_node(NODE_STRING, loc);
    node->data.string.value = strdup(value);
    return node;
}

ASTNode* create_float_node(double value, SourceLocation loc) {
    ASTNode* node = new_node(NODE_FLOAT, loc);
    node->data.float_val.value = value;
    return node;
}

ASTNode* create_char_node(char value, SourceLocation loc) {
    ASTNode* node = new_node(NODE_CHAR, loc);
    node->data.char_val.value = value;
    return node;
}

ASTNode* create_bool_node(bool value, SourceLocation loc) {
    ASTNode* node = new_node(NODE_BOOL, loc);
    node->data.bool_val.value = value;
    return node;
}

ASTNode* create_variable_node(char* name, SourceLocation loc) {
    ASTNode* node = new_node(NODE_VARIABLE, loc);
    node->data.variable.name = strdup(name);
    return node;
}

ASTNode* create_var_declaration_node(char* name, DataType type, ASTNode* init_expr, SourceLocation loc) {
    ASTNode* node = new_node(NODE_VAR_DECLARATION, loc);
    node->data.var_declaration.name = strdup(name);
    node->data.var_declaration.type = type;
    node->data.var_declaration.init_expr = init_expr;
    return node;
}

void free_log_elements(LogElement* elements) {
    LogElement* current = elements;
    while (current != NULL) {
        LogElement* next = current->next;
        if (current->type == NODE_STRING || current->type == NODE_VARIABLE) {
            free(current->value.string);
        }
        free(current);
        current = next;
    }
}

void free_ast(ASTNode* node) {
    if (node) {
        switch (node->type) {
            case NODE_PROGRAM:
                free_ast(node->data.program.functions);
                free_ast(node->data.program.globals);
                break;
            case NODE_FUNCTION: {
                free(node->data.function.name);
                // free parameters
                Parameter* param = node->data.function.parameters;
                while (param != NULL) {
                    Parameter* next = param->next;
                    free(param->name);
                    free(param);
                    param = next;
                }
                free_ast(node->data.function.body);
                break;
            }
            case NODE_FUNCTION_CALL:
                free(node->data.function_call.name);
                if (node->data.function_call.args) {
                    for (int i = 0; i < node->data.function_call.arg_count; i++) {
                        free_ast(node->data.function_call.args[i]);
                    }
                    free(node->data.function_call.args);
                }
                break;
            case NODE_LOG:
                free_log_elements(node->data.log.elements);
                break;
            case NODE_BINARY_EXPR:
                free_ast(node->data.binary_expr.left);
                free_ast(node->data.binary_expr.right);
                break;
            case NODE_UNARY_EXPR:
                free_ast(node->data.unary_expr.operand);
                break;
            case NODE_STRING:
                free(node->data.string.value);
                break;
            case NODE_VARIABLE:
                free(node->data.variable.name);
                break;
            case NODE_VAR_DECLARATION:
                free(node->data.var_declaration.name);
                free_ast(node->data.var_declaration.init_expr);
                break;
            case NODE_ASSIGNMENT:
                free(node->data.assignment.target);
                free_ast(node->data.assignment.value);
                break;
            case NODE_RETURN:
                free_ast(node->data.return_statement.expression);
                break;
            case NODE_IF:
                free_ast(node->data.if_statement.condition);
                free_ast(node->data.if_statement.body);
                free_ast(node->data.if_statement.else_branch);
                break;
            case NODE_LOOP:
                free_ast(node->data.loop_statement.condition);
                free_ast(node->data.loop_statement.body);
                break;
            case NODE_SKIP:
            case NODE_STOP:
                break;
            case NODE_NUMBER:
            case NODE_FLOAT:
            case NODE_CHAR:
            case NODE_BOOL:
                break;
        }
        free_ast(node->next);
        free(node);
    }
}
