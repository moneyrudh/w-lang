#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

#include "driver.h"
#include "version.h"

// exit code for a bad command line (W errors exit 1; `run` exits with the program's code)
#define EXIT_USAGE 2

#define USAGE \
    "Usage:\n" \
    "  wlang <file.w> [args...]          build and run (same as wlang run)\n" \
    "  wlang run <file.w> [args...]      build and run; exits with w()'s return value\n" \
    "  wlang build <file.w>              build an executable, ./<name> by default\n" \
    "  wlang check <file.w>              report errors only; writes nothing\n" \
    "\n" \
    "Options:\n" \
    "  -o <path>          build: executable path\n" \
    "  --emit-c [path.c]  build: also keep the generated C (default: <executable>.c)\n" \
    "  --cc <compiler>    run, build: C compiler (default: $CC, else cc)\n" \
    "  -h, --help         show this help\n" \
    "  -v, --version      show the version\n" \
    "\n" \
    "Arguments after the .w file are passed to the program.\n"

typedef enum { CMD_RUN, CMD_BUILD, CMD_CHECK } Command;

typedef struct {
    Command command;
    const char* file;
    const char* output;      // -o
    bool emit_c;             // --emit-c
    const char* c_output;    // --emit-c <path.c>
    const char* cc;          // --cc
    char** program_args;     // run: everything after the file
    int program_arg_count;
} Options;

static int usage_error(const char* format, const char* arg) {
    fprintf(stderr, "wlang: ");
    fprintf(stderr, format, arg);
    fprintf(stderr, "\nRun 'wlang --help' for usage.\n");
    return EXIT_USAGE;
}

static bool ends_with(const char* s, const char* suffix) {
    size_t s_len = strlen(s), suffix_len = strlen(suffix);
    return s_len >= suffix_len && strcmp(s + s_len - suffix_len, suffix) == 0;
}

// "dir/hello.w" -> "hello", malloc'd
static char* program_name(const char* path) {
    const char* name = strrchr(path, '/');
    name = name ? name + 1 : path;
    size_t len = strlen(name) - strlen(".w");
    char* result = malloc(len + 1);
    memcpy(result, name, len);
    result[len] = '\0';
    return result;
}

// fills opts from argv; returns 0, or the exit code after reporting a bad command line
static int parse_args(int argc, char** argv, Options* opts) {
    memset(opts, 0, sizeof(*opts));

    // the command is optional: `wlang hello.w` means `wlang run hello.w`
    int i = 1;
    opts->command = CMD_RUN;
    if (strcmp(argv[1], "run") == 0) {
        i = 2;
    } else if (strcmp(argv[1], "build") == 0) {
        opts->command = CMD_BUILD;
        i = 2;
    } else if (strcmp(argv[1], "check") == 0) {
        opts->command = CMD_CHECK;
        i = 2;
    } else if (argv[1][0] != '-' && !ends_with(argv[1], ".w")) {
        return usage_error("unknown command '%s'", argv[1]);
    }

    for (; i < argc; i++) {
        const char* arg = argv[i];
        bool has_value = i + 1 < argc;

        if (strcmp(arg, "-o") == 0) {
            if (opts->command != CMD_BUILD) return usage_error("%s only works with 'build'", arg);
            if (!has_value) return usage_error("%s needs a path", arg);
            opts->output = argv[++i];
        } else if (strcmp(arg, "--emit-c") == 0) {
            if (opts->command != CMD_BUILD) return usage_error("%s only works with 'build'", arg);
            opts->emit_c = true;
            // the path is optional; it's taken only when it names a .c file
            if (has_value && ends_with(argv[i + 1], ".c")) {
                opts->c_output = argv[++i];
            }
        } else if (strcmp(arg, "--cc") == 0) {
            if (opts->command == CMD_CHECK) return usage_error("%s doesn't apply to 'check'", arg);
            if (!has_value) return usage_error("%s needs a compiler", arg);
            opts->cc = argv[++i];
        } else if (arg[0] == '-') {
            return usage_error("unknown option '%s'", arg);
        } else if (opts->file) {
            return usage_error("unexpected argument '%s'", arg);
        } else if (!ends_with(arg, ".w") || strlen(arg) == strlen(".w")) {
            return usage_error("expected a .w file, got '%s'", arg);
        } else {
            opts->file = arg;
            if (opts->command == CMD_RUN) {
                // the rest belongs to the program
                opts->program_args = argv + i + 1;
                opts->program_arg_count = argc - i - 1;
                break;
            }
        }
    }

    if (!opts->file) {
        return usage_error("%s", "no .w file given");
    }
    if (!opts->cc) {
        opts->cc = default_c_compiler();
    }
    return 0;
}

// transpiles file into c_path; the file is removed again if there were errors
static bool transpile_to(const char* file, const char* c_path) {
    FILE* output = fopen(c_path, "w");
    if (!output) {
        perror("wlang: can't write C output");
        return false;
    }
    bool ok = transpile(file, output);
    fclose(output);
    if (!ok) {
        remove(c_path);
    }
    return ok;
}

static int command_check(const Options* opts) {
    return transpile(opts->file, NULL) ? 0 : 1;
}

static int command_run(const Options* opts) {
    char* dir = make_temp_dir();
    if (!dir) {
        return 1;
    }

    char* name = program_name(opts->file);
    char* exe_path = path_join(dir, name);
    char* c_path = path_join(dir, "main.c");

    int code = 1;
    if (transpile_to(opts->file, c_path) && compile_c(opts->cc, c_path, exe_path)) {
        code = run_program(exe_path, opts->program_args, opts->program_arg_count);
        if (code == -1) {
            code = 1;
        }
    }

    remove_temp_dir(dir);
    free(c_path);
    free(exe_path);
    free(name);
    free(dir);
    return code;
}

static int command_build(const Options* opts) {
    char* exe_path = opts->output ? strdup(opts->output) : program_name(opts->file);

    // the C goes where --emit-c says, else into a temp dir that's removed afterwards
    char* dir = NULL;
    char* c_path;
    if (opts->c_output) {
        c_path = strdup(opts->c_output);
    } else if (opts->emit_c) {
        c_path = malloc(strlen(exe_path) + strlen(".c") + 1);
        sprintf(c_path, "%s.c", exe_path);
    } else {
        dir = make_temp_dir();
        if (!dir) {
            free(exe_path);
            return 1;
        }
        c_path = path_join(dir, "main.c");
    }

    bool ok = transpile_to(opts->file, c_path) && compile_c(opts->cc, c_path, exe_path);

    if (dir) {
        remove_temp_dir(dir);
        free(dir);
    }
    free(c_path);
    free(exe_path);
    return ok ? 0 : 1;
}

int main(int argc, char** argv) {
    if (argc < 2) {
        fprintf(stderr, USAGE);
        return EXIT_USAGE;
    }
    if (strcmp(argv[1], "-h") == 0 || strcmp(argv[1], "--help") == 0 || strcmp(argv[1], "help") == 0) {
        printf(USAGE);
        return 0;
    }
    if (strcmp(argv[1], "-v") == 0 || strcmp(argv[1], "--version") == 0) {
        printf("wlang " WLANG_VERSION "\n");
        return 0;
    }

    Options opts;
    int status = parse_args(argc, argv, &opts);
    if (status != 0) {
        return status;
    }

    switch (opts.command) {
        case CMD_RUN:   return command_run(&opts);
        case CMD_BUILD: return command_build(&opts);
        case CMD_CHECK: return command_check(&opts);
    }
    return 1;
}
