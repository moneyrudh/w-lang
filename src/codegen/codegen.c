#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "codegen.h"
#include "codegen/internal.h"
#include "codegen/c_syntax.h"
#include "language/type_registry.h"

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
    emit_arithmetic_runtime(output);
    if (program_uses_str(node)) {
        emit_string_runtime(output);
    }
    emit_pod_types(output, node->data.program.pods);
    emit_pod_printers(output, node->data.program.pods);

    // emit global variables (a str global starts as NULL; main() copies its value in first)
    set_program_globals(node->data.program.globals);
    set_in_global_value(true);
    if (node->data.program.globals) {
        for (ASTNode* global = node->data.program.globals; global; global = global->next) {
            if (global->data.var_declaration.type == TYPE_STR) {
                fprintf(output, "%s %s" C_ASSIGN C_NULL C_SEMICOLON_NL,
                        get_c_type_from_enum(TYPE_STR), mangle_identifier(global->data.var_declaration.name, false));
            } else {
                generate(output, global, 0);
            }
        }
        fprintf(output, C_NEWLINE);
    }
    set_in_global_value(false);

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
        // temporaries live until the end of the statement that made them (ret frees its own)
        if (statement->type != NODE_RETURN && statement_makes_temps(statement)) {
            strings_free_temps(output, indent_level);
        }
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
            generate_log_statement(output, node, indent_level);
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
        case NODE_FIELD_ACCESS:
            generate_field_access(output, node);
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
