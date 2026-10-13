#include <stdlib.h>
#include <string.h>

#include "lexer.h"
#include "parser.h"
#include "parser/internal.h"
#include "language/operators.h"

// ==================== operands ====================

static ASTNode* parse_factor(void) {
    SourceLocation loc = {yylineno, 0, NULL};
    switch (token) {
        case INT_LITERAL:
            {
                ASTNode* node = create_number_node(yylval.number, loc);
                eat(INT_LITERAL);
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
        case STRING_LITERAL:
            {
                ASTNode* node = create_string_node(yylval.string, loc);
                eat(STRING_LITERAL);
                return node;
            }
        case IDENTIFIER:
            {
                char* name = strdup(yylval.string);
                eat(IDENTIFIER);

                // a name followed by '(' is a call, otherwise a variable reference
                ASTNode* node = (token == LPAREN)
                    ? parse_function_call(name, loc)
                    : create_variable_node(name, loc);
                free(name);
                return node;
            }
        case LPAREN:
            {
                eat(LPAREN);
                ASTNode* node = parse_expression();
                eat(RPAREN);
                return node;
            }
        case NOT:
        case BANG:
            {
                // 'not' and '!' are both unary negation
                eat(token);
                ASTNode* operand = parse_factor();
                return create_unary_expr_node(OP_NOT, operand, loc);
            }
        default:
            parser_error("Unexpected token in factor");
            return NULL;
    }
}

// ==================== binary operators ====================

// operator tokens accepted at each precedence level (lowest to highest)
static const TokenType OR_TOKENS[]         = {OR};
static const TokenType AND_TOKENS[]        = {AND};
static const TokenType EQUALITY_TOKENS[]   = {EQUAL, EQ, IS, NOT_EQUAL, NE};
static const TokenType COMPARISON_TOKENS[] = {LESS, LT, GREATER, GT, LESS_EQUAL, LE, GREATER_EQUAL, GE};
static const TokenType ADDITIVE_TOKENS[]   = {PLUS, MINUS};
static const TokenType TERM_TOKENS[]       = {MULTIPLY, DIVIDE};

#define TOKEN_COUNT(arr) (sizeof(arr) / sizeof((arr)[0]))

static bool token_in(TokenType tok, const TokenType* set, size_t count) {
    for (size_t i = 0; i < count; i++) {
        if (set[i] == tok) return true;
    }
    return false;
}

// parses a left-associative chain: operand (op operand)*
static ASTNode* parse_binary_level(ASTNode* (*parse_operand)(void), const TokenType* ops, size_t op_count) {
    SourceLocation loc = {yylineno, 0, NULL};
    ASTNode* node = parse_operand();
    while (token_in(token, ops, op_count)) {
        TokenType op_token = token;
        OperatorType op;
        token_to_operator(op_token, &op);
        eat(op_token);

        // 'is not' -> !=  ('is !x' stays ==, the '!' is parsed as unary in parse_factor)
        if (op_token == IS && token == NOT) {
            eat(NOT);
            op = OP_NE;
        }

        node = create_binary_expr_node(node, parse_operand(), op, loc);
    }
    return node;
}

static ASTNode* parse_term(void) {
    return parse_binary_level(parse_factor, TERM_TOKENS, TOKEN_COUNT(TERM_TOKENS));
}

static ASTNode* parse_additive(void) {
    return parse_binary_level(parse_term, ADDITIVE_TOKENS, TOKEN_COUNT(ADDITIVE_TOKENS));
}

static ASTNode* parse_comparison(void) {
    return parse_binary_level(parse_additive, COMPARISON_TOKENS, TOKEN_COUNT(COMPARISON_TOKENS));
}

static ASTNode* parse_equality(void) {
    return parse_binary_level(parse_comparison, EQUALITY_TOKENS, TOKEN_COUNT(EQUALITY_TOKENS));
}

static ASTNode* parse_and(void) {
    return parse_binary_level(parse_equality, AND_TOKENS, TOKEN_COUNT(AND_TOKENS));
}

static ASTNode* parse_or(void) {
    return parse_binary_level(parse_and, OR_TOKENS, TOKEN_COUNT(OR_TOKENS));
}

ASTNode* parse_expression(void) {
    return parse_or();
}
