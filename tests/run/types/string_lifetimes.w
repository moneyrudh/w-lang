-- string memory edge cases (each line: expected => actual); run under ASan by make test-asan
dec g: str = "old"

-- these replace g with the same text: a new copy, and the old one is freed. The caller may
-- still be holding the old one (C doesn't say whether g or the call is evaluated first)
fun bump(): str
    g = "old"
    ret "!"

fun grow(): str
    g = g + ""
    ret "?"

fun shadow_and_return(): num
    -- leaving the inner scope early must free the inner s once and the outer s once
    dec s := "outer"
    say true
        dec s := "inner"
        ret 1
    ret 0

fun shadow_in_loop(): num
    dec s := "outer"
    dec i := 0
    loop while i < 3
        dec s := "inner" + "!"
        i = i + 1
        say i is 1, skip
        say i is 3, stop
    ret i

fun stop_with_temps()
    loop forever
        say "a" + "b" is "ab", stop

fun w(): num
    log("shadowing with an early ret 1 =>", shadow_and_return())
    log("shadowing in a loop with skip and stop 3 =>", shadow_in_loop())
    stop_with_temps()
    log("a global replaced while the caller reads it old ! =>", g, bump())
    dec s := g + grow()
    log("a global replaced inside a join old? =>", s)
    ret 0
