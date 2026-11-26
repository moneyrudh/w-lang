#include <stdio.h>
#include <stdlib.h>
#include <limits.h>
#include <string.h>

#include "lexer.h"
#include "parser.h"
#include "parser/internal.h"
#include "language/operators.h"
#include "language/token_registry.h"

// the smallest real literal that no longer rounds to a finite float (FLT_MAX + 2^103)
#define REAL_LITERAL_LIMIT 3.4028235677973366e+38

// ==================== nesting limit ====================

static int expression_depth = 0;

// true, after reporting it, when one more level would pass MAX_EXPRESSION_DEPTH
static bool too_deep(void) {
    if (expression_depth < MAX_EXPRESSION_DEPTH) return false;
    char error_msg[100];
    snprintf(error_msg, sizeof(error_msg),
             "Expression too deeply nested or too long (more than %d levels)", MAX_EXPRESSION_DEPTH);
    parser_error(error_msg);
    return true;
}

// ==================== operands ====================

static ASTNode* parse_factor(void) {
    SourceLocation loc = current_location();
    switch (token) {
        case INT_LITERAL:
            {
                if (yylval.number > INT_MAX) {
                    parser_error("Number too large for num: the largest is 2147483647");
                }
                ASTNode* node = create_number_node(yylval.number > INT_MAX ? INT_MAX : (int)yylval.number, loc);
                eat(INT_LITERAL);
                return node;
            }
        case FLOAT_LITERAL:
            {
                // real is a C float: anything that rounds to at most FLT_MAX fits. The cutoff is
                // halfway to the next power of two, FLT_MAX + 2^103 (compared as a double, since
                // converting an out-of-range double to float is undefined)
                if (yylval.float_val >= REAL_LITERAL_LIMIT) {
                    parser_error("Number too large for real: the largest is about 3.4e38");
                }
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
                if (too_deep()) return NULL;
                eat(token);
                expression_depth++;
                ASTNode* operand = parse_factor();
                expression_depth--;
                return create_unary_expr_node(OP_NOT, operand, loc);
            }
        case MINUS:
            {
                // unary minus: -x, -3, -(a + b). (Two minus signs need a space, `- -x`:
                // `--` starts a comment.)
                if (too_deep()) return NULL;
                eat(MINUS);

                // a minus written right before a number is part of it: one negative literal.
                // That's how the smallest num, -2147483648, can be written at all
                if (token == INT_LITERAL) {
                    long long magnitude = yylval.number;
                    if (magnitude > -(long long)INT_MIN) {
                        parser_error("Number too small for num: the smallest is -2147483648");
                        magnitude = -(long long)INT_MIN;
                    }
                    eat(INT_LITERAL);
                    return create_number_node((int)-magnitude, loc);
                }

                expression_depth++;
                ASTNode* operand = parse_factor();
                expression_depth--;
                return create_unary_expr_node(OP_NEG, operand, loc);
            }
        default: {
            char error_msg[100];
            snprintf(error_msg, sizeof(error_msg), "Expected a value, got %s", token_registry_describe(token));
            parser_error(error_msg);
            return NULL;
        }
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
    SourceLocation loc = current_location();
    ASTNode* node = parse_operand();

    // each operator in the chain makes the tree one level deeper on the left
    int levels = 0;
    while (token_in(token, ops, op_count)) {
        if (too_deep()) break;
        expression_depth++;
        levels++;

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
    expression_depth -= levels;
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
    if (too_deep()) return NULL;
    expression_depth++;
    ASTNode* node = parse_or();
    expression_depth--;
    return node;
}
