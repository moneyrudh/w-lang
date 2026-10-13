#include <stdio.h>

#include "codegen/internal.h"
#include "codegen/c_syntax.h"
#include "language/type_registry.h"

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

    generate_block(output, node->data.function.body, indent_level + 1);
    fprintf(output, C_RBRACE);
}
