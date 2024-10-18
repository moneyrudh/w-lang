%{
    #include "parser.h"
    #include <string.h>

    YYSTYPE yylval;
%}

%option noyywrap

%%
"w"         { printf("LEX: MAIN\n"); yylval.string = strdup("main"); return MAIN; }
"int"       { printf("LEX: INT\n"); yylval.string = strdup("int"); return INT; }
"void"      { printf("LEX: VOID\n"); yylval.string = strdup("void"); return VOID; }
"log"       { printf("LEX: LOG\n"); return LOG; }
"("         { printf("LEX: LPAREN\n"); return LPAREN; }
")"         { printf("LEX: RPAREN\n"); return RPAREN; }
"{"         { printf("LEX: LBRACE\n"); return LBRACE; }
"}"         { printf("LEX: RBRACE\n"); return RBRACE; }
";"         { printf("LEX: SEMICOLON\n"); return SEMICOLON; }
"+"         { printf("LEX: PLUS\n"); return PLUS; }
"-"         { printf("LEX: MINUS\n"); return MINUS; }
"*"         { printf("LEX: MULTIPLY\n"); return MULTIPLY; }
"/"         { printf("LEX: DIVIDE\n"); return DIVIDE; }
[0-9]+      { printf("LEX: NUMBER %s\n", yytext); yylval.number = atoi(yytext); return NUMBER; }
[a-zA-Z_][a-zA-Z0-9_]* { printf("LEX: IDENTIFIER %s\n", yytext); yylval.string = strdup(yytext); return IDENTIFIER; }

\"(\\.|[^"\\])*\" {
    yylval.string = strdup(yytext + 1);
    yylval.string[strlen(yylval.string)-1] = '\0';
    return STRING;
}

[ \t\n]+    { /* Ignore whitespace */ }
.           { printf("Unexpected character: %s\n", yytext); }

%%