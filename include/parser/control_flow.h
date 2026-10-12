#ifndef W_PARSER_CONTROL_FLOW_H
#define W_PARSER_CONTROL_FLOW_H

#include "ast.h"

// say / when / lest  cond  body  { elf [lest] cond body | nor cond body }  [ else body ]
ASTNode* parse_conditional(void);

// 'elf', 'nor' or 'else' with no conditional before it: reports, parses, discards
ASTNode* parse_stray_branch(void);

// loop while cond body | loop until cond body | loop forever body
ASTNode* parse_loop(void);

// skip / stop
ASTNode* parse_loop_jump(void);

#endif
