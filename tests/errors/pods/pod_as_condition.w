-- error: line 6: A condition can't be a pod (P)
pod P
    a: num

fun w(): num
    say P(a: 1), ret 1
    ret 0
