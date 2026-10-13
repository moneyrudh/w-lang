#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "lexer.h"
#include "parser.h"
#include "parser/internal.h"
#include "checker.h"
#include "language/type_registry.h"

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

        // parse parameter type
        DataType param_type = parse_type_specifier();

        // create parameter node
        Parameter* param = malloc(sizeof(Parameter));
        if (!param) {
            parser_error("Memory allocation failed for parameter");
            free(param_name);
            return head;
        }
        param->name = param_name;
        param->type = param_type;
        param->next = NULL;

        // add to symbol table
        if (!add_symbol(getSymbolTable(), param_name, param_type)) {
            parser_error("Duplicate parameter name");
            free(param);
            return head;
        }

        // add to linked list
        if (head == NULL) {
            head = param;
            tail = param;
        } else {
            tail->next = param;
            tail = param;
        }
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

ASTNode* parse_function(void) {
    SourceLocation loc = {yylineno, 0, NULL};
    
    if (token != FUN) {
        parser_error("Expected 'fun' keyword for function definition");
        return NULL;
    }
    eat(FUN);

    if (token != MAIN && token != IDENTIFIER) {
        parser_error("Expected function name after 'fun'");
        return NULL;
    }

    char* name = strdup(yylval.string);
    // eat "main" or identifier
    eat(token);
    eat(LPAREN);

    // parameters and the body share one scope (as in C)
    push_scope(getSymbolTable());

    // parse parameters
    int param_count = 0;
    Parameter* parameters = parse_parameter_list(&param_count);

    eat(RPAREN);

    char* return_type = strdup("zil");
    DataType return_data_type = TYPE_ZIL;
    if (token == COLON) {
        eat(COLON);

        const TypeMapping* mapping = type_registry_get_by_token(token);
        if (mapping) {
            free(return_type);
            return_type = strdup(mapping->w_lang_name);
            return_data_type = mapping->enum_value;
            eat(token);
        } else {
            parser_error("Expected return type after ':'");
            pop_scope(getSymbolTable());
            free(name);
            free(return_type);
            return NULL;
        }
    }

    // register function in function table
    if (!add_function(getFunctionTable(), name, return_data_type)) {
        char error_msg[100];
        snprintf(error_msg, sizeof(error_msg),
            "Function '%s' already declared", name);
        parser_error(error_msg);
    }

    if (parser_state.function_context) {
        free(parser_state.function_context->current_name);
        free(parser_state.function_context->current_return_type);
        parser_state.function_context->current_name = strdup(name);
        parser_state.function_context->current_return_type = strdup(return_type);
        parser_state.function_context->has_return = 0;
        parser_state.function_context->return_value_required = strcmp(return_type, "zil") != 0;
    }

    parser_state.in_function_body = 1;
    ASTNode* body = parse_block_statements();
    parser_state.in_function_body = 0;
    pop_scope(getSymbolTable());

    if (parser_state.function_context) {
        if (parser_state.function_context->return_value_required &&
            !parser_state.function_context->has_return) {
            char error_msg[200];
            snprintf(
                error_msg, 
                sizeof(error_msg),
                "Function '%s' with return type '%s' must return a value",
                name,
                return_type
            );
            parser_error(error_msg);
        }
    }

    return create_function_node(
        return_type,
        name,
        parameters,
        param_count,
        body,
        parser_state.function_context->has_return,
        loc
    );
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

ASTNode* parse_return_statement(void) {
    eat(RETURN);
    ASTNode* expr = NULL;
    int has_value = 0;

    if (token != NEWLINE) {
        expr = parse_expression();
        has_value = 1;
    }

    if (parser_state.function_context) {
        parser_state.function_context->has_return = 1;

        if (strcmp(parser_state.function_context->current_return_type, "zil") == 0) {
            if (has_value) {
                char error_msg[100];
                snprintf(error_msg, sizeof(error_msg),
                    "Function '%s' declared as void, cannot return a value",
                    parser_state.function_context->current_name);
                parser_error(error_msg);
            }
        } else {
            if (!has_value) {
                char error_msg[100];
                snprintf(error_msg, sizeof(error_msg),
                    "Function '%s' with return type '%s' must return a value",
                    parser_state.function_context->current_name,
                    parser_state.function_context->current_return_type);
                parser_error(error_msg);
            } else if (expr) {
                DataType expr_type = get_expression_type(expr, getSymbolTable());
                DataType func_type = type_registry_string_to_enum(parser_state.function_context->current_return_type);

                if (expr_type != TYPE_ZIL && expr_type != func_type) {
                    char error_msg[100];
                    snprintf(error_msg, sizeof(error_msg),
                        "Return type mismatch in function '%s'. Expected %s, got %s",
                        parser_state.function_context->current_name,
                        parser_state.function_context->current_return_type,
                        get_wlang_type_from_enum(expr_type));
                    parser_error(error_msg);
                }
            }
        }
    }

    expect_end_of_statement();

    ASTNode* node = malloc(sizeof(ASTNode));
    node->type = NODE_RETURN;
    node->data.return_statement.expression = expr;
    node->next = NULL;
    return node;
}
