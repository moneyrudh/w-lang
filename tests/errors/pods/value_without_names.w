-- error: line 7: Values of pod 'Vec' need their field's name: Vec(field: value, ...)
pod Vec
    x: real
    y: real

fun w(): num
    dec v := Vec(1, 2)
    ret 0
