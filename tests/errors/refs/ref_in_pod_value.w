-- error: line 7: 'ref' is for a function's ref parameters, not a pod's values
pod P
    a: num

fun w(): num
    dec n := 1
    dec p := P(a: ref n)
    ret 0
