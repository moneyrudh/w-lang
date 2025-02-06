%{
    #include "parser.h"
    #include <string.h>
    #include <stdbool.h>

    YYSTYPE yylval;
%}

%option noyywrap
%option yylineno

%%
"w"         { yylval.string = strdup("main"); return MAIN; }
"int"       { yylval.string = strdup("int"); return INT; }
"void"      { yylval.string = strdup("void"); return VOID; }
"float"     { yylval.string = strdup("float"); return FLOAT; }
"char"      { yylval.string = strdup("char"); return CHAR; }
"string"    { yylval.string = strdup("string"); return STRING; }
"bool"      { yylval.string = strdup("bool"); return BOOL; }
"true"      { yylval.bool_val = true; return BOOL_LITERAL; }
"false"     { yylval.bool_val = false; return BOOL_LITERAL; }
"log"       { return LOG; }
"="         { return ASSIGNMENT; }
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

[0-9]+      { yylval.number = atoi(yytext); return INT_LITERAL; }
[0-9]+\.[0-9]+ { 
    yylval.float_val = atof(yytext); 
    return FLOAT_LITERAL; 
}

[a-zA-Z_][a-zA-Z0-9_]* { 
    yylval.string = strdup(yytext); 
    return IDENTIFIER; 
}

\"(\\.|[^"\\])*\" {
    yylval.string = strdup(yytext + 1);
    yylval.string[strlen(yylval.string)-1] = '\0';
    return STRING_LITERAL;
}

\'([^\'\\]|\\[\'\"\\nt])\' {
    if (strlen(yytext) == 3) {
        yylval.char_val = yytext[1];
    } else if (strlen(yytext) == 4 && yytext[1] == '\\') {
        switch(yytext[2]) {
            case 'n': yylval.char_val = '\n'; break;
            case 't': yylval.char_val = '\t'; break;
            case '\\': yylval.char_val = '\\'; break;
            case '\'': yylval.char_val = '\''; break;
            case '\"': yylval.char_val = '\"'; break;
            default: yylval.char_val = yytext[2];
        }
    }
    return CHAR_LITERAL;
}

[ \t\n]+      { /* Ignore whitespace */ }
.           { printf("Unexpected character: %s\n", yytext); return yytext[0]; }
<<EOF>>     { return 0; }
%%