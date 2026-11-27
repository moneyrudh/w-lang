// `ref` arguments: what may be passed with ref, and the one-use-per-statement rule.
//
// C doesn't fix the order in which a call's arguments (or an operator's two sides) are worked
// out. So in `log(grow(ref s), s)` the second `s` could be read before or after grow() changes
// it, and for a str, after grow() freed the text it pointed to. A variable passed with ref is
// therefore used nowhere else in the same statement. Fields that don't overlap are separate:
// `move(ref b.pos, b.vel)` is fine, `move(ref b.pos, b.pos.x)` isn't.

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "checker/internal.h"
#include "diagnostics.h"

ASTNode* ref_root(ASTNode* arg) {
    while (arg && arg->type == NODE_FIELD_ACCESS) arg = arg->data.field_access.object;
    return arg && arg->type == NODE_VARIABLE ? arg : NULL;
}

// ==================== collecting a statement's uses ====================

typedef struct {
    ASTNode* path;   // NODE_VARIABLE, or a NODE_FIELD_ACCESS chain rooted at one
    bool is_ref;
} Use;

typedef struct {
    Use* uses;
    int count, capacity;
} Uses;

static void add_use(Uses* uses, ASTNode* path, bool is_ref) {
    if (uses->count == uses->capacity) {
        uses->capacity = uses->capacity ? uses->capacity * 2 : 16;
        uses->uses = realloc(uses->uses, sizeof(Use) * (size_t)uses->capacity);
        if (!uses->uses) {
            report_error(0, "Out of memory");
            exit(1);
        }
    }
    uses->uses[uses->count++] = (Use){ path, is_ref };
}

static void collect(ASTNode* expr, Uses* uses) {
    if (!expr) return;
    switch (expr->type) {
        case NODE_VARIABLE:
            add_use(uses, expr, false);
            break;
        case NODE_FIELD_ACCESS:
            if (ref_root(expr)) {
                add_use(uses, expr, false);
            } else {
                collect(expr->data.field_access.object, uses);   // make().x
            }
            break;
        case NODE_BINARY_EXPR:
            collect(expr->data.binary_expr.left, uses);
            collect(expr->data.binary_expr.right, uses);
            break;
        case NODE_UNARY_EXPR:
            collect(expr->data.unary_expr.operand, uses);
            break;
        case NODE_FUNCTION_CALL: {
            bool* refs = expr->data.function_call.arg_is_ref;
            for (int i = 0; i < expr->data.function_call.arg_count; i++) {
                ASTNode* arg = expr->data.function_call.args[i];
                if (refs && refs[i] && ref_root(arg)) {
                    add_use(uses, arg, true);
                } else {
                    collect(arg, uses);
                }
            }
            break;
        }
        default:
            break;
    }
}

// ==================== overlap ====================

// the names along a path, outermost first: b.pos.x -> b, pos, x. -1 if it has more than `max`
// names or isn't rooted at a variable
static int path_names(ASTNode* path, const char** names, int max) {
    if (!ref_root(path)) return -1;
    int count = 1;   // the variable
    for (ASTNode* p = path; p->type == NODE_FIELD_ACCESS; p = p->data.field_access.object) count++;
    if (count > max) return -1;

    ASTNode* p = path;
    for (int i = count - 1; i > 0; i--, p = p->data.field_access.object) {
        names[i] = p->data.field_access.field;
    }
    names[0] = p->data.variable.name;
    return count;
}

// two paths overlap when one is the other or inside it: b and b.pos.x, b.pos and b.pos
static bool overlap(ASTNode* a, ASTNode* b) {
    enum { MAX_PATH = 256 };
    const char* a_names[MAX_PATH];
    const char* b_names[MAX_PATH];
    int a_count = path_names(a, a_names, MAX_PATH);
    int b_count = path_names(b, b_names, MAX_PATH);
    if (a_count < 0 || b_count < 0) return true;   // too deep to tell: the safe answer
    int shorter = a_count < b_count ? a_count : b_count;
    for (int i = 0; i < shorter; i++) {
        if (strcmp(a_names[i], b_names[i]) != 0) return false;
    }
    return true;
}

static void path_text(ASTNode* path, char* out, size_t size) {
    const char* names[256];
    int count = path_names(path, names, 256);
    out[0] = '\0';
    if (count < 0) {
        ASTNode* root = ref_root(path);
        snprintf(out, size, "%s", root ? root->data.variable.name : "?");
        return;
    }
    size_t used = 0;
    for (int i = 0; i < count && used < size; i++) {
        used += (size_t)snprintf(out + used, size - used, "%s%s", i ? "." : "", names[i]);
    }
}

void check_ref_uses(ASTNode** roots, int count, int line) {
    Uses uses = { 0 };
    for (int i = 0; i < count; i++) collect(roots[i], &uses);

    for (int i = 0; i < uses.count; i++) {
        if (!uses.uses[i].is_ref) continue;
        for (int j = 0; j < uses.count; j++) {
            if (j == i || !overlap(uses.uses[i].path, uses.uses[j].path)) continue;
            char text[256];
            path_text(uses.uses[i].path, text, sizeof(text));
            report_error(line, "'%s' is passed with ref, so it can't also be used elsewhere in the same statement", text);
            free(uses.uses);
            return;   // one error per statement
        }
    }
    free(uses.uses);
}
