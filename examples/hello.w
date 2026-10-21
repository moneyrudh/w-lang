note A small tour of W: functions, variables, conditionals and loops.
note Run it with:  wlang examples/hello.w   (or ./build/wlang after make)

fun square(n: num): num
    ret n * n

fun describe(n: num)
    say n < 10, log("small")
    elf n < 100, log("medium")
    else, log("large")

fun w(): num
    dec name: str = "W"
    log("Hello from", name)

    -- count up, skipping the odd numbers
    dec i: num = 0
    loop while i < 6
        i = i + 1
        say i is 1 or i is 3 or i is 5, skip
        dec sq: num = square(i)
        log(i, "squared is", sq)

    describe(square(4))
    describe(square(12))

    ret 0
