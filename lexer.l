%{
    #include "parser.h"
    #include <string.h>

    YYSTYPE yylval;
%}

%option noyywrap
%option yylineno

%%
"w"         { yylval.string = strdup("main"); return MAIN; }
"int"       { yylval.string = strdup("int"); return INT; }
"void"      { yylval.string = strdup("void"); return VOID; }
"log"       { return LOG; }
"("         { return LPAREN; }
")"         { return RPAREN; }
"{"         { return LBRACE; }
"}"         { return RBRACE; }
";"         { return SEMICOLON; }
"+"         { return PLUS; }
"-"         { return MINUS; }
"*"         { return MULTIPLY; }
"/"         { return DIVIDE; }
"return"    { return RETURN; }
[0-9]+      { printf("LEX: NUMBER %s\n", yytext); yylval.number = atoi(yytext); return NUMBER; }
[a-zA-Z_][a-zA-Z0-9_]* { printf("LEX: IDENTIFIER %s\n", yytext); yylval.string = strdup(yytext); return IDENTIFIER; }

\"(\\.|[^"\\])*\" {
    yylval.string = strdup(yytext + 1);
    yylval.string[strlen(yylval.string)-1] = '\0';
    return STRING;
}

[ \t\n]+      { /* Ignore whitespace */ }
.           { printf("Unexpected character: %s\n", yytext); }

%%