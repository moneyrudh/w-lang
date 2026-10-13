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

#endif
