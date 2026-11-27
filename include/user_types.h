#ifndef W_USER_TYPES_H
#define W_USER_TYPES_H

#include <stdbool.h>

#include "types.h"

// Types a program defines (pods; later vec[T]). A type is still a DataType, so the rest of the
// compiler passes it around unchanged: the built-in ones are the enum's values, and a program's
// own types are numbers from TYPE_USER_BASE up, one per distinct name.
#define TYPE_USER_BASE 1000

struct ASTNode;

// the type named `name` (the parser calls this for a type it doesn't know; the checker decides
// whether such a type really exists). The same name always gives the same type
DataType user_type_named(const char* name);

bool is_user_type(DataType type);

// the type named `name` if some construct has named it, else TYPE_ERROR (adds nothing)
DataType user_type_find(const char* name);

// "Ball" / "W__Ball_p" (the C struct's name)
const char* user_type_name(DataType type);
const char* user_type_c_name(DataType type);

// the NODE_POD_DECL that defines the type, once the checker has seen it (NULL: no such pod)
void user_type_set_pod(DataType type, struct ASTNode* pod);
struct ASTNode* user_type_pod(DataType type);

// forgets every user type (at the end of a run)
void user_types_cleanup(void);

#endif
