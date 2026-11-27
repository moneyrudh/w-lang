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
void generate_log_statement(FILE* output, ASTNode* node, int indent_level);
void generate_return(FILE* output, ASTNode* node, int indent_level);

// ==================== functions.c ====================

// full definition: signature + body
void generate_function(FILE* output, ASTNode* node, int indent_level);

// forward declaration: signature + ';'
void generate_function_declaration(FILE* output, ASTNode* node);

// the globals list, so w() (main) can initialize str globals; and the current function's return type
void set_program_globals(ASTNode* globals);
DataType current_return_type(void);
// a `fun w()` without a return type: its main() returns 0
bool current_is_zil_entry_point(void);

// ==================== control_flow.c ====================

// NODE_IF chain -> if (...) { } else if (...) { } else { }
void generate_if(FILE* output, ASTNode* node, int indent_level);

// NODE_LOOP -> while (...) { }
void generate_loop(FILE* output, ASTNode* node, int indent_level);

// NODE_SKIP / NODE_STOP -> continue; / break;
void generate_loop_jump(FILE* output, ASTNode* node, int indent_level);

// ==================== arithmetic.c ====================
// num arithmetic: + - * -x wrap around (D11), / % stop on a zero divisor with the W line

void emit_arithmetic_runtime(FILE* output);

// emits a num + - * / % and returns true; false (nothing emitted) for anything else. Inside a
// global's value (a C constant) / and % don't call W__div
bool generate_num_arithmetic(FILE* output, ASTNode* binary_expr, bool in_global);
void generate_num_negation(FILE* output, ASTNode* operand);

// set while emitting globals' values (file-scope C constants)
void set_in_global_value(bool in_global);

// ==================== strings.c ====================
// Strings own their text; the generated C frees it (see the top of strings.c)

bool program_uses_str(ASTNode* program);
void emit_string_runtime(FILE* output);

// a join (a + b) or a call returning str somewhere in the expression / the statement's own expressions
bool expression_makes_temps(ASTNode* expr);
bool statement_makes_temps(ASTNode* statement);

// a function's start (W__fmark, copies of str parameters) and end
void strings_begin_function(FILE* output, ASTNode* function, int indent_level);
void strings_end_function(void);

// blocks: closing one frees the str variables declared in it
void strings_open_scope(void);
void strings_close_scope(FILE* output, int indent_level);
// closes a scope whose end can't be reached (nothing is emitted)
void strings_discard_scope(void);
void strings_own(const char* mangled_name);
// before declaring a str that shadows one this function owns: keep the outer one under an alias
void strings_shadow(FILE* output, int indent_level, const char* mangled_name);

// a loop body, so stop / skip know which variables they leave behind
void strings_begin_loop_body(void);
void strings_end_loop_body(void);

// W__temps_free(W__fmark); when the function makes temporaries
void strings_free_temps(FILE* output, int indent_level);
bool strings_function_has_mark(void);

// before ret: free temporaries and every live str variable
void strings_free_for_return(FILE* output, int indent_level);

// before stop / skip: the str variables declared inside the loop body
void strings_free_for_loop_jump(FILE* output, int indent_level);

// main()'s first lines: give str globals their first value
void strings_emit_global_inits(FILE* output, ASTNode* globals, int indent_level);

// ==================== formatters.c ====================

// W__name_v / W__name_f; w() -> main. Returns a static buffer (copy it to keep it)
const char* mangle_identifier(const char* w_name, bool is_function);
const char* mangle_init_temp(const char* w_name);

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
