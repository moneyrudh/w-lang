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
make
./transpiler input.w output.c
gcc output.c -o program
./program
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

See `grammar/status.txt` for implementation roadmap.

Key items:
- Function parameters
- Control flow (if/else, loops)
- Comparison operators
- Data structures (map, vec, set)

## Structure

```
/include
  /data_structures   Generic HashMap (used internally & for map(T,U))
  /transpiler        Type registry and utilities
  /runtime           Helpers for generated code
/src                 Implementation
/grammar             Language specs and status
```
