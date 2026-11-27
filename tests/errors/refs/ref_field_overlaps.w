-- error: line 14: 'b.pos' is passed with ref, so it can't also be used elsewhere in the same statement
pod Vec
    x: num
    y: num

pod Ball
    pos: Vec

fun move(p: ref Vec, dx: num)
    p.x = p.x + dx

fun w(): num
    dec b: Ball
    move(ref b.pos, b.pos.x)
    ret 0
