-- error: line 12: A condition can't be a pod (P)
-- error: line 13: A condition can't be a pod (P)
-- error: line 15: A condition can't be a pod (P)
-- error: line 16: A condition can't be a pod (P)
pod P
    a: num

fun w(): num
    dec b := P(a: 1)
    dec x := 1
    -- every branch takes a condition
    when b, ret 1
    lest P(), ret 2
    say x is 2, ret 3
    elf b, ret 4
    nor b, ret 5
    ret 0
