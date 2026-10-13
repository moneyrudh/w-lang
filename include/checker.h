#ifndef W_CHECKER_H
#define W_CHECKER_H

#include <stdbool.h>

#include "types.h"
#include "ast.h"

// Resolves every name, fills in expr_type / target_type / LogElement.var_type,
// and reports semantic errors. Functions may be called before they're defined:
// all signatures are collected before any body is checked.
void check_program(ASTNode* program);

// ==================== type rules (also used by codegen) ====================

// can a value of type `value` be stored in / passed as `target`?
bool compare_types(DataType target, DataType value);

// can `from` be cast to `to` in the generated C?
bool can_convert_type(DataType from, DataType to);

#endif
