#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "lexer.h"
#include "parser.h"
#include "parser/internal.h"
#include "diagnostics.h"
#include "language/token_registry.h"

TokenType token;

// the token consumed before `token`, to tell whether we're at the start of a statement
static TokenType prev_token = NEWLINE;

// ==================== errors & recovery ====================
//
// Panic mode: the first syntax error in a statement is reported, later ones in the same
// statement are not (they're almost always caused by the first). After the statement,
// recover() skips to the next line and parsing goes on, so each broken statement gives
// one error and the rest of the file is still checked.

// errors counted before the current statement started (lexer errors count too)
static int statement_mark = 0;
static bool panicking = false;

// tokens consumed so far, and when the current statement started: recovery must always
// move forward, or a statement that consumed nothing would be retried forever
static long tokens_consumed = 0;
static long statement_start = 0;

bool in_panic(void) {
    return panicking || error_count() > statement_mark;
}

static void begin_statement(void) {
    statement_mark = error_count();
    panicking = false;
    statement_start = tokens_consumed;
}

void parser_error(const char* message) {
    if (in_panic()) return;
    report_error(token_line, "%s", message);
    panicking = true;
}

static void advance(void) {
    prev_token = token;
    token = yylex();
    tokens_consumed++;

    // reported here rather than by the lexer, so it counts as this statement's one error
    char symbol;
    int line;
    if (lexer_unclosed_bracket(&symbol, &line) && !in_panic()) {
        report_error(line, "'%c' was never closed", symbol);
        panicking = true;
    }
}

SourceLocation current_location(void) {
    SourceLocation loc = {token_line, 0, NULL};
    return loc;
}

// ==================== statements & blocks ====================

void skip_rest_of_line(void) {
    // a bracket the broken statement left open must not swallow the lines after it
    lexer_close_brackets();
    while (token != NEWLINE && token != DEDENT && token != 0) {
        advance();
    }
    if (token == NEWLINE) advance();
}

// after a statement with an error: move to the start of the next statement
static void recover(void) {
    bool moved = tokens_consumed > statement_start;
    bool at_statement_start = moved && (prev_token == NEWLINE || prev_token == DEDENT || prev_token == INDENT);
    if (!at_statement_start && token != DEDENT && token != 0) {
        skip_rest_of_line();
    }
    // the broken statement's indented body is still parsed for its own errors
    if (token == INDENT) {
        free_ast(parse_block());
    }
}

void expect_end_of_statement(void) {
    if (token == NEWLINE) {
        eat(NEWLINE);
        return;
    }
    if (token == SEMICOLON) {
        parser_error("W doesn't use ';' - end the statement with a new line");
    } else {
        parser_error("Expected end of line after statement");
    }
    skip_rest_of_line();
}

ASTNode* parse_block(void) {
    if (token == NEWLINE) advance();
    if (token != INDENT) {
        parser_error("Expected an indented block");
        return NULL;
    }
    advance();   // INDENT

    // errors inside the block belong to its statements, not to the statement that owns it
    bool outer_panicking = in_panic();

    ASTNode* head = NULL;
    ASTNode* tail = NULL;
    while (token != DEDENT && token != 0) {
        begin_statement();
        ASTNode* statement = parse_statement();
        if (in_panic()) recover();
        if (!statement) continue;
        if (head == NULL) {
            head = statement;
        } else {
            tail->next = statement;
        }
        tail = statement;
    }

    if (token == DEDENT) advance();
    statement_mark = error_count();
    panicking = outer_panicking;
    return head;
}

ASTNode* parse_statement(void) {
    SourceLocation loc = current_location();
    switch (token) {
        case NEWLINE:
            // left over after an error that stopped at the end of a line
            eat(NEWLINE);
            return NULL;
        case INDENT: {
            parser_error("Unexpected indentation");
            // parse the stray block anyway so its contents get checked
            ASTNode* stray = parse_block();
            free_ast(stray);
            return NULL;
        }
        case LOG:
            return parse_log();
        case INT_LITERAL:
        case LPAREN:
            {
                ASTNode* expr = parse_expression();
                expect_end_of_statement();
                return expr;
            }
        case DEC:
            return parse_variable_declaration();
        case RETURN:
            return parse_return_statement();
        case SAY:
        case WHEN:
        case LEST:
            return parse_conditional();
        case ELF:
        case NOR:
        case ELSE:
            return parse_stray_branch();
        case LOOP:
            return parse_loop();
        case SKIP:
        case STOP:
            return parse_loop_jump();
        case IDENTIFIER: {
            char* name = strdup(yylval.string);
            eat(IDENTIFIER);

            ASTNode* node = NULL;
            if (token == ASSIGNMENT) {
                node = parse_assignment(name, loc);
            } else if (token == LPAREN) {
                node = parse_function_call(name, loc);
                expect_end_of_statement();
            } else {
                parser_error("Expected '=' or '(' after identifier");
            }
            free(name);
            return node;
        }
        default: {
            parser_error("Unexpected token in statement.");
            skip_rest_of_line();
            return NULL;
        }
    }
}

ASTNode* parse(void) {
    prev_token = NEWLINE;
    tokens_consumed = statement_start = 0;
    statement_mark = 0;
    panicking = false;
    advance();

    ASTNode* program = create_program_node(current_location());

    // yylex() returns 0 at end of input
    ASTNode* last_function = NULL;
    while (token != 0) {
        begin_statement();
        if (token != FUN) {
            char error_msg[100];
            snprintf(error_msg, sizeof(error_msg),
                "Unexpected %s at top level. Expected 'fun' for function definition.",
                token_registry_describe(token));
            parser_error(error_msg);
            recover();
            continue;
        }

        ASTNode* function = parse_function();
        if (in_panic()) recover();
        if (!function) continue;

        if (last_function == NULL) {
            program->data.program.functions = function;
        } else {
            last_function->next = function;
        }
        last_function = function;
    }

    lexer_close_brackets();
    return program;
}

bool eat(TokenType _token) {
    if (token == _token) {
        advance();
        return true;
    }
    char error_msg[100];
    snprintf(error_msg, sizeof(error_msg), "Expected %s, got %s",
             token_registry_describe(_token), token_registry_describe(token));
    parser_error(error_msg);
    return false;
}

