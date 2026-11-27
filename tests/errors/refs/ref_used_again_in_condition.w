-- error: line 14: 'n' is passed with ref, so it can't also be used elsewhere in the same statement
fun bump(n: ref num)
    n = n + 1

fun next(n: ref num): num
    n = n + 1
    ret n

fun plain(n: num)
    log(n)

fun w(): num
    dec n := 1
    say next(ref n) is n, ret 1
    ret 0
