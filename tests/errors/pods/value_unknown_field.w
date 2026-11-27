-- error: line 7: Pod 'Vec' has no field 'z'
pod Vec
    x: real
    y: real

fun w(): num
    dec v := Vec(x: 1, z: 2)
    ret 0
