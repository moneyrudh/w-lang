-- error: line 6: Unexpected 'dec' at top level. Expected 'fun' for function definition.
-- error: line 9: Expected end of line after statement
-- parsing goes on after a top-level mistake: the next function is still checked
fun w(): num
    ret 0
dec g: num = 1

fun h()
    log("h") 1
