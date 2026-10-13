#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>

#include "diagnostics.h"

#define MAX_ERRORS 5

static int errors = 0;

void report_error(int line, const char* format, ...) {
    if (line > 0) {
        fprintf(stderr, "Error on line %d: ", line);
    } else {
        fprintf(stderr, "Error: ");
    }

    va_list args;
    va_start(args, format);
    vfprintf(stderr, format, args);
    va_end(args);
    fputc('\n', stderr);

    if (++errors >= MAX_ERRORS) {
        fprintf(stderr, "Too many errors, exiting.\n");
        exit(1);
    }
}

int error_count(void) {
    return errors;
}
