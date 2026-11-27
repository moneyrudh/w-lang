-- error: line 8: Function 'next' takes 1 argument, got 2
fun next(n: ref num): num
    n = n + 1
    ret n

fun w(): num
    dec n := 1
    next(ref n, ref n)
    ret 0
