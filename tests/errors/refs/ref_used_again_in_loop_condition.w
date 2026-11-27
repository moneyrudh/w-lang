-- error: line 8: 'n' is passed with ref, so it can't also be used elsewhere in the same statement
fun next(n: ref num): num
    n = n + 1
    ret n

fun w(): num
    dec n := 1
    loop while next(ref n) < n
        stop
    ret 0
