# Roadmap

Where W is going and in what order. The long-term goal is **simple games written in W** (raylib through
W wrappers), then playing them in a browser.

How this file is used:
- Work happens one **step** at a time. Before a step starts, its plan (including any syntax choices from
  [`DECISIONS.md`](DECISIONS.md)) is approved; then it's built as one or more PRs, each reviewed, merged on
  green CI, and checked off here.
- Every task is small enough for one PR and says when it's **done**. "Done" always also means: tests
  added, `make test` / `make test-asan` / CI green, `docs/LANGUAGE.md` updated.
- [`docs/LANGUAGE.md`](docs/LANGUAGE.md) stays the source of truth for what works. This file is the plan.

Status: ✅ done · 🔨 in progress · ⏳ next · ⬜ later · ❓ blocked on a decision

---

## Step 1 — Automation ✅

- ✅ CI on every PR and push: build with `-Werror`, tests, sanitizers (leaks on Linux), static analyzer, fuzzer
- ✅ `ROADMAP.md`, `DECISIONS.md`, and the working process in `CLAUDE.md`

## Step 2 — Finish the core language 🔨

1. ✅ **Parser error recovery.** Done: an unclosed `(` / `[` / `{` reports `'(' was never closed`
   where it opened when a statement or the end of the file follows (a line starting with a name is
   reported where it stops making sense); one mistake gives one error (`fun f(a: num b: num)`, `fun w() num`); `eat()` no
   longer exits the process.
2. ✅ **Nesting limit.** Done: deep nesting and long chains give one error instead of a segfault
   (150 expression levels, 100 statement levels, 1,000 branches per conditional, sized so the C compiles
   with clang); long statement lists and `elf` chains no longer overflow `free_ast()`; the fuzzer gains a deep-nesting mutation.
3. ✅ **`log` takes any expression.** Done: `log(x + 1)` prints `2` for `x = 1`, `log(f(), x > y)` work,
   `bool` prints `true` / `false` (D10), and one test covers every type × operator pair (D8, D9);
   pieces are separated by spaces as today.
4. ✅ **Unary minus.** Done: `-x`, `-3`, `-(a + b)`, `- -x` for `num` and `real`.
5. ⬜ **`%` / `mod` modulo** for `num` (D1).
6. ⬜ **`str + str`.** Done: concatenation produces a new string (needs a small runtime: allocation and
   freeing rules, D2): the generated C frees it, W code never does.
7. ⬜ **`dec x := 5`** type inference. Done: the type comes from the initializer; `:=` without a
   value is an error.
8. ⬜ **Global variables** (`dec` at top level). Done: readable and assignable from every function;
   initializers must be constant.
9. ⬜ **Per-path return checking.** Done: a `ret` inside only one branch of `say` no longer satisfies
   "must return a value".
10. ⬜ **Integer literal range.** Done: a literal that doesn't fit `num` is an error instead of `atoi`'s
   undefined result.

## Step 3 — Data ⬜

1. ⬜ **Structs (`pod`).** Declaration, literals, field access and assignment, passing to functions. ❓ D3.
2. ⬜ **Arrays (`vec`).** Fixed size first, then growable; indexing with bounds checks. ❓ D4.
3. ⬜ **Loops over arrays.** ❓ D5 (W-style `for each` spelling).
4. ⬜ **Modules (`use`).** A program across several `.w` files. ❓ D6.

## Step 4 — Games ⬜

1. ⬜ **Calling C functions** (`ext fun …`), including C structs passed by value. ❓ D7.
2. ⬜ **`raylib.w`**: declarations plus W-style wrappers (`draw_rect`, `key_down`, a game loop helper).
   Done: a window opens and draws from a W program; `wlang` links raylib when a program uses it.
3. ⬜ **Pong** in W, in `examples/games/`.
4. ⬜ **Snake**, then **Breakout**.

## Step 5 — Web ⬜

1. ⬜ Site for W (docs from `LANGUAGE.md`, install instructions).
2. ⬜ Games compiled to WebAssembly (raylib + emscripten) and playable on the site.
3. ⬜ A small IR between the checker and codegen, then a WebAssembly backend for an in-browser playground.

## Tooling, any time ⬜

- ⬜ Make `transpile()` re-entrant (no process-global state), then a libFuzzer target alongside the
  mutation fuzzer.
- ⬜ Fuzzer oracle: every program `wlang check` accepts must produce C that compiles.
- ⬜ Release script: tag, GitHub release, bump the Homebrew formula (still asks before running).
