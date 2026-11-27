// num arithmetic in the generated C (DECISIONS.md, D11):
//  - + - * and unary - wrap around on overflow (2147483647 + 1 is -2147483648), like Java and
//    Go. Signed overflow is undefined in C, so they're done on unsigned values and converted
//    back: (int)((unsigned)(a) + (unsigned)(b)). No call, no check: as fast as plain C.
//  - / and % stop the program on a zero divisor, naming the W line, instead of crashing:
//    W__div(a, b, line). -2147483648 / -1 wraps too (to -2147483648).
// real arithmetic is plain C (1.0 / 0 is infinity, as in C).
//
// A global's value is a C constant, so it uses plain operators: C folds them before the
// program starts, and the checker has made sure no constant divisor is zero.

#include <stdio.h>

#include "codegen/internal.h"
#include "codegen/c_syntax.h"

static const char* const ARITHMETIC_RUNTIME =
    "// ---- W run-time errors and whole-number division ----\n"
    "void W__runtime_error(int line, const char* message) {\n"
    "    fflush(stdout);\n"
    "    fprintf(stderr, \"W error on line %d: %s\\n\", line, message);\n"
    "    exit(1);\n"
    "}\n"
    "\n"
    "int W__div(int a, int b, int line) {\n"
    "    if (b == 0) W__runtime_error(line, \"division by zero\");\n"
    "    if (b == -1) return (int)(0u - (unsigned)a);   // -2147483648 / -1 wraps around\n"
    "    return a / b;\n"
    "}\n"
    "\n"
    "int W__mod(int a, int b, int line) {\n"
    "    if (b == 0) W__runtime_error(line, \"modulo by zero\");\n"
    "    if (b == -1) return 0;\n"
    "    return a % b;\n"
    "}\n"
    "// ---- end of W division ----\n\n";

void emit_arithmetic_runtime(FILE* output) {
    fputs(ARITHMETIC_RUNTIME, output);
}

// the value of an operand as the result type, e.g. a chr operand of a num operation
static void generate_arith_operand(FILE* output, ASTNode* operand, DataType result_type) {
    fputs(C_LPAREN, output);
    generate_cast_if_needed(output, operand->expr_type, result_type);
    generate(output, operand, 0);
    fputs(C_RPAREN, output);
}

bool generate_num_arithmetic(FILE* output, ASTNode* node, bool in_global) {
    if (node->expr_type != TYPE_NUM || in_global) return false;

    ASTNode* left = node->data.binary_expr.left;
    ASTNode* right = node->data.binary_expr.right;
    OperatorType op = node->data.binary_expr.operator;

    if (op == OP_DIV || op == OP_MOD) {
        fputs(op == OP_DIV ? C_NUM_DIV C_LPAREN : C_NUM_MOD C_LPAREN, output);
        generate_arith_operand(output, left, TYPE_NUM);
        fputs(C_COMMA, output);
        generate_arith_operand(output, right, TYPE_NUM);
        fprintf(output, C_COMMA "%d" C_RPAREN, node->location.line);
        return true;
    }

    // (int)((unsigned)(a) + (unsigned)(b))
    fputs(C_WRAP_OPEN, output);
    generate_arith_operand(output, left, TYPE_NUM);
    fputs(get_binary_operator_string(op), output);
    fputs(C_TO_UNSIGNED, output);
    generate_arith_operand(output, right, TYPE_NUM);
    fputs(C_WRAP_CLOSE, output);
    return true;
}

void generate_num_negation(FILE* output, ASTNode* operand) {
    // (int)(0u - (unsigned)(x))
    fputs(C_WRAP_NEG_OPEN, output);
    generate_arith_operand(output, operand, TYPE_NUM);
    fputs(C_WRAP_CLOSE, output);
}
