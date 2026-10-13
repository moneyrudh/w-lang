#ifndef W_PARSER_H
#define W_PARSER_H

#include "ast.h"

// parses all of yyin into an untyped NODE_PROGRAM (NULL if parsing gave up).
// syntax only: names and types are resolved by the checker
ASTNode* parse(void);

#endif
