-- top-level dec: a global, visible in every function, wherever it's declared (expected => actual)
-- exit: 42
dec count: num = 0
dec rate := 2.5
dec name: str = "W"
dec on: bool = 1 < 2
dec big: num = -2147483648
dec unset: num

fun bump()
    count = count + 1

fun w(): num
    bump()
    bump()
    log("count 2 =>", count)
    log("rate 2.500000, name W, on true =>", rate, name, on)
    log("big -2147483648, unset 0, later 42 =>", big, unset, later)
    dec count := count + 40
    log("a local shadowing count 42 =>", count)
    ret count

dec later: num = 7 * 6
