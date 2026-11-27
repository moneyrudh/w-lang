-- % and mod: the remainder of whole-number division (expected => actual)
fun w(): num
    dec x := 17
    log("17 % 5 2, 17 mod 5 2 =>", x % 5, x mod 5)
    log("-17 % 5 -2 (the sign follows the left side, as in C) =>", -17 % 5)
    log("17 % -5 2 =>", 17 % -5)
    log("'a' % 10 7 =>", 'a' % 10)
    log("precedence like *: 1 + 7 % 4 * 2 7 =>", 1 + 7 % 4 * 2)
    dec i := 0
    dec evens := 0
    loop while i < 10
        say i mod 2 is 0, evens = evens + 1
        i = i + 1
    log("evens below 10 5 =>", evens)
    ret 0
