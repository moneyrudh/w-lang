#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "gen.h"
#include "symbol_table.h"
#include "types.h"
#include "ast.h"
#include "parser.h"

ParserState parser_state;

void init_parser_state() {
    parser_state.brace_depth = 0;
    parser_state.block_capacity = 10;
    parser_state.block_lines = malloc(sizeof(int) * parser_state.block_capacity);
    parser_state.error_count = 0;
    parser_state.in_function_body = 0;
    parser_state.function_context = malloc(sizeof(FunctionContext));
    parser_state.function_context->current_name = NULL;
    parser_state.function_context->current_return_type = NULL;
    parser_state.function_context->has_return = 0;
    parser_state.function_context->return_value_required = 0;
}

void init_parser() {
    create_symbol_table();
    init_parser_state();
}

void cleanup_parser_state() {
    free(parser_state.block_lines);
    if (parser_state.function_context) {
        free(parser_state.function_context->current_name);
        free(parser_state.function_context->current_return_type);
        free(parser_state.function_context);
    }
}

void cleanup_parser() {
    free_symbol_table();
    cleanup_parser_state();
}

const char *tokenToString(TokenType token) {
    static char single_char[2] = {0, 0};

    if (token < 256) {
        single_char[0] = (char)token;
        return single_char;
    }

    switch (token) {
        case INT:
            return "INT";
        case VOID:
            return "VOID";
        case IDENTIFIER:
            return "IDENTIFIER";
        case LOG:
            return "LOG";
        case LPAREN:
            return "LPAREN";
        case RPAREN:
            return "RPAREN";
        case LBRACE:
            return "LBRACE";
        case RBRACE:
            return "RBRACE";
        case SEMICOLON:
            return "SEMICOLON";
        case STRING:
            return "STRING";
        case NUMBER:
            return "NUMBER";
        case PLUS:
            return "PLUS";
        case MINUS:
            return "MINUS";
        case MULTIPLY:
            return "MULTIPLY";
        case DIVIDE:
            return "DIVIDE";
        case MAIN:
            return "MAIN";
        case RETURN:
            return "RETURN";
        default:
            return "UNKNOWN";
    }
}

void parser_error(const char* message) {
    fprintf(stderr, "Error on line %d: %s\n", yylineno, message);
    parser_state.error_count++;
    if (parser_state.error_count >= 5) {
        fprintf(stderr, "Too many errors, exiting.\n");
        free_ast(ast);
        // if (output_file_name != NULL) unlink(output_file_name);
        cleanup_parser();
        exit(1);
    }
}

void enter_block() {
    if (parser_state.brace_depth >= parser_state.block_capacity) {
        parser_state.block_capacity *= 2;
        parser_state.block_lines = realloc(parser_state.block_lines, sizeof(int) * parser_state.block_capacity);
    }
    parser_state.block_lines[parser_state.brace_depth++] = yylineno;
}

void exit_block() {
    if (parser_state.brace_depth <= 0) {
        parser_error("Unexpected closing brace");
        return;
    }
    parser_state.brace_depth--;
}

ASTNode* parse_factor() {
    if (token == NUMBER) {
        ASTNode* node = create_number_node(yylval.number);
        eat(NUMBER);
        return node;
    } else if (token == STRING) {
        ASTNode* node = create_string_node(yylval.string);
        eat(STRING);
        return node;
    } else if (token == IDENTIFIER) {
        ASTNode* node = create_variable_node(yylval.string);
        eat(IDENTIFIER);
        return node;
    } else if (token == LPAREN) {
        eat(LPAREN);
        ASTNode* node = parse_expression();
        eat(RPAREN);
        return node;
    }
    parser_error("Unexpected token");
    return NULL;
}

ASTNode* parse_term() {
    ASTNode* node = parse_factor();
    while (token == MULTIPLY || token == DIVIDE) {
        char op = (token == MULTIPLY) ? '*' : '/';
        eat(token);
        node = create_binary_expr_node(node, parse_factor(), op);
    }
    return node;
}

