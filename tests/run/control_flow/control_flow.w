-- exit: 1
-- every conditional and loop form: each line prints expected => actual
fun classify(n: num): num
    say n < 0
        ret 0 - 1
    elf n is 0
        ret 0
    else
        ret 1

fun w(): num
    dec x: num = 7

    -- say / when / lest
    say x > 5, log("say ok")
    when x is 7
        log("when ok")
    lest x > 100, log("lest ok")
    say x > 100, log("BAD say")

    -- a chain picks exactly one branch
    dec pick: num = 0
    say x < 0
        pick = 1
    elf x < 5
        pick = 2
    elf lest x is 7
        pick = 3
    nor x > 6
        pick = 4
    else
        pick = 5
    log("chain 5 =>", pick)

    say x is 7, pick = 10
    elf x is 8, pick = 20
    else, pick = 30
    log("one-liner chain 10 =>", pick)

    lest x is 7
        pick = 1
    nor x > 100
        pick = 2
    log("lest/nor 2 =>", pick)

    say x is 7,
        pick = 99
    log("comma before block 99 =>", pick)

    say x is 0, pick = 0
    else pick = 42
    log("else one-liner 42 =>", pick)

    -- scoping: shadowing and sibling blocks
    dec t: num = 1
    say x > 0
        dec t: num = 100
        say t is 100
            dec inner: num = 5
            t = t + inner
        log("shadow t 105 =>", t)
    else
        dec t: num = 200
        log("never", t)
    log("outer t 1 =>", t)

    -- loops
    dec i: num = 0
    dec sum: num = 0
    loop while i < 10
        i = i + 1
        say i is 3, skip
        say i is 8, stop
        sum = sum + i
    log("while sum 25 =>", sum, "i 8 =>", i)

    dec n: num = 0
    loop until n >= 5, n = n + 1
    log("until n 5 =>", n)

    dec k: num = 0
    loop forever
        k = k + 1
        lest k < 4, stop
    log("forever k 4 =>", k)

    dec outer: num = 0
    dec total: num = 0
    loop while outer < 3
        outer = outer + 1
        dec j: num = 0
        loop forever
            j = j + 1
            say j > 2, stop
            total = total + 1
    log("nested stop total 6 =>", total)

    -- calls in conditions, condition spanning lines
    say (classify(x) is 1 and
         classify(0) is 0)
        log("calls ok")

    ret classify(0 - 3) + 2
