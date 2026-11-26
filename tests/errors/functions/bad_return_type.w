-- error: line 3: Expected return type after ':'
-- the parameter list built before the bad return type must be freed (LeakSanitizer in CI)
fun f(a: num, b: num): 5
    ret a

fun w(): num
    ret 0
