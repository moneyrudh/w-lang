-- fun w() without a return type: the program exits with 0, also after an early ret
fun w()
    log("start")
    dec i := 0
    loop while i < 5
        say i is 2, ret
        i = i + 1
    log("never")
