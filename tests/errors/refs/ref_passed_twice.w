-- error: line 9: 'n' is passed with ref, so it can't also be used elsewhere in the same statement
fun swap(a: ref num, b: ref num)
    dec t := a
    a = b
    b = t

fun w(): num
    dec n := 1
    swap(ref n, ref n)
    ret 0
