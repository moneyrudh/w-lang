# Design decisions

Language design choices that are the user's to make: syntax, keywords, and anything that shapes how W
feels. Each one lists a proposal and alternatives. Open questions are answered when the roadmap step that
needs them is planned; the answer then moves to **Decided** and into [`docs/LANGUAGE.md`](docs/LANGUAGE.md).

Implementation details (how the compiler does it) are not listed here.

## Open

### D1. Modulo operator (`ROADMAP` step 2.4)
- **Proposal:** `%`, plus the word form `mod`, matching how comparisons have both (`<` / `lt`).
- Alternatives: only `%`; only `mod`.

### D2. Strings and memory (step 2.5)
`str + str` needs new memory. W has no `free`.
- **Proposal:** strings made by `+` live until the end of the function that made them, unless returned;
  the compiler frees them on `ret`. Simple, no garbage collector, covers games and small programs.
- Alternatives: never free (leaks, fine for short programs); reference counting (more work, handles
  everything); a garbage collector (largest).

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

- **Package manager direction:** packages are git repos installed with `wlang install` (Go-style), not
  Homebrew. Details in `docs/LANGUAGE.md` §11.
- **Distribution:** Homebrew tap `moneyrudh/w` (`brew install moneyrudh/w/wlang`).
- **Backend:** C stays the main backend; WebAssembly is a later second backend.
