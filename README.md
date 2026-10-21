# W Lang

A small language with its own keywords and indentation-based blocks that compiles to C.

## Example

```w
fun square(n: num): num
    ret n * n

fun w(): num
    dec i: num = 0
    loop while i < 3
        i = i + 1
        dec sq: num = square(i)
        log(i, "squared is", sq)
    ret 0
```

```
$ wlang squares.w
1 squared is 1
2 squared is 4
3 squared is 9
```

`wlang build squares.w --emit-c` keeps the generated C next to the executable:

```c
int W__square_f(int W__n_v) {
    return W__n_v * W__n_v;
}

int main(void) {
    int W__i_v = 0;
    while (W__i_v < 3) {
        W__i_v = W__i_v + 1;
        int W__sq_v = W__square_f(W__i_v);
        printf("%d squared is %d\n", W__i_v, W__sq_v);
    }
    return 0;
}
```

## Install

With [Homebrew](https://brew.sh):

```bash
brew install moneyrudh/w/wlang
```

From source (needs a C compiler, `make` and `flex`):

```bash
make
make install                  # installs /usr/local/bin/wlang (set PREFIX=... to change)
```

`wlang` compiles the generated C with your system C compiler (`$CC`, else `cc`).

## Usage

```bash
wlang hello.w                 # build and run (same as: wlang run hello.w)
wlang build hello.w           # writes the executable ./hello
wlang build hello.w --emit-c  # also keeps ./hello.c
wlang check hello.w           # report errors only
wlang --help                  # all options (-o, --emit-c, --cc)
```

## Types

```
num  -> int
real -> float
chr  -> char
str  -> char*
bool -> bool
zil  -> void
```

## Language

[`docs/LANGUAGE.md`](docs/LANGUAGE.md) has the full syntax and what works today, what's decided, and what's still an idea. [`examples/hello.w`](examples/hello.w) is a short tour.

## Development

```bash
make                # builds build/wlang
make test           # language tests, then tests of the wlang command
make test-asan      # same, with AddressSanitizer + UBSan
```

```
src/
  main.c       the wlang command
  driver/      transpile pipeline, C compiler and program runner
  core/        AST, error reporting
  language/    token, type and operator tables
  lexer/       flex rules + indentation layout
  parser/      recursive descent parser
  checker/     name resolution and type checking
  codegen/     C code generation
  util/        hashmap
  runtime/     helpers for generated programs
include/       public header for each stage
examples/      example W programs
tests/         make test (run/ and errors/ by language feature, driver/ for the wlang command)
docs/          LANGUAGE.md: syntax and feature status
```

## License

[MIT](LICENSE)
