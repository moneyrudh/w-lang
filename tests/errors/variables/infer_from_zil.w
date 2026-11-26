-- error: line 6: Variable 'x' has no type to take: the call returns nothing (zil)
fun nothing()
    ret

fun w(): num
    dec x := nothing()
    x = 5
    ret 0
