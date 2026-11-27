-- pods: named fields, used as values (copied, D13) (each line: expected => actual)
pod Vec
    x: real
    y: real

pod Ball
    pos: Vec
    speed: real
    alive: bool
    hits: num
    tag: chr

dec origin: Vec = Vec(x: -1, y: 2.5)

fun moved(b: Ball): Ball
    -- b is this function's own copy
    b.pos.x = b.pos.x + b.speed
    ret b

fun make(): Ball
    ret Ball(pos: Vec(x: 7, y: 8), tag: 'm')

fun w(): num
    dec b := Ball(pos: Vec(x: 1, y: 2), speed: 3, alive: true, tag: 'b')
    log("fields 1.000000 2.000000 3.000000 true 0 b =>", b.pos.x, b.pos.y, b.speed, b.alive, b.hits, b.tag)
    dec c := moved(b)
    log("the caller's copy is unchanged 1.000000 4.000000 =>", b.pos.x, c.pos.x)
    dec d := b
    d.hits = 5
    log("assigning copies 0 5 =>", b.hits, d.hits)
    log("left-out fields are zero false 0 =>", Ball(speed: 1).alive, Ball(speed: 1).hits)
    dec empty: Ball
    log("a pod with no value is zero 0.000000 false =>", empty.speed, empty.alive)
    log("a global pod -1.000000 2.500000 =>", origin.x, origin.y)
    log("fields of a call and of a value 7.000000 3.000000 =>", make().pos.x, Vec(x: 3, y: 4).x)
    log("logging a pod =>", Vec(x: 1, y: 2), "and", 9)
    ret 0
