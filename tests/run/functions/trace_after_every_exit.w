-- exit: 1
-- functions left in every way, many times, before an error: the trace must still be exactly
-- the three calls that are really running (a missed pop would add frames)
fun via_loop_ret(n: num): num
    loop forever
        say n > 2, ret n
        n = n + 1

fun via_nested(n: num): num
    say n > 0
        say n > 5, ret 1
        else, ret 2
    else, ret 3

fun via_str(s: str): str
    dec t := s + "x"
    ret t

fun via_zil_ret(n: num)
    say n > 0, ret
    log("never")

fun via_zil_end()
    dec s := "a"

fun via_stop(): num
    dec i := 0
    loop forever
        dec s := "x"
        i = i + 1
        say i is 1, skip
        say i is 3, stop
    ret i

fun inner(d: num): num
    ret 10 / d

fun middle(d: num): num
    ret inner(d)

fun w(): num
    dec i := 0
    dec total := 0
    loop while i < 100
        total = total + via_loop_ret(i) + via_nested(i) + via_stop()
        dec s := via_str("y")
        via_zil_ret(i + 1)
        via_zil_end()
        i = i + 1
    log("called them all, total 5363 =>", total)
    ret middle(0)
