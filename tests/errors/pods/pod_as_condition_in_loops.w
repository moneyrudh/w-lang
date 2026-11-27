-- error: line 9: A condition can't be a pod (P)
-- error: line 11: A condition can't be a pod (P)
pod P
    a: num

fun w(): num
    dec b := P(a: 1)
    dec x := 1
    loop while b
        stop
    loop until b
        stop
    ret 0
