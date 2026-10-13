-- exit: 0
-- grouping from the source must survive into the C output
fun w(): num
    dec a: num = 2
    dec b: num = 3
    dec c: num = 4
    dec r1: num = a * (b + c)
    dec r2: num = (a + b) * (c - a)
    dec r3: num = c - (b - a)
    dec r4: num = c / (a * 2)
    dec r5: num = a + b * c
    log("14 10 3 1 14 =>", r1, r2, r3, r4, r5)
    ret 0
