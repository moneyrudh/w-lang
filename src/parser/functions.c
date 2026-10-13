#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "lexer.h"
#include "parser.h"
#include "parser/internal.h"
#include "language/token_registry.h"

// ==================== definitions ====================

static Parameter* parse_parameter_list(int* param_count) {
    Parameter* head = NULL;
    Parameter* tail = NULL;
    *param_count = 0;

    // if next token is RPAREN, no parameters
    if (token == RPAREN) {
        return NULL;
    }

    while (1) {
        // expect parameter name
        if (token != IDENTIFIER) {
            parser_error("Expected parameter name");
            return head;
        }

        char* param_name = strdup(yylval.string);
        eat(IDENTIFIER);

        // expect colon
        if (token != COLON) {
            parser_error("Expected ':' after parameter name");
            free(param_name);
            return head;
        }
        eat(COLON);

        Parameter* param = malloc(sizeof(Parameter));
        param->name = param_name;
        param->type = parse_type_specifier();
        param->next = NULL;

        // add to linked list
        if (head == NULL) {
            head = param;
        } else {
            tail->next = param;
        }
        tail = param;
        (*param_count)++;

        // check for comma (more parameters) or closing paren
        if (token == COMMA) {
            eat(COMMA);
            continue;
        } else if (token == RPAREN) {
            break;
        } else {
            parser_error("Expected ',' or ')' after parameter");
            break;
        }
    }

    return head;
}

// fun name(params) [: type] block
ASTNode* parse_function(void) {
    SourceLocation loc = current_location();
    eat(FUN);

    if (token != MAIN && token != IDENTIFIER) {
        parser_error("Expected function name after 'fun'");
        return NULL;
    }

    char* name = strdup(yylval.string);
    eat(token);   // w or identifier
    eat(LPAREN);

    int param_count = 0;
    Parameter* parameters = parse_parameter_list(&param_count);

    eat(RPAREN);

    // omitted return type means zil
    DataType return_type = TYPE_ZIL;
    if (token == COLON) {
        eat(COLON);
        if (!token_registry_is_type(token)) {
            parser_error("Expected return type after ':'");
            free(name);
            return NULL;
        }
        return_type = parse_type_specifier();
    }

    ASTNode* body = parse_block();
    return create_function_node(return_type, name, parameters, param_count, body, loc);
}

// ==================== calls ====================

ASTNode* parse_function_call(char* name, SourceLocation loc) {
    eat(LPAREN);

    ASTNode** args = NULL;
    int arg_count = 0;
    int arg_capacity = 4;

    if (token != RPAREN) {
        args = malloc(sizeof(ASTNode*) * arg_capacity);

        while (1) {
            if (arg_count >= arg_capacity) {
                arg_capacity *= 2;
                args = realloc(args, sizeof(ASTNode*) * arg_capacity);
            }

            args[arg_count++] = parse_expression();

            if (token == COMMA) {
                eat(COMMA);
            } else if (token == RPAREN) {
                break;
            } else {
                parser_error("Expected ',' or ')' in function call");
                break;
            }
        }
    }

    eat(RPAREN);
    ASTNode* node = create_function_call_node(name, args, arg_count, loc);
    if (args) free(args);
    return node;
}

// ==================== return ====================

// ret [expr]
ASTNode* parse_return_statement(void) {
    SourceLocation loc = current_location();
    eat(RETURN);

    ASTNode* expr = NULL;
    if (token != NEWLINE) {
        expr = parse_expression();
    }

    expect_end_of_statement();
    return create_return_node(expr, loc);
}
