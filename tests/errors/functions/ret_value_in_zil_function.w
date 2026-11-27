-- error: line 3: Function 'f' returns nothing (no return type), so 'ret' can't have a value
fun f()
    ret 5

fun w(): num
    f()
    ret 0
