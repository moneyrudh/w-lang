-- error: line 3: Function 'f' with return type 'num' doesn't return a value on every path (it can reach the end of its body)
-- the elf branch doesn't return
fun f(n: num): num
    say n > 0, ret 1
    elf n < 0, log("negative")
    else, ret 0

fun w(): num
    ret f(1)
