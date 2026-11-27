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
    bool is_entry_point = strcmp(node->data.function.name, "w") == 0;

    // this call's frame, for traces: W__push("name", W__line); main() has no caller
    emit_indent(output, indent_level + 1);
    fprintf(output, C_PUSH_FRAME C_LPAREN C_STRING_QUOTE "%s" C_STRING_QUOTE C_COMMA "%s" C_RPAREN C_SEMICOLON_NL,
            node->data.function.name, is_entry_point ? C_ZERO : C_CALL_LINE);

    strings_begin_function(output, node, indent_level + 1);
    if (is_entry_point) {
        strings_emit_global_inits(output, program_globals, indent_level + 1);
    }
    generate_block(output, node->data.function.body, indent_level + 1);
    if (node->data.function.return_type == TYPE_ZIL) {
        strings_close_scope(output, indent_level + 1);   // falling off the end of the body
        emit_indent(output, indent_level + 1);
        fprintf(output, C_POP_FRAME_NL);
    } else {
        // the checker made sure every path returns, so this is never reached. Saying so keeps
        // C compilers that can't follow every path (gcc with sanitizers) from warning
        emit_indent(output, indent_level + 1);
        fprintf(output, C_UNREACHABLE "\n");
        strings_discard_scope();
    }
    strings_end_function();
    current_function = NULL;
    fprintf(output, C_RBRACE);
}
