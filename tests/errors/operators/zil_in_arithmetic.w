-- error: line 5: zil + num: zil has no value (it's the result of a call that returns nothing)
fun nothing()
    ret
fun w(): num
    ret nothing() + 1
