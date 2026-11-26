#ifndef W_LEXER_H
#define W_LEXER_H

#include <stdio.h>
#include <stdbool.h>

#include "types.h"

// value of the token yylex() just returned
typedef struct {
    union {
        char* string;
        int number;
        double float_val;
        char char_val;
        bool bool_val;
    };
} YYSTYPE;

extern YYSTYPE yylval;
extern FILE* yyin;
extern int yylineno;     // line flex is currently reading
extern int token_line;   // source line of the token yylex() just returned

// next token, with NEWLINE / INDENT / DEDENT for line structure (see lexer/layout.c)
int yylex(void);

// forgets every open bracket, so the next newline ends the statement again
// (the parser calls it when it gives up on a statement)
void lexer_close_brackets(void);

// a bracket found never closed (at the next statement or the end of the file), once:
// its symbol and the line it was opened on. The parser reports it
bool lexer_unclosed_bracket(char* symbol, int* line);

// frees the lexer's buffers once the input has been parsed
void lexer_cleanup(void);

#endif
