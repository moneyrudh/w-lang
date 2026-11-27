-- a ref str may be a global's slot: a call in the same expression can replace (and free) the
-- global's text, so the ref is read through a copy, as the global itself is
dec g := "original"

fun setg(): str
    g = "replaced"
    ret "x"

fun grow(s: ref str)
    s = s + "!"

fun show(a: str, b: str)
    log("show =>", a, b)

fun f(a: ref str)
    log("log original x =>", a, setg())
    g = "original"
    show(a, setg())
    g = "original"
    dec t := a + setg()
    log("join originalx =>", t)
    log("before grow replaced =>", a)

fun w(): num
    f(ref g)
    log("global replaced =>", g)
    ret 0
