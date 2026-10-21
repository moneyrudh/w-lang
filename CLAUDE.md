# W Lang Transpiler

## What This Is
A transpiler that converts W Lang (`.w` files) into C code. The pipeline is: **Lexer → Parser → (untyped AST) → Checker → (typed AST) → Code Generator → `.c` output file**. Each stage runs only if the stages before it reported no errors. The `wlang` command wraps it: it then compiles the C with the system C compiler and, for `wlang run`, runs the result.

## How It Works
Each stage is a folder under `src/`. Its public API is one header in `include/` (`lexer.h`, `parser.h`, `checker.h`, `codegen.h`); helpers shared only inside the stage go in `src/<stage>/internal.h`. Other stages must not include another stage's `internal.h`.

- `src/main.c` — The `wlang` command: parses arguments and runs `run` / `build` / `check` (`include/version.h` holds the version)
- `src/driver/` — What the commands are built from (`include/driver.h`):
  - `pipeline.c` — `transpile()`: lex → parse → check → codegen for one file, including registry setup
  - `toolchain.c` — Calling the C compiler (`$CC`, else `cc`), running the program, temp directories
- `src/core/ast.c` — AST node creation and memory management (`include/ast.h`). Constructors only build nodes; they never look names up or report errors. `include/types.h` holds the shared enums: `TokenType`, `DataType`, `NodeType`, `OperatorType`, plus shared structs
- `src/core/diagnostics.c` — `report_error(line, fmt, …)` and `error_count()`, used by every stage (`include/diagnostics.h`)
- `src/language/` — W's definition as tables, no stage logic:
  - `token_registry.c` — Map-backed registry of all token metadata (display names, lexemes, categories)
  - `type_registry.c` — Map-backed registry mapping W Lang types to C types, format specifiers, defaults
  - `operators.c` — Table mapping operator tokens (symbol and word forms) to `OperatorType`, plus operator categories
- `src/lexer/` — `lexer.l` is the flex rules (scanner is `raw_yylex()`, generated into `build/lexer.c`, never committed). `layout.c` is the `yylex()` wrapper that turns newlines + indentation into `NEWLINE` / `INDENT` / `DEDENT` (like Python's tokenizer) and sets `token_line`
- `src/parser/` — Recursive descent parser. Syntax only: no scopes, no types
  - `parser.c` — Token handling (`eat`), errors, blocks, `parse_statement()` dispatch, top level
  - `expressions.c` — Precedence chain and operands
  - `statements.c` — Variable declarations, assignment, `log`, type specifiers
  - `functions.c` — Definitions, parameters, calls, `ret`
  - `control_flow.c` — `say`/`when`/`lest`/`elf`/`nor`/`else`, `loop`, `skip`/`stop`
- `src/checker/` — Semantic analysis. `check_program()` declares every function first (so calls can precede definitions), then checks each body: resolves names, fills in types, reports type errors
  - `checker.c` — Entry point, the two passes, entry point check
  - `expressions.c` — `check_expression()`: sets `node->expr_type`
  - `statements.c` — `dec`, assignment, `log`, blocks (`check_block()` opens a scope)
  - `functions.c` — Signatures, bodies and return checks, calls (argument count and types)
  - `control_flow.c` — Conditions, loops, `skip`/`stop` placement
  - `scopes.c` — Scoped variable table (`push_scope`/`pop_scope`) and function table
  - `types.c` — Pure type rules (`compare_types`, `can_convert_type`, `operation_type`); no AST, no printing
- `src/codegen/` — Walks the AST and emits C via `fprintf`:
  - `codegen.c` — `generate()` dispatch and the program layout (includes, forward declarations, `w()` last)
  - `expressions.c`, `statements.c`, `functions.c`, `control_flow.c` — One file per kind of construct
  - `formatters.c` — Helpers for emitting C constructs (mangling, signatures, casts, operators, indentation)
  - `c_syntax.h` — `#define` constants for all C syntax strings (keywords, operators, punctuation, escapes)
- `src/util/map.c` — Generic hashmap used internally by the registries
- `src/runtime/` — Helpers for generated programs (not yet linked into them)

## Goal
W Lang should feel like a modern, expressive language while transpiling cleanly to C. Keywords should be creative and intentional — not just copies of C/Java/Python. The language should have its own identity.

## Code Style & Conventions

### Reading Files
- Never read entire files unless necessary. Read specific functions, specific line ranges, or whatever is relevant to the current task.
- But don't be so conservative that you miss context or rewrite something that already exists. If unsure, read more.

### Dynamic Over Hardcoded
- Avoid raw string `fprintf` calls in codegen whenever possible.
- C syntax strings belong in `src/codegen/c_syntax.h` as `#define` constants (e.g., `C_IF`, `C_ASSIGN`, `C_SEMICOLON_NL`).
- Use the formatter helpers in `src/codegen/formatters.c` for emitting structured C constructs.
- If a string is used more than once in codegen, it should be a constant or a helper function.
- The registries (`token_registry`, `type_registry`) are map-backed. New tokens/types should be registered there, not handled via ad-hoc switch statements.

### Name Mangling
- All W Lang identifiers are mangled in the output: `W__name_v` for variables, `W__name_f` for functions.
- The entry point `w()` maps to C `main()`.

### AST Pattern
- Each new language construct needs: a `NodeType` enum value, a union member in `ASTNode`, a `create_*_node()` function, and a `free_ast()` case.
- Nodes are linked via `node->next` for statement lists. Every `create_*_node()` must set `next = NULL`.
- The checker stores every resolved type on the node (`expr_type`, `target_type`, `LogElement.var_type`). Its scopes are gone by the time codegen runs, so codegen reads those fields and never looks names up.
- `node->location.line` is the line the construct starts on (set from `current_location()` in the parser). Checker errors are reported on it.

### Parser Pattern
- Expression parsing uses recursive descent with precedence levels: `parse_expression()` → `parse_or()` → `parse_and()` → `parse_equality()` → `parse_comparison()` → `parse_additive()` → `parse_term()` → `parse_factor()`, all built on `parse_binary_level()`.
- Statement parsing is in `parse_statement()` (`src/parser/parser.c`) with a switch on the current token. Each construct lives in the file for its kind (`statements.c`, `functions.c`, `control_flow.c`, …) and `parse_statement()` routes to it. Declare cross-file parser functions in `src/parser/internal.h`, not `include/parser.h`.
- Statements end with `expect_end_of_statement()` (consumes `NEWLINE`). Indented bodies use `parse_block()`.
- The parser reports only syntax errors, with `parser_error()` (the current token's line). Anything about names or types belongs in the checker. Name tokens in messages with `token_registry_describe()` (`')'`, `'fun'`, `end of line`), never with enum numbers or `display_name`.

### Checker Pattern
- Report errors with `report_error(node->location.line, fmt, …)`.
- `check_expression()` returns `TYPE_ERROR` when an error was already reported for that expression; don't report a second one for it. `TYPE_ZIL` is a real type (a `zil` call's result) and is reported normally.
- Check an expression before declaring the name it initializes, and declare names even after a type error, so later uses don't cascade into "undefined" errors.
- `eat(TokenType)` consumes and advances. The global `token` holds the current lookahead.

