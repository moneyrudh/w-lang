-- error: line 8: Type mismatch in assignment to 'v': cannot assign str to real
pod Vec
    x: real
    y: real

fun w(): num
    dec v := Vec(x: 1)
    v.x = "one"
    ret 0
