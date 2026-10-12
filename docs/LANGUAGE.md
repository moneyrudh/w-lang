# W Lang — Language Reference & Status

This is the single source of truth for what W Lang **is**, what **works today**, and what is **decided but not built yet**.

**Rule:** every change that adds, removes, or changes a language feature updates this file in the same commit. Status reflects what was *verified by running it*, not what the code looks like it should do.

Legend: ✅ works end-to-end (transpiles, compiles with gcc, runs correctly) · 🚧 partial / buggy · 📐 decided, not built · 💭 idea, not decided

---

## 1. Program structure

| Feature | Status | Notes |
|---|---|---|
| Entry point `fun w(): num` → C `main()` | ✅ | Return value becomes the process exit code |
| Functions in any order (auto forward declarations) | ✅ | |
| Name mangling (`x` → `W__x_v`, `f` → `W__f_f`) | ✅ | Lets W names like `main` coexist with C |
| Global variables (`dec` at top level) | 🚧 | AST/codegen support exists; parser rejects `dec` at top level |
| `use` imports | 💭 | Token exists, nothing parses it |

## 2. Block syntax

| Feature | Status | Notes |
|---|---|---|
| `{ }` blocks, `;` statement terminators | ✅ | **Current syntax — being replaced** |
| Indentation-based blocks (Python style) | 📐 | Spaces only; tabs are an error |
| Newline ends a statement (no `;`) | 📐 | Newlines inside `( )` are ignored, so expressions can span lines |
| `,` ends a condition — enables one-liners | 📐 | `say x is 5, stop`. Optional before an indented block |
| Parens around conditions optional | 📐 | Falls out naturally: `(cond)` is just a grouped expression |
| `.` as statement terminator | ✗ rejected | Conflicts with member access, ranges, floats |

Target syntax:
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
| `zil` | `void` | ✅ | Function return type only |
| `vec map set ref heap stack que link tree pod` | — | 💭 | Tokens only. Spec drafts disagree on syntax: `map[num, str]` vs `map(num, str)` |

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
| Block scoping | 📐 | **Bug today:** one flat symbol table, so two functions can't both declare `x` or a param `n` |

## 6. Functions

| Feature | Status | Notes |
|---|---|---|
| `fun name(a: num, b: real): num` | ✅ | |
| Omitted return type means `zil` | ✅ | |
| `ret` / `ret expr` | ✅ | Return type checked against the declaration |
| Missing `ret` in non-`zil` function is an error | ✅ | Any `ret` anywhere counts; no per-path analysis |
| Calls as expressions and statements | ✅ | |

## 7. Operators

| Operators | Status | Notes |
|---|---|---|
| `+ - * /` | ✅ | Grouping with `( )` preserved |
| `==` `eq` `is` · `!=` `ne` `is not` | ✅ | `is !x` means `== (!x)` |
| `<` `lt` · `>` `gt` · `<=` `le` · `>=` `ge` | ✅ | `str` vs `str` emits `strcmp` |
| `and` `&&` · `or` `\|\|` · `not` `!` | ✅ | `not` binds tightly like C's `!` |
| Unary minus `-x` | 📐 | `dec x: num = -3` currently fails |
| `%` modulo | 💭 | |

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
| `say cond` / `when cond` | `if (cond)` | 📐 |
| `lest cond` | `if (!(cond))` | 📐 |
| `elf cond` | `else if (cond)` | 📐 |
| `elf lest cond` / `nor cond` | `else if (!(cond))` | 📐 |
| `else` | `else` | 📐 |

Errors: `elf say`, `elf when`, `elf nor`; `elf`/`nor`/`else` without a preceding `say`/`when`/`lest`; `str` or `zil` conditions.

```
say x > 10 and not is_error
    log("big and safe")
elf lest is_blocked
    log("not blocked")
nor logged_out
    log("not logged out")
else
    log("fallback")
```

## 10. Loops

| W Lang | C | Status |
|---|---|---|
| `loop while cond` | `while (cond)` | 📐 |
| `loop until cond` | `while (!(cond))` | 📐 |
| `loop forever` | `while (true)` | 📐 |
| `skip` | `continue` | 📐 |
| `stop` | `break` | 📐 |
| `loop i in 0..10`, `loop item in v`, `loop k, v in m` | — | 💭 replaces `for` from early drafts |
| `exit` (end program) | — | 💭 |

`skip` / `stop` outside a loop is an error.

## 11. Tooling

| Feature | Status | Notes |
|---|---|---|
| `./transpiler in.w out.c` | ✅ | Exits 1 on any error; output file kept for inspection |
| `wlang` CLI (`wlang file.w`, `wlang build`, `wlang run`) | 📐 | Name decided. Arguments not yet designed |
| `brew install` via own tap (`anirudhananth/homebrew-wlang`) | 📐 | Needs CLI, `make install`, tagged release, license |
| homebrew-core (`brew install wlang`) | 💭 | Needs notability first |

## 12. Known bugs

- Flat symbol table: duplicate names across functions/params are rejected (§5)
- `str + str` emits invalid C (§4)
- `+` inside `log` is always string concatenation (§8)
- Parser cascades extra "Unexpected token" errors after a type error
- Bare `{ }` blocks inside a function parse their statements, then drop them
