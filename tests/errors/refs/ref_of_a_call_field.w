-- error: line 12: Argument 1 of 'bump': only a variable or a field of one can be passed with ref
pod P
    a: num

fun make(): P
    ret P(a: 1)

fun bump(n: ref num)
    n = n + 1

fun w(): num
    bump(ref make().a)
    ret 0
