-- exit: 1
-- dividing by zero stops the program with the W line, after what ran before it
fun w(): num
    dec x := 3
    dec y := 6
    loop while x >= 0
        log(y / x)
        x = x - 1
    ret 0
