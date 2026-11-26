-- error: line 4: Undefined variable: 'missing'
-- a value that's already an error gives the variable no type, and its uses report nothing more
fun w(): num
    dec n := missing + 1
    log(n + 1)
    ret 0
