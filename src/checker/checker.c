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

    // pass 1: globals, then every signature, so a function can use a global, and a call can
    // come before the function's definition, wherever they appear in the file
    for (ASTNode* global = program->data.program.globals; global != NULL; global = global->next) {
        check_global(global);
    }
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
