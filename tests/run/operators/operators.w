-- exit: 2
-- comparison and logical operators: each line prints expected => actual
fun w(): num
    dec x: num = 5
    dec y: num = 10
    dec f: real = 2.5
    dec s: str = "apple"
    dec t: str = "banana"

    -- symbol forms
    dec a1: bool = x < y
    dec a2: bool = x > y
    dec a3: bool = x <= 5
    dec a4: bool = y >= 11
    dec a5: bool = x == 5
    dec a6: bool = x != 5
    log("sym  1 0 1 0 1 0 =>", a1, a2, a3, a4, a5, a6)

    -- word forms
    dec b1: bool = x lt y
    dec b2: bool = x gt y
    dec b3: bool = x le 5
    dec b4: bool = y ge 11
    dec b5: bool = x eq 5
    dec b6: bool = x ne 5
    log("word 1 0 1 0 1 0 =>", b1, b2, b3, b4, b5, b6)

    -- is / is not / is !
    dec c1: bool = x is 5
    dec c2: bool = x is not 5
    dec c3: bool = a2 is !a1
    dec c4: bool = a2 is not a1
    log("is   1 0 1 1 =>", c1, c2, c3, c4)

    -- logical, both spellings
    dec d1: bool = x < y and y < 20
    dec d2: bool = x > y or y is 10
    dec d3: bool = not a1
    dec d4: bool = !a2
    dec d5: bool = x < y && x > y
    dec d6: bool = x > y || not (x is y)
    dec d7: bool = not not a1
    log("log  1 1 0 1 0 1 1 =>", d1, d2, d3, d4, d5, d6, d7)

    -- precedence: arithmetic > comparison > equality > and > or
    dec e1: bool = x + 5 is y
    dec e2: bool = x * 2 > y - 1 and not (y < x)
    dec e3: bool = a2 or a1 and a2
    dec e4: num = x * (y - 2)
    dec e5: num = (x + y) * (y - x)
    dec e6: num = y - (x - 2)
    log("prec 1 1 0 40 75 7 =>", e1, e2, e3, e4, e5, e6)

    -- mixed num/real, chr, str
    dec g1: bool = f < x
    dec g2: bool = 'a' < 'b'
    dec g3: bool = s < t
    dec g4: bool = s is "apple"
    dec g5: bool = s is not t
    log("mix  1 1 1 1 1 =>", g1, g2, g3, g4, g5)

    -- bool used as num
    dec n: num = x < y
    ret n + (x < y)
