-- error: line 14: Argument 1 of 'bump': a ref num needs a num variable, got real
fun bump(n: ref num)
    n = n + 1

fun next(n: ref num): num
    n = n + 1
    ret n

fun plain(n: num)
    log(n)

fun w(): num
    dec r := 1.5
    bump(ref r)
    ret 0
