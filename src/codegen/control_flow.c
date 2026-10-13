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

// " {\n" body "}" (no trailing newline, so 'else' can follow on the same line)
static void generate_body(FILE* output, ASTNode* body, int indent_level) {
    fprintf(output, C_LBRACE);
    generate_block(output, body, indent_level + 1);
    emit_indent(output, indent_level);
    fprintf(output, C_RBRACE_INLINE);
}

void generate_if(FILE* output, ASTNode* node, int indent_level) {
    emit_indent(output, indent_level);
    fprintf(output, C_IF C_SPACE);
    generate_condition(output, node->data.if_statement.condition, node->data.if_statement.negated);
    generate_body(output, node->data.if_statement.body, indent_level);

    for (ASTNode* branch = node->data.if_statement.else_branch; branch != NULL;
         branch = branch->data.if_statement.else_branch) {
        fprintf(output, C_SPACE C_ELSE);
        if (branch->data.if_statement.condition) {
            fprintf(output, C_SPACE C_IF C_SPACE);
            generate_condition(output, branch->data.if_statement.condition, branch->data.if_statement.negated);
        }
        generate_body(output, branch->data.if_statement.body, indent_level);
    }

    fprintf(output, C_NEWLINE);
}

void generate_loop(FILE* output, ASTNode* node, int indent_level) {
    emit_indent(output, indent_level);
    fprintf(output, C_WHILE C_SPACE);

    if (node->data.loop_statement.condition) {
        generate_condition(output, node->data.loop_statement.condition, node->data.loop_statement.negated);
    } else {
        fprintf(output, C_LPAREN C_TRUE C_RPAREN);   // loop forever
    }

    generate_body(output, node->data.loop_statement.body, indent_level);
    fprintf(output, C_NEWLINE);
}

void generate_loop_jump(FILE* output, ASTNode* node, int indent_level) {
    emit_indent(output, indent_level);
    fprintf(output, "%s" C_SEMICOLON_NL, node->type == NODE_SKIP ? C_CONTINUE : C_BREAK);
}
