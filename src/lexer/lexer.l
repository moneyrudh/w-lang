%{
    #include <string.h>
    #include <stdbool.h>

    #include "lexer.h"
    #include "diagnostics.h"
    #include "lexer/internal.h"

    YYSTYPE yylval;

    // line where the currently open '---' comment started, for error messages
    static int comment_start_line = 0;

    // newlines inside ( ) [ ] { } don't end a statement. The opening brackets are kept
    // (with their lines) so one left unclosed can be reported where it was opened
    #define MAX_TRACKED_BRACKETS 256
    static int bracket_depth = 0;
    static struct { char symbol; int line; } open_brackets[MAX_TRACKED_BRACKETS];

    static void open_bracket(char symbol) {
        if (bracket_depth < MAX_TRACKED_BRACKETS) {
            open_brackets[bracket_depth].symbol = symbol;
            open_brackets[bracket_depth].line = yylineno;
        }
        bracket_depth++;
    }

    static void close_bracket(void) {
        if (bracket_depth > 0) bracket_depth--;
    }

    // an unclosed bracket found at the end of a statement, for the parser to report
    // (it knows whether the statement already has an error)
    static char unclosed_symbol = 0;
    static int unclosed_line = 0;

    // remembers the innermost bracket still open, then treats every bracket as closed
    static void found_unclosed_bracket(void) {
        int top = (bracket_depth < MAX_TRACKED_BRACKETS ? bracket_depth : MAX_TRACKED_BRACKETS) - 1;
        unclosed_symbol = open_brackets[top].symbol;
        unclosed_line = open_brackets[top].line;
        bracket_depth = 0;
    }

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

    // the end of a line, followed by the next line's indentation
    static int newline_token(bool has_tab, int indent) {
        if (has_tab) {
            report_error(yylineno, "Use spaces for indentation, not tabs");
        }
        line_indent = indent;
        return NEWLINE;
    }

    // Inside brackets a newline usually continues the expression, so it isn't returned. But
    // a line that starts with a statement keyword can't continue one: the bracket was never
    // closed. Such a keyword pushes itself back (yyless) and the held newline is returned first.
    static bool newline_held = false;     // a newline inside brackets, nothing scanned since
    static bool held_has_tab = false;
    static int held_indent = 0;
    static bool at_held_line_start = false;   // the current token is the first after a held newline

    #define YY_USER_ACTION  at_held_line_start = newline_held; newline_held = false;

    #define STATEMENT_KEYWORD(tok)                                  \
        if (at_held_line_start && bracket_depth > 0) {              \
            found_unclosed_bracket();                               \
            yyless(0);                                              \
            return newline_token(held_has_tab, held_indent);        \
        }                                                           \
        return tok;

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
    bool has_tab = memchr(yytext, '\t', yyleng) != NULL;
    int indent = yyleng - (yytext[0] == '\r' ? 2 : 1);
    if (bracket_depth == 0) {
        return newline_token(has_tab, indent);
    }
    newline_held = true;
    held_has_tab = has_tab;
    held_indent = indent;
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
"log"       { STATEMENT_KEYWORD(LOG) }

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

"dec" 	    { STATEMENT_KEYWORD(DEC) }
"fun"	    { STATEMENT_KEYWORD(FUN) }
"use"	    { return USE; }

"ret"       { STATEMENT_KEYWORD(RETURN) }

"say"       { STATEMENT_KEYWORD(SAY) }
"when"      { STATEMENT_KEYWORD(WHEN) }
"lest"      { STATEMENT_KEYWORD(LEST) }
"elf"       { STATEMENT_KEYWORD(ELF) }
"nor"       { STATEMENT_KEYWORD(NOR) }
"else"      { STATEMENT_KEYWORD(ELSE) }
"loop"      { STATEMENT_KEYWORD(LOOP) }
"while"     { return WHILE; }
"until"     { return UNTIL; }
"forever"   { return FOREVER; }
"skip"      { STATEMENT_KEYWORD(SKIP) }
"stop"      { STATEMENT_KEYWORD(STOP) }

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
"["	    { open_bracket('['); return LBRACKET; }
"]"	    { close_bracket(); return RBRACKET; }
"("         { open_bracket('('); return LPAREN; }
")"         { close_bracket(); return RPAREN; }
"{"         { open_bracket('{'); return LBRACE; }
"}"         { close_bracket(); return RBRACE; }
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
<<EOF>>     {
    if (bracket_depth > 0) {
        found_unclosed_bracket();
    }
    return 0;
}
%%

void lexer_close_brackets(void) {
    bracket_depth = 0;
    newline_held = false;
}

bool lexer_unclosed_bracket(char* symbol, int* line) {
    if (!unclosed_symbol) return false;
    *symbol = unclosed_symbol;
    *line = unclosed_line;
    unclosed_symbol = 0;
    return true;
}

void lexer_cleanup(void) {
    layout_cleanup();
    bracket_depth = 0;
    unclosed_symbol = 0;
    free(token_text);
    token_text = NULL;
    yylval.string = NULL;
    yylex_destroy();
}
