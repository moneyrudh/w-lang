-- exit: 1
-- exactly 20 frames (w and 19 countdowns): all of them are shown
fun countdown(n: num): num
    say n is 0, ret 1 / n
    ret countdown(n - 1)

fun w(): num
    ret countdown(18)
