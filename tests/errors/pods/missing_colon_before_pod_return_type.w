-- error: line 5: Expected ':' before the return type
pod P
    a: num

fun f() P
    ret P(a: 1)

fun w(): num
    ret 0
