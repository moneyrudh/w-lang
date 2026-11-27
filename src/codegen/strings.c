// Strings in the generated C (DECISIONS.md, D2): W code never frees anything, so the C does.
//
//  - Every str variable (local, parameter, global) owns its own heap copy of its text.
//    Assigning frees the old text (after copying the new one); a local's text is freed when
//    its block ends, and before ret / stop / skip leave the block.
//  - A join (a + b) or a call that returns a str makes a temporary: it's recorded in a list
//    and freed after the statement that made it (W__temps_free back to the function's mark).
//  - ret copies a str result, frees the function's temporaries and variables, then hands the
//    copy to the caller as one of the caller's temporaries.
//
// Memory stays flat in loops: nothing lives longer than its variable or statement.

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "codegen/internal.h"
#include "codegen/c_syntax.h"

// ==================== the runtime emitted into programs that use str ====================

// plain (not static) functions: compilers don't warn about unused ones, and the generated
// program is a single file, so the names can't clash
static const char* const STRING_RUNTIME =
    "// ---- W strings: every str variable owns a copy; joins and str results are temporaries ----\n"
    "static char** W__temps = NULL;\n"
    "static size_t W__temps_count = 0, W__temps_capacity = 0;\n"
    "\n"
    "void W__out_of_memory(void) {\n"
    "    fputs(\"W: out of memory\\n\", stderr);\n"
    "    exit(1);\n"
    "}\n"
    "\n"
    "char* W__str_copy(const char* s) {\n"
    "    size_t size = strlen(s) + 1;\n"
    "    char* copy = malloc(size);\n"
    "    if (!copy) W__out_of_memory();\n"
    "    memcpy(copy, s, size);\n"
    "    return copy;\n"
    "}\n"
    "\n"
    "char* W__temp(char* s) {\n"
    "    if (W__temps_count == W__temps_capacity) {\n"
    "        W__temps_capacity = W__temps_capacity ? W__temps_capacity * 2 : 16;\n"
    "        W__temps = realloc(W__temps, W__temps_capacity * sizeof(char*));\n"
    "        if (!W__temps) W__out_of_memory();\n"
    "    }\n"
    "    W__temps[W__temps_count++] = s;\n"
    "    return s;\n"
    "}\n"
    "\n"
    "size_t W__temps_mark(void) {\n"
    "    return W__temps_count;\n"
    "}\n"
    "\n"
    "void W__temps_free(size_t mark) {\n"
    "    while (W__temps_count > mark) free(W__temps[--W__temps_count]);\n"
    "}\n"
    "\n"
    "char* W__str_join(const char* a, const char* b) {\n"
    "    size_t a_length = strlen(a), b_length = strlen(b);\n"
    "    char* joined = malloc(a_length + b_length + 1);\n"
    "    if (!joined) W__out_of_memory();\n"
    "    memcpy(joined, a, a_length);\n"
    "    memcpy(joined + a_length, b, b_length + 1);\n"
    "    return W__temp(joined);\n"
    "}\n"
    "\n"
    "// copy first: the new value may be the old one, or part of it\n"
    "void W__str_set(char** variable, const char* value) {\n"
    "    char* copy = W__str_copy(value);\n"
    "    free(*variable);\n"
    "    *variable = copy;\n"
    "}\n"
    "// ---- end of W strings ----\n\n";

// ==================== finding what makes temporaries ====================

static bool is_str_join(ASTNode* expr) {
    return expr->type == NODE_BINARY_EXPR && expr->expr_type == TYPE_STR &&
           expr->data.binary_expr.operator == OP_ADD;
}

bool expression_makes_temps(ASTNode* expr) {
    if (!expr) return false;
    switch (expr->type) {
        case NODE_BINARY_EXPR:
            return is_str_join(expr) || expression_makes_temps(expr->data.binary_expr.left) ||
                   expression_makes_temps(expr->data.binary_expr.right);
        case NODE_UNARY_EXPR:
            return expression_makes_temps(expr->data.unary_expr.operand);
        case NODE_FUNCTION_CALL:
            if (expr->expr_type == TYPE_STR) return true;
            for (int i = 0; i < expr->data.function_call.arg_count; i++) {
                if (expression_makes_temps(expr->data.function_call.args[i])) return true;
            }
            return false;
        default:
            return false;
    }
}

// log prints a top-level join as its pieces side by side, so only the pieces can make temps
static bool log_arg_makes_temps(ASTNode* arg) {
    if (is_str_join(arg)) {
        return log_arg_makes_temps(arg->data.binary_expr.left) ||
               log_arg_makes_temps(arg->data.binary_expr.right);
    }
    return expression_makes_temps(arg);
}

