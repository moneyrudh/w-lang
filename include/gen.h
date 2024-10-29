#ifndef W_GEN_H
#define W_GEN_H

#include <stdio.h>
#include <unistd.h>

#include "ast.h"
#include "types.h"

void generate(FILE* output, ASTNode* node, int indent_level);
void generate_code(FILE* output, ASTNode* node);
void generate_log_statement(FILE* output, LogElement* elements, int indent_level);

#endif