-- error: line 7: Argument 1 of 'half': a ref real needs a real variable, got num
fun half(x: ref real)
    x = x / 2

fun w(): num
    dec n := 4
    half(ref n)
    ret 0
