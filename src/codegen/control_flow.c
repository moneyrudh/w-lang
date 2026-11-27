#include "codegen/internal.h"
#include "codegen/c_syntax.h"

// (cond) or (!(cond))
static void generate_condition(FILE* output, ASTNode* condition, bool negated) {
    fprintf(output, C_LPAREN);
    if (negated) fprintf(output, C_NOT_OPEN);
    generate(output, condition, 0);   // indent 0: an expression, not a statement
    if (negated) fprintf(output, C_RPAREN);
    fprintf(output, C_RPAREN);
}

// " {\n" body "}" (no trailing newline, so 'else' can follow on the same line). A body is a
// scope: the str variables declared in it are freed at its end. A loop's body (loop != NULL)
// starts by checking the loop's condition: `if (!(cond)) break;`
static void generate_body(FILE* output, ASTNode* body, int indent_level, ASTNode* loop) {
    fprintf(output, C_LBRACE);
    ASTNode* condition = loop ? loop->data.loop_statement.condition : NULL;
    if (condition) {
        emit_indent(output, indent_level + 1);
        fprintf(output, C_IF C_SPACE);
        // break when the loop should end: `while` ends when it's false, `until` when it's true
        generate_condition(output, condition, !loop->data.loop_statement.negated);
        fprintf(output, C_SPACE C_BREAK C_SEMICOLON_NL);
        if (expression_makes_temps(condition)) {
            strings_free_temps(output, indent_level + 1);   // the condition's temporaries
        }
    }
    strings_open_scope();
    generate_block(output, body, indent_level + 1);
    strings_close_scope(output, indent_level + 1);
    emit_indent(output, indent_level);
    fprintf(output, C_RBRACE_INLINE);
}

void generate_if(FILE* output, ASTNode* node, int indent_level) {
    emit_indent(output, indent_level);
    fprintf(output, C_IF C_SPACE);
    generate_condition(output, node->data.if_statement.condition, node->data.if_statement.negated);
    generate_body(output, node->data.if_statement.body, indent_level, NULL);

    for (ASTNode* branch = node->data.if_statement.else_branch; branch != NULL;
         branch = branch->data.if_statement.else_branch) {
        fprintf(output, C_SPACE C_ELSE);
        if (branch->data.if_statement.condition) {
            fprintf(output, C_SPACE C_IF C_SPACE);
            generate_condition(output, branch->data.if_statement.condition, branch->data.if_statement.negated);
        }
        generate_body(output, branch->data.if_statement.body, indent_level, NULL);
    }

    fprintf(output, C_NEWLINE);
}

// every loop is `while (true) { if (!(cond)) break; ... }`. C lets an optimizer assume that a
// loop whose condition isn't constant, and whose body has no visible effects, ends; then an
// endless W loop (loop while x > 0 with x never changing) could be deleted at -O2. A constant
// condition is never assumed to end. (skip -> continue re-checks the condition at the top.)
void generate_loop(FILE* output, ASTNode* node, int indent_level) {
    emit_indent(output, indent_level);
    fprintf(output, C_WHILE C_SPACE C_LPAREN C_TRUE C_RPAREN);

    strings_begin_loop_body();
    generate_body(output, node->data.loop_statement.body, indent_level, node);
    strings_end_loop_body();
    fprintf(output, C_NEWLINE);
}

void generate_loop_jump(FILE* output, ASTNode* node, int indent_level) {
    strings_free_for_loop_jump(output, indent_level);
    emit_indent(output, indent_level);
    fprintf(output, "%s" C_SEMICOLON_NL, node->type == NODE_SKIP ? C_CONTINUE : C_BREAK);
}
