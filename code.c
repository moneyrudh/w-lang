#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include "parser.h"
#include "symbol_table.h"

typedef struct {
    char* current_name;
    char* current_return_type;
    int has_return;
    int return_value_required;
} FunctionContext;

typedef struct {
    int brace_depth;
    int* block_lines;
    int block_capacity;
    int error_count;
    int in_function_body;
    FunctionContext* function_context;
} ParserState;

ASTNode* ast;
YYSTYPE yylval;
TokenType token;
SymbolTable* symbol_table;
ParserState parser_state;
char* output_file_name;

ASTNode* parse_expression();
ASTNode* parse_variable_declaration();

void free_ast(ASTNode* node);

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
    symbol_table = create_symbol_table();
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
    free_symbol_table(symbol_table);
    cleanup_parser_state();
}

void parser_error(const char* message) {
    fprintf(stderr, "Error on line %d: %s\n", yylineno, message);
    parser_state.error_count++;
    if (parser_state.error_count >= 5) {
        fprintf(stderr, "Too many errors, exiting.\n");
        free_ast(ast);
        if (output_file_name != NULL) unlink(output_file_name);
        cleanup_parser();
        exit(1);
    }
}

// void error(const char* message) {
//     fprintf(stderr, "Error on line %d: %s\n", yylineno, message);
//     free_ast(ast);
//     exit(1);
// }

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

ASTNode* create_function_node(char* return_type, char* name, ASTNode* body, int has_return) {
    ASTNode* node = malloc(sizeof(ASTNode));
    node->type = NODE_FUNCTION;
    node->data.function.return_type = return_type;
    node->data.function.name = name;
    node->data.function.body = body;
    node->data.function.has_return = has_return;
    return node;
}

ASTNode* create_log_node(LogElement* elements) {
    ASTNode* node = malloc(sizeof(ASTNode));
    node->type = NODE_LOG;
    node->data.log.elements = elements;
    return node;
}

ASTNode* create_binary_expr_node(ASTNode* left, ASTNode* right, char operator) {
    ASTNode* node = malloc(sizeof(ASTNode));
    node->type = NODE_BINARY_EXPR;
    node->data.binary_expr.left = left;
    node->data.binary_expr.right = right;
    node->data.binary_expr.operator = operator;
    return node;
}

ASTNode* create_number_node(int value) {
    ASTNode* node = malloc(sizeof(ASTNode));
    node->type = NODE_NUMBER;
    node->data.number.value = value;
    return node;
}

ASTNode* create_string_node(char* value) {
    ASTNode* node = malloc(sizeof(ASTNode));
    node->type = NODE_STRING;
    node->data.string.value = strdup(value);
    return node;
}

ASTNode* create_variable_node(char* name) {
    ASTNode* node = malloc(sizeof(ASTNode));
    node->type = NODE_VARIABLE;
    node->data.variable.name = strdup(name);
    return node;
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

    if (!add_symbol(symbol_table, var_name, type)) {
        parser_error("Variable already declared");
        exit(1);
    }

    free(type_name);
    free(var_name);

    eat(SEMICOLON);
    // return create_variable_node(var_name);
    return NULL;
}