### Adding a New Feature (checklist)
1. **Tokens** — Add to `TokenType` enum in `types.h`, add lexer rule in `src/lexer/lexer.l`, register in `src/language/token_registry.c`
2. **C syntax constants** — Add output strings to `src/codegen/c_syntax.h`
3. **AST** — Add `NodeType`, union member, `create_*_node()` in `ast.h`/`ast.c`, update `free_ast()`
4. **Parser** — Route from `parse_statement()` or the expression parser to the matching `src/parser/` file (syntax only)
5. **Checker** — Add a case in `check_statement()` or `expression_type()` that calls into the matching `src/checker/` file: resolve names, fill in types, report semantic errors
6. **Codegen** — Add a case in `generate()` (`src/codegen/codegen.c`) that calls into the matching `src/codegen/` file, use formatters/constants
7. **Tests** — Add `tests/run/<feature>/*.w` (+ `.out`) for what should work and `tests/errors/<feature>/*.w` for each new error, then `make test` and `make test-asan`
8. **Status** — Update `docs/LANGUAGE.md`

### Build
```
make clean && make                          # everything goes to build/ (gitignored)
./build/wlang examples/hello.w              # build and run
./build/wlang build examples/hello.w --emit-c out.c   # see the generated C (also writes ./hello)
./build/wlang check examples/hello.w        # errors only
```

### Tests
```
make test          # all tests: tests/run.sh, then tests/driver/test.sh
make test-asan     # same tests, wlang built with AddressSanitizer + UBSan
tests/run.sh loops # only tests whose path contains "loops" (a feature folder or a file name)
```
- Tests are grouped by language feature, named after the sections of `docs/LANGUAGE.md` (`blocks`, `comments`, `types`, `variables`, `functions`, `operators`, `control_flow`, `loops`, …), not by compiler stage: every test runs the whole pipeline. Add a new folder when a new section gets its first test.
- `tests/run/<feature>/NAME.w` must build with `wlang build` (C compiled with `-Wall -Wextra -Werror`) and print exactly `NAME.out`. Put `-- exit: N` at the top for a non-zero exit code.
- `tests/errors/<feature>/NAME.w` must fail `wlang check`. List every expected error as `-- error: line N: message`. Extra errors fail the test, so error cascades get caught.
- `tests/driver/test.sh` tests the `wlang` command itself (commands, options, exit codes, which files get written). Add a case there when the command line changes.
- Never generate a `.out` file without checking its contents by hand first. Prefer output lines like `label EXPECTED => ACTUAL` so the file shows its own correctness.
- `playground/` is the user's private scratch space (gitignored). Don't put tests there.
- `examples/` holds public, tracked example programs. Keep them working: transpile and run them after language changes.

## Language Status — `docs/LANGUAGE.md`
`docs/LANGUAGE.md` is the single source of truth for W Lang syntax and what is supported, partial, decided, or just an idea.
- **Read it before any language work.** Don't infer features from the code or from `grammar/` (older drafts).
- **Update it in the same change** whenever a feature is added, changed, removed, or found broken. Only mark ✅ after transpiling, compiling, and running a test program.
- Don't duplicate its tables here.
