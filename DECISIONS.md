# Design decisions

Language design choices that are the user's to make: syntax, keywords, and anything that shapes how W
feels. Each one lists a proposal and alternatives. Open questions are answered when the roadmap step that
needs them is planned; the answer then moves to **Decided** and into [`docs/LANGUAGE.md`](docs/LANGUAGE.md).

Implementation details (how the compiler does it) are not listed here.

## Open

### D3. Structs (`pod`) (step 3.1)
Early drafts used `pod` for a struct with functions and `type` for plain data.
- **Proposal:** one keyword, `pod`, plain data first; functions on pods later.
  ```
  pod Ball
      x: real
      y: real
      speed: real

  dec b: Ball = Ball(x: 10, y: 20, speed: 3)
  b.x = b.x + b.speed
  ```
- Alternatives: `type` for data and `pod` for data + functions; positional literals `Ball(10, 20, 3)`.

### D4. Arrays (`vec`) (step 3.2)
Drafts disagree: `vec[num]` vs `vec(num)`.
- **Proposal:** `vec[num]` (brackets read as "of"), literals `[1, 2, 3]`, indexing `v[0]`, length `v.len`.
- Alternatives: `vec(num)`; `num[]`.

### D5. Looping over an array (step 3.3)
- **Proposal:** `loop each x in v` (fits the existing `loop while` / `until` / `forever`).
- Alternatives: `loop x in v`; `loop over v as x`.

### D6. Modules (step 3.4)
- **Proposal:** `use name` loads `name.w` from the same folder, then the standard library; everything a
  module defines is visible to the file that uses it.
- Alternatives: explicit exports (`pub fun`); qualified names only (`math.sqrt`).

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
  (`W error on line 7: division by zero`) instead of crashing. The generated C checks the divisor
  (`W__div`). `real` division follows C (`1.0 / 0` is infinity).
- **D12. Traces** (2026-11-27): run-time errors print the chain of W calls that led to them, like
  Python's traceback; always on (a later `wlang build --release` may turn them off).
- **`:=` type inference** (2026-11-25): kept. The type is the type of the value, which the checker
  already computes; function signatures always spell out types, so inference stays local. A value with
  no type to take (a `zil` call, later an empty `[]`) is an error asking for the type.

- **Package manager direction:** packages are git repos installed with `wlang install` (Go-style), not
  Homebrew. Details in `docs/LANGUAGE.md` §11.
- **Distribution:** Homebrew tap `moneyrudh/w` (`brew install moneyrudh/w/wlang`).
- **Backend:** C stays the main backend; WebAssembly is a later second backend.