ASTNode* parse_expression() {
    ASTNode* node = parse_term();
    while (token == PLUS || token == MINUS) {
        char op = (token == PLUS) ? '+' : '-';
        eat(token);
        node = create_binary_expr_node(node, parse_term(), op);
    }
    return node;
}

ASTNode* parse_variable_declaration() {
    char* type_name = strdup(yytext);
    eat(IDENTIFIER);

    char* var_name = strdup(yytext);
    eat(IDENTIFIER);

    DataType type;
    if (strcmp(type_name, "int") == 0) {
        type = TYPE_INT;
    } else if (strcmp(type_name, "string") == 0) {
        type = TYPE_STRING;
    } else {
        parser_error("Unknown type");
        exit(1);
    }

    if (!add_symbol(getSymbolTable(), var_name, type)) {
        parser_error("Variable already declared");
        exit(1);
    }

    free(type_name);
    free(var_name);

    eat(SEMICOLON);
    return NULL;
}

ASTNode* parse_log() {
    eat(LOG);
    eat(LPAREN);

    LogElement* head = NULL;
    LogElement* current = NULL;

    while (token != RPAREN) {
        LogElement* element = malloc(sizeof(LogElement));

        if (token == STRING) {
            element->type = NODE_STRING;
            element->value.string = strdup(yylval.string);
            eat(STRING);
        } else if (token == PLUS) {
            if (current->type == NODE_STRING) {
                element->type = NODE_STRING;
                element->value.string = strdup(" ");
                eat(PLUS);
            }
        } else {
            ASTNode* expr = parse_expression();
            DataType type = get_expression_type(expr, getSymbolTable());

            if (type == TYPE_INT) {
                element->type = NODE_NUMBER;
                element->value.number = expr->data.number.value;
            } else if (type == TYPE_STRING) {
                element->type = NODE_STRING;
                element->value.string = strdup(expr->data.string.value);
            } else {
                parser_error("Invalid type in log statement");
            }

            free_ast(expr);
        }

        if (head == NULL) {
            head = element;
            current = element;
        } else {
            current->next = element;
            current = element;
        }
    }

    eat(RPAREN);
    eat(SEMICOLON);

    return create_log_node(head);
}

ASTNode* parse_return_statement() {
    eat(RETURN);
    ASTNode* expr = NULL;
    int has_value = 0;
    if (token != SEMICOLON) {
        expr = parse_expression();
        has_value = 1;
    }

    eat(SEMICOLON);

    if (parser_state.function_context) {
        parser_state.function_context->has_return = 1;

        if (strcmp(parser_state.function_context->current_return_type, "void") == 0) {
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
                DataType func_type;

                if (strcmp(parser_state.function_context->current_return_type, "int") == 0) {
                    func_type = TYPE_INT;
                } else if (strcmp(parser_state.function_context->current_return_type, "string") == 0) {
                    func_type = TYPE_STRING;
                } else {
                    func_type = TYPE_VOID;
                }

                if (expr_type != func_type) {
                    char error_msg[100];
                    snprintf(error_msg, sizeof(error_msg),
                        "Return type mismatch in function '%s'. Expected %s, got %s",
                        parser_state.function_context->current_name,
                        parser_state.function_context->current_return_type,
                        expr_type == TYPE_INT ? "int" :
                        expr_type == TYPE_STRING ? "string" : "unknown");
                    parser_error(error_msg);
                }
            }
        }
    }

    ASTNode* node = malloc(sizeof(ASTNode));
    node->type = NODE_RETURN;
    node->data.return_statement.expression = expr;
    return node;
}

ASTNode* parse_statement() {
    printf("Token to string %s\n", tokenToString(token));
    switch (token) {
        case LBRACE:
            eat(LBRACE);
            enter_block();
            while (token != RBRACE && token != EOF) {
                parse_statement();
            }
            if (token == EOF) {
                parser_error("Unexpected end of file. Missing closing brace.");
                return NULL;
            }
            eat(RBRACE);
            exit_block();
            return NULL;
        case LOG:
            return parse_log();
        case NUMBER:
        case LPAREN:
            {
                ASTNode* expr = parse_expression();
                eat(SEMICOLON);
                return expr;
            }
        case INT:
        case STRING:
            return parse_variable_declaration();
        case RETURN:
            return parse_return_statement();
        default:
            {
                parser_error("Unexpected token in statement.");
                eat(token);
                return NULL;
            }
    }
}

