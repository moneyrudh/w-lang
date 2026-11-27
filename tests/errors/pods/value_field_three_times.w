-- error: line 6: Field 'a' is given twice
pod P
    a: num

fun w(): num
    dec p := P(a: 1, a: 2, a: 3)
    ret 0
