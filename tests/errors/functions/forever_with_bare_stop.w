-- error: line 3: Function 'f' with return type 'num' doesn't return a value on every path (it can reach the end of its body)
-- a stop directly in the loop's body
fun f(n: num): num
    loop forever
        say n is 3, ret n
        stop

fun w(): num
    ret f(1)