// the statement's own expressions (not the statements in its bodies)
bool statement_makes_temps(ASTNode* s) {
    switch (s->type) {
        case NODE_VAR_DECLARATION:
            return expression_makes_temps(s->data.var_declaration.init_expr);
        case NODE_ASSIGNMENT:
            return expression_makes_temps(s->data.assignment.value);
        case NODE_LOG:
            for (int i = 0; i < s->data.log.arg_count; i++) {
                if (log_arg_makes_temps(s->data.log.args[i])) return true;
            }
            return false;
        case NODE_RETURN:
            return expression_makes_temps(s->data.return_statement.expression);
        case NODE_IF:
            for (ASTNode* branch = s; branch; branch = branch->data.if_statement.else_branch) {
                if (expression_makes_temps(branch->data.if_statement.condition)) return true;
            }
            return false;
        case NODE_LOOP:
            return expression_makes_temps(s->data.loop_statement.condition);
        case NODE_SKIP:
        case NODE_STOP:
            return false;
        default:
            return expression_makes_temps(s);   // an expression statement (a call)
    }
}

// any statement in the list, or in the bodies inside it
static bool statements_make_temps(ASTNode* statements) {
    for (ASTNode* s = statements; s; s = s->next) {
        if (statement_makes_temps(s)) return true;
        if (s->type == NODE_IF) {
            for (ASTNode* branch = s; branch; branch = branch->data.if_statement.else_branch) {
                if (statements_make_temps(branch->data.if_statement.body)) return true;
            }
        } else if (s->type == NODE_LOOP && statements_make_temps(s->data.loop_statement.body)) {
            return true;
        }
    }
    return false;
}

// ==================== does the program use str at all ====================

static bool expression_uses_str(ASTNode* expr) {
    if (!expr) return false;
    if (expr->expr_type == TYPE_STR) return true;
    switch (expr->type) {
        case NODE_BINARY_EXPR:
            return expression_uses_str(expr->data.binary_expr.left) ||
                   expression_uses_str(expr->data.binary_expr.right);
        case NODE_UNARY_EXPR:
            return expression_uses_str(expr->data.unary_expr.operand);
        case NODE_FUNCTION_CALL:
            for (int i = 0; i < expr->data.function_call.arg_count; i++) {
                if (expression_uses_str(expr->data.function_call.args[i])) return true;
            }
            return false;
        default:
            return false;
    }
}

static bool statements_use_str(ASTNode* statements) {
    for (ASTNode* s = statements; s; s = s->next) {
        switch (s->type) {
            case NODE_VAR_DECLARATION:
                if (s->data.var_declaration.type == TYPE_STR ||
                    expression_uses_str(s->data.var_declaration.init_expr)) return true;
                break;
            case NODE_ASSIGNMENT:
                if (expression_uses_str(s->data.assignment.value)) return true;
                break;
            case NODE_LOG:
                for (int i = 0; i < s->data.log.arg_count; i++) {
                    if (expression_uses_str(s->data.log.args[i])) return true;
                }
                break;
            case NODE_RETURN:
                if (expression_uses_str(s->data.return_statement.expression)) return true;
                break;
            case NODE_IF:
                for (ASTNode* branch = s; branch; branch = branch->data.if_statement.else_branch) {
                    if (expression_uses_str(branch->data.if_statement.condition) ||
                        statements_use_str(branch->data.if_statement.body)) return true;
                }
                break;
            case NODE_LOOP:
                if (expression_uses_str(s->data.loop_statement.condition) ||
                    statements_use_str(s->data.loop_statement.body)) return true;
                break;
            default:
                if (expression_uses_str(s)) return true;
                break;
        }
    }
    return false;
}

static bool function_uses_str(ASTNode* function) {
    if (function->data.function.return_type == TYPE_STR) return true;
    for (Parameter* p = function->data.function.parameters; p; p = p->next) {
        if (p->type == TYPE_STR) return true;
    }
    return statements_use_str(function->data.function.body);
}

bool program_uses_str(ASTNode* program) {
    for (ASTNode* g = program->data.program.globals; g; g = g->next) {
        if (g->data.var_declaration.type == TYPE_STR) return true;
    }
    for (ASTNode* f = program->data.program.functions; f; f = f->next) {
        if (function_uses_str(f)) return true;
    }
    return false;
}

void emit_string_runtime(FILE* output) {
    fputs(STRING_RUNTIME, output);
}

// ==================== the current function's owned variables ====================

static struct {
    bool has_mark;          // the function declared W__fmark (it makes temps or returns a str)
    char** owned;           // mangled names of the str variables alive right now, innermost last
    int owned_count, owned_capacity;
    int* scope_starts;      // where each open scope's variables begin in `owned`
    int scope_count, scope_capacity;
    int* loop_scopes;       // the scope index of each enclosing loop's body
    int loop_count, loop_capacity;
} fn;

