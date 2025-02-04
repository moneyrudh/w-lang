#ifndef W_PARSER_H
#define W_PARSER_H

#include "types.h"
#include "ast.h"

void init_parser_state();
void init_parser();
void cleanup_parser_state();
void cleanup_parser();

void parser_error(const char* message);

void enter_block();
void exit_block();

void eat(TokenType _token);

ASTNode* parse(void);
ASTNode* parser_factor(void);
ASTNode* parse_term(void);
ASTNode* parse_expression(void);
ASTNode* parse_variable_declaration(void);
ASTNode* parse_log(void);
ASTNode* parse_return_statement(void);
ASTNode* parse_statement(void);
ASTNode* parse_function(void);

const char* token_to_string(TokenType token);
const char* type_to_string(DataType type);

extern ParserState parser_state;

ParserState getParserState(void);

#endif