-- error: line 3: Function 'f' with return type 'num' doesn't return a value on every path (it can reach the end of its body)
-- the stop (inside a say) leaves the forever loop
fun f(n: num): num
    loop forever
        say n > 5, stop
        say n is 3, ret n
        n = n + 1

fun w(): num
    ret f(1)
