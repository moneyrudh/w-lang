-- dec x := value: the variable takes the value's type (each line: expected => actual)
fun square(n: num): num
    ret n * n

fun w(): num
    dec n := 5
    dec r := 2.5
    dec c := 'w'
    dec s := "text"
    dec b := n > 3
    dec sq := square(n)
    dec mixed := n + r
    dec neg := -n
    log("num 5, real 2.500000, chr w, str text, bool true =>", n, r, c, s, b)
    log("square(5) 25, n + r 7.500000, -n -5 =>", sq, mixed, neg)

    -- the inferred type sticks: n is a num, so a real assigned to it converts
    n = 7
    mixed = mixed * 2
    log("n 7, mixed 15.000000 =>", n, mixed)
    ret 0
