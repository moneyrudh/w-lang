-- error: line 5: Cannot perform operations on zil type
fun nothing()
    ret
fun w(): num
    ret nothing() + 1
