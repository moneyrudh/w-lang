// num arithmetic in the generated C (DECISIONS.md, D11):
//  - + - * and unary - wrap around on overflow (2147483647 + 1 is -2147483648), like Java and
//    Go. Signed overflow is undefined in C, so they're done on unsigned values and converted
//    back: (int)((unsigned)(a) + (unsigned)(b)). No call, no check: as fast as plain C.
//  - / and % stop the program on a zero divisor, naming the W line, instead of crashing:
//    W__div(a, b, line). -2147483648 / -1 wraps too (to -2147483648).
// real arithmetic is plain C (1.0 / 0 is infinity, as in C).
//
// A global's value must be a C constant, so it can't call W__div. + - * use the same wrap form
// (it's a constant expression); / and % become ((b) == -1 ? <wrapped> : (a) / (b)), which is
// constant too. The checker has made sure no constant divisor is zero.

#include <stdio.h>

#include "codegen/internal.h"
#include "codegen/c_syntax.h"

// Traces (D12): every W function pushes a frame (its name, and later the line it's at) when it
// starts and pops it when it returns. Each call passes the caller's line as a hidden first
// argument, which the callee records in the caller's frame, so a run-time error can print the
// whole chain of calls, like Python's traceback.
static const char* const ARITHMETIC_RUNTIME =
    "// ---- W call frames (for traces), run-time errors and whole-number division ----\n"
    "typedef struct { const char* name; int line; } W__frame;\n"
    "static W__frame* W__frames = NULL;\n"
    "static int W__depth = 0, W__frames_capacity = 0;\n"
    "\n"
    "void W__push(const char* name, int call_line) {\n"
    "    if (W__depth > 0) W__frames[W__depth - 1].line = call_line;\n"
    "    if (W__depth == W__frames_capacity) {\n"
    "        W__frames_capacity = W__frames_capacity ? W__frames_capacity * 2 : 64;\n"
    "        W__frames = realloc(W__frames, (size_t)W__frames_capacity * sizeof(W__frame));\n"
    "        if (!W__frames) {\n"
    "            fputs(\"W: out of memory\\n\", stderr);\n"
    "            exit(1);\n"
    "        }\n"
    "    }\n"
    "    W__frames[W__depth].name = name;\n"
    "    W__frames[W__depth].line = 0;\n"
    "    W__depth++;\n"
    "}\n"
    "\n"
    "void W__pop(void) {\n"
    "    W__depth--;\n"
    "}\n"
    "\n"
    "static void W__print_frame(int i) {\n"
    "    fprintf(stderr, \"  %s %s, line %d\\n\", i == W__depth - 1 ? \"in\" : \"called from\",\n"
    "            W__frames[i].name, W__frames[i].line);\n"
    "}\n"
    "\n"
    "// prints the message and the chain of W calls that led to it (innermost first), then stops\n"
    "void W__runtime_error(int line, const char* message) {\n"
    "    fflush(stdout);\n"
    "    fprintf(stderr, \"W error on line %d: %s\\n\", line, message);\n"
    "    if (W__depth > 0) W__frames[W__depth - 1].line = line;\n"
    "    if (W__depth <= 20) {\n"
    "        for (int i = W__depth - 1; i >= 0; i--) W__print_frame(i);\n"
    "    } else {\n"
    "        for (int i = W__depth - 1; i >= W__depth - 10; i--) W__print_frame(i);\n"
    "        fprintf(stderr, \"  ... %d more calls ...\\n\", W__depth - 15);\n"
    "        for (int i = 4; i >= 0; i--) W__print_frame(i);\n"
    "    }\n"
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

// in a global's value: ((b) == -1 ? (int)(0u - (unsigned)(a)) : (a) / (b)), or 0 for %
static void generate_constant_division(FILE* output, ASTNode* left, ASTNode* right, OperatorType op) {
    fputs(C_LPAREN, output);
    generate_arith_operand(output, right, TYPE_NUM);
    fputs(C_CONST_IS_MINUS_ONE, output);
    if (op == OP_DIV) {
        fputs(C_WRAP_NEG_OPEN, output);
        generate_arith_operand(output, left, TYPE_NUM);
        fputs(C_WRAP_CLOSE, output);
    } else {
        fputs(C_ZERO, output);
    }
    fputs(C_CONST_OTHERWISE, output);
    generate_arith_operand(output, left, TYPE_NUM);
    fputs(get_binary_operator_string(op), output);
    generate_arith_operand(output, right, TYPE_NUM);
    fputs(C_RPAREN, output);
}

bool generate_num_arithmetic(FILE* output, ASTNode* node, bool in_global) {
    if (node->expr_type != TYPE_NUM) return false;

    ASTNode* left = node->data.binary_expr.left;
    ASTNode* right = node->data.binary_expr.right;
    OperatorType op = node->data.binary_expr.operator;

    if ((op == OP_DIV || op == OP_MOD) && in_global) {
        generate_constant_division(output, left, right, op);
        return true;
    }
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
