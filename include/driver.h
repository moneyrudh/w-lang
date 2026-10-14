#ifndef W_DRIVER_H
#define W_DRIVER_H

#include <stdio.h>
#include <stdbool.h>

// ==================== pipeline.c ====================

// lex, parse and check input_path, then generate C into output (skipped when output
// is NULL). Errors go to stderr. Returns true if none were reported.
// Call at most once per process: the lexer and diagnostics keep global state.
bool transpile(const char* input_path, FILE* output);

// ==================== toolchain.c ====================

// $CC, else "cc"
const char* default_c_compiler(void);

// runs `cc c_path -o exe_path`. Returns false after explaining why on stderr
bool compile_c(const char* cc, const char* c_path, const char* exe_path);

// runs path with args (argv[0] is set to path), waits, and returns its exit code:
// 128 + N if signal N killed it, -1 if it couldn't be started
int run_program(const char* path, char* const args[], int arg_count);

// a new private directory under $TMPDIR (NULL after reporting on failure)
char* make_temp_dir(void);

// removes dir and the files in it
void remove_temp_dir(const char* dir);

// dir + "/" + name, malloc'd
char* path_join(const char* dir, const char* name);

#endif
