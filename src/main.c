#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <sys/stat.h>

#include "types.h"
#include "ast.h"
#include "lexer.h"
#include "parser.h"
#include "checker.h"
#include "codegen.h"
#include "diagnostics.h"
#include "language/type_registry.h"
#include "language/token_registry.h"

#define DEFAULT_OUTPUT_DIR "build"

// build/<name>.c for input path .../<name>.w
static char* default_output_path(const char* input_path) {
    const char* name = strrchr(input_path, '/');
    name = name ? name + 1 : input_path;

    size_t name_len = strlen(name);
    if (name_len > 2 && strcmp(name + name_len - 2, ".w") == 0) {
        name_len -= 2;
    }

    size_t size = strlen(DEFAULT_OUTPUT_DIR) + 1 + name_len + 3;
    char* path = malloc(size);
    snprintf(path, size, DEFAULT_OUTPUT_DIR "/%.*s.c", (int)name_len, name);
    return path;
}

int main(int argc, char* argv[]) {
    if (argc != 2 && argc != 3) {
        fprintf(stderr, "Usage: %s input.w [output.c]   (default output: " DEFAULT_OUTPUT_DIR "/<input>.c)\n", argv[0]);
        return 1;
    }

    FILE* input = fopen(argv[1], "r");
    if (!input) {
        perror("Error opening input file.");
        return 1;
    }

    char* output_path;
    if (argc == 3) {
        output_path = strdup(argv[2]);
    } else {
        mkdir(DEFAULT_OUTPUT_DIR, 0755);   // fails harmlessly if it exists
        output_path = default_output_path(argv[1]);
    }

    FILE* output = fopen(output_path, "w");
    if (!output) {
        perror("Error opening output file.");
        free(output_path);
        fclose(input);
        return 1;
    }
    free(output_path);

    type_registry_init();
    token_registry_init();

    // lex + parse -> check -> generate. Each stage runs only if the ones before it
    // reported no errors, so a syntax error never cascades into type errors.
    yyin = input;
    ast = parse();
    if (ast && error_count() == 0) {
        check_program(ast);
    }
    if (ast && error_count() == 0) {
        generate_code(output, ast);
    }

    if (!ast && error_count() == 0) {
        fprintf(stderr, "Failed to parse program.\n");
    }
    bool failed = !ast || error_count() > 0;

    free_ast(ast);
    fclose(input);
    fclose(output);

    token_registry_cleanup();
    type_registry_cleanup();

    return failed ? 1 : 0;
}
