-- exit: 3
-- unary minus on num, real and chr (each line: expected => actual); w() returns -5 + 8
fun neg(n: num): num
    ret -n

fun w(): num
    dec x: num = 5
    dec r: real = 2.5
    log("-3 -3 =>", -3)
    log("-x -5 =>", -x)
    log("- -x 5 =>", - -x)
    log("-(x + 1) -6 =>", -(x + 1))
    log("x - -3 8 =>", x - -3)
    log("-x * 2 -10 =>", -x * 2)
    log("-r -2.500000 =>", -r)
    log("-'a' -97 =>", -'a')
    log("neg(-7) 7 =>", neg(-7))
    dec y: num = -x + 8
    ret y
