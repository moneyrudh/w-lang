-- the largest and smallest values each number type can be written as (expected => actual)
fun w(): num
    dec big: num = 2147483647
    dec small: num = -2147483648
    log("2147483647 -2147483648 =>", big, small)
    log("leading zeros 42 =>", 00042)
    log("- 5 and -5 are the same -5 -5 =>", - 5, -5)
    dec r: real = 340282346638528859811704183484516925440.0
    log("largest real 340282346638528859811704183484516925440.000000 =>", r)
    dec just_below: real = 340282356779733661637539395458142568447.0
    log("just below the cutoff rounds to the largest 340282346638528859811704183484516925440.000000 =>", just_below)

    -- a negative literal inside expressions
    dec one: num = 1
    log("-2147483648 + 1 -2147483647 =>", -2147483648 + 1)
    log("same(-2147483648) -2147483648 =>", same(-2147483648))
    log("2 * -3 -6 =>", 2 * -3)
    log("one - -1 2 =>", one - -1)
    log("-1 * 2 -2 =>", -1 * 2)
    ret 0

fun same(n: num): num
    ret n
