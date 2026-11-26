-- a declaration that shadows an outer variable reads the outer one in its own value
-- (in C the new variable is already in scope there, so wlang computes the value first)
fun w(): num
    dec n := 5
    say true
        dec n := n + 1
        log("inner 6 =>", n)
        say true
            dec n: num = n * 10
            log("innermost 60 =>", n)
    log("outer 5 =>", n)
    ret 0
