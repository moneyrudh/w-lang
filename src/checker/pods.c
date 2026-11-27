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

// does `pod` contain `target` by value, directly or through other pods? (`seen` guards
// against looping through a cycle that doesn't involve target)
static bool contains(ASTNode* pod, ASTNode* target, ASTNode** seen, int seen_count, int capacity) {
    for (Parameter* f = pod->data.pod.fields; f; f = f->next) {
        ASTNode* inner = is_user_type(f->type) ? user_type_pod(f->type) : NULL;
        if (!inner) continue;
        if (inner == target) return true;
        bool visited = false;
        for (int i = 0; i < seen_count; i++) visited |= seen[i] == inner;
        if (visited || seen_count == capacity) continue;
        seen[seen_count] = inner;
        if (contains(inner, target, seen, seen_count + 1, capacity)) return true;
    }
    return false;
}

void declare_pods(ASTNode* pods) {
    int count = 0;
    for (ASTNode* pod = pods; pod; pod = pod->next) {
        if (user_type_pod(pod->data.pod.type)) {
            report_error(pod->location.line, "Pod '%s' already declared", pod->data.pod.name);
            continue;
        }
        user_type_set_pod(pod->data.pod.type, pod);
        count++;
    }

    for (ASTNode* pod = pods; pod; pod = pod->next) {
        if (user_type_pod(pod->data.pod.type) != pod) continue;   // a duplicate, reported above
        int line = pod->location.line;
        for (Parameter* f = pod->data.pod.fields; f; f = f->next) {
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

    // a pod that contains itself would be infinitely big
    ASTNode** seen = malloc(sizeof(ASTNode*) * (size_t)(count > 0 ? count : 1));
    for (ASTNode* pod = pods; pod; pod = pod->next) {
        if (user_type_pod(pod->data.pod.type) != pod) continue;
        // once per cycle: skip a pod when an earlier pod is part of the same cycle
        bool reported = false;
        for (ASTNode* earlier = pods; earlier != pod; earlier = earlier->next) {
            if (user_type_pod(earlier->data.pod.type) == earlier &&
                contains(pod, earlier, seen, 0, count) && contains(earlier, pod, seen, 0, count)) {
                reported = true;
            }
        }
        if (!reported && contains(pod, pod, seen, 0, count)) {
            report_error(pod->location.line, "Pod '%s' contains itself, so it would never end", pod->data.pod.name);
        }
    }
    free(seen);
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
        for (int j = 0; j < i; j++) {
            if (names[j] && strcmp(names[j], name) == 0) {
                report_error(line, "Field '%s' is given twice", name);
                }
        }
        if (type != TYPE_ERROR && f->type != TYPE_ERROR && !compare_types(f->type, type)) {
            report_error(line, "Field '%s' of '%s': cannot use %s as %s", name, pod->data.pod.name,
                         get_wlang_type_from_enum(type), get_wlang_type_from_enum(f->type));
        }
    }
    return pod->data.pod.type;
}
