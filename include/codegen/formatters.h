#ifndef FORMATTERS_H
#define FORMATTERS_H

#include <stdio.h>
#include "types.h"
#include "ast.h"

// ==================== output helpers ====================

// emit indentation (level * 4 spaces)
void emit_indent(FILE* out, int level);

// emit standard C includes block
void emit_c_includes(FILE* out);

// ==================== function generation ====================

// emit complete function signature with parameters
void emit_function_signature(FILE* out, const char* return_type, const char* name,
                             Parameter* params, int param_count);

// ==================== type conversion ====================

// emit type cast if needed (handles conversion rules)
void emit_cast(FILE* out, DataType from_type, DataType to_type);

// ==================== operator formatting ====================

// get binary operator string with proper spacing (e.g., " + ", " * ")
const char* get_binary_operator_string(char op);

#endif // FORMATTERS_H
