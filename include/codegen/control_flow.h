#ifndef W_CODEGEN_CONTROL_FLOW_H
#define W_CODEGEN_CONTROL_FLOW_H

#include <stdio.h>
#include "ast.h"

// NODE_IF chain -> if (...) { } else if (...) { } else { }
void generate_if(FILE* output, ASTNode* node, int indent_level);

// NODE_LOOP -> while (...) { }
void generate_loop(FILE* output, ASTNode* node, int indent_level);

// NODE_SKIP / NODE_STOP -> continue; / break;
void generate_loop_jump(FILE* output, ASTNode* node, int indent_level);

#endif
