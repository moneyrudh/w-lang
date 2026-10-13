#include "checker.h"
#include "checker/internal.h"
#include "diagnostics.h"
#include "language/type_registry.h"

// ==================== signatures ====================

void declare_functions(ASTNode* functions) {
    for (ASTNode* function = functions; function != NULL; function = function->next) {
        if (!declare_function(function)) {
            report_error(function->location.line, "Function '%s' already declared",
                         function->data.function.name);
        }
    }
}

// ==================== definitions ====================

void check_function(ASTNode* function) {
    const char* name = function->data.function.name;
    DataType return_type = function->data.function.return_type;
    int line = function->location.line;

    checker_context.function = function;
    checker_context.has_return = false;
    checker_context.loop_depth = 0;

    // parameters and the body share one scope (as in C)
    push_scope(line);
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

    if (return_type != TYPE_ZIL && !checker_context.has_return) {
        report_error(line, "Function '%s' with return type '%s' must return a value",
                     name, get_wlang_type_from_enum(return_type));
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
            report_error(line, "Function '%s' declared as void, cannot return a value", name);
        }
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

    // arguments are checked even when the call itself is wrong, so their errors still show
    DataType arg_types[arg_count > 0 ? arg_count : 1];
    for (int i = 0; i < arg_count; i++) {
        arg_types[i] = check_expression(node->data.function_call.args[i]);
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
            if (arg_types[i] != TYPE_ERROR && !compare_types(param->type, arg_types[i])) {
                report_error(line, "Argument %d of '%s': cannot pass %s as %s",
                             i + 1, name, get_wlang_type_from_enum(arg_types[i]),
                             get_wlang_type_from_enum(param->type));
            }
        }
    }

    return function->data.function.return_type;
}
