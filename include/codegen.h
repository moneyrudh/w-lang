#ifndef W_CODEGEN_H
#define W_CODEGEN_H

#include <stdio.h>

#include "ast.h"

// writes the C translation of a NODE_PROGRAM to output
void generate_code(FILE* output, ASTNode* program);

#endif
