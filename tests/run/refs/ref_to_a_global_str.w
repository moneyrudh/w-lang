-- a ref str may be a global's slot: a call in the same expression can replace (and free) the
-- global's text, so the ref is read through a copy, as the global itself is. setg() puts the
-- same text back as a new copy (freeing the old one), so the output doesn't depend on the
-- order C works the arguments out in; only reading the freed text would go wrong
dec g := "original"

fun setg(): str
    g = "origin" + "al"
    ret "x"

fun show(a: str, b: str)
    log("show =>", a, b)

fun f(a: ref str)
    log("log original x =>", a, setg())
    show(a, setg())
    dec t := a + setg()
    log("join originalx =>", t)
    a = "changed"

fun w(): num
    f(ref g)
    log("global changed =>", g)
    ret 0