ASTNode* parse_log() {
    // eat(LOG);
    // eat(LPAREN);
    // char* message = yylval.string;
    // printf("Message: %s\n", message);
    // eat(STRING);
    // eat(RPAREN);
    // eat(SEMICOLON);
    // return create_log_node(message);
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
            DataType type = get_expression_type(expr, symbol_table);

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

        // if (token == PLUS) {
        //     eat(PLUS);
        // } else {
        //     break;
        // }
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
                DataType expr_type = get_expression_type(expr, symbol_table);
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

ASTNode* create_program_node() {
    ASTNode* node = malloc(sizeof(ASTNode));
    if (!node) {
        parser_error("Memory allocation failed");
        return NULL;
    }
    node->type = NODE_PROGRAM;
    node->data.program.functions = NULL;
    node->data.program.globals = NULL;
    node->next = NULL;
    return node;
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

void generate_log_statement(FILE* output, LogElement* elements, int indent_level) {
    char indent[256] = {0};
    for (int i=0; i < indent_level; i++) {
        strcat(indent, "    ");
    }

    fprintf(output, "%sprintf(\"", indent);

    LogElement* current = elements;
    int arg_count = 0;
    while (current != NULL) {
        switch (current->type) {
            case NODE_STRING:
                fprintf(output, "%s", current->value.string);
                break;
            case NODE_NUMBER:
                fprintf(output, "%%d");
                arg_count++;
                break;
            case NODE_VARIABLE: {
                Symbol* symbol = lookup_symbol(symbol_table, current->value.string);
                if (symbol == NULL) {
                    parser_error("Undefined variable in log statement");
                } else if (symbol->type == TYPE_INT) {
                    fprintf(output, "%%d");
                } else if (symbol->type == TYPE_STRING) {
                    fprintf(output, "%%s");
                } else {
                    parser_error("Invalid type in log statement");
                }
                arg_count++;
                break;
            }
            default:
                parser_error("Unknown node type in log statement");
        }
        current = current->next;
    }

    fprintf(output, "\\n\"");

    current = elements;
    while (current != NULL) {
        if (current->type == NODE_NUMBER) {
            fprintf(output, ", %d", current->value.number);
        } else if (current->type == NODE_VARIABLE) {
            fprintf(output, ", %s", current->value.string);
        }
        current = current->next;
    }

    fprintf(output, ");\n");
}

void generate(FILE* output, ASTNode* node, int indent_level) {
    if (!node) return;

    printf("Generating node of type %d\n", node->type);
    char indent[256] = {0};
    for (int i=0; i < indent_level; i++) {
        strcat(indent, "    ");
    }

    switch (node->type) {
        case NODE_PROGRAM: {
            fprintf(output, "#include <stdio.h>\n\n");
            ASTNode* function = node->data.program.functions;
            while (function != NULL) {
                generate(output, function, indent_level);
                function = function->next;
            }
        } break;
        case NODE_FUNCTION:
            fprintf(output, "%s %s() {\n", node->data.function.return_type, node->data.function.name);
            ASTNode* statement = node->data.function.body;
            while (statement != NULL) {
                generate(output, statement, indent_level + 1);
                statement = statement->next;
            }
            fprintf(output, "}\n");
            break;
        case NODE_LOG:
            generate_log_statement(output, node->data.log.elements, indent_level);
            break;
        case NODE_BINARY_EXPR:
            fprintf(output, "%s", indent);
            generate(output, node->data.binary_expr.left, 0);
            fprintf(output, " %c ", node->data.binary_expr.operator);
            generate(output, node->data.binary_expr.right, 0);
            fprintf(output, ";\n");
            break;
        case NODE_NUMBER:
            fprintf(output, "%d", node->data.number.value);
            break;
        case NODE_STRING:
            fprintf(output, "\"%s\"", node->data.string.value);
            break;
        case NODE_VARIABLE:
            fprintf(output, "%s", node->data.variable.name);
            break;
        case NODE_RETURN: {
            fprintf(output, "%sreturn ", indent);
            if (node->data.return_statement.expression) {
                DataType type = get_expression_type(node->data.return_statement.expression, symbol_table);
                generate(output, node->data.return_statement.expression, 0);
            }
            fprintf(output, ";\n");
            break;
        }
        default:
            fprintf(stderr, "Unknown node type in AST\n");
            exit(1);
    }
}

void generate_code(FILE* output, ASTNode* node) {
    if (!node) return;

    if (node->type != NODE_PROGRAM) {
        fprintf(stderr, "Expected program node at root.\n");
        return;
    }

    generate(output, node, 0);
}

void free_log_elements(LogElement* elements) {
    LogElement* current = elements;
    while (current != NULL) {
        LogElement* next = current->next;
        if (current->type == NODE_STRING) {
            free(current->value.string);
        }
        free(current);
        current = next;
    }
}

void free_ast(ASTNode* node) {
    if (node) {
        switch (node->type) {
            case NODE_PROGRAM:
                free_ast(node->data.program.functions);
                free_ast(node->data.program.globals);
                break;
            case NODE_FUNCTION:
                free(node->data.function.return_type);
                free(node->data.function.name);
                free_ast(node->data.function.body);
                break;
            case NODE_LOG:
                free_log_elements(node->data.log.elements);
                break;
            case NODE_BINARY_EXPR:
                free_ast(node->data.binary_expr.left);
                free_ast(node->data.binary_expr.right);
                break;
            case NODE_STRING:
                free(node->data.string.value);
                break;
            case NODE_VARIABLE:
                free(node->data.variable.name);
                break;
            case NODE_RETURN:
                free_ast(node->data.return_statement.expression);
                break;
            case NODE_NUMBER:
                break;
        }
        if (node->next) free(node->next);
        free(node);
    }
}

int main(int argc, char* argv[]) {
    if (argc != 3) {
        fprintf(stderr, "Usage: %s input.w output.c\n", argv[0]);
        return 1;
    }

    FILE* input = fopen(argv[1], "r");
    if (!input) {
        perror("Error opening input file.");
        return 1;
    }

    output_file_name = argv[2];
    FILE* output = fopen(output_file_name, "w");
    if (!output) {
        perror("Error opening output file.");
        fclose(input);
        return 1;
    }

    init_parser();
    yyin = input;
    token = yylex();
    printf("Token: %d\n", token);
    ast = parse();

    if (!ast || ast->type != NODE_PROGRAM) {
        fprintf(stderr, "Failed to parse program.\n");
        return 1;
    }

    generate_code(output, ast);

    free_ast(ast);
    fclose(input);
    fclose(output);

    cleanup_parser();

    if (parser_state.error_count > 0) {
        unlink(output_file_name);
        return 1;
    }

    return 0;
}