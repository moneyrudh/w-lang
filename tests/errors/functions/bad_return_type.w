-- error: line 4: Expected return type after ':'
-- error: line 4: Failed to parse function
-- the parameter list built before the bad return type must be freed (LeakSanitizer in CI)
fun f(a: num, b: num): 5
    ret a

fun w(): num
    ret 0
