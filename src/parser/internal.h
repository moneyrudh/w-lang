#ifndef W_PARSER_INTERNAL_H
#define W_PARSER_INTERNAL_H

// shared between the parser's files only; the public API is include/parser.h

#include "types.h"
#include "ast.h"

// current lookahead token
extern TokenType token;

// ==================== parser.c: tokens, statements, blocks ====================

// reports a syntax error on the current token's line
void parser_error(const char* message);

// the current token's line, for the node being built
SourceLocation current_location(void);

// consumes the current token, which must be _token
void eat(TokenType _token);

// error recovery: drop the rest of the current line
void skip_rest_of_line(void);

// a statement ends at the end of its line
void expect_end_of_statement(void);

ASTNode* parse_statement(void);

// an indented block: statements up to the DEDENT that closes it
ASTNode* parse_block(void);

// ==================== expressions.c ====================

ASTNode* parse_expression(void);

// ==================== statements.c ====================

// num, real, chr, str, bool, zil (TYPE_ERROR after reporting if it isn't a type)
DataType parse_type_specifier(void);

ASTNode* parse_variable_declaration(void);

// called after the target name, at '='
ASTNode* parse_assignment(char* name, SourceLocation loc);

ASTNode* parse_log(void);

// ==================== functions.c ====================

ASTNode* parse_function(void);

// called after the function name, at '('
ASTNode* parse_function_call(char* name, SourceLocation loc);

ASTNode* parse_return_statement(void);

// ==================== control_flow.c ====================

// say / when / lest  cond  body  { elf [lest] cond body | nor cond body }  [ else body ]
ASTNode* parse_conditional(void);

// 'elf', 'nor' or 'else' with no conditional before it: reports, parses, discards
ASTNode* parse_stray_branch(void);

// loop while cond body | loop until cond body | loop forever body
ASTNode* parse_loop(void);

// skip / stop
ASTNode* parse_loop_jump(void);

#endif
