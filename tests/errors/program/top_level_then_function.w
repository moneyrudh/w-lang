-- error: line 6: Unexpected 'log' at top level. Expected 'fun', 'dec' or 'pod'.
-- error: line 9: Expected end of line after statement
-- parsing goes on after a top-level mistake: the next function is still checked
fun w(): num
    ret 0
log("g")

fun h()
    log("h") 1
