#ifndef C_SYNTAX_H
#define C_SYNTAX_H

// ===== C standard library includes =====
#define C_INCLUDE_STDIO       "#include <stdio.h>\n"
#define C_INCLUDE_STDBOOL     "#include <stdbool.h>\n"
#define C_INCLUDE_STRING      "#include <string.h>\n"
#define C_INCLUDES_BLOCK      C_INCLUDE_STDIO C_INCLUDE_STDBOOL C_INCLUDE_STRING "\n"

// ===== C keywords =====
#define C_VOID                "void"
#define C_RETURN              "return"
#define C_IF                  "if"
#define C_ELSE                "else"
#define C_WHILE               "while"
#define C_FOR                 "for"
#define C_TRUE                "true"
#define C_FALSE               "false"

// ===== punctuation & delimiters =====
#define C_LPAREN              "("
#define C_RPAREN              ")"
#define C_LBRACE              " {\n"
#define C_RBRACE              "}\n"
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

// ===== single character operators (no spacing) =====
#define C_OP_PLUS             "+"
#define C_OP_MINUS            "-"
#define C_OP_MULTIPLY         "*"
#define C_OP_DIVIDE           "/"

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
#define C_FMT_BOOL            "%d"

// ===== printf/function names =====
#define C_PRINTF              "printf"

#endif // C_SYNTAX_H
