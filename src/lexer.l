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
"num"       { yylval.string = strdup("num"); return NUM; }
"zil"      { yylval.string = strdup("zil"); return ZIL; }
"real"     { yylval.string = strdup("real"); return REAL; }
"chr"      { yylval.string = strdup("chr"); return CHR; }
"str"    { yylval.string = strdup("str"); return STR; }
"bool"      { yylval.string = strdup("bool"); return BOOL; }
"true"      { yylval.bool_val = true; return BOOL_LITERAL; }
"false"     { yylval.bool_val = false; return BOOL_LITERAL; }
"log"       { return LOG; }

"vec"	    { yylval.string = strdup("vec"); return VEC; }
"map"	    { yylval.string = strdup("map"); return MAP; }
"set" 	    { yylval.string = strdup("set"); return SET; }
"ref"       { yylval.string = strdup("ref"); return REF; }
"heap" 	    { yylval.string = strdup("heap"); return HEAP; }
"stack"	    { yylval.string = strdup("stack"); return STACK; }
"que"	    { yylval.string = strdup("que"); return QUE; }
"link" 	    { yylval.string = strdup("link"); return LINK; }
"tree"	    { yylval.string = strdup("tree"); return TREE; }
"pod"	    { yylval.string = strdup("pod"); return POD; }

"dec" 	    { yylval.string = strdup("dec"); return DEC; }
"fun"	    { yylval.string = strdup("fun"); return FUN; }
"use"	    { yylval.string = strdup("use"); return USE; }

"ret"       { return RETURN; }

":"	    { return COLON; }
":="	    { return INFER_ASSIGN; }
"="         { return ASSIGNMENT; }
"["	    { return LBRACKET; }
"]"	    { return RBRACKET; }
"("         { return LPAREN; }
")"         { return RPAREN; }
"{"         { return LBRACE; }
"}"         { return RBRACE; }
","	    { return COMMA; }
";"         { return SEMICOLON; }
"+"         { return PLUS; }
"-"         { return MINUS; }
"*"         { return MULTIPLY; }
"/"         { return DIVIDE; }

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