static void* grow(void* array, int* capacity, size_t element_size) {
    *capacity = *capacity ? *capacity * 2 : 16;
    array = realloc(array, element_size * (size_t)*capacity);
    if (!array) {
        fprintf(stderr, "Out of memory\n");
        exit(1);
    }
    return array;
}

void strings_begin_function(FILE* output, ASTNode* function, int indent_level) {
    fn.owned_count = fn.scope_count = fn.loop_count = 0;
    fn.has_mark = function->data.function.return_type == TYPE_STR ||
                  statements_make_temps(function->data.function.body);

    if (fn.has_mark) {
        emit_indent(output, indent_level);
        fprintf(output, C_SIZE_T " " C_FUNCTION_MARK C_ASSIGN C_TEMPS_MARK_CALL C_SEMICOLON_NL);
    }

    // the function's own scope: its str parameters become copies it owns
    strings_open_scope();
    for (Parameter* p = function->data.function.parameters; p; p = p->next) {
        if (p->type != TYPE_STR) continue;
        const char* name = mangle_identifier(p->name, false);
        emit_indent(output, indent_level);
        fprintf(output, "%s" C_ASSIGN C_STR_COPY "(%s)" C_SEMICOLON_NL, name, name);
        strings_own(name);
    }
}

void strings_end_function(void) {
    for (int i = 0; i < fn.owned_count; i++) free(fn.owned[i]);
    fn.owned_count = fn.scope_count = fn.loop_count = 0;
}

void strings_open_scope(void) {
    if (fn.scope_count == fn.scope_capacity) {
        fn.scope_starts = grow(fn.scope_starts, &fn.scope_capacity, sizeof(int));
    }
    fn.scope_starts[fn.scope_count++] = fn.owned_count;
}

static void emit_frees_down_to(FILE* output, int indent_level, int first) {
    for (int i = fn.owned_count - 1; i >= first; i--) {
        emit_indent(output, indent_level);
        fprintf(output, C_FREE "(%s)" C_SEMICOLON_NL, fn.owned[i]);
    }
}

static void forget_scope(void) {
    int first = fn.scope_starts[--fn.scope_count];
    for (int i = first; i < fn.owned_count; i++) free(fn.owned[i]);
    fn.owned_count = first;
}

void strings_close_scope(FILE* output, int indent_level) {
    emit_frees_down_to(output, indent_level, fn.scope_starts[fn.scope_count - 1]);
    forget_scope();
}

void strings_discard_scope(void) {
    forget_scope();
}

void strings_own(const char* mangled_name) {
    if (fn.owned_count == fn.owned_capacity) {
        fn.owned = grow(fn.owned, &fn.owned_capacity, sizeof(char*));
    }
    char* copy = strdup(mangled_name);
    if (!copy) {
        fprintf(stderr, "Out of memory\n");
        exit(1);
    }
    fn.owned[fn.owned_count++] = copy;
}

void strings_begin_loop_body(void) {
    if (fn.loop_count == fn.loop_capacity) {
        fn.loop_scopes = grow(fn.loop_scopes, &fn.loop_capacity, sizeof(int));
    }
    fn.loop_scopes[fn.loop_count++] = fn.scope_count;   // the body's scope opens next
}

void strings_end_loop_body(void) {
    fn.loop_count--;
}

bool strings_function_has_mark(void) {
    return fn.has_mark;
}

void strings_free_temps(FILE* output, int indent_level) {
    if (!fn.has_mark) return;
    emit_indent(output, indent_level);
    fprintf(output, C_TEMPS_FREE "(" C_FUNCTION_MARK ")" C_SEMICOLON_NL);
}

bool strings_return_needs_cleanup(void) {
    return fn.has_mark || fn.owned_count > 0;
}

void strings_free_for_return(FILE* output, int indent_level) {
    strings_free_temps(output, indent_level);
    emit_frees_down_to(output, indent_level, 0);
}

void strings_free_for_loop_jump(FILE* output, int indent_level) {
    if (fn.loop_count == 0) return;
    int body_scope = fn.loop_scopes[fn.loop_count - 1];
    emit_frees_down_to(output, indent_level, fn.scope_starts[body_scope]);
}

// ==================== globals ====================

// file scope: str globals start as NULL; main() gives them their first value
void strings_emit_global_inits(FILE* output, ASTNode* globals, int indent_level) {
    for (ASTNode* g = globals; g; g = g->next) {
        if (g->data.var_declaration.type != TYPE_STR) continue;
        emit_indent(output, indent_level);
        fprintf(output, "%s" C_ASSIGN C_STR_COPY "(", mangle_identifier(g->data.var_declaration.name, false));
        if (g->data.var_declaration.init_expr) {
            generate(output, g->data.var_declaration.init_expr, 0);
        } else {
            fputs(C_EMPTY_STRING, output);
        }
        fprintf(output, ")" C_SEMICOLON_NL);
    }
}
