-- exit: 110
-- the same names in different functions and scopes
fun a(n: num): num
    dec x: num = n * 2
    ret x

fun b(n: num): num
    dec x: num = n + 100
    ret x

fun w(): num
    dec x: real = 0.5
    log("x is real here:", x)
    ret a(3) + b(4)
