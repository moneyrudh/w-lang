%{
    #include "parser.h"
    #include <string.h>
    #include <stdbool.h>

    YYSTYPE yylval;

    // line where the currently open '---' comment started, for error messages
    static int comment_start_line = 0;

    // newlines inside ( ) [ ] { } don't end a statement
    static int bracket_depth = 0;

    // indentation (in spaces) of the line that follows the last NEWLINE
    static int line_indent = 0;

    // flex's scanner is raw_yylex(); yylex() (below) adds NEWLINE / INDENT / DEDENT
    #define YY_DECL int raw_yylex(void)
%}

%option noyywrap
%option yylineno

%x THROWAWAY_BLOCK

%%
"---"       { comment_start_line = yylineno; BEGIN(THROWAWAY_BLOCK); }
"--"([^-\n].*)? { /* single-line throwaway comment */ }

\r?\n[ \t]*  {
    if (bracket_depth == 0) {
        if (memchr(yytext, '\t', yyleng)) {
            fprintf(stderr, "Error on line %d: Use spaces for indentation, not tabs\n", yylineno);
            parser_state.error_count++;
        }
        line_indent = yyleng - (yytext[0] == '\r' ? 2 : 1);
        return NEWLINE;
    }
}
"note"([^a-zA-Z0-9_\n].*)? { /* doc comment (stack lines for multi-line docs) */ }

<THROWAWAY_BLOCK>"---"  { BEGIN(INITIAL); }
<THROWAWAY_BLOCK>\n|.   { /* skip comment body */ }
<THROWAWAY_BLOCK><<EOF>> {
    fprintf(stderr, "Error on line %d: Unterminated '---' comment\n", comment_start_line);
    parser_state.error_count++;
    BEGIN(INITIAL);
    return 0;
}

"w"         { yylval.string = strdup("w"); return MAIN; }
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

^[ \t]+/[^ \t\r\n]  {
    // only reachable on line 1: later lines' indentation is consumed with their newline
    if (yylineno == 1) {
        fprintf(stderr, "Error on line 1: Unexpected indentation\n");
        parser_state.error_count++;
    }
}
[ \t\r]+      { /* whitespace within a line */ }
.           { printf("Unexpected character: %s\n", yytext); return yytext[0]; }
<<EOF>>     { return 0; }
%%

// ==================== layout ====================
// Turns raw NEWLINE tokens into the token stream the parser sees:
//   - blank and comment-only lines are dropped
//   - a NEWLINE is followed by INDENT when the next line is indented further,
//     or by one DEDENT per block it closes
//   - end of file closes the last line and every open block

#define MAX_INDENT_DEPTH 128

int token_line = 1;

static int indent_stack[MAX_INDENT_DEPTH] = {0};
static int indent_top = 0;
static int pending_dedents = 0;
static bool pending_indent = false;
static bool has_buffered = false;
static int buffered_token = 0;
static int buffered_line = 0;
static bool line_has_code = false;

static void schedule_indentation(int indent, int line) {
    if (indent > indent_stack[indent_top]) {
        if (indent_top + 1 >= MAX_INDENT_DEPTH) {
            fprintf(stderr, "Error on line %d: Blocks nested too deeply\n", line);
            parser_state.error_count++;
            return;
        }
        indent_stack[++indent_top] = indent;
        pending_indent = true;
        return;
    }

    while (indent < indent_stack[indent_top]) {
        indent_top--;
        pending_dedents++;
    }

    if (indent != indent_stack[indent_top]) {
        fprintf(stderr, "Error on line %d: Indentation doesn't line up with any enclosing block\n", line);
        parser_state.error_count++;

        // recover by keeping the line in the deeper block it fell short of,
        // rather than closing that block and cascading errors
        indent_top++;
        pending_dedents--;
    }
}

int yylex(void) {
    if (pending_indent) {
        pending_indent = false;
        token_line = buffered_line;
        return INDENT;
    }
    if (pending_dedents > 0) {
        pending_dedents--;
        token_line = buffered_line;
        return DEDENT;
    }

    int tok;
    if (has_buffered) {
        has_buffered = false;
        tok = buffered_token;
        token_line = buffered_line;
    } else {
        tok = raw_yylex();
        token_line = yylineno;
    }

    if (tok == NEWLINE) {
        int newline_line = token_line - 1;

        // skip blank and comment-only lines: the next real line sets the indentation
        do {
            tok = raw_yylex();
        } while (tok == NEWLINE);

        has_buffered = true;
        buffered_token = tok;
        buffered_line = yylineno;
        schedule_indentation(tok == 0 ? 0 : line_indent, buffered_line);

        // nothing on this line yet (start of file): no NEWLINE to report
        if (!line_has_code) {
            return yylex();
        }

        line_has_code = false;
        token_line = newline_line;
        return NEWLINE;
    }

    if (tok == 0) {
        // end of file: finish the last line, then close every open block
        if (line_has_code || indent_top > 0) {
            has_buffered = true;
            buffered_token = 0;
            buffered_line = token_line;
            schedule_indentation(0, token_line);
            if (line_has_code) {
                line_has_code = false;
                return NEWLINE;
            }
            return yylex();
        }
        return 0;
    }

    line_has_code = true;
    return tok;
}
