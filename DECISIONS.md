# Design decisions

Language design choices that are the user's to make: syntax, keywords, and anything that shapes how W
feels. Each one lists a proposal and alternatives. Open questions are answered when the roadmap step that
needs them is planned; the answer then moves to **Decided** and into [`docs/LANGUAGE.md`](docs/LANGUAGE.md).

Implementation details (how the compiler does it) are not listed here.

## Open

### D7. Declaring C functions (step 4.1)
- **Proposal:** `ext fun DrawRectangle(x: num, y: num, w: num, h: num, color: Color)` with no body; the
  C name is the W name.
- Alternatives: `ext "c" fun …`; a block `ext raylib` containing several declarations.

## Decided

- **D1. Modulo** (2026-11-25): both `%` and `mod`, like `<` / `lt`.
- **D2. String memory** (2026-11-25, refined 2026-11-27): the generated C frees strings; W code never
  does. Chores C can do for the programmer, the compiler does. How (2026-11-27): each `str` variable owns
  its own copy of its text (assigning frees the old text, the copy is freed when its block ends or a
  `ret` / `stop` / `skip` leaves it); joins and `str` results of calls are temporaries freed after their
  statement; a returned `str` is handed to the caller. The first version ("freed when the function
  returns") would have grown without limit in a game's main loop, which never returns.
- **D8. `str + num`** (2026-11-25): an error, with a hint to use `log("text", value)`; an explicit
  conversion function comes later. `str + str` joins strings.
- **D9. `bool` in arithmetic** (2026-11-25): an error (`true + 1`, `n + flag`). `chr` arithmetic stays
  (`'a' + 1`). Comparisons and `and` / `or` / `not` on `bool` are unaffected.
- **D10. Printing `bool`** (2026-11-25): `log` prints `true` / `false`, not `1` / `0`.
- **D11. Integer overflow** (2026-11-27): `num` arithmetic wraps around (`2147483647 + 1` is
  `-2147483648`), like Java and Go. The generated C does `+ - * -x` on unsigned values, so it's
  defined behaviour on every C compiler, at no cost.
- **Division by zero** (2026-11-27): `/` and `%` on whole numbers stop the program with the W line
  (`W error on line 7: division by zero`, `modulo by zero` for `%`) instead of crashing. The generated C checks the divisor
  (`W__div`). `real` division follows C (`1.0 / 0` is infinity).
- **D12. Traces** (2026-11-27): run-time errors print the chain of W calls that led to them, like
  Python's traceback; always on (a later `wlang build --release` may turn them off).
- **D3. Pods** (2026-11-28): `pod Name` with an indented list of `field: type`; values are written
  with field names, `Ball(x: 10, y: 20)` (left-out fields are zero); fields are read and assigned
  with `.`. Functions on pods come later.
- **D13. Pods are values** (2026-11-28): assigning or passing a pod copies it, like a C struct (cheap:
  a pod of numbers lives on the stack). A function changes its caller's value only through a `ref`
  parameter: `fun bounce(b: ref Ball)`.
- **D4. Arrays** (2026-11-28): `vec[num]` for the type, `[1, 2, 3]` for a value, `v[0]` to index.
- **D14. Arrays are copy-on-write** (2026-11-28): assigning or passing a `vec` shares it, so it's
  as fast as passing a pointer; a real copy happens only when a shared `vec` is changed. No aliasing
  surprises, no garbage collector (values can't form cycles).
- **D15. Array operations** (2026-11-28): `v.add(x)`, `v.pop()`, `v.add_front(x)`, `v.pop_front()`,
  `v.len`. A `vec` is a ring buffer: both ends and `v[i]` are constant time.
- **D5. Looping over an array** (2026-11-28): `loop x in v`, and `loop i, x in v` with the index.
- **D6. Modules** (2026-11-28): `use name` loads `name.w` from the same folder (then the standard
  library); `use folder.name` loads `folder/name.w`. What a module defines is used qualified:
  `use math` then `math.sqrt(2)`. Bringing names in directly (`use math.sqrt`) may come later.
- **`:=` type inference** (2026-11-25): kept. The type is the type of the value, which the checker
  already computes; function signatures always spell out types, so inference stays local. A value with
  no type to take (a `zil` call, later an empty `[]`) is an error asking for the type.

- **Package manager direction:** packages are git repos installed with `wlang install` (Go-style), not
  Homebrew. Details in `docs/LANGUAGE.md` §11.
- **Distribution:** Homebrew tap `moneyrudh/w` (`brew install moneyrudh/w/wlang`).
- **Backend:** C stays the main backend; WebAssembly is a later second backend.
