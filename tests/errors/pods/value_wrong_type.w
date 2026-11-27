-- error: line 7: Field 'x' of 'Vec': cannot use str as real
pod Vec
    x: real
    y: real

fun w(): num
    dec v := Vec(x: "one")
    ret 0
