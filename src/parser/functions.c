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
    if (token_registry_is_type(token)) {
        parser_error("Expected ':' before the return type");
    } else if (token == COLON) {
        eat(COLON);
        if (!token_registry_is_type(token) && token != IDENTIFIER) {
            parser_error("Expected return type after ':'");
        } else {
            return_type = parse_type_specifier();
        }
    }

    // a broken header: the caller's recovery still parses the body for its own errors
    if (in_panic()) {
        free(name);
        free_parameters(parameters);
        return NULL;
    }

    ASTNode* body = parse_block();
    return create_function_node(return_type, name, parameters, param_count, body, loc);
}

// ==================== calls ====================

ASTNode** parse_argument_list(const char* context, int* arg_count, char*** arg_names) {
    eat(LPAREN);

    ASTNode** args = NULL;
    char** names = NULL;
    *arg_count = 0;
    int arg_capacity = 4;
    if (arg_names) *arg_names = NULL;

    if (token != RPAREN) {
        args = malloc(sizeof(ASTNode*) * arg_capacity);

        while (1) {
            if (*arg_count >= arg_capacity) {
                arg_capacity *= 2;
                args = realloc(args, sizeof(ASTNode*) * arg_capacity);
                if (names) {
                    names = realloc(names, sizeof(char*) * arg_capacity);
                    if (!names) {
                        fprintf(stderr, "Out of memory\n");
                        exit(1);
                    }
                }
            }

            // `name: value`: parsed as a value first (one token of lookahead), then a ':'
            // after a bare name makes it the argument's name
            ASTNode* arg = parse_expression();
            char* name = NULL;
            if (token == COLON && arg && arg->type == NODE_VARIABLE) {
                name = strdup(arg->data.variable.name);
                free_ast(arg);
                eat(COLON);
                // a pod value nests one bracket level more in the C than a call
                if (expression_enter()) {
                    arg = parse_expression();
                    expression_leave();
                } else {
                    arg = NULL;
                }
                if (!arg_names) {
                    char error_msg[100];
                    snprintf(error_msg, sizeof(error_msg), "Named values (name: value) aren't allowed in %s", context);
                    parser_error(error_msg);
                    free(name);
                    name = NULL;
                }
            }
            if (name && !names) {
                names = calloc((size_t)arg_capacity, sizeof(char*));
                if (!names) {
                    fprintf(stderr, "Out of memory\n");
                    exit(1);
                }
            }
            if (names) names[*arg_count] = name;
            args[(*arg_count)++] = arg;

            if (token == COMMA) {
                eat(COMMA);
            } else if (token == RPAREN) {
                break;
            } else {
                char error_msg[100];
                snprintf(error_msg, sizeof(error_msg), "Expected ',' or ')' in %s", context);
                parser_error(error_msg);
                break;
            }
        }
    }

    eat(RPAREN);
    if (arg_names) *arg_names = names;
    return args;
}

ASTNode* parse_function_call(char* name, SourceLocation loc) {
    int arg_count;
    char** arg_names;
    ASTNode** args = parse_argument_list("function call", &arg_count, &arg_names);
    ASTNode* node = create_function_call_node(name, args, arg_count, loc);
    node->data.function_call.arg_names = arg_names;   // the call owns them now
    free(args);
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
