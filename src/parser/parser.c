#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "lexer.h"
#include "parser.h"
#include "parser/internal.h"
#include "diagnostics.h"
#include "language/token_registry.h"

TokenType token;

void parser_error(const char* message) {
    report_error(token_line, "%s", message);
}

SourceLocation current_location(void) {
    SourceLocation loc = {token_line, 0, NULL};
    return loc;
}

// ==================== statements & blocks ====================

void skip_rest_of_line(void) {
    while (token != NEWLINE && token != DEDENT && token != 0) {
        token = yylex();
    }
    if (token == NEWLINE) eat(NEWLINE);
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
    if (token == NEWLINE) eat(NEWLINE);
    if (token != INDENT) {
        parser_error("Expected an indented block");
        return NULL;
    }
    eat(INDENT);

    ASTNode* head = NULL;
    ASTNode* tail = NULL;
    while (token != DEDENT && token != 0) {
        ASTNode* statement = parse_statement();
        if (!statement) continue;
        if (head == NULL) {
            head = statement;
        } else {
            tail->next = statement;
        }
        tail = statement;
    }

    if (token == DEDENT) eat(DEDENT);
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
    token = yylex();

    ASTNode* program = create_program_node(current_location());

    // yylex() returns 0 at end of input
    ASTNode* last_function = NULL;
    while (token != 0) {
        if (token != FUN) {
            char error_msg[100];
            snprintf(error_msg, sizeof(error_msg),
                "Unexpected token %s at top level. Expected 'fun' for function definition.",
                token_registry_get_display_name(token));
            parser_error(error_msg);

            // nothing else can be recovered at the top level
            while (token != 0) {
                token = yylex();
            }
            break;
        }

        ASTNode* function = parse_function();
        if (!function) {
            parser_error("Failed to parse function");
            free_ast(program);
            return NULL;
        }

        if (last_function == NULL) {
            program->data.program.functions = function;
        } else {
            last_function->next = function;
        }
        last_function = function;
    }

    return program;
}

void eat(TokenType _token) {
    if (token == _token) {
        token = yylex();
    } else {
        char error_msg[100];
        snprintf(error_msg, sizeof(error_msg), 
            "Unexpected token. Expected %d (%s), got %d (%s)", 
            _token, token_registry_get_display_name(_token), 
            token, token_registry_get_display_name(token));
        parser_error(error_msg);
        exit(1);
    }
}

