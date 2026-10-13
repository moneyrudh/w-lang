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
    if (init) {
        DataType init_type = check_expression(init);
        if (init_type != TYPE_ERROR && !compare_types(type, init_type)) {
            report_error(line, "Type mismatch in initialization: cannot assign %s to %s",
                         get_wlang_type_from_enum(init_type), get_wlang_type_from_enum(type));
        }
    }

    if (type == TYPE_ZIL) {
        report_error(line, "Variable '%s' can't be zil", name);
    }

    // declared even after a type error, so later uses don't cascade into "undefined" errors
    if (!declare_variable(name, type)) {
        report_error(line, "Variable already declared in this scope");
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

    if (value_type != TYPE_ERROR && !compare_types(symbol->type, value_type)) {
        report_error(line, "Type mismatch in assignment to '%s': cannot assign %s to %s",
                     target, get_wlang_type_from_enum(value_type), get_wlang_type_from_enum(symbol->type));
    }
}

// ==================== log ====================

static void check_log(ASTNode* node) {
    for (LogElement* element = node->data.log.elements; element != NULL; element = element->next) {
        if (element->type != NODE_VARIABLE) continue;

        Symbol* symbol = lookup_variable(element->value.string);
        if (!symbol) {
            report_error(node->location.line, "Undefined variable in log statement: '%s'", element->value.string);
            element->var_type = TYPE_ERROR;
        } else {
            element->var_type = symbol->type;
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
    push_scope(statements ? statements->location.line : 0);
    check_statements(statements);
    pop_scope();
}
