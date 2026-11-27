#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "user_types.h"

typedef struct {
    char* name;
    char* c_name;
    struct ASTNode* pod;
} UserType;

static UserType* user_types = NULL;
static int user_type_count = 0, user_type_capacity = 0;

static void out_of_memory(void) {
    fprintf(stderr, "Out of memory\n");
    exit(1);
}

DataType user_type_named(const char* name) {
    for (int i = 0; i < user_type_count; i++) {
        if (strcmp(user_types[i].name, name) == 0) return (DataType)(TYPE_USER_BASE + i);
    }
    if (user_type_count == user_type_capacity) {
        user_type_capacity = user_type_capacity ? user_type_capacity * 2 : 16;
        user_types = realloc(user_types, sizeof(UserType) * (size_t)user_type_capacity);
        if (!user_types) out_of_memory();
    }

    size_t c_size = strlen(name) + sizeof("W___p");
    UserType* t = &user_types[user_type_count];
    t->name = strdup(name);
    t->c_name = malloc(c_size);
    if (!t->name || !t->c_name) out_of_memory();
    snprintf(t->c_name, c_size, "W__%s_p", name);
    t->pod = NULL;
    return (DataType)(TYPE_USER_BASE + user_type_count++);
}

DataType user_type_find(const char* name) {
    for (int i = 0; i < user_type_count; i++) {
        if (strcmp(user_types[i].name, name) == 0) return (DataType)(TYPE_USER_BASE + i);
    }
    return TYPE_ERROR;
}

bool is_user_type(DataType type) {
    return (int)type >= TYPE_USER_BASE && (int)type < TYPE_USER_BASE + user_type_count;
}

static UserType* get(DataType type) {
    return is_user_type(type) ? &user_types[(int)type - TYPE_USER_BASE] : NULL;
}

const char* user_type_name(DataType type) {
    UserType* t = get(type);
    return t ? t->name : "unknown";
}

const char* user_type_c_name(DataType type) {
    UserType* t = get(type);
    return t ? t->c_name : "void";
}

void user_type_set_pod(DataType type, struct ASTNode* pod) {
    UserType* t = get(type);
    if (t) t->pod = pod;
}

struct ASTNode* user_type_pod(DataType type) {
    UserType* t = get(type);
    return t ? t->pod : NULL;
}

void user_types_cleanup(void) {
    for (int i = 0; i < user_type_count; i++) {
        free(user_types[i].name);
        free(user_types[i].c_name);
    }
    free(user_types);
    user_types = NULL;
    user_type_count = user_type_capacity = 0;
}
