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

fun while_true(n: num): num
    -- a literal true condition never ends the loop, like forever
    loop while true
        say n > 3, ret n
        n = n + 1

fun until_false(n: num): num
    loop until false
        say n > 3, ret n
        n = n + 2

fun one_line(n: num): num
    loop forever, ret n

fun with_nor(n: num): num
    say n > 10, ret 3
    elf lest n > 0, ret 1
    nor n > 5, ret 2
    else, ret 0

fun w(): num
    log("sign 1 -1 0 =>", sign(5), sign(-5), sign(0))
    log("first_over_ten(1) 13 =>", first_over_ten(1))
    log("nested 1 2 3 =>", nested(true, true), nested(true, false), nested(false, true))
    log("opposite 2 1 =>", opposite(5), opposite(-5))
    log("inner_stop(-2) 1 =>", inner_stop(-2))
    log("while_true(0) 4, until_false(0) 4 =>", while_true(0), until_false(0))
    log("one_line(9) 9 =>", one_line(9))
    log("with_nor 3 1 2 =>", with_nor(11), with_nor(-1), with_nor(3))
    ret 0
