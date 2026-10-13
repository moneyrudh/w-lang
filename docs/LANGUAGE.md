# W Lang — Language Reference & Status

This is the single source of truth for what W Lang **is**, what **works today**, and what is **decided but not built yet**.

**Rule:** every change that adds, removes, or changes a language feature updates this file in the same commit. Status reflects what was *verified by running it*, not what the code looks like it should do. A ✅ should be covered by a test in `tests/` (`make test`).

Legend: ✅ works end-to-end (transpiles, compiles with gcc, runs correctly) · 🚧 partial / buggy · 📐 decided, not built · 💭 idea, not decided

---

## 1. Program structure

| Feature | Status | Notes |
|---|---|---|
| Entry point `fun w(): num` → C `main()` | ✅ | Return value becomes the process exit code |
| Functions in any order | ✅ | The checker collects every signature before checking bodies, so calls may come before definitions (including mutual recursion) |
| Name mangling (`x` → `W__x_v`, `f` → `W__f_f`) | ✅ | Lets W names like `main` coexist with C |
| Global variables (`dec` at top level) | 🚧 | AST/codegen support exists; parser rejects `dec` at top level |
| `use` imports | 💭 | Token exists, nothing parses it |
| Macros, namespaces | 💭 | Listed as top-level constructs in early drafts |

## 2. Block syntax

