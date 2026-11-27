-- error: line 7: Field 'x' is given twice
pod Vec
    x: real
    y: real

fun w(): num
    dec v := Vec(x: 1, x: 2)
    ret 0
