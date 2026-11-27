-- error: line 3: Function 'f' with return type 'num' doesn't return a value on every path (it can reach the end of its body)
-- the else branch has a say without its own else
fun f(n: num): num
    say n > 0, ret 1
    else
        say n < -5, ret 2

fun w(): num
    ret f(1)
