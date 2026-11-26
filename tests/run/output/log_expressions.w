-- log takes any expressions, printed separated by spaces (each line: expected => actual)
fun square(n: num): num
    ret n * n

fun w(): num
    dec x: num = 1
    log("x + 1 is 2 =>", x + 1)
    log("square(3) 9, x > 0 true =>", square(3), x > 0)
    log("not true false =>", not true)
    log("(x + 2) * 3 9 =>", (x + 2) * 3)
    log("chr real w 2.500000 =>", 'w', 2.5)
    log("'a' + 1 98 =>", 'a' + 1)
    log("100% stays 100% =>", "100%")
    log("nested square(square(2)) 16 =>", square(square(2)))
    log()
    log("after an empty line")
    ret 0
