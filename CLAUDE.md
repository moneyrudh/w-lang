# W Lang Transpiler

## What This Is
A transpiler that converts W Lang (`.w` files) into C code. The pipeline is: **Lexer → Parser → AST → Code Generator → `.c` output file**. The generated C is then compiled with gcc.

## How It Works
- `src/lexer.l` — Flex-based lexer. Flex's scanner is `raw_yylex()`; the `yylex()` wrapper at the bottom turns newlines + indentation into `NEWLINE` / `INDENT` / `DEDENT` (like Python's tokenizer) and sets `token_line`
- `src/parser.c` — Recursive descent parser: expressions, declarations, functions, blocks, `parse_statement()` dispatch
- `src/parser/control_flow.c` — Parsing for `say`/`when`/`lest`/`elf`/`nor`/`else`, `loop`, `skip`/`stop`
- `src/ast.c` — AST node creation and memory management
- `src/gen.c` — Walks the AST and emits C code via `fprintf` to an output file
- `src/codegen/control_flow.c` — Codegen for if-chains, loops, `continue`/`break`
- `src/codegen/formatters.c` — Helper functions for emitting C constructs (signatures, casts, operators, indentation)
- `src/transpiler/token_registry.c` — Map-backed registry of all token metadata (display names, lexemes, categories)
- `src/transpiler/type_registry.c` — Map-backed registry mapping W Lang types to C types, format specifiers, defaults
- `src/symbol_table.c` — Scoped variable table (`push_scope`/`pop_scope`) and function table, plus type rules
- `src/data_structures/map.c` — Generic hashmap used internally by registries, and will also back W Lang's `map` type
- `include/codegen/c_syntax.h` — `#define` constants for all C syntax strings (keywords, operators, punctuation, escapes)
- `include/types.h` — All enums: `TokenType`, `DataType`, `NodeType`, `OperatorType`, plus shared structs
- `src/operator_utils.c` — Table mapping operator tokens (symbol and word forms) to `OperatorType`, plus operator categories

## Goal
W Lang should feel like a modern, expressive language while transpiling cleanly to C. Keywords should be creative and intentional — not just copies of C/Java/Python. The language should have its own identity.

## Code Style & Conventions

### Reading Files
- Never read entire files unless necessary. Read specific functions, specific line ranges, or whatever is relevant to the current task.
- But don't be so conservative that you miss context or rewrite something that already exists. If unsure, read more.

### Dynamic Over Hardcoded
- Avoid raw string `fprintf` calls in codegen whenever possible.
- C syntax strings belong in `include/codegen/c_syntax.h` as `#define` constants (e.g., `C_IF`, `C_ASSIGN`, `C_SEMICOLON_NL`).
- Use the formatter helpers in `src/codegen/formatters.c` for emitting structured C constructs.
- If a string is used more than once in codegen, it should be a constant or a helper function.
- The registries (`token_registry`, `type_registry`) are map-backed. New tokens/types should be registered there, not handled via ad-hoc switch statements.

### Name Mangling
- All W Lang identifiers are mangled in the output: `W__name_v` for variables, `W__name_f` for functions.
- The entry point `w()` maps to C `main()`.

### AST Pattern
- Each new language construct needs: a `NodeType` enum value, a union member in `ASTNode`, a `create_*_node()` function, and a `free_ast()` case.
- Nodes are linked via `node->next` for statement lists. Every `create_*_node()` must set `next = NULL`.
- Types are resolved at parse time and stored on the node (`expr_type`, `target_type`, `LogElement.var_type`). Scopes are popped as soon as a block is parsed, so codegen must never look names up in the symbol table.

### Parser Pattern
- Expression parsing uses recursive descent with precedence levels: `parse_expression()` → `parse_or()` → `parse_and()` → `parse_equality()` → `parse_comparison()` → `parse_additive()` → `parse_term()` → `parse_factor()`, all built on `parse_binary_level()`.
- Statement parsing is in `parse_statement()` with a switch on the current token. Larger constructs live in their own module (e.g. `src/parser/control_flow.c`) and `parse_statement()` routes to them.
- Statements end with `expect_end_of_statement()` (consumes `NEWLINE`). Indented bodies use `parse_block()`, which opens and closes a scope.
- Report errors with `parser_error()`, which uses `token_line`. Check things *before* consuming the line's `NEWLINE`, or the error is reported on the next line. A `TYPE_ZIL` expression means an error was already reported, so don't report a second one for it.
- `eat(TokenType)` consumes and advances. The global `token` holds the current lookahead.

### Adding a New Feature (checklist)
1. **Tokens** — Add to `TokenType` enum in `types.h`, add lexer rule in `lexer.l`, register in `token_registry.c`
2. **C syntax constants** — Add output strings to `c_syntax.h`
3. **AST** — Add `NodeType`, union member, `create_*_node()` in `ast.h`/`ast.c`, update `free_ast()`
4. **Parser** — Handle in `parse_statement()` or expression parser as appropriate
5. **Codegen** — Add case in `generate()` in `gen.c`, use formatters/constants
6. **Tests** — Add `tests/run/*.w` (+ `.out`) for what should work and `tests/errors/*.w` for each new error, then `make test` and `make test-asan`
7. **Status** — Update `docs/LANGUAGE.md`

### Build
```
make clean && make
./transpiler hello_world.w output.c
gcc output.c -o output && ./output
```

### Tests
```
make test          # all tests
make test-asan     # same tests, transpiler built with AddressSanitizer + UBSan
tests/run.sh elf   # only tests whose path contains "elf"
```
- `tests/run/NAME.w` must transpile, compile with `-Wall -Wextra -Werror`, and print exactly `NAME.out`. Put `-- exit: N` at the top for a non-zero exit code.
- `tests/errors/NAME.w` must fail. List every expected error as `-- error: line N: message`. Extra errors fail the test, so error cascades get caught.
- Never generate a `.out` file without checking its contents by hand first. Prefer output lines like `label EXPECTED => ACTUAL` so the file shows its own correctness.
- `hello_world.w` is the user's playground. Don't put tests there.

## Language Status — `docs/LANGUAGE.md`
`docs/LANGUAGE.md` is the single source of truth for W Lang syntax and what is supported, partial, decided, or just an idea.
- **Read it before any language work.** Don't infer features from the code or from `grammar/` (older drafts).
- **Update it in the same change** whenever a feature is added, changed, removed, or found broken. Only mark ✅ after transpiling, compiling, and running a test program.
- Don't duplicate its tables here.
