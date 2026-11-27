-- error: line 3: Function 'f' with return type 'num' doesn't return a value on every path (it can reach the end of its body)
-- a say without else can be skipped
fun f(n: num): num
    say n > 0, ret 1

fun w(): num
    ret f(1)
