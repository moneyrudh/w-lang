#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "lexer.h"
#include "parser.h"
#include "parser/internal.h"
#include "checker.h"
#include "language/token_registry.h"
#include "language/type_registry.h"

DataType parse_type_specifier(void) {
    if (!token_registry_is_type(token)) {
        parser_error("Expected type specifier (num, real, chr, str, bool, zil)");
        return TYPE_ZIL;
    }

    DataType var_type = convert_token_to_data_type(token);
    eat(token);
    return var_type;
}

// ==================== variables ====================

ASTNode* parse_variable_declaration(void) {
    SourceLocation loc = {yylineno, 0, NULL};

    eat(DEC);

    // parse variable name
    if (token != IDENTIFIER) {
        parser_error("Expected identifier in variable declaration");
        return NULL;
    }
    
    char* var_name = strdup(yylval.string);
    eat(IDENTIFIER);

    // expect colon for type annotation
    if (token != COLON) {
        parser_error("Expected ':' after variable name in declaration");
        free(var_name);
        return NULL;
    }

    eat(COLON);

    DataType var_type = parse_type_specifier();
    if (var_type == TYPE_ZIL) {
        free(var_name);
        return NULL;
    }

    ASTNode* init_expr = NULL;
    if (token == ASSIGNMENT) {
        eat(ASSIGNMENT);
        init_expr = parse_expression();

        if (init_expr) {
            DataType expr_type = get_expression_type(init_expr, getSymbolTable());
            // zil means the expression already reported an error
            if (expr_type != TYPE_ZIL && !compare_types(var_type, expr_type)) {
                char error_msg[100];
                snprintf(error_msg, sizeof(error_msg),
                    "Type mismatch in initialization: cannot assign %s to %s",
                    get_wlang_type_from_enum(expr_type),
                    get_wlang_type_from_enum(var_type));
                parser_error(error_msg);
                free_ast(init_expr);
                free(var_name);
                return NULL;
            }
        }
    }

    if (!add_symbol(getSymbolTable(), var_name, var_type)) {
        parser_error("Variable already declared in this scope");
        skip_rest_of_line();
        if (init_expr) free_ast(init_expr);
        free(var_name);
        return NULL;
    }

    expect_end_of_statement();

    return create_var_declaration_node(var_name, var_type, init_expr, loc);
}

ASTNode* parse_assignment(char* name, SourceLocation loc) {
    eat(ASSIGNMENT);
    ASTNode* value = parse_expression();
    expect_end_of_statement();
    return create_assignment_node(name, value, loc);
}

// ==================== log ====================

ASTNode* parse_log(void) {
    eat(LOG);
    eat(LPAREN);

    LogElement* head = NULL;
    LogElement* current = NULL;

    while (token != RPAREN) {
        LogElement* element = malloc(sizeof(LogElement));

        if (token == STRING_LITERAL) {
            element->type = NODE_STRING;
            element->value.string = strdup(yylval.string);
            eat(STRING_LITERAL);
        } else if (token == COMMA) {
            // comma adds a space between elements
            element->type = NODE_STRING;
            element->value.string = strdup(" ");
            eat(COMMA);
        } else if (token == PLUS) {
            // plus concatenates without space
            eat(PLUS);
            free(element);
            continue;
        } else if (token == IDENTIFIER) {
            // handle variable reference
            element->type = NODE_VARIABLE;
            element->value.string = strdup(yylval.string);
            Symbol* symbol = lookup_symbol(getSymbolTable(), yylval.string);
            if (!symbol) {
                char error_msg[100];
                snprintf(error_msg, sizeof(error_msg), "Undefined variable in log statement: '%s'", yylval.string);
                parser_error(error_msg);
                element->var_type = TYPE_ZIL;
            } else {
                element->var_type = symbol->type;
            }
            eat(IDENTIFIER);
        } else if (token == INT_LITERAL) {
            // handle number literal
            element->type = NODE_NUMBER;
            element->value.number = yylval.number;
            eat(INT_LITERAL);
        } else {
            parser_error("Invalid token in log statement");
            free(element);
            break;
        }

        element->next = NULL;
        if (head == NULL) {
            head = element;
            current = element;
        } else {
            current->next = element;
            current = element;
        }
    }

    eat(RPAREN);
    expect_end_of_statement();

    return create_log_node(head);
}
