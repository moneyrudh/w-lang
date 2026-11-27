#include <string.h>

#include "checker.h"
#include "checker/internal.h"
#include "diagnostics.h"
#include "language/type_registry.h"

// ==================== signatures ====================

void declare_functions(ASTNode* functions) {
    for (ASTNode* function = functions; function != NULL; function = function->next) {
        int line = function->location.line;
        check_type_known(&function->data.function.return_type, line);
        for (Parameter* p = function->data.function.parameters; p; p = p->next) {
            check_type_known(&p->type, line);
        }
        if (find_pod(function->data.function.name)) {
            report_error(line, "'%s' is already a pod's name", function->data.function.name);
        } else if (!declare_function(function)) {
            report_error(function->location.line, "Function '%s' already declared",
                         function->data.function.name);
        }
    }
}

// ==================== definitions ====================

// ==================== return paths ====================
//
// A function with a return type must return on every path: running off the end of its body
// would leave the caller with no value (undefined behaviour in C). Recursion here follows
// statement nesting, which the parser limits.

static bool always_returns(ASTNode* statements);

// a `stop` that leaves this loop (one inside a nested loop belongs to that loop)
static bool stops_loop(ASTNode* statements) {
    for (ASTNode* s = statements; s != NULL; s = s->next) {
        switch (s->type) {
            case NODE_STOP:
                return true;
            case NODE_IF:
                for (ASTNode* branch = s; branch != NULL; branch = branch->data.if_statement.else_branch) {
                    if (stops_loop(branch->data.if_statement.body)) return true;
                }
                break;
            default:
                break;
        }
    }
    return false;
}

// `loop forever`, or a loop whose condition is a literal that never ends it
// (`loop while true`, `loop until false`)
static bool loops_forever(ASTNode* loop) {
    ASTNode* condition = loop->data.loop_statement.condition;
    if (condition == NULL) return true;
    return condition->type == NODE_BOOL &&
           condition->data.bool_val.value != loop->data.loop_statement.negated;
}

static bool statement_always_returns(ASTNode* s) {
    switch (s->type) {
        case NODE_RETURN:
            return true;
        case NODE_IF:
            // every branch returns, and there's an `else`, so some branch always runs
            for (ASTNode* branch = s; branch != NULL; branch = branch->data.if_statement.else_branch) {
                if (!always_returns(branch->data.if_statement.body)) return false;
                if (branch->data.if_statement.else_branch == NULL) {
                    return branch->data.if_statement.condition == NULL;
                }
            }
            return false;
        case NODE_LOOP:
            // a loop that never ends by itself only ends through a stop (or a ret inside it)
            return loops_forever(s) && !stops_loop(s->data.loop_statement.body);
        default:
            return false;
    }
}

static bool always_returns(ASTNode* statements) {
    for (ASTNode* s = statements; s != NULL; s = s->next) {
        if (statement_always_returns(s)) return true;
    }
    return false;
}

void check_function(ASTNode* function) {
    const char* name = function->data.function.name;
    DataType return_type = function->data.function.return_type;
    int line = function->location.line;

    checker_context.function = function;
    checker_context.has_return = false;
    checker_context.loop_depth = 0;

    // parameters and the body share one scope (as in C)
    push_scope();
    for (Parameter* param = function->data.function.parameters; param != NULL; param = param->next) {
        if (param->type == TYPE_ZIL) {
            report_error(line, "Parameter '%s' can't be zil", param->name);
        }
        if (!declare_variable(param->name, param->type)) {
            report_error(line, "Duplicate parameter name");
        }
    }
    check_statements(function->data.function.body);
    pop_scope();

    // a w() whose return type was already rejected (checker.c) gets no second error about it
    bool rejected_entry_point = strcmp(name, "w") == 0 && return_type != TYPE_NUM && return_type != TYPE_ZIL;

    if (rejected_entry_point || return_type == TYPE_ERROR) {
        // reported once already (the entry point's rules, or an unknown type)
    } else if (return_type != TYPE_ZIL && !checker_context.has_return) {
        report_error(line, "Function '%s' with return type '%s' must return a value",
                     name, get_wlang_type_from_enum(return_type));
    } else if (return_type != TYPE_ZIL && !always_returns(function->data.function.body)) {
        report_error(line, "Function '%s' with return type '%s' doesn't return a value on every path "
                     "(it can reach the end of its body)", name, get_wlang_type_from_enum(return_type));
    }

    checker_context.function = NULL;
}

void check_return(ASTNode* node) {
    ASTNode* function = checker_context.function;
    const char* name = function->data.function.name;
    DataType return_type = function->data.function.return_type;
    ASTNode* value = node->data.return_statement.expression;
    int line = node->location.line;

    checker_context.has_return = true;

    if (return_type == TYPE_ZIL) {
        if (value) {
            check_expression(value);
            report_error(line, "Function '%s' returns nothing (no return type), so 'ret' can't have a value", name);
        }
        return;
    }

    if (return_type == TYPE_ERROR) {   // an unknown return type, already reported
        if (value) check_expression(value);
        return;
    }

    if (!value) {
        report_error(line, "Function '%s' with return type '%s' must return a value",
                     name, get_wlang_type_from_enum(return_type));
        return;
    }

    DataType value_type = check_expression(value);
    if (value_type != TYPE_ERROR && value_type != return_type) {
        report_error(line, "Return type mismatch in function '%s'. Expected %s, got %s",
                     name, get_wlang_type_from_enum(return_type), get_wlang_type_from_enum(value_type));
    }
}

// ==================== calls ====================

DataType check_call(ASTNode* node) {
    const char* name = node->data.function_call.name;
    int arg_count = node->data.function_call.arg_count;
    int line = node->location.line;

    // Name(field: value, ...) is a pod value (it checks its own values)
    ASTNode* pod = find_pod(name);
    if (pod) {
        return check_pod_literal(node, pod);
    }

    // arguments are checked even when the call itself is wrong, so their errors still show
    DataType arg_types[arg_count > 0 ? arg_count : 1];
    for (int i = 0; i < arg_count; i++) {
        arg_types[i] = check_expression(node->data.function_call.args[i]);
    }

    if (node->data.function_call.arg_names) {
        report_error(line, "Named values (name: value) are only for pod values, and '%s' isn't a pod", name);
        return TYPE_ERROR;
    }

    FunctionSymbol* symbol = lookup_function(name);
    if (!symbol) {
        report_error(line, "Undefined function: '%s'", name);
        return TYPE_ERROR;
    }

    ASTNode* function = symbol->function;
    int param_count = function->data.function.param_count;
    if (arg_count != param_count) {
        report_error(line, "Function '%s' takes %d argument%s, got %d",
                     name, param_count, param_count == 1 ? "" : "s", arg_count);
    } else {
        Parameter* param = function->data.function.parameters;
        for (int i = 0; i < arg_count; i++, param = param->next) {
            if (arg_types[i] != TYPE_ERROR && param->type != TYPE_ERROR && !compare_types(param->type, arg_types[i])) {
                report_error(line, "Argument %d of '%s': cannot pass %s as %s",
                             i + 1, name, get_wlang_type_from_enum(arg_types[i]),
                             get_wlang_type_from_enum(param->type));
            }
        }
    }

    return function->data.function.return_type;
}
