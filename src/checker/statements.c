#include <limits.h>

#include "checker.h"
#include "checker/internal.h"
#include "diagnostics.h"
#include "language/type_registry.h"

// ==================== variables ====================

static void check_var_declaration(ASTNode* node) {
    const char* name = node->data.var_declaration.name;
    DataType type = node->data.var_declaration.type;
    int line = node->location.line;

    // the initializer is checked first: it can't see the variable it initializes
    ASTNode* init = node->data.var_declaration.init_expr;
    if (type == TYPE_INFER) {
        // dec x := value: the variable takes the value's type
        type = check_expression(init);
        if (type == TYPE_ZIL) {
            report_error(line, "Variable '%s' has no type to take: the call returns nothing (zil)", name);
            type = TYPE_ERROR;
        }
        node->data.var_declaration.type = type;
    } else if (init) {
        DataType init_type = check_expression(init);
        if (init_type != TYPE_ERROR && !compare_types(type, init_type)) {
            report_error(line, "Type mismatch in initialization: cannot assign %s to %s",
                         get_wlang_type_from_enum(init_type), get_wlang_type_from_enum(type));
        }
    }

    if (type == TYPE_ZIL) {
        report_error(line, "Variable '%s' can't be zil", name);
    }
    // (an inferred type of TYPE_ERROR is declared as is: uses of it report nothing more)

    // `dec n := n + 1` in an inner block reads the outer n. In C the new variable is already in
    // scope in its own initializer, so codegen has to compute the value before declaring it
    if (lookup_variable(name)) {
        node->data.var_declaration.shadows_outer = true;
    }

    // declared even after a type error, so later uses don't cascade into "undefined" errors
    if (!declare_variable(name, type)) {
        report_error(line, "Variable already declared in this scope");
    }
}

// ==================== globals ====================

// a global's value is computed when the program starts, before any function runs, and C
// requires a constant there: literals and operators only (no variables, no calls)
static bool uses_names(ASTNode* expr) {
    if (!expr) return false;
    switch (expr->type) {
        case NODE_VARIABLE:
        case NODE_FUNCTION_CALL:
            return true;
        case NODE_BINARY_EXPR:
            return uses_names(expr->data.binary_expr.left) || uses_names(expr->data.binary_expr.right);
        case NODE_UNARY_EXPR:
            return uses_names(expr->data.unary_expr.operand);
        default:
            return false;
    }
}

// after type checking: an operation on str becomes a call in C (strcmp), so it isn't constant
static bool has_text_operation(ASTNode* expr) {
    if (!expr) return false;
    switch (expr->type) {
        case NODE_BINARY_EXPR:
            return expr->data.binary_expr.left->expr_type == TYPE_STR ||
                   has_text_operation(expr->data.binary_expr.left) ||
                   has_text_operation(expr->data.binary_expr.right);
        case NODE_UNARY_EXPR:
            return has_text_operation(expr->data.unary_expr.operand);
        default:
            return false;
    }
}

// the value of a constant integer expression (num / chr literals and + - * / and unary -),
// or false when it isn't one or overflows (run-time overflow is undecided, D11)
static bool constant_int(ASTNode* expr, long long* out) {
    long long a, b;
    switch (expr->type) {
        case NODE_NUMBER: *out = expr->data.number.value; return true;
        case NODE_CHAR:   *out = expr->data.char_val.value; return true;
        case NODE_UNARY_EXPR:
            if (expr->data.unary_expr.operator != OP_NEG || !constant_int(expr->data.unary_expr.operand, &a)) {
                return false;
            }
            return !__builtin_sub_overflow(0, a, out);
        case NODE_BINARY_EXPR:
            if (!constant_int(expr->data.binary_expr.left, &a) || !constant_int(expr->data.binary_expr.right, &b)) {
                return false;
            }
            switch (expr->data.binary_expr.operator) {
                case OP_ADD: return !__builtin_add_overflow(a, b, out);
                case OP_SUB: return !__builtin_sub_overflow(a, b, out);
                case OP_MUL: return !__builtin_mul_overflow(a, b, out);
                case OP_DIV:
                    if (b == 0 || (a == LLONG_MIN && b == -1)) return false;
                    *out = a / b;
                    return true;
                case OP_MOD:
                    if (b == 0 || (a == LLONG_MIN && b == -1)) return false;
                    *out = a % b;
                    return true;
                default: return false;
            }
        default:
            return false;
    }
}

