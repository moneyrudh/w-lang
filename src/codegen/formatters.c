#include "codegen/formatters.h"
#include "codegen/c_syntax.h"
#include "transpiler/type_registry.h"
#include <string.h>
#include <stdbool.h>

// ==================== output helpers ====================

void emit_indent(FILE* out, int level) {
    for (int i = 0; i < level; i++) {
        fprintf(out, C_INDENT);
    }
}

void emit_c_includes(FILE* out) {
    fprintf(out, C_INCLUDES_BLOCK);
}

// ==================== function generation ====================

void emit_function_signature(FILE* out, const char* return_type, const char* name,
                             Parameter* params, int param_count) {
    fprintf(out, "%s %s" C_LPAREN, return_type, name);

    if (param_count == 0) {
        fprintf(out, C_VOID);
    } else {
        Parameter* param = params;
        bool first = true;
        while (param) {
            if (!first) {
                fprintf(out, C_COMMA);
            }
            fprintf(out, "%s %s", get_c_type_from_enum(param->type), param->name);
            first = false;
            param = param->next;
        }
    }

    fprintf(out, C_RPAREN C_LBRACE);
}

// ==================== type conversion ====================

void emit_cast(FILE* out, DataType from_type, DataType to_type) {
    if (from_type == to_type) return;

    fprintf(out, C_LPAREN "%s" C_RPAREN, get_c_type_from_enum(to_type));
}

// ==================== operator formatting ====================

const char* get_binary_operator_string(char op) {
    switch (op) {
        case '+': return C_PLUS;
        case '-': return C_MINUS;
        case '*': return C_MULTIPLY;
        case '/': return C_DIVIDE;
        default:  return C_SPACE;
    }
}
