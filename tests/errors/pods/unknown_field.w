-- error: line 8: Pod 'Vec' has no field 'z'
pod Vec
    x: real
    y: real

fun w(): num
    dec v := Vec(x: 1, y: 2)
    log(v.z)
    ret 0