| Feature | Status | Notes |
|---|---|---|
| Indentation-based blocks (Python style) | ✅ | Spaces only; tabs, an indented first line, and dedents that match no outer block are errors |
| Newline ends a statement | ✅ | Newlines inside `( )` `[ ]` `{ }` are ignored, so expressions can span lines. Blank and comment-only lines are ignored |
| `,` ends a condition — enables one-liners | ✅ | `say x is 5, stop`. Optional before an indented block. After `else` / `loop forever` (no condition) the comma is optional: `else stop` |
| Parens around conditions optional | ✅ | `(cond)` is just a grouped expression |
| `;` | ✗ removed | Error: "W doesn't use ';'" |
| `{ }` blocks | ✗ removed | `{ }` reserved for future map literals |
| `.` as statement terminator | ✗ rejected | Conflicts with member access, ranges, floats |
| Empty block placeholder (like Python's `pass`) | 💭 | A block currently needs at least one statement |

Example:
```
fun w(): num
    dec x: num = 0
    loop while x < 10
        x = x + 1
        say x is 3, skip
        say x is 9, stop
    ret x
```

## 3. Comments

| Syntax | Status | Meaning |
|---|---|---|
| `-- text` | ✅ | Single-line throwaway comment (also after code) |
| `--- … ---` | ✅ | Multi-line throwaway comment; unclosed is an error |
| `note text` | ✅ | Doc comment; stack consecutive `note` lines for multi-line docs |
| Doc comments carried into the C output | 💭 | Currently discarded like throwaway comments |

Gotcha: `x--1` is `x` followed by a comment.

## 4. Types

| W Lang | C | Status | Notes |
|---|---|---|---|
| `num` | `int` | ✅ | |
| `real` | `float` | ✅ | |
| `chr` | `char` | ✅ | Escapes `\n \t \\ \' \"` |
| `str` | `char*` | 🚧 | Literals, variables, comparison work. `a + b` passes type check but emits invalid C |
| `bool` | `bool` | ✅ | `true` / `false`. Prints as `1` / `0` |
| `zil` | `void` | ✅ | Function return type only: a `zil` variable or parameter is an error, and so is using a `zil` call's result as a value |
| `vec map set ref heap stack que link tree pod` | — | 💭 | Tokens only. Spec drafts disagree on syntax: `map[num, str]` vs `map(num, str)` |
| `pod` (struct with functions) vs `type` (plain data) | — | 💭 | |

Collection ideas from early drafts (💭): data structures ship with the transpiler and are pulled in with `use`; `map` uses a random hash seed per run; `que` may get a priority option; `link[Node]` / `tree[Node]` take a `pod`; `ref` (pointers) may be dropped entirely. Literal syntax drafts:
```
dec m: map(num, str) = {2: "two", 3: "three"}
dec q: que(str) = ["one", "two", "three"]
```

Implicit conversions: `num ↔ chr`, `num ↔ bool`, `num/chr/bool → real`. `str` converts to nothing.

## 5. Variables

| Feature | Status | Notes |
|---|---|---|
| `dec x: num = 5` | ✅ | |
| `dec x: num` (default value) | ✅ | Defaults from the type registry (`num` → 0) |
| Assignment `x = expr` | ✅ | Type-checked |
| `dec x := 5` (type inference) | 📐 | Token exists; parser rejects it |
| `x: num = 5` (without `dec`) | 🚧 | Old notes claim it works; it does not |
| `+=`, `++` | 💭 | In early spec drafts only |
| Block scoping | ✅ | Every block and one-line body is a scope. Inner blocks may shadow outer names; redeclaring in the same scope is an error. Parameters share the function body's scope (as in C) |

## 6. Functions

| Feature | Status | Notes |
|---|---|---|
| `fun name(a: num, b: real): num` | ✅ | |
| Omitted return type means `zil` | ✅ | |
| `ret` / `ret expr` | ✅ | Return type checked against the declaration |
| Missing `ret` in non-`zil` function is an error | ✅ | Any `ret` anywhere counts; no per-path analysis |
| Calls as expressions and statements | ✅ | |
| Argument count and types checked against the signature | ✅ | Arguments convert like assignments (e.g. `num` → `real`) |

## 7. Operators

| Operators | Status | Notes |
|---|---|---|
| `+ - * /` | ✅ | Grouping with `( )` preserved |
| `==` `eq` `is` · `!=` `ne` `is not` | ✅ | `is !x` means `== (!x)` |
| `<` `lt` · `>` `gt` · `<=` `le` · `>=` `ge` | ✅ | `str` vs `str` emits `strcmp` |
| `and` `&&` · `or` `\|\|` · `not` `!` | ✅ | `not` binds tightly like C's `!` |
| Unary minus `-x` | 📐 | `dec x: num = -3` currently fails |
| `%` modulo | 💭 | |
| Ternary `cond ? a : b` | 💭 | Early drafts; may get a W-style spelling instead |

Precedence, lowest to highest: `or` → `and` → equality → comparison → `+ -` → `* /` → unary.

## 8. Output

| Feature | Status | Notes |
|---|---|---|
| `log("text", x, y)` | ✅ | `,` joins with a space, ends with newline |
| `log("a" + x)` | ✅ | `+` inside `log` concatenates without a space |
| `log(x + 1)` | 🚧 | **Bug:** prints `11` for `x = 1` — `+` is always concatenation inside `log` |
| `log(f())`, `log(x > y)` | 🚧 | Only literals and plain variables are accepted |

## 9. Control flow

| W Lang | C | Status |
|---|---|---|
| `say cond` / `when cond` | `if (cond)` | ✅ |
| `lest cond` | `if (!(cond))` | ✅ |
| `elf cond` | `else if (cond)` | ✅ |
| `elf lest cond` / `nor cond` | `else if (!(cond))` | ✅ |
| `else` | `else` | ✅ |

Errors (all ✅): `elf say`, `elf when`, `elf nor` (with the fix in the message); `elf`/`nor`/`else` without a preceding `say`/`when`/`lest`; a `str` condition; a call to a `zil` function as a condition.

```
say x > 10 and not is_error
    log("big and safe")
elf lest is_blocked
    log("not blocked")
nor logged_out, log("not logged out")
else
    log("fallback")
```

## 10. Loops

| W Lang | C | Status |
|---|---|---|
| `loop while cond` | `while (cond)` | ✅ |
| `loop until cond` | `while (!(cond))` | ✅ |
| `loop forever` | `while (true)` | ✅ |
| `skip` | `continue` | ✅ |
| `stop` | `break` (innermost loop) | ✅ |
| `loop i in 0..10`, `loop item in v`, `loop k, v in m` | — | 💭 replaces `for` from early drafts |
| `exit` (end program) | — | 💭 |

Errors (all ✅): `loop` without `while`/`until`/`forever`; `skip` / `stop` outside a loop.

```
loop while i < 10
    i = i + 1
    say i is 3, skip
    say i is 8, stop
loop until n >= 5, n = n + 1
```

## 11. Tooling

| Feature | Status | Notes |
|---|---|---|
| `build/transpiler in.w [out.c]` | ✅ | Output defaults to `build/<name>.c`. Exits 1 on any error. A program with errors isn't generated (output file is left empty). Syntax errors are reported first; type errors only once there are none, so one typo doesn't cascade |
| `wlang` CLI | 📐 | See below |

```
wlang hello.w [args…]           build to a temp dir and run; exit code = w()'s return value
wlang run hello.w [args…]       same (bare form is shorthand)
wlang build hello.w             native binary → ./hello
      -o <name>                 output name
      --emit-c [path]           also keep the generated C (default hello.c)
      --cc <compiler>           C compiler (default: $CC, else cc)
wlang check hello.w             type-check only; no C compiler needed
wlang --version | --help
```
Arguments after the `.w` file are passed to the program (reading them from W is 💭).

| Feature | Status | Notes |
|---|---|---|
| `brew install` via own tap (`anirudhananth/homebrew-wlang`) | 📐 | Needs CLI, `make install`, tagged release, license |
| homebrew-core (`brew install wlang`) | 💭 | Needs notability first |
| W packages (installable libraries, imported with `use`) | 💭 | |

## 12. Known bugs

- `str + str` emits invalid C (§4)
- `+` inside `log` is always string concatenation (§8)
- Return checking isn't per-path: any `ret` anywhere satisfies "must return a value" (§6)
- An unclosed `(`, `[` or `{` makes every following line part of the same statement, so the errors that follow can be confusing
