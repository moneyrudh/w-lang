// Layout pass: turns raw NEWLINE tokens into the token stream the parser sees:
//   - blank and comment-only lines are dropped
//   - a NEWLINE is followed by INDENT when the next line is indented further,
//     or by one DEDENT per block it closes
//   - end of file closes the last line and every open block

#include <stdio.h>
#include <stdbool.h>

#include "lexer.h"
#include "diagnostics.h"
#include "lexer/internal.h"

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
            report_error(line, "Blocks nested too deeply");
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
        report_error(line, "Indentation doesn't line up with any enclosing block");

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
