-- exit: 1
-- a deep chain of calls is shortened: the 10 innermost frames, then the 5 outermost
fun countdown(n: num): num
    say n is 0, ret 1 / n
    ret countdown(n - 1)

fun w(): num
    ret countdown(30)
