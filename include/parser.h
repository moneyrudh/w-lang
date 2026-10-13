#ifndef W_PARSER_H
#define W_PARSER_H

#include "types.h"
#include "ast.h"

void init_parser(void);
void cleanup_parser(void);

// parses all of yyin into a NODE_PROGRAM (NULL if parsing gave up)
ASTNode* parse(void);

// reports an error on the current token's line and counts it
void parser_error(const char* message);

extern ParserState parser_state;

ParserState getParserState(void);

#endif
