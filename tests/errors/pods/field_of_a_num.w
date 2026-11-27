-- error: line 8: num has no fields (looking for 'x')
pod Vec
    x: real
    y: real

fun w(): num
    dec n := 5
    log(n.x)
    ret 0
