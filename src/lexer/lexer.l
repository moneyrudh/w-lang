%{
    #include <string.h>
    #include <stdbool.h>

    #include "lexer.h"
    #include "diagnostics.h"
    #include "lexer/internal.h"

    YYSTYPE yylval;

    // line where the currently open '---' comment started, for error messages
    static int comment_start_line = 0;

    // newlines inside ( ) [ ] { } don't end a statement
    static int bracket_depth = 0;

    int line_indent = 0;

    // text of the current IDENTIFIER / STRING_LITERAL / MAIN token. The lexer owns it and
    // frees it when the next one replaces it, so the parser copies what it keeps
    static char* token_text = NULL;

    static void keep_text(const char* text) {
        free(token_text);
        token_text = strdup(text);
        yylval.string = token_text;
    }

    // a character that can't start any token: reported once, then skipped
    static void unexpected_character(const char* text) {
        unsigned char c = (unsigned char)text[0];
        if (c == '\'') {
            report_error(yylineno, "Unexpected character \"'\"");
        } else if (c >= 0x80 || (c > ' ' && c < 0x7f)) {
            report_error(yylineno, "Unexpected character '%s'", text);
        } else {
            report_error(yylineno, "Unexpected byte 0x%02X", c);
        }
    }

    // flex's scanner is raw_yylex(); yylex() (layout.c) adds NEWLINE / INDENT / DEDENT
    #define YY_DECL int raw_yylex(void)
%}

%option noyywrap
%option yylineno
%option nounput noinput

%x THROWAWAY_BLOCK

%%
"---"       { comment_start_line = yylineno; BEGIN(THROWAWAY_BLOCK); }
"--"([^-\n].*)? { /* single-line throwaway comment */ }

\r?\n[ \t]*  {
    if (bracket_depth == 0) {
        if (memchr(yytext, '\t', yyleng)) {
            report_error(yylineno, "Use spaces for indentation, not tabs");
        }
        line_indent = yyleng - (yytext[0] == '\r' ? 2 : 1);
        return NEWLINE;
    }
}
"note"([^a-zA-Z0-9_\n].*)? { /* doc comment (stack lines for multi-line docs) */ }

<THROWAWAY_BLOCK>"---"  { BEGIN(INITIAL); }
<THROWAWAY_BLOCK>\n|.   { /* skip comment body */ }
<THROWAWAY_BLOCK><<EOF>> {
    report_error(comment_start_line, "Unterminated '---' comment");
    BEGIN(INITIAL);
    return 0;
}

"w"         { keep_text("w"); return MAIN; }
"num"       { return NUM; }
"zil"      { return ZIL; }
"real"     { return REAL; }
"chr"      { return CHR; }
"str"    { return STR; }
"bool"      { return BOOL; }
"true"      { yylval.bool_val = true; return BOOL_LITERAL; }
"false"     { yylval.bool_val = false; return BOOL_LITERAL; }
"log"       { return LOG; }

"vec"	    { return VEC; }
"map"	    { return MAP; }
"set" 	    { return SET; }
"ref"       { return REF; }
"heap" 	    { return HEAP; }
"stack"	    { return STACK; }
"que"	    { return QUE; }
"link" 	    { return LINK; }
"tree"	    { return TREE; }
"pod"	    { return POD; }

"dec" 	    { return DEC; }
"fun"	    { return FUN; }
"use"	    { return USE; }

"ret"       { return RETURN; }

"say"       { return SAY; }
"when"      { return WHEN; }
"lest"      { return LEST; }
"elf"       { return ELF; }
"nor"       { return NOR; }
"else"      { return ELSE; }
"loop"      { return LOOP; }
"while"     { return WHILE; }
"until"     { return UNTIL; }
"forever"   { return FOREVER; }
"skip"      { return SKIP; }
"stop"      { return STOP; }

"is"        { return IS; }
"eq"        { return EQ; }
"ne"        { return NE; }
"gt"        { return GT; }
"lt"        { return LT; }
"ge"        { return GE; }
"le"        { return LE; }
"and"       { return AND; }
"or"        { return OR; }
"not"       { return NOT; }

":"	    { return COLON; }
":="	    { return INFER_ASSIGN; }
"=="        { return EQUAL; }
"!="        { return NOT_EQUAL; }
"<="        { return LESS_EQUAL; }
">="        { return GREATER_EQUAL; }
"&&"        { return AND; }
"||"        { return OR; }
"="         { return ASSIGNMENT; }
"<"         { return LESS; }
">"         { return GREATER; }
"!"         { return BANG; }
"["	    { bracket_depth++; return LBRACKET; }
"]"	    { if (bracket_depth > 0) bracket_depth--; return RBRACKET; }
"("         { bracket_depth++; return LPAREN; }
")"         { if (bracket_depth > 0) bracket_depth--; return RPAREN; }
"{"         { bracket_depth++; return LBRACE; }
"}"         { if (bracket_depth > 0) bracket_depth--; return RBRACE; }
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
    keep_text(yytext);
    return IDENTIFIER;
}

\"(\\.|[^"\\])*\" {
    keep_text(yytext + 1);
    token_text[strlen(token_text) - 1] = '\0';   // drop the closing quote
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

^[ \t]+/[^ \t\r\n]  {
    // only reachable on line 1: later lines' indentation is consumed with their newline
    if (yylineno == 1) {
        report_error(1, "Unexpected indentation");
    }
}
[ \t\r]+      { /* whitespace within a line */ }
[\xC2-\xF4][\x80-\xBF]+ { unexpected_character(yytext); /* a whole UTF-8 character */ }
.           { unexpected_character(yytext); }
<<EOF>>     { return 0; }
%%

void lexer_cleanup(void) {
    free(token_text);
    token_text = NULL;
    yylval.string = NULL;
    yylex_destroy();
}
