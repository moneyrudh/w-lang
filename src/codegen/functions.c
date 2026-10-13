#include <stdio.h>

#include "codegen/internal.h"
#include "codegen/c_syntax.h"
#include "language/type_registry.h"

// W Lang return type name -> C type
static const char* c_return_type(ASTNode* function) {
    const TypeMapping* mapping = type_registry_get_by_wlang_name(function->data.function.return_type);
    return mapping ? mapping->c_equivalent : function->data.function.return_type;
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
