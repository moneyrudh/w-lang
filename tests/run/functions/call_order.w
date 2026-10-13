-- exit: 3
-- functions can be called before they're defined: w() comes first here,
-- is_even / is_odd call each other, factorial calls itself

fun w(): num
    dec f: num = factorial(5)
    log("factorial(5) 120 =>", f)

    dec even: bool = is_even(10)
    dec odd: bool = is_odd(7)
    log("is_even(10) 1 =>", even)
    log("is_odd(7) 1 =>", odd)

    dec half: real = halve(3)
    log("halve(3) 1.500000 =>", half)

    ret later()

fun factorial(n: num): num
    say n <= 1, ret 1
    ret n * factorial(n - 1)

fun is_even(n: num): bool
    say n is 0, ret true
    ret is_odd(n - 1)

fun is_odd(n: num): bool
    say n is 0, ret false
    ret is_even(n - 1)

-- a num argument is converted to the real parameter
fun halve(x: real): real
    ret x / 2.0

fun later(): num
    ret 3
