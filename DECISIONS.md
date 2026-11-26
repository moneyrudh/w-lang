# Design decisions

Language design choices that are the user's to make: syntax, keywords, and anything that shapes how W
feels. Each one lists a proposal and alternatives. Open questions are answered when the roadmap step that
needs them is planned; the answer then moves to **Decided** and into [`docs/LANGUAGE.md`](docs/LANGUAGE.md).

Implementation details (how the compiler does it) are not listed here.

## Open

### D11. Integer overflow (step 2, before `%`)
`num` is a C `int`. Today `2147483647 + 1` or `-x` on the smallest `num` overflows, which is
undefined behaviour in C (anything may happen; in practice it wraps around).
- **Proposal:** wrap around, and make it defined: the generated C does `num` arithmetic in a way that
  wraps like Java and Go (`2147483647 + 1` is `-2147483648`), at almost no cost.
- Alternatives: stop the program with an error on overflow (safer, like Swift and Rust's debug
  builds, a little slower); leave it undefined like C.

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
- **D2. Strings made by `+`** (2026-11-25): freed automatically by the generated C when the function that
  made them returns; returning one or storing it in a global keeps a copy. W code never frees anything:
  chores C can do for the programmer, the compiler does.
- **D8. `str + num`** (2026-11-25): an error, with a hint to use `log("text", value)`; an explicit
  conversion function comes later. `str + str` joins strings.
- **D9. `bool` in arithmetic** (2026-11-25): an error (`true + 1`, `n + flag`). `chr` arithmetic stays
  (`'a' + 1`). Comparisons and `and` / `or` / `not` on `bool` are unaffected.
- **D10. Printing `bool`** (2026-11-25): `log` prints `true` / `false`, not `1` / `0`.
- **`:=` type inference** (2026-11-25): kept. The type is the type of the value, which the checker
  already computes; function signatures always spell out types, so inference stays local. A value with
  no type to take (a `zil` call, later an empty `[]`) is an error asking for the type.

- **Package manager direction:** packages are git repos installed with `wlang install` (Go-style), not
  Homebrew. Details in `docs/LANGUAGE.md` §11.
- **Distribution:** Homebrew tap `moneyrudh/w` (`brew install moneyrudh/w/wlang`).
- **Backend:** C stays the main backend; WebAssembly is a later second backend.
