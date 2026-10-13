#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "codegen.h"
#include "codegen/internal.h"
#include "codegen/c_syntax.h"

static bool is_entry_point(ASTNode* function) {
    return strcmp(function->data.function.name, "w") == 0;
}

static ASTNode* find_entry_point(ASTNode* functions) {
    for (ASTNode* current = functions; current != NULL; current = current->next) {
        if (current->type == NODE_FUNCTION && is_entry_point(current)) {
            return current;
        }
    }
    return NULL;
}

// includes, globals, forward declarations, functions, then w() as main()
static void generate_program(FILE* output, ASTNode* node, int indent_level) {
    // the checker has verified that w() exists
    ASTNode* entry_point = find_entry_point(node->data.program.functions);

    emit_c_includes(output);

    // emit global variables
    if (node->data.program.globals) {
        for (ASTNode* global = node->data.program.globals; global; global = global->next) {
            generate(output, global, 0);
            fprintf(output, C_NEWLINE);
        }
        fprintf(output, C_NEWLINE);
    }

    // emit forward declarations for all non-w functions
    for (ASTNode* function = node->data.program.functions; function != NULL; function = function->next) {
        if (!is_entry_point(function)) {
            generate_function_declaration(output, function);
        }
    }
    fprintf(output, C_NEWLINE);

    // emit definitions for all non-w functions
    for (ASTNode* function = node->data.program.functions; function != NULL; function = function->next) {
        if (!is_entry_point(function)) {
            generate_function(output, function, indent_level);
            fprintf(output, C_NEWLINE);
        }
    }

    // emit w() function definition last (becomes main)
    generate_function(output, entry_point, indent_level);
    fprintf(output, C_NEWLINE);
}

void generate_block(FILE* output, ASTNode* statements, int indent_level) {
    for (ASTNode* statement = statements; statement != NULL; statement = statement->next) {
        generate(output, statement, indent_level);
    }
}

void generate(FILE* output, ASTNode* node, int indent_level) {
    if (!node) return;

    switch (node->type) {
        case NODE_PROGRAM:
            generate_program(output, node, indent_level);
            break;
        case NODE_FUNCTION:
            generate_function(output, node, indent_level);
            break;
        case NODE_LOG:
            generate_log_statement(output, node->data.log.elements, indent_level);
            break;
        case NODE_VAR_DECLARATION:
            generate_var_declaration(output, node, indent_level);
            break;
        case NODE_ASSIGNMENT:
            generate_assignment(output, node, indent_level);
            break;
        case NODE_RETURN:
            generate_return(output, node, indent_level);
            break;
        case NODE_IF:
            generate_if(output, node, indent_level);
            break;
        case NODE_LOOP:
            generate_loop(output, node, indent_level);
            break;
        case NODE_SKIP:
        case NODE_STOP:
            generate_loop_jump(output, node, indent_level);
            break;
        case NODE_BINARY_EXPR:
            generate_binary_expr(output, node, indent_level);
            break;
        case NODE_UNARY_EXPR:
            generate_unary_expr(output, node, indent_level);
            break;
        case NODE_FUNCTION_CALL:
            generate_function_call(output, node, indent_level);
            break;
        case NODE_VARIABLE:
            generate_variable(output, node);
            break;
        case NODE_NUMBER:
        case NODE_STRING:
        case NODE_FLOAT:
        case NODE_CHAR:
        case NODE_BOOL:
            generate_literal(output, node);
            break;
        default:
            fprintf(stderr, "Unknown node type in AST\n");
            exit(1);
    }
}

void generate_code(FILE* output, ASTNode* node) {
    if (!node) return;

    if (node->type != NODE_PROGRAM) {
        fprintf(stderr, "Expected program node at root.\n");
        return;
    }

    generate(output, node, 0);
}
