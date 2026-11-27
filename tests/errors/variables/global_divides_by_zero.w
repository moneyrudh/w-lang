-- error: line 3: The value of global 'x' divides by zero
-- C can't compute this before the program starts (in a function it would be a run-time error)
dec x: num = 10 / (3 - 3)

fun w(): num
    ret x
