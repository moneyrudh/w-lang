-- exit: 1
-- a run-time error prints the chain of W calls that led to it, innermost first
fun average(total: num, count: num): num
    ret total / count

fun update(scores: num): num
    dec a := average(scores, 0)
    ret a

fun w(): num
    log("starting")
    ret update(10)
