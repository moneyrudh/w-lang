-- error: line 3: Function 'f' with return type 'num' doesn't return a value on every path (it can reach the end of its body)
-- a while loop may run zero times
fun f(n: num): num
    loop while n > 0
        ret n

fun w(): num
    ret f(1)
