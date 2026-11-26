#ifndef W_PARSER_INTERNAL_H
#define W_PARSER_INTERNAL_H

// shared between the parser's files only; the public API is include/parser.h

#include <stdbool.h>

#include "types.h"
#include "ast.h"

// current lookahead token
extern TokenType token;

// Nesting limits. The parser, checker, code generator and free_ast() recurse over the
// tree, so input nested beyond these is an error rather than a stack overflow.
// Expression levels: each '(', 'not' / '!', call argument list, and each operator in a
// chain (a + b + c is two levels deep). Statement levels: blocks and one-line bodies.
// Each level becomes at most one bracket level in the generated C, and clang rejects C
// nested more than 256 brackets deep, so together they stay under that (with room for the
// function's own braces and a printf call).
#define MAX_EXPRESSION_DEPTH 150
#define MAX_STATEMENT_DEPTH 100

// elf / nor / else branches in one conditional. The C is an `else if` chain, which C
// compilers parse recursively (Apple clang crashes at about 5,000)
#define MAX_CONDITIONAL_BRANCHES 1000

// ==================== parser.c: tokens, statements, blocks ====================

// reports a syntax error on the current token's line, unless this statement already has
// an error (panic mode: one error per broken statement, see parser.c)
void parser_error(const char* message);

// true once the current statement has a syntax error (later ones in it are not reported)
bool in_panic(void);

// the current token's line, for the node being built
SourceLocation current_location(void);

// consumes the current token if it is _token. Otherwise reports "Expected X, got Y",
// consumes nothing and returns false; callers carry on and recovery happens per statement
bool eat(TokenType _token);

// error recovery: drop the rest of the current line (closing any brackets left open)
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
