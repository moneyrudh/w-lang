-- exit: 0
-- defaults, conversions, and printing each type

fun w(): num
    dec n: num
    dec r: real
    dec b: bool
    log("defaults 0 0.000000 0 =>", n, r, b)

    dec i: num = 7
    dec f: real = 2.5
    dec mixed: real = f * i
    log("mixed 17.500000 =>", mixed)

    dec c: chr = 'w'
    dec t: bool = true
    dec s: str = "text"
    log("w 1 text =>", c, t, s)

    dec from_bool: num = t
    dec from_chr: num = 'A'
    log("1 65 =>", from_bool, from_chr)
    ret 0
