-- error: line 13: Argument 1 of 'bump': only a variable or a field of one can be passed with ref
fun bump(n: ref num)
    n = n + 1

fun next(n: ref num): num
    n = n + 1
    ret n

fun plain(n: num)
    log(n)

fun w(): num
    bump(ref 3)
    ret 0
