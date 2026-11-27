// pods (DECISIONS.md, D3): a type with named fields, used as a value (D13: copied, like a C
// struct). Declared at the top level; checked before anything that could use them.

#include <stdlib.h>
#include <string.h>

#include "checker.h"
#include "checker/internal.h"
#include "diagnostics.h"
#include "user_types.h"
#include "language/type_registry.h"

ASTNode* find_pod(const char* name) {
    DataType type = user_type_find(name);
    return type == TYPE_ERROR ? NULL : user_type_pod(type);
}

Parameter* pod_field(ASTNode* pod, const char* name) {
    for (Parameter* f = pod->data.pod.fields; f; f = f->next) {
        if (strcmp(f->name, name) == 0) return f;
    }
    return NULL;
}

bool check_type_known(DataType* type, int line) {
    if (is_user_type(*type) && !user_type_pod(*type)) {
        report_error(line, "Unknown type '%s'", user_type_name(*type));
        *type = TYPE_ERROR;
        return false;
    }
    return true;
}

// a pod that contains itself by value (directly or through other pods) would be infinitely big.
// One depth-first walk over the pods: each is visited once, so this is linear in their fields.
// A field leading back to a pod still being walked closes a cycle; it's reported at that pod
typedef enum { NOT_VISITED, IN_PROGRESS, DONE } VisitState;

static void find_cycles(ASTNode* pod, VisitState* state) {
    int index = (int)pod->data.pod.type - TYPE_USER_BASE;
    state[index] = IN_PROGRESS;
    for (Parameter* f = pod->data.pod.fields; f; f = f->next) {
        ASTNode* inner = is_user_type(f->type) ? user_type_pod(f->type) : NULL;
        if (!inner) continue;
        int inner_index = (int)inner->data.pod.type - TYPE_USER_BASE;
        if (state[inner_index] == IN_PROGRESS) {
            report_error(inner->location.line, "Pod '%s' contains itself, so it would never end",
                         inner->data.pod.name);
        } else if (state[inner_index] == NOT_VISITED) {
            find_cycles(inner, state);
        }
    }
    state[index] = DONE;
}

void declare_pods(ASTNode* pods) {
    for (ASTNode* pod = pods; pod; pod = pod->next) {
        if (user_type_pod(pod->data.pod.type)) {
            report_error(pod->location.line, "Pod '%s' already declared", pod->data.pod.name);
            continue;
        }
        user_type_set_pod(pod->data.pod.type, pod);
    }

    for (ASTNode* pod = pods; pod; pod = pod->next) {
        if (user_type_pod(pod->data.pod.type) != pod) continue;   // a duplicate, reported above
        for (Parameter* f = pod->data.pod.fields; f; f = f->next) {
            int line = f->line;
            for (Parameter* g = pod->data.pod.fields; g != f; g = g->next) {
                if (strcmp(f->name, g->name) == 0) {
                    report_error(line, "Pod '%s' has two fields named '%s'", pod->data.pod.name, f->name);
                    break;
                }
            }
            if (f->type == TYPE_ZIL) {
                report_error(line, "Field '%s' can't be zil", f->name);
                f->type = TYPE_ERROR;
            } else if (f->type == TYPE_STR) {
                report_error(line, "Field '%s': str fields in pods aren't supported yet (roadmap 3.3)", f->name);
                f->type = TYPE_ERROR;
            } else {
                check_type_known(&f->type, line);
            }
        }
    }

    // cycles (one walk; every pod's type number is below TYPE_USER_BASE + user type count)
    int max_index = 0;
    for (ASTNode* pod = pods; pod; pod = pod->next) {
        int index = (int)pod->data.pod.type - TYPE_USER_BASE;
        if (index + 1 > max_index) max_index = index + 1;
    }
    VisitState* state = calloc((size_t)(max_index > 0 ? max_index : 1), sizeof(VisitState));
    if (!state) {
        report_error(0, "Out of memory");
        return;
    }
    for (ASTNode* pod = pods; pod; pod = pod->next) {
        if (user_type_pod(pod->data.pod.type) != pod) continue;
        if (state[(int)pod->data.pod.type - TYPE_USER_BASE] == NOT_VISITED) {
            find_cycles(pod, state);
        }
    }
    free(state);
}

DataType check_field_access(ASTNode* node) {
    DataType object = check_expression(node->data.field_access.object);
    if (object == TYPE_ERROR) return TYPE_ERROR;

    const char* field = node->data.field_access.field;
    ASTNode* pod = is_user_type(object) ? user_type_pod(object) : NULL;
    if (!pod) {
        report_error(node->location.line, "%s has no fields (looking for '%s')", get_wlang_type_from_enum(object), field);
        return TYPE_ERROR;
    }
    Parameter* f = pod_field(pod, field);
    if (!f) {
        report_error(node->location.line, "Pod '%s' has no field '%s'", pod->data.pod.name, field);
        return TYPE_ERROR;
    }
    return f->type;
}

// Name(field: value, ...): every value names its field; left-out fields are zero
DataType check_pod_literal(ASTNode* node, ASTNode* pod) {
    int line = node->location.line;
    int count = node->data.function_call.arg_count;
    char** names = node->data.function_call.arg_names;
    node->data.function_call.is_pod_literal = true;

    bool reported_unnamed = false;
    for (int i = 0; i < count; i++) {
        ASTNode* value = node->data.function_call.args[i];
        DataType type = value ? check_expression(value) : TYPE_ERROR;
        const char* name = names ? names[i] : NULL;
        if (!name) {
            if (!reported_unnamed) {   // once per pod value
                report_error(line, "Values of pod '%s' need their field's name: %s(field: value, ...)",
                             pod->data.pod.name, pod->data.pod.name);
                reported_unnamed = true;
            }
            continue;
        }
        Parameter* f = pod_field(pod, name);
        if (!f) {
            report_error(line, "Pod '%s' has no field '%s'", pod->data.pod.name, name);
            continue;
        }
        // reported once per repeated name, at its second appearance
        int earlier = 0;
        for (int j = 0; j < i; j++) {
            if (names[j] && strcmp(names[j], name) == 0) earlier++;
        }
        if (earlier == 1) {
            report_error(line, "Field '%s' is given twice", name);
        }
        if (type != TYPE_ERROR && f->type != TYPE_ERROR && !compare_types(f->type, type)) {
            report_error(line, "Field '%s' of '%s': cannot use %s as %s", name, pod->data.pod.name,
                         get_wlang_type_from_enum(type), get_wlang_type_from_enum(f->type));
        }
    }
    return pod->data.pod.type;
}
