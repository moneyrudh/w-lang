#ifndef W_CODEGEN_INTERNAL_H
#define W_CODEGEN_INTERNAL_H

// shared between the code generator's files only; the public API is include/codegen.h

#include <stdio.h>
#include <stdbool.h>

#include "types.h"
#include "ast.h"

// ==================== codegen.c ====================

// emits any node: statements at indent_level, expressions inline (indent_level 0)
void generate(FILE* output, ASTNode* node, int indent_level);

// generates each statement in a linked list at the given indent
void generate_block(FILE* output, ASTNode* statements, int indent_level);

// ==================== expressions.c ====================

// NODE_NUMBER / STRING / FLOAT / CHAR / BOOL
void generate_literal(FILE* output, ASTNode* node);

void generate_variable(FILE* output, ASTNode* node);
void generate_binary_expr(FILE* output, ASTNode* node, int indent_level);
void generate_unary_expr(FILE* output, ASTNode* node, int indent_level);

// as a statement when indent_level > 0, otherwise inline in an expression
void generate_function_call(FILE* output, ASTNode* node, int indent_level);

// expr, cast to target_type when the types differ
void generate_expression_with_cast(FILE* output, ASTNode* expr, DataType target_type);

// "(type)" when converting from -> to needs a cast
void generate_cast_if_needed(FILE* output, DataType from, DataType to);

// ==================== statements.c ====================

void generate_var_declaration(FILE* output, ASTNode* node, int indent_level);
void generate_assignment(FILE* output, ASTNode* node, int indent_level);
void generate_log_statement(FILE* output, LogElement* elements, int indent_level);
void generate_return(FILE* output, ASTNode* node, int indent_level);

// ==================== functions.c ====================

// full definition: signature + body
void generate_function(FILE* output, ASTNode* node, int indent_level);

// forward declaration: signature + ';'
void generate_function_declaration(FILE* output, ASTNode* node);

// ==================== control_flow.c ====================

// NODE_IF chain -> if (...) { } else if (...) { } else { }
void generate_if(FILE* output, ASTNode* node, int indent_level);

// NODE_LOOP -> while (...) { }
void generate_loop(FILE* output, ASTNode* node, int indent_level);

// NODE_SKIP / NODE_STOP -> continue; / break;
void generate_loop_jump(FILE* output, ASTNode* node, int indent_level);

// ==================== formatters.c ====================

// W__name_v / W__name_f; w() -> main. Returns a static buffer (copy it to keep it)
const char* mangle_identifier(const char* w_name, bool is_function);

// emit indentation (level * 4 spaces)
void emit_indent(FILE* out, int level);

// emit standard C includes block
void emit_c_includes(FILE* out);

// emit complete function signature with parameters, then " {\n"
void emit_function_signature(FILE* out, const char* return_type, const char* name,
                             Parameter* params, int param_count);

// emit function forward declaration (signature + semicolon)
void emit_function_declaration(FILE* out, const char* return_type, const char* name,
                                Parameter* params, int param_count);

// emit "(type)" cast to to_type if the types differ
void emit_cast(FILE* out, DataType from_type, DataType to_type);

// get binary operator string with proper spacing (e.g., " + ", " * ")
const char* get_binary_operator_string(OperatorType op);

#endif
