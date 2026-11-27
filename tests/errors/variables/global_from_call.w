-- error: line 5: The value of global 'n' must be a constant: literals and operators, without variables, calls or text operations
fun five(): num
    ret 5

dec n := five()

fun w(): num
    n = n + 1
    ret n
