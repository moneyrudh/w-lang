#include <stdio.h>
#include <stdbool.h>

#include "parser.h"
#include "parser/control_flow.h"
#include "symbol_table.h"
#include "transpiler/token_registry.h"

// ==================== shared helpers ====================

static void check_condition(ASTNode* condition) {
    if (!condition) return;

    DataType type = get_expression_type(condition, getSymbolTable());
    if (type == TYPE_STR) {
        parser_error("A condition can't be a str - compare it instead (e.g. name is \"bob\")");
    } else if (type == TYPE_ZIL && condition->type == NODE_FUNCTION_CALL) {
        parser_error("A condition can't be a call to a zil function");
    }
}

// the body after a condition: an indented block, or ", statement" on the same line.
// a comma before an indented block is allowed. after 'else' / 'loop forever' there's
// no condition to end, so the comma is optional there too.
static ASTNode* parse_body(bool comma_required) {
    bool has_comma = false;
    if (token == COMMA) {
        eat(COMMA);
        has_comma = true;
    }

    if (token == NEWLINE) {
        return parse_block();
    }

    if (comma_required && !has_comma) {
        parser_error("Expected ',' or a new line after the condition");
    }

    // one-line body: its own scope, like a block
    push_scope(getSymbolTable());
    ASTNode* statement = parse_statement();
    pop_scope(getSymbolTable());
    return statement;
}

// ==================== conditionals ====================

static ASTNode* parse_conditional_branch(bool negated, SourceLocation loc) {
    ASTNode* condition = parse_expression();
    check_condition(condition);
    ASTNode* body = parse_body(true);
    return create_if_node(condition, negated, body, loc);
}

// elf / elf lest / nor / else
static ASTNode* parse_else_branch(void) {
    SourceLocation loc = {token_line, 0, NULL};

    switch (token) {
        case ELSE:
            eat(ELSE);
            return create_if_node(NULL, false, parse_body(false), loc);
        case NOR:
            eat(NOR);
            return parse_conditional_branch(true, loc);
        case ELF:
            eat(ELF);
            switch (token) {
                case LEST:
                    eat(LEST);
                    return parse_conditional_branch(true, loc);
                case NOR:
                    parser_error("'elf nor' isn't valid - use 'nor' or 'elf lest'");
                    eat(NOR);
                    return parse_conditional_branch(true, loc);
                case SAY:
                case WHEN: {
                    char error_msg[100];
                    snprintf(error_msg, sizeof(error_msg), "'elf %s' isn't valid - just use 'elf'",
                             token_registry_get_lexeme(token));
                    parser_error(error_msg);
                    eat(token);
                    return parse_conditional_branch(false, loc);
                }
                default:
                    return parse_conditional_branch(false, loc);
            }
        default:
            return NULL;
    }
}

ASTNode* parse_conditional(void) {
    SourceLocation loc = {token_line, 0, NULL};
    bool negated = (token == LEST);
    eat(token);   // SAY, WHEN or LEST

    ASTNode* head = parse_conditional_branch(negated, loc);
    ASTNode* tail = head;

    // the chain continues while the next line (same indentation) starts with elf / nor / else
    while (tail && (token == ELF || token == NOR || token == ELSE)) {
        ASTNode* branch = parse_else_branch();
        tail->data.if_statement.else_branch = branch;
        tail = branch;

        // 'else' ends the chain; anything after it is a stray branch
        if (branch && branch->data.if_statement.condition == NULL) break;
    }

    return head;
}

ASTNode* parse_stray_branch(void) {
    char error_msg[100];
    snprintf(error_msg, sizeof(error_msg), "'%s' needs a 'say', 'when' or 'lest' before it",
             token_registry_get_lexeme(token));
    parser_error(error_msg);

    // parse it anyway so its body is checked and the rest of the file lines up
    free_ast(parse_else_branch());
    return NULL;
}

// ==================== loops ====================

ASTNode* parse_loop(void) {
    SourceLocation loc = {token_line, 0, NULL};
    eat(LOOP);

    ASTNode* condition = NULL;
    bool negated = false;
    bool comma_required = true;

    switch (token) {
        case WHILE:
            eat(WHILE);
            condition = parse_expression();
            break;
        case UNTIL:
            eat(UNTIL);
            negated = true;
            condition = parse_expression();
            break;
        case FOREVER:
            eat(FOREVER);
            comma_required = false;
            break;
        default:
            parser_error("Expected 'while', 'until' or 'forever' after 'loop'");
            if (token != NEWLINE && token != COMMA) {
                condition = parse_expression();
            }
            break;
    }

    check_condition(condition);

    parser_state.loop_depth++;
    ASTNode* body = parse_body(comma_required);
    parser_state.loop_depth--;

    return create_loop_node(condition, negated, body, loc);
}

ASTNode* parse_loop_jump(void) {
    SourceLocation loc = {token_line, 0, NULL};
    NodeType type = (token == SKIP) ? NODE_SKIP : NODE_STOP;

    if (parser_state.loop_depth == 0) {
        char error_msg[100];
        snprintf(error_msg, sizeof(error_msg), "'%s' can only be used inside a loop",
                 token_registry_get_lexeme(token));
        parser_error(error_msg);
    }

    eat(token);   // SKIP or STOP
    expect_end_of_statement();
    return create_jump_node(type, loc);
}
