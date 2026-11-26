-- error: line 4: Expected ',' or ')' after parameter
-- error: line 5: Expected end of line after statement
-- one error for the broken header; the body is still parsed and its own mistake reported
fun f(a: num b: num): num
    ret a 1

fun w(): num
    ret 0