ASTNode* parse_function() {
    char* return_type = strdup(yylval.string);
    if (!return_type) {
        parser_error("Memory allocation error");
        return NULL;
    }
    
    if (token == INT) {
        eat(INT);
    } else if (token == VOID) {
        eat(VOID);
    }
    
    char* name = strdup(yylval.string);
    printf("Function name: %s\n", name);
    eat(MAIN);
    eat(LPAREN);
    eat(RPAREN);

    if (parser_state.function_context) {
        free(parser_state.function_context->current_name);
        free(parser_state.function_context->current_return_type);
        parser_state.function_context->current_name = strdup(name);
        parser_state.function_context->current_return_type = strdup(return_type);
        parser_state.function_context->has_return = 0;
        parser_state.function_context->return_value_required = strcmp(return_type, "void") != 0;
    }

    if (token != LBRACE) {
        parser_error("Expected '{' to begin function boddy");
        return NULL;
    }
    eat(LBRACE);
    enter_block();
    parser_state.in_function_body = 1;

    int has_return = 0;
    ASTNode* body = NULL;
    ASTNode* last = NULL;

    while (token != RBRACE && token != EOF) {
        ASTNode* statement = parse_statement();
        if (statement) {
            if (statement->type == NODE_RETURN) {
                has_return = 1;
            }
            if (body == NULL) {
                body = statement;
                last = statement;
            } else {
                last->next = statement;
                last = statement;
            }
        }
    }

    if (token == EOF) {
        parser_error("Unexpected end of file.");
        return NULL;
    }

    eat(RBRACE);
    exit_block();
    parser_state.in_function_body = 0;

    // if (parser_state.brace_depth != 0) {
    //     parser_error("Mismatched braces in function body.");
    // }

    // if (strcmp(return_type, "void")!=0 && has_return == 0) { 
    //     error("Function must have a return statement.");
    // }

    if (parser_state.function_context) {
        if (parser_state.function_context->return_value_required &&
            !parser_state.function_context->has_return) {
            char error_msg[200];
            snprintf(error_msg, sizeof(error_msg),
                "Function '%s' with return type '%s' must return a value",
                name,
                return_type);
            parser_error(error_msg);
        }
    }

    return create_function_node(
        return_type, 
        name, 
        body, 
        parser_state.function_context->has_return
    );
}

ASTNode* parse() {
    ASTNode* program = create_program_node();
    if (!program) return NULL;

    ASTNode* last_function = NULL;
    while (token != EOF) {
        switch(token) {
            case INT:
            case VOID: {
                ASTNode* function = parse_function();
                if (function) {
                    if (last_function == NULL) {
                        program->data.program.functions = function;
                    } else {
                        last_function->next = function;
                    }
                    last_function = function;
                } else {
                    parser_error("Failed to parse function");
                    free_ast(program);
                    return NULL;
                }
                break;
            }
            default: {
                if (token == 0 || token == EOF) {
                    goto end;
                }

                char error_msg[100];
                snprintf(error_msg, sizeof(error_msg),
                    "Unexpected token %s at top level",
                    tokenToString(token));
                parser_error(error_msg);
                printf("Current token: %d, %s\n", token, tokenToString(token));

                while (token != 0 && token != EOF) {
                    token = yylex();
                }
            } break;
        }
    }

    end:
        return program;
}

void eat(TokenType _token) {
    if (token == _token) {
        printf("Eating token: %s\n", tokenToString(token));
        token = yylex();
        printf("Token: %s\n", tokenToString(token));
    } else {
        char error_msg[100];
        snprintf(error_msg, sizeof(error_msg), 
            "Unexpected token. Expected %d (%s), got %d (%s)", 
            _token, tokenToString(_token), 
            token, tokenToString(token));
        parser_error(error_msg);
        exit(1);
    }
}

ParserState getParserState(void) {
    return parser_state;
}