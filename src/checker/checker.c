#include <string.h>

#include "checker.h"
#include "checker/internal.h"
#include "diagnostics.h"

CheckerContext checker_context;

static ASTNode* find_entry_point(ASTNode* functions) {
    for (ASTNode* function = functions; function != NULL; function = function->next) {
        if (strcmp(function->data.function.name, "w") == 0) {
            return function;
        }
    }
    return NULL;
}

// w() becomes C's main(): no parameters, and its result is the exit code (none means 0)
static void check_entry_point(ASTNode* entry) {
    int line = entry->location.line;
    if (entry->data.function.param_count > 0) {
        report_error(line, "The entry point w() takes no parameters");
    }
    DataType type = entry->data.function.return_type;
    if (type != TYPE_NUM && type != TYPE_ZIL) {
        report_error(line, "The entry point w() must return num (the exit code) or nothing");
    }
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

    ASTNode* entry = find_entry_point(functions);
    if (entry) {
        check_entry_point(entry);
    } else {
        report_error(0, "Entry point function 'w()' not defined");
    }

    // pass 2: bodies
    for (ASTNode* function = functions; function != NULL; function = function->next) {
        check_function(function);
    }

    free_scopes();
}
