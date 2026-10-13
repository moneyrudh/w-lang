#include "checker/internal.h"
#include "diagnostics.h"
#include "language/token_registry.h"

static void check_condition(ASTNode* condition) {
    // NULL: a plain 'else' or 'loop forever' (or a condition the parser already reported)
    if (!condition) return;

    DataType type = check_expression(condition);
    if (type == TYPE_STR) {
        report_error(condition->location.line, "A condition can't be a str - compare it instead (e.g. name is \"bob\")");
    } else if (type == TYPE_ZIL && condition->type == NODE_FUNCTION_CALL) {
        report_error(condition->location.line, "A condition can't be a call to a zil function");
    }
}

void check_if(ASTNode* node) {
    for (ASTNode* branch = node; branch != NULL; branch = branch->data.if_statement.else_branch) {
        check_condition(branch->data.if_statement.condition);
        check_block(branch->data.if_statement.body);
    }
}

void check_loop(ASTNode* node) {
    check_condition(node->data.loop_statement.condition);

    checker_context.loop_depth++;
    check_block(node->data.loop_statement.body);
    checker_context.loop_depth--;
}

void check_loop_jump(ASTNode* node) {
    if (checker_context.loop_depth == 0) {
        TokenType keyword = (node->type == NODE_SKIP) ? SKIP : STOP;
        report_error(node->location.line, "'%s' can only be used inside a loop",
                     token_registry_get_lexeme(keyword));
    }
}
