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
        case ASSIGNMENT:
            return "ASSIGNMENT";
        case FLOAT_TYPE:
            return "FLOAT_TYPE";
        case FLOAT_LITERAL:
            return "FLOAT_LITERAL";
        case CHAR_TYPE:
            return "CHAR_TYPE";
        case CHAR_LITERAL:
            return "CHAR_LITERAL";
        case STRING_TYPE:
            return "STRING_TYPE";
        case BOOL_TYPE:
            return "BOOL_TYPE";
        case BOOL_LITERAL:
            return "BOOL_LITERAL";
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
    SourceLocation loc = {yylineno, 0, NULL};
    switch (token) {
        case NUMBER:
            {
                ASTNode* node = create_number_node(yylval.number, loc);
                eat(NUMBER);
                return node;
            }
        case FLOAT_LITERAL:
            {
                ASTNode* node = create_float_node(yylval.float_val, loc);
                eat(FLOAT_LITERAL);
                return node;
            }
        case CHAR_LITERAL:
            {
                ASTNode* node = create_char_node(yylval.char_val, loc);
                eat(CHAR_LITERAL);
                return node;
            }
        case BOOL_LITERAL:
            {
                ASTNode* node = create_bool_node(yylval.bool_val, loc);
                eat(BOOL_LITERAL);
                return node;
            }
        case STRING:
            {
                ASTNode* node = create_string_node(yylval.string, loc);
                eat(STRING);
                return node;
            }
        case IDENTIFIER:
            {
                ASTNode* node = create_variable_node(yylval.string, loc);
                eat(IDENTIFIER);
                return node;
            }
        case LPAREN:
            {
                eat(LPAREN);
                ASTNode* node = parse_expression();
                eat(RPAREN);
                return node;
            }
        default:
            parser_error("Unexpected token in factor");
            return NULL;
    }
}

ASTNode* parse_term() {
    SourceLocation loc = {yylineno, 0, NULL};
    ASTNode* node = parse_factor();
    while (token == MULTIPLY || token == DIVIDE) {
        char op = (token == MULTIPLY) ? '*' : '/';
        eat(token);
        node = create_binary_expr_node(node, parse_factor(), op, loc);
    }
    return node;
}

ASTNode* parse_expression() {
    SourceLocation loc = {yylineno, 0, NULL};
    ASTNode* node = parse_term();
    while (token == PLUS || token == MINUS) {
        char op = (token == PLUS) ? '+' : '-';
        eat(token);
        node = create_binary_expr_node(node, parse_term(), op, loc);
    }
    return node;
}

ASTNode* parse_variable_declaration() {
    SourceLocation loc = {yylineno, 0, NULL};
    

    DataType var_type;
    if (token == INT) {
        var_type = TYPE_INT;
        eat(INT);
    } else if (token == FLOAT_TYPE) {
        var_type = TYPE_FLOAT;
        eat(FLOAT_TYPE);
    } else if (token == CHAR_TYPE) {
        var_type = TYPE_CHAR;
        eat(CHAR_TYPE);
    } else if (token == STRING_TYPE) {
        var_type = TYPE_STRING;
        eat(STRING_TYPE);
    } else if (token == BOOL_TYPE) {
        var_type = TYPE_BOOL;
        eat(BOOL_TYPE);
    } else {
        parser_error("Expected type specifier");
        return NULL;
    }

    if (token != IDENTIFIER) {
        parser_error("Expected identifier after type");
        return NULL;
    }

    char* var_name = strdup(yylval.string);
    eat(IDENTIFIER);

    ASTNode* init_expr = NULL;
    if (token == ASSIGNMENT) {
        eat(ASSIGNMENT);
        init_expr = parse_expression();

        if (init_expr) {
            DataType expr_type = get_expression_type(init_expr, getSymbolTable());
            if (!compare_types(var_type, expr_type)) {
                char error_msg[100];
                snprintf(error_msg, sizeof(error_msg),
                    "Type mismatch in initialization: cannot assign %s to %s",
                    type_to_string(expr_type),
                    type_to_string(var_type));
                parser_error(error_msg);
                free_ast(init_expr);
                free(var_name);
                return NULL;
            }
        }
    }

    if (token != SEMICOLON) {
        parser_error("Expected semicolon after variable declaration");
        if (init_expr) free_ast(init_expr);
        free(var_name);
        return NULL;
    }

    eat(SEMICOLON);

    if (!add_symbol(getSymbolTable(), var_name, var_type)) {
        parser_error("Variable already declared in this scope");
        if (init_expr) free_ast(init_expr);
        free(var_name);
        return NULL;
    }

    return create_var_declaration_node(var_name, var_type, init_expr, loc);
}

const char* type_to_string(DataType type) {
    switch (type) {
        case TYPE_INT: return "int";
        case TYPE_FLOAT: return "float";
        case TYPE_CHAR: return "char";
        case TYPE_BOOL: return "bool";
        case TYPE_STRING: return "string";
        case TYPE_VOID: return "void";
        default: return "unknown";
    }
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
    SourceLocation loc = {yylineno, 0, NULL};
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
        case FLOAT_TYPE:
        case CHAR_TYPE:
        case STRING_TYPE:
        case BOOL_TYPE:
            return parse_variable_declaration();
        case RETURN:
            return parse_return_statement();
        case IDENTIFIER: {
            char*name = strdup(yylval.string);
            eat(IDENTIFIER);

            if (token == ASSIGNMENT) {
                eat(ASSIGNMENT);
                ASTNode* value = parse_expression();
                eat(SEMICOLON);
                return create_assignment_node(name, value, loc);
            } else if (token == LPAREN) {
                // handle function call
                eat(LPAREN);
                // parse arguments
                eat(RPAREN);
                eat(SEMICOLON);
                return create_function_call_node(name, NULL, 0, loc);
            }

            parser_error("Expected '=' or '(' after identifier");
            free(name);
            return NULL;
        }
        default: {
            parser_error("Unexpected token in statement.");
            eat(token);
            return NULL;    
        }
    }
}

ASTNode* parse_function() {
    SourceLocation loc = {yylineno, 0, NULL};
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
    // printf("Function name: %s\n", name);
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
        parser_state.function_context->has_return,
        loc
    );
}

ASTNode* parse() {
    SourceLocation loc = {yylineno, 0, NULL};
    ASTNode* program = create_program_node(loc);
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
        // printf("Eating token: %s\n", tokenToString(token));
        token = yylex();
        // printf("Token: %s\n", tokenToString(token));
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