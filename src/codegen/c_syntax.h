#ifndef W_CODEGEN_C_SYNTAX_H
#define W_CODEGEN_C_SYNTAX_H

// ===== C standard library includes =====
#define C_INCLUDE_STDIO       "#include <stdio.h>\n"
#define C_INCLUDE_STDBOOL     "#include <stdbool.h>\n"
#define C_INCLUDE_STRING      "#include <string.h>\n"
#define C_INCLUDE_STDLIB      "#include <stdlib.h>\n"
#define C_INCLUDES_BLOCK      C_INCLUDE_STDIO C_INCLUDE_STDBOOL C_INCLUDE_STRING C_INCLUDE_STDLIB "\n"

// ===== C keywords =====
#define C_MAIN                "main"
#define C_VOID                "void"
#define C_RETURN              "return"
#define C_IF                  "if"
#define C_ELSE                "else"
#define C_WHILE               "while"
#define C_BREAK               "break"
#define C_CONTINUE            "continue"
#define C_FOR                 "for"
#define C_TRUE                "true"
#define C_FALSE               "false"

// ===== punctuation & delimiters =====
#define C_LPAREN              "("
#define C_RPAREN              ")"
#define C_LBRACE              " {\n"
#define C_RBRACE              "}\n"
#define C_RBRACE_INLINE       "}"
#define C_SEMICOLON           ";"
#define C_SEMICOLON_NL        ";\n"
#define C_COMMA               ", "
#define C_COLON               ":"
#define C_SPACE               " "
#define C_NEWLINE             "\n"
#define C_DOUBLE_NEWLINE      "\n\n"

// ===== indentation =====
#define C_INDENT              "    "
#define C_INDENT_SIZE         4

// ===== operators (with spacing for binary expressions) =====
#define C_ASSIGN              " = "
#define C_PLUS                " + "
#define C_MINUS               " - "
#define C_MULTIPLY            " * "
#define C_DIVIDE              " / "
#define C_MODULO              " % "
#define C_EQUAL               " == "
#define C_NOT_EQUAL           " != "
#define C_LESS                " < "
#define C_GREATER             " > "
#define C_LESS_EQUAL          " <= "
#define C_GREATER_EQUAL       " >= "
#define C_AND                 " && "
#define C_OR                  " || "

// ===== single character operators (no spacing) =====
#define C_OP_PLUS             "+"
#define C_OP_MINUS            "-"
#define C_OP_MULTIPLY         "*"
#define C_OP_DIVIDE           "/"
#define C_OP_NOT              "!"
#define C_NOT_OPEN            "!("
#define C_NEG_OPEN            "-("   // always parenthesized: - -x must not become --x

// ===== string escape sequences (for code generation) =====
#define C_ESC_NEWLINE         "\\n"
#define C_ESC_TAB             "\\t"
#define C_ESC_QUOTE           "\\\""
#define C_ESC_BACKSLASH       "\\\\"
#define C_ESC_CARRIAGE        "\\r"
#define C_ESC_SINGLE_QUOTE    "\\'"

// ===== string delimiters =====
#define C_STRING_QUOTE        "\""
#define C_CHAR_QUOTE          "'"

// ===== printf format specifiers =====
#define C_FMT_INT             "%d"
#define C_FMT_FLOAT           "%f"
#define C_FMT_CHAR            "%c"
#define C_FMT_STRING          "%s"
#define C_FMT_BOOL            "%s"   // a bool is printed as the string "true" / "false"

// ===== log: a bool value becomes ((value) ? "true" : "false") =====
#define C_BOOL_TO_TEXT_OPEN   "(("
#define C_BOOL_TO_TEXT_CLOSE  ") ? \"true\" : \"false\")"

// ===== printf/function names =====
#define C_PRINTF              "printf"
#define C_STRCMP              "strcmp"
#define C_ZERO                "0"
// ===== strings (src/codegen/strings.c emits the helpers these name) =====
#define C_SIZE_T              "size_t"
#define C_FREE                "free"
#define C_EMPTY_STRING        "\"\""
#define C_STR_TYPE            "char*"
#define C_STR_COPY            "W__str_copy"
#define C_STR_JOIN            "W__str_join"
#define C_STR_SET             "W__str_set"
#define C_TEMP                "W__temp"
#define C_TEMPS_FREE          "W__temps_free"
#define C_TEMPS_MARK_CALL     "W__temps_mark()"
#define C_FUNCTION_MARK       "W__fmark"      // the temporaries count when the function started
#define C_RETURN_VALUE        "W__ret"        // a return value, computed before cleaning up
#define C_ADDRESS_OF          "&"
#define C_NULL                "NULL"
#define C_UNREACHABLE         "abort();"   // compilers know it never returns
#define C_BLOCK_OPEN          "{\n"   // a bare block (C_LBRACE follows a statement head)

// ===== num arithmetic (src/codegen/arithmetic.c): wrapping + - * -x, checked / % =====
#define C_TO_UNSIGNED         "(unsigned)"
#define C_WRAP_OPEN           "(int)((unsigned)"     // ... + (unsigned)(b))
#define C_WRAP_NEG_OPEN       "(int)(0u - (unsigned)"
#define C_WRAP_CLOSE          ")"
#define C_NUM_DIV             "W__div"
#define C_CALL_LINE_PARAM     "int W__line"   // the caller's line, every W function's first parameter
#define C_CALL_LINE           "W__line"
#define C_PUSH_FRAME          "W__push"
#define C_POP_FRAME           "W__pop();"
#define C_NUM_MOD             "W__mod"
#define C_CONST_IS_MINUS_ONE  " == -1 ? "     // a global's constant division: (b) == -1 ? ...
#define C_CONST_OTHERWISE     " : "

#define C_INT_MIN             "(-2147483647 - 1)"   // C has no literal for the smallest int

#endif // W_CODEGEN_C_SYNTAX_H
