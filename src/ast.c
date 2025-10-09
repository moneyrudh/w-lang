#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "ast.h"
#include "parser.h"
#include "symbol_table.h"

ASTNode* ast = NULL;

void set_node_location(ASTNode* node, SourceLocation loc) {
    if (node) {
        node->location = loc;
    }
}

static void init_expression(Expression* expr, NodeType type, SourceLocation loc) {
    expr->type = type;
    expr->location = loc;
    expr->expr_type = TYPE_ZIL;
}

ASTNode* create_program_node(SourceLocation loc) {
    ASTNode* node = malloc(sizeof(ASTNode));
    if (!node) {
        parser_error("Memory allocation failed");
        return NULL;
    }
    node->type = NODE_PROGRAM;
    set_node_location(node, loc);
    node->data.program.functions = NULL;
    node->data.program.globals = NULL;
    node->next = NULL;
    return node;
}

ASTNode* create_function_node(char* return_type, char* name, Parameter* parameters, int param_count, ASTNode* body, int has_return, SourceLocation loc) {
    ASTNode* node = malloc(sizeof(ASTNode));
    if (!node) {
        parser_error("Memory allocation failed");
        return NULL;
    }
    node->type = NODE_FUNCTION;
    set_node_location(node, loc);
    node->data.function.return_type = return_type;
    node->data.function.name = name;
    node->data.function.parameters = parameters;
    node->data.function.param_count = param_count;
    node->data.function.body = body;
    node->data.function.has_return = has_return;
    node->next = NULL;
    return node;
}

ASTNode* create_function_call_node(char* name, ASTNode** args, int arg_count, SourceLocation loc) {
    ASTNode* node = malloc(sizeof(ASTNode));
    if (!node) {
        parser_error("Memory allocation failed");
        return NULL;
    }
    node->type = NODE_FUNCTION_CALL;
    init_expression(&node->data.function_call.base, NODE_FUNCTION_CALL, loc);
    node->data.function_call.name = strdup(name);
    node->data.function_call.args = malloc(sizeof(ASTNode*) * arg_count);
    if (!node->data.function_call.args) {
        free(node->data.function_call.name);
        free(node);
        parser_error("Memory allocation failed for function arguments");
        return NULL;
    }
    memcpy(node->data.function_call.args, args, sizeof(ASTNode*) * arg_count);
    node->data.function_call.arg_count = arg_count;
    node->next = NULL;
    return node;
}

ASTNode* create_log_node(LogElement* elements) {
    ASTNode* node = malloc(sizeof(ASTNode));
    if (!node) {
        parser_error("Memory allocation failed");
        return NULL;
    }
    node->type = NODE_LOG;
    node->data.log.elements = elements;
    return node;
}

ASTNode* create_assignment_node(char* target, ASTNode* value, SourceLocation loc) {
    // First, validate our inputs
    if (!target || !value) {
        parser_error("Invalid assignment: missing target or value");
        return NULL;
    }

    // Create and initialize the node
    ASTNode* node = malloc(sizeof(ASTNode));
    if (!node) {
        parser_error("Memory allocation failed for assignment node");
        return NULL;
    }

    // Set up the node's basic properties
    node->type = NODE_ASSIGNMENT;
    node->location = loc;
    
    // Initialize the base expression
    init_expression(&node->data.assignment.base, NODE_ASSIGNMENT, loc);

    // Set up the assignment-specific data
    node->data.assignment.target = strdup(target);  // Make a copy of the target name
    if (!node->data.assignment.target) {
        parser_error("Memory allocation failed for assignment target");
        free(node);
        return NULL;
    }
    
    node->data.assignment.value = value;
    node->next = NULL;

    // The type of an assignment expression is the type of its value
    node->data.assignment.base.expr_type = get_expression_type(value, getSymbolTable());

    // Verify that the target exists in the symbol table
    Symbol* symbol = lookup_symbol(getSymbolTable(), target);
    if (!symbol) {
        char error_msg[100];
        snprintf(error_msg, sizeof(error_msg), 
                "Assignment to undeclared variable '%s'", target);
        parser_error(error_msg);
        free(node->data.assignment.target);
        free(node);
        return NULL;
    }

    // Check type compatibility
    if (!compare_types(symbol->type, node->data.assignment.base.expr_type)) {
        char error_msg[100];
        snprintf(error_msg, sizeof(error_msg),
                "Type mismatch in assignment to '%s': cannot assign %s to %s",
                target,
                type_to_string(node->data.assignment.base.expr_type),
                type_to_string(symbol->type));
        parser_error(error_msg);
        free(node->data.assignment.target);
        free(node);
        return NULL;
    }

    return node;
}


