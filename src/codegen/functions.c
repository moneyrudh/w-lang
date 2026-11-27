#include <stdio.h>
#include <string.h>

#include "codegen/internal.h"
#include "codegen/c_syntax.h"
#include "language/type_registry.h"

// the program's globals, for main() to give str globals their first value
static ASTNode* program_globals = NULL;

void set_program_globals(ASTNode* globals) {
    program_globals = globals;
}

// the function being generated, for ret (its return type)
static ASTNode* current_function = NULL;

DataType current_return_type(void) {
    return current_function ? current_function->data.function.return_type : TYPE_ZIL;
}

static const char* c_return_type(ASTNode* function) {
    return get_c_type_from_enum(function->data.function.return_type);
}

void generate_function_declaration(FILE* output, ASTNode* node) {
    emit_function_declaration(output, c_return_type(node),
                              mangle_identifier(node->data.function.name, true),
                              node->data.function.parameters,
                              node->data.function.param_count);
}

void generate_function(FILE* output, ASTNode* node, int indent_level) {
    emit_function_signature(output, c_return_type(node),
                            mangle_identifier(node->data.function.name, true),
                            node->data.function.parameters,
                            node->data.function.param_count);

    current_function = node;
    strings_begin_function(output, node, indent_level + 1);
    if (strcmp(node->data.function.name, "w") == 0) {
        strings_emit_global_inits(output, program_globals, indent_level + 1);
    }
    generate_block(output, node->data.function.body, indent_level + 1);
    strings_close_scope(output, indent_level + 1);   // the end of a zil function's body
    strings_end_function();
    current_function = NULL;
    fprintf(output, C_RBRACE);
}
