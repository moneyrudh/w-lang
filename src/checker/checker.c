#include <string.h>

#include "checker.h"
#include "checker/internal.h"
#include "diagnostics.h"

CheckerContext checker_context;

static bool has_entry_point(ASTNode* functions) {
    for (ASTNode* function = functions; function != NULL; function = function->next) {
        if (strcmp(function->data.function.name, "w") == 0) {
            return true;
        }
    }
    return false;
}

void check_program(ASTNode* program) {
    ASTNode* functions = program->data.program.functions;

    init_scopes();
    checker_context.function = NULL;
    checker_context.has_return = false;
    checker_context.loop_depth = 0;

    // pass 1: every signature, so a call can come before the function's definition
    declare_functions(functions);

    if (!has_entry_point(functions)) {
        report_error(0, "Entry point function 'w()' not defined");
    }

    // pass 2: bodies
    for (ASTNode* function = functions; function != NULL; function = function->next) {
        check_function(function);
    }

    free_scopes();
}
