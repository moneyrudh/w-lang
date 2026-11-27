-- error: line 7: A pod value on its own does nothing (assign it or pass it)
pod Vec
    x: real
    y: real

fun w(): num
    Vec(x: 1)
    ret 0
