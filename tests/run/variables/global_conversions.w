-- global values converted to their declared types, logic operators, escapes (expected => actual)
dec letter: chr = 'w'
dec tab: chr = '\t'
dec from_num: real = -3
dec flag: bool = 5
dec one: num = true
dec logic: bool = not (1 > 2) and (0 or 1)
dec text: str = "a\tb"
dec mixed: real = 7 / 2 + 0.5
dec empty: str

fun w(): num
    log("chr w =>", letter)
    log("tab is 9 =>", tab + 0)
    log("real from -3 -3.000000 =>", from_num)
    log("bool from 5 true, num from true 1 =>", flag, one)
    log("logic true =>", logic)
    log("text a<tab>b =>", text)
    log("7 / 2 + 0.5 is 3.500000 =>", mixed)
    log("unset str is empty [] =>", "[" + empty + "]")
    ret 0
