-- error: line 7: Vec + Vec: operators don't work on pods (use their fields)
pod Vec
    x: real
    y: real

fun w(): num
    dec v := Vec(x: 1) + Vec(x: 2)
    ret 0
