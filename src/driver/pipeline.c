#include <stdio.h>
#include <string.h>
#include <errno.h>

#include "driver.h"
#include "ast.h"
#include "lexer.h"
#include "parser.h"
#include "checker.h"
#include "codegen.h"
#include "diagnostics.h"
#include "language/type_registry.h"
#include "language/token_registry.h"

bool transpile(const char* input_path, FILE* output) {
    FILE* input = fopen(input_path, "r");
    if (!input) {
        fprintf(stderr, "wlang: can't open '%s': %s\n", input_path, strerror(errno));
        return false;
    }

    type_registry_init();
    token_registry_init();

    // lex + parse -> check -> generate. Each stage runs only if the ones before it
    // reported no errors, so a syntax error never cascades into type errors.
    yyin = input;
    ast = parse();
    if (ast && error_count() == 0) {
        check_program(ast);
    }
    if (ast && error_count() == 0 && output) {
        generate_code(output, ast);
    }

    if (!ast && error_count() == 0) {
        fprintf(stderr, "Failed to parse program.\n");
    }
    bool ok = ast && error_count() == 0;

    free_ast(ast);
    ast = NULL;
    fclose(input);

    token_registry_cleanup();
    type_registry_cleanup();

    return ok;
}
