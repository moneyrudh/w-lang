-- error: line 6: Can't log a zil value: the call returns nothing
fun nothing()
    ret

fun w(): num
    log("result", nothing())
    ret 0