ASTNode* create_binary_expr_node(ASTNode* left, ASTNode* right, char operator, SourceLocation loc) {
    ASTNode* node = malloc(sizeof(ASTNode));
    if (!node) {
        parser_error("Memory allocation failed");
        return NULL;
    }
    node->type = NODE_BINARY_EXPR;
    init_expression(&node->data.binary_expr.base, NODE_BINARY_EXPR, loc);
    node->data.binary_expr.left = left;
    node->data.binary_expr.right = right;
    node->data.binary_expr.operator = operator;
    node->next = NULL;
    return node;
}

ASTNode* create_unary_expr_node(char operator, ASTNode* operand, SourceLocation loc) {
    ASTNode* node = malloc(sizeof(ASTNode));
    if (!node) {
        parser_error("Memory allocation failed");
        return NULL;
    }
    node->type = NODE_UNARY_EXPR;
    init_expression(&node->data.unary_expr.base, NODE_UNARY_EXPR, loc);
    node->data.unary_expr.operator = operator;
    node->data.unary_expr.operand = operand;
    node->next = NULL;
    return node;
}

ASTNode* create_number_node(int value, SourceLocation loc) {
    ASTNode* node = malloc(sizeof(ASTNode));
    if (!node) {
        parser_error("Memory allocation failed");
        return NULL;
    }
    node->type = NODE_NUMBER;
    init_expression(&node->data.number.base, NODE_NUMBER, loc);
    node->data.number.base.expr_type = TYPE_NUM;  // Numbers are always int type
    node->data.number.value = value;
    node->next = NULL;
    return node;
}

ASTNode* create_string_node(char* value, SourceLocation loc) {
    ASTNode* node = malloc(sizeof(ASTNode));
    if (!node) {
        parser_error("Memory allocation failed");
        return NULL;
    }
    node->type = NODE_STRING;
    init_expression(&node->data.string.base, NODE_STRING, loc);
    node->data.string.base.expr_type = TYPE_STR;
    node->data.string.value = strdup(value);
    node->next = NULL;
    return node;
}

ASTNode* create_float_node(double value, SourceLocation loc) {
    ASTNode* node = malloc(sizeof(ASTNode));
    if (!node) {
        parser_error("Memory allocation failed for float node");
        return NULL;
    }
    
    node->type = NODE_FLOAT;
    node->location = loc;
    
    // Initialize the base expression fields
    init_expression(&node->data.float_val.base, NODE_FLOAT, loc);
    node->data.float_val.base.expr_type = TYPE_REAL;
    
    // Set the value
    node->data.float_val.value = value;
    node->next = NULL;
    
    return node;
}

ASTNode* create_char_node(char value, SourceLocation loc) {
    ASTNode* node = malloc(sizeof(ASTNode));
    if (!node) {
        parser_error("Memory allocation failed for char node");
        return NULL;
    }
    
    node->type = NODE_CHAR;
    node->location = loc;
    
    // Initialize the base expression fields
    init_expression(&node->data.char_val.base, NODE_CHAR, loc);
    node->data.char_val.base.expr_type = TYPE_CHR;
    
    // Set the value
    node->data.char_val.value = value;
    node->next = NULL;
    
    return node;
}

ASTNode* create_bool_node(bool value, SourceLocation loc) {
    ASTNode* node = malloc(sizeof(ASTNode));
    if (!node) {
        parser_error("Memory allocation failed for bool node");
        return NULL;
    }
    
    node->type = NODE_BOOL;
    node->location = loc;
    
    // Initialize the base expression fields
    init_expression(&node->data.bool_val.base, NODE_BOOL, loc);
    node->data.bool_val.base.expr_type = TYPE_BOOL;
    
    // Set the value
    node->data.bool_val.value = value;
    node->next = NULL;
    
    return node;
}

ASTNode* create_variable_node(char* name, SourceLocation loc) {
    ASTNode* node = malloc(sizeof(ASTNode));
    if (!node) {
        parser_error("Memory allocation failed");
        return NULL;
    }
    node->type = NODE_VARIABLE;
    init_expression(&node->data.variable.base, NODE_VARIABLE, loc);
    node->data.variable.name = strdup(name);
    node->next = NULL;
    return node;
}

ASTNode* create_var_declaration_node(char* name, DataType type, ASTNode* init_expr, SourceLocation loc) {
    ASTNode* node = malloc(sizeof(ASTNode));
    if (!node) {
        parser_error("Memory allocation failed");
        return NULL;
    }
    node->type = NODE_VAR_DECLARATION;
    set_node_location(node, loc);
    node->data.var_declaration.name = strdup(name);
    node->data.var_declaration.type = type;
    node->data.var_declaration.init_expr = init_expr;
    node->next = NULL;
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
            case NODE_FUNCTION: {
                free(node->data.function.return_type);
                free(node->data.function.name);
                // Free parameters
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
            case NODE_NUMBER:
            case NODE_FLOAT:
            case NODE_CHAR:
            case NODE_BOOL:
                break;
        }
        if (node->next) free(node->next);
        free(node);
    }
}