// a num division (C integer division) by a constant zero: C rejects it in a global's value
static bool divides_by_zero(ASTNode* expr) {
    if (!expr) return false;
    switch (expr->type) {
        case NODE_BINARY_EXPR: {
            long long divisor;
            OperatorType op = expr->data.binary_expr.operator;
            if ((op == OP_DIV || op == OP_MOD) && expr->expr_type == TYPE_NUM &&
                constant_int(expr->data.binary_expr.right, &divisor) && divisor == 0) {
                return true;
            }
            return divides_by_zero(expr->data.binary_expr.left) || divides_by_zero(expr->data.binary_expr.right);
        }
        case NODE_UNARY_EXPR:
            return divides_by_zero(expr->data.unary_expr.operand);
        default:
            return false;
    }
}

#define GLOBAL_NOT_CONSTANT "The value of global '%s' must be a constant: literals and operators, " \
                            "without variables, calls or text operations"

void check_global(ASTNode* node) {
    ASTNode* init = node->data.var_declaration.init_expr;
    const char* name = node->data.var_declaration.name;

    if (uses_names(init)) {
        report_error(node->location.line, GLOBAL_NOT_CONSTANT, name);
        // still declared, so functions using it don't add "undefined" errors
        if (node->data.var_declaration.type == TYPE_INFER) {
            node->data.var_declaration.type = TYPE_ERROR;
        }
        if (!declare_variable(name, node->data.var_declaration.type)) {
            report_error(node->location.line, "Variable already declared in this scope");
        }
        return;
    }

    int errors_before = error_count();
    check_var_declaration(node);
    if (error_count() == errors_before && has_text_operation(init)) {
        report_error(node->location.line, GLOBAL_NOT_CONSTANT, name);
    } else if (error_count() == errors_before && divides_by_zero(init)) {
        report_error(node->location.line, "The value of global '%s' divides by zero", name);
    }
}

static void check_assignment(ASTNode* node) {
    const char* target = node->data.assignment.target;
    int line = node->location.line;

    DataType value_type = check_expression(node->data.assignment.value);
    node->expr_type = value_type;

    Symbol* symbol = lookup_variable(target);
    if (!symbol) {
        report_error(line, "Assignment to undeclared variable '%s'", target);
        return;
    }
    node->data.assignment.target_type = symbol->type;

    // a variable whose type is already an error (dec n := missing) was reported where it was declared
    if (value_type != TYPE_ERROR && symbol->type != TYPE_ERROR && !compare_types(symbol->type, value_type)) {
        report_error(line, "Type mismatch in assignment to '%s': cannot assign %s to %s",
                     target, get_wlang_type_from_enum(value_type), get_wlang_type_from_enum(symbol->type));
    }
}

// ==================== log ====================

// every argument is an ordinary expression; anything with a value can be printed
static void check_log(ASTNode* node) {
    for (int i = 0; i < node->data.log.arg_count; i++) {
        ASTNode* arg = node->data.log.args[i];
        if (!arg) continue;   // a syntax error, already reported
        if (check_expression(arg) == TYPE_ZIL) {
            report_error(arg->location.line, "Can't log a zil value: the call returns nothing");
        }
    }
}

// ==================== statements & blocks ====================

void check_statement(ASTNode* node) {
    switch (node->type) {
        case NODE_VAR_DECLARATION:
            check_var_declaration(node);
            break;
        case NODE_ASSIGNMENT:
            check_assignment(node);
            break;
        case NODE_LOG:
            check_log(node);
            break;
        case NODE_RETURN:
            check_return(node);
            break;
        case NODE_IF:
            check_if(node);
            break;
        case NODE_LOOP:
            check_loop(node);
            break;
        case NODE_SKIP:
        case NODE_STOP:
            check_loop_jump(node);
            break;
        default:
            // an expression used as a statement, e.g. a call
            check_expression(node);
            break;
    }
}

void check_statements(ASTNode* statements) {
    for (ASTNode* statement = statements; statement != NULL; statement = statement->next) {
        check_statement(statement);
    }
}

void check_block(ASTNode* statements) {
    push_scope();
    check_statements(statements);
    pop_scope();
}
