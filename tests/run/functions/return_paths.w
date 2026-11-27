-- functions that return on every path without a ret at the end (expected => actual)
fun sign(n: num): num
    say n > 0, ret 1
    elf n < 0, ret -1
    else, ret 0

fun first_over_ten(n: num): num
    -- a forever loop only ends through ret here: no path reaches the end
    loop forever
        say n > 10, ret n
        n = n + 3

fun nested(a: bool, b: bool): num
    say a
        say b, ret 1
        else, ret 2
    else
        ret 3

fun opposite(x: num): num
    lest x > 0, ret 1
    else, ret 2

fun inner_stop(n: num): num
    -- the stop belongs to the inner loop, so the outer forever loop still never ends
    loop forever
        loop while true
            stop
        say n > 0, ret n
        n = n + 1

fun w(): num
    log("sign 1 -1 0 =>", sign(5), sign(-5), sign(0))
    log("first_over_ten(1) 13 =>", first_over_ten(1))
    log("nested 1 2 3 =>", nested(true, true), nested(true, false), nested(false, true))
    log("opposite 2 1 =>", opposite(5), opposite(-5))
    log("inner_stop(-2) 1 =>", inner_stop(-2))
    ret 0
