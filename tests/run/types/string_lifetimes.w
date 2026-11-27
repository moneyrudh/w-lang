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

fun num_shadows_str(): num
    -- an inner variable of another type with the same name: early exits free the outer str only
    dec s := "outer"
    say true
        dec s := 5
        ret s
    ret 0

fun num_shadows_str_in_loop(): num
    dec s := "outer"
    dec i := 0
    loop while i < 4
        dec s := i * 10
        i = i + 1
        say s is 10, skip
        say s is 20
            dec s := "innermost"
            stop
    ret i

fun stop_with_temps()
    loop forever
        say "a" + "b" is "ab", stop

fun w(): num
    log("shadowing with an early ret 1 =>", shadow_and_return())
    log("shadowing in a loop with skip and stop 3 =>", shadow_in_loop())
    log("a num shadowing a str, early ret 5 =>", num_shadows_str())
    log("in a loop with skip and stop 3 =>", num_shadows_str_in_loop())
    stop_with_temps()
    log("a global replaced while the caller reads it old ! =>", g, bump())
    dec s := g + grow()
    log("a global replaced inside a join old? =>", s)
    ret 0
