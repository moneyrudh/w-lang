-- error: line 14: Argument 1 of 'bump' is a ref parameter: pass it with ref (the function can change it)
fun bump(n: ref num)
    n = n + 1

fun next(n: ref num): num
    n = n + 1
    ret n

fun plain(n: num)
    log(n)

fun w(): num
    dec n := 1
    bump(n)
    ret 0
