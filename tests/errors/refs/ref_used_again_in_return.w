-- error: line 7: 'n' is passed with ref, so it can't also be used elsewhere in the same statement
fun next(n: ref num): num
    n = n + 1
    ret n

fun f(n: num): num
    ret next(ref n) + n

fun w(): num
    ret 0
