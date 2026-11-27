-- error: line 4: This field is indented further than the ones above it
pod P
    a: num
        b: num
    c: num

fun w(): num
    dec p := P(a: 1, c: 2)
    ret 0
