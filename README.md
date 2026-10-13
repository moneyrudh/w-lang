# W Lang

A transpiler that converts modern syntax to C.

## Example

**W Lang:**
```w
fun w(): num {
    dec count: num;
    dec total: num = 100;
    dec rate: real = 3.14;
    count = total + 50;
    log("Count:", count);
    ret count;
}
```

**Generated C:**
```c
int main() {
    int count = 0;
    int total = 100;
    float rate = 3.14f;
    count = total + 50;
    printf("Count is: %d\n", count);
    return count;
}
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

## Build & Run

```bash
make                                   # builds build/transpiler
./build/transpiler examples/hello.w    # writes build/hello.c
gcc build/hello.c -o build/hello
./build/hello
```

## What Works

- Variable declarations: `dec x: num = 5;`
- Type inference: `dec x := 5;`
- Functions with return types: `fun name(): num { }`
- Basic arithmetic: `+`, `-`, `*`, `/`
- Type checking and automatic conversions
- Return statements: `ret value;`
- Log statements: `log("message", variable);`

## What's Next

See `docs/LANGUAGE.md` for syntax and feature status.

Key items:
- Function parameters
- Control flow (if/else, loops)
- Comparison operators
- Data structures (map, vec, set)

## Structure

```
src/
  main.c       driver
  core/        AST
  language/    token, type and operator tables
  lexer/       flex rules + indentation layout
  parser/      recursive descent parser
  checker/     scopes and type rules
  codegen/     C code generation
  util/        hashmap
  runtime/     helpers for generated programs
include/       public header for each stage
examples/      example W programs
tests/         make test (run/ and errors/, grouped by language feature)
docs/          LANGUAGE.md: syntax and feature status
```
