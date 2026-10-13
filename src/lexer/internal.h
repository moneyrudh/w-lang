#ifndef W_LEXER_INTERNAL_H
#define W_LEXER_INTERNAL_H

// shared between the flex rules (lexer.l) and the layout pass (layout.c)

// flex's scanner: raw tokens, with a NEWLINE at the end of every line
int raw_yylex(void);

// indentation (in spaces) of the line that follows the last NEWLINE
extern int line_indent;

#endif
