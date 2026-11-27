-- ref parameters change the caller's variable; everything else is still copied
pod Vec
    x: real
    y: real

pod Ball
    pos: Vec
    vel: Vec
    bounces: num

dec score := 0

fun bump(n: ref num)
    n = n + 1

fun bounce(b: ref Ball)
    b.vel.y = -b.vel.y
    b.bounces = b.bounces + 1

fun step(p: ref Vec, v: Vec)
    p.x = p.x + v.x
    p.y = p.y + v.y

-- a ref passed on to another ref
fun bounce_twice(b: ref Ball)
    bounce(ref b)
    bounce(ref b)

-- a ref read as a value is copied
fun copy_of(b: ref Ball): Ball
    dec c := b
    c.bounces = 100
    ret c

fun reset(b: ref Ball)
    b = Ball(pos: Vec(x: 0, y: 0))

fun grow(s: ref str)
    s = s + "!"

fun swap(a: ref str, b: ref str)
    dec t := a
    a = b
    b = t

-- a local shadowing a ref parameter is its own variable
fun shadow(s: ref str)
    say true
        dec s := "inner"
        log("shadow inner =>", s)
    s = s + "?"

fun flag(f: ref bool)
    f = not f

-- returning a ref str returns a copy of the caller's text
fun shout(s: ref str): str
    s = s + "!"
    ret s

fun next(n: ref num): num
    n = n + 1
    ret n

fun w(): num
    dec n := 1
    bump(ref n)
    bump(ref n)
    log("n 3 =>", n)

    bump(ref score)
    log("global score 1 =>", score)

    dec b := Ball(pos: Vec(x: 1, y: 2), vel: Vec(x: 3, y: 4))
    bounce(ref b)
    log("vel.y -4 =>", b.vel.y, "bounces 1 =>", b.bounces)
    step(ref b.pos, b.vel)
    log("pos 4 -2 =>", b.pos.x, b.pos.y)
    bounce_twice(ref b)
    log("bounces 3 =>", b.bounces)
    dec c := copy_of(ref b)
    log("copy 100 =>", c.bounces, "original 3 =>", b.bounces)
    reset(ref b)
    log("reset =>", b)

    dec s := "hi"
    grow(ref s)
    grow(ref s)
    log("grown hi!! =>", s)
    dec t := "there"
    swap(ref s, ref t)
    log("swapped there hi!! =>", s, t)
    shadow(ref t)
    log("shadow outer hi!!? =>", t)

    dec f := false
    flag(ref f)
    log("flag true =>", f)

    -- in a loop: the same variable each time
    dec i := 0
    loop while i < 5
        bump(ref i)
        grow(ref s)
    log("loop 5 there!!!!! =>", i, s)

    dec r := "a"
    dec loud := shout(ref r)
    log("returned a! a! =>", loud, r)
    log("joined a!!? =>", shout(ref r) + "?")
    -- the assignment's own target may be passed with ref
    n = next(ref n)
    log("n 4 =>", n)
    -- fields that don't overlap may both be used
    step(ref b.pos, Vec(x: b.vel.x + 1, y: 1))
    log("pos 1 1 =>", b.pos.x, b.pos.y)
    ret 0
