// pods in the generated C: a struct per pod, compound literals for pod values, and a print
// function per pod for log. A pod is a plain value (D13), exactly like a C struct.
//
//   pod Ball               typedef struct W__Ball_p {
//       x: real                float W__x_m;
//       hp: num                int W__hp_m;
//                          } W__Ball_p;
//   Ball(x: 1, hp: 3)      ((W__Ball_p){ .W__x_m = (float)1, .W__hp_m = 3 })

#include <stdio.h>
#include <string.h>

#include "codegen/internal.h"
#include "codegen/c_syntax.h"
#include "language/type_registry.h"
#include "user_types.h"

// ==================== types ====================

// a pod's struct after the structs of the pods it contains (the checker ruled out cycles)
static void emit_pod_type(FILE* output, ASTNode* pod, ASTNode** done, int* done_count) {
    for (int i = 0; i < *done_count; i++) {
        if (done[i] == pod) return;
    }
    done[(*done_count)++] = pod;
    for (Parameter* f = pod->data.pod.fields; f; f = f->next) {
        ASTNode* inner = is_user_type(f->type) ? user_type_pod(f->type) : NULL;
        if (inner) emit_pod_type(output, inner, done, done_count);
    }

    const char* c_name = get_c_type_from_enum(pod->data.pod.type);
    fprintf(output, C_TYPEDEF_STRUCT "%s" C_LBRACE, c_name);
    for (Parameter* f = pod->data.pod.fields; f; f = f->next) {
        emit_indent(output, 1);
        fprintf(output, "%s %s" C_SEMICOLON_NL, get_c_type_from_enum(f->type), mangle_field(f->name));
    }
    fprintf(output, C_RBRACE_INLINE C_SPACE "%s" C_SEMICOLON_NL C_NEWLINE, c_name);
}

void emit_pod_types(FILE* output, ASTNode* pods) {
    int count = 0;
    for (ASTNode* p = pods; p; p = p->next) count++;
    if (count == 0) return;

    ASTNode* done[count];
    int done_count = 0;
    for (ASTNode* p = pods; p; p = p->next) {
        emit_pod_type(output, p, done, &done_count);
    }
}

// ==================== printing (log) ====================

// void W__Ball_print(W__Ball_p v) { printf("Ball(x: %f, hp: %d)", v.W__x_m, v.W__hp_m); }
// (nested pods call their own print function, so they're declared first)
void emit_pod_printers(FILE* output, ASTNode* pods) {
    for (ASTNode* pod = pods; pod; pod = pod->next) {
        fprintf(output, C_VOID C_SPACE C_POD_PRINT_FORMAT C_PRINT_PARAM C_SEMICOLON_NL, pod->data.pod.name,
                get_c_type_from_enum(pod->data.pod.type));
    }
    for (ASTNode* pod = pods; pod; pod = pod->next) {
        fprintf(output, C_VOID C_SPACE C_POD_PRINT_FORMAT C_PRINT_PARAM C_LBRACE, pod->data.pod.name,
                get_c_type_from_enum(pod->data.pod.type));
        emit_indent(output, 1);
        fprintf(output, C_PRINTF C_LPAREN C_STRING_QUOTE "%s(" C_STRING_QUOTE C_RPAREN C_SEMICOLON_NL, pod->data.pod.name);
        for (Parameter* f = pod->data.pod.fields; f; f = f->next) {
            const char* field = mangle_field(f->name);
            emit_indent(output, 1);
            fprintf(output, C_PRINTF C_LPAREN C_STRING_QUOTE "%s%s: " C_STRING_QUOTE C_RPAREN C_SEMICOLON_NL,
                    f == pod->data.pod.fields ? "" : C_LIST_SEPARATOR, f->name);
            emit_indent(output, 1);
            if (is_user_type(f->type)) {
                fprintf(output, C_POD_PRINT_FORMAT C_LPAREN C_PRINT_FIELD_OF_V C_RPAREN C_SEMICOLON_NL, user_type_name(f->type), field);
            } else if (f->type == TYPE_BOOL) {
                fprintf(output, C_PRINTF C_LPAREN C_STRING_QUOTE "%%s" C_STRING_QUOTE C_COMMA
                        C_BOOL_TO_TEXT_OPEN C_PRINT_FIELD_OF_V C_BOOL_TO_TEXT_CLOSE C_RPAREN C_SEMICOLON_NL, field);
            } else {
                fprintf(output, C_PRINTF C_LPAREN C_STRING_QUOTE "%s" C_STRING_QUOTE C_COMMA C_PRINT_FIELD_OF_V C_RPAREN C_SEMICOLON_NL,
                        get_format_spec_from_enum(f->type), field);
            }
        }
        emit_indent(output, 1);
        fprintf(output, C_PRINTF C_LPAREN C_STRING_QUOTE ")" C_STRING_QUOTE C_RPAREN C_SEMICOLON_NL);
        fprintf(output, C_RBRACE C_NEWLINE);
    }
}

void generate_pod_print(FILE* output, ASTNode* value) {
    fprintf(output, C_POD_PRINT_FORMAT C_LPAREN, user_type_name(value->expr_type));
    generate(output, value, 0);
    fprintf(output, C_RPAREN);
}

// ==================== values ====================

// ((W__Ball_p){ .W__x_m = (float)1, ... }); in a global's value a plain { ... } initializer,
// which C requires to be constant there
void generate_pod_literal(FILE* output, ASTNode* call, bool in_global) {
    DataType pod_type = call->expr_type;
    ASTNode* pod = user_type_pod(pod_type);
    int count = call->data.function_call.arg_count;

    if (!in_global) {
        fprintf(output, C_LPAREN C_LPAREN "%s" C_RPAREN, get_c_type_from_enum(pod_type));   // ((W__Ball_p)
    }
    fputs(C_INIT_OPEN, output);
    if (count == 0) {
        fputs(C_ZERO, output);
    }
    for (int i = 0; i < count; i++) {
        const char* name = call->data.function_call.arg_names[i];
        if (i > 0) fputs(C_COMMA, output);
        fprintf(output, C_FIELD_SELECT C_ASSIGN, mangle_field(name));
        generate_expression_with_cast(output, call->data.function_call.args[i], pod_field_type(pod, name));
    }
    fputs(C_INIT_CLOSE, output);
    if (!in_global) {
        fputs(C_RPAREN, output);
    }
}

void generate_field_access(FILE* output, ASTNode* node) {
    generate(output, node->data.field_access.object, 0);
    fprintf(output, C_FIELD_SELECT, mangle_field(node->data.field_access.field));
}

DataType pod_field_type(ASTNode* pod, const char* name) {
    for (Parameter* f = pod->data.pod.fields; f; f = f->next) {
        if (strcmp(f->name, name) == 0) return f->type;
    }
    return TYPE_ERROR;
}
