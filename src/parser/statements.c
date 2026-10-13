#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "lexer.h"
#include "parser.h"
#include "parser/internal.h"
#include "language/token_registry.h"
#include "language/type_registry.h"

DataType parse_type_specifier(void) {
    if (!token_registry_is_type(token)) {
        parser_error("Expected type specifier (num, real, chr, str, bool, zil)");
        return TYPE_ERROR;
    }

    DataType type = convert_token_to_data_type(token);
    eat(token);
    return type;
}

// ==================== variables ====================

// dec name: type [= expr]
ASTNode* parse_variable_declaration(void) {
    SourceLocation loc = current_location();
    eat(DEC);

    if (token != IDENTIFIER) {
        parser_error("Expected identifier in variable declaration");
        skip_rest_of_line();
        return NULL;
    }
    char* name = strdup(yylval.string);
    eat(IDENTIFIER);

    if (token != COLON) {
        parser_error("Expected ':' after variable name in declaration");
        skip_rest_of_line();
        free(name);
        return NULL;
    }
    eat(COLON);

    DataType type = parse_type_specifier();
    if (type == TYPE_ERROR) {
        skip_rest_of_line();
        free(name);
        return NULL;
    }

    ASTNode* init_expr = NULL;
    if (token == ASSIGNMENT) {
        eat(ASSIGNMENT);
        init_expr = parse_expression();
    }

    expect_end_of_statement();

    ASTNode* node = create_var_declaration_node(name, type, init_expr, loc);
    free(name);
    return node;
}

ASTNode* parse_assignment(char* name, SourceLocation loc) {
    eat(ASSIGNMENT);
    ASTNode* value = parse_expression();
    expect_end_of_statement();
    return create_assignment_node(name, value, loc);
}

// ==================== log ====================

ASTNode* parse_log(void) {
    SourceLocation loc = current_location();
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
            // variable reference: its type is filled in by the checker
            element->type = NODE_VARIABLE;
            element->value.string = strdup(yylval.string);
            element->var_type = TYPE_ERROR;
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

    return create_log_node(head, loc);
}
