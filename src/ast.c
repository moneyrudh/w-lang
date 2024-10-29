#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "ast.h"
#include "parser.h"

ASTNode* ast = NULL;

ASTNode* create_program_node() {
    ASTNode* node = malloc(sizeof(ASTNode));
    if (!node) {
        parser_error("Memory allocation failed");
        return NULL;
    }
    node->type = NODE_PROGRAM;
    node->data.program.functions = NULL;
    node->data.program.globals = NULL;
    node->next = NULL;
    return node;
}

ASTNode* create_function_node(char* return_type, char* name, ASTNode* body, int has_return) {
    ASTNode* node = malloc(sizeof(ASTNode));
    node->type = NODE_FUNCTION;
    node->data.function.return_type = return_type;
    node->data.function.name = name;
    node->data.function.body = body;
    node->data.function.has_return = has_return;
    return node;
}

ASTNode* create_log_node(LogElement* elements) {
    ASTNode* node = malloc(sizeof(ASTNode));
    node->type = NODE_LOG;
    node->data.log.elements = elements;
    return node;
}

ASTNode* create_binary_expr_node(ASTNode* left, ASTNode* right, char operator) {
    ASTNode* node = malloc(sizeof(ASTNode));
    node->type = NODE_BINARY_EXPR;
    node->data.binary_expr.left = left;
    node->data.binary_expr.right = right;
    node->data.binary_expr.operator = operator;
    return node;
}

ASTNode* create_number_node(int value) {
    ASTNode* node = malloc(sizeof(ASTNode));
    node->type = NODE_NUMBER;
    node->data.number.value = value;
    return node;
}

ASTNode* create_string_node(char* value) {
    ASTNode* node = malloc(sizeof(ASTNode));
    node->type = NODE_STRING;
    node->data.string.value = strdup(value);
    return node;
}

ASTNode* create_variable_node(char* name) {
    ASTNode* node = malloc(sizeof(ASTNode));
    node->type = NODE_VARIABLE;
    node->data.variable.name = strdup(name);
    return node;
}

void free_log_elements(LogElement* elements) {
    LogElement* current = elements;
    while (current != NULL) {
        LogElement* next = current->next;
        if (current->type == NODE_STRING) {
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
            case NODE_FUNCTION:
                free(node->data.function.return_type);
                free(node->data.function.name);
                free_ast(node->data.function.body);
                break;
            case NODE_LOG:
                free_log_elements(node->data.log.elements);
                break;
            case NODE_BINARY_EXPR:
                free_ast(node->data.binary_expr.left);
                free_ast(node->data.binary_expr.right);
                break;
            case NODE_STRING:
                free(node->data.string.value);
                break;
            case NODE_VARIABLE:
                free(node->data.variable.name);
                break;
            case NODE_RETURN:
                free_ast(node->data.return_statement.expression);
                break;
            case NODE_NUMBER:
                break;
        }
        if (node->next) free(node->next);
        free(node);
    }
}
