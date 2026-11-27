-- error: line 14: Argument 1 of 'plain' isn't a ref parameter: drop the 'ref'
fun bump(n: ref num)
    n = n + 1

fun next(n: ref num): num
    n = n + 1
    ret n

fun plain(n: num)
    log(n)

fun w(): num
    dec n := 1
    plain(ref n)
    ret 0
