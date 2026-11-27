-- exit: 1
-- 21 frames: shortened to the 10 innermost and the 5 outermost
fun countdown(n: num): num
    say n is 0, ret 1 / n
    ret countdown(n - 1)

fun w(): num
    ret countdown(19)
