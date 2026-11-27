-- error: line 4: Unexpected 'log' at top level. Expected 'fun', 'dec' or 'pod'.
fun w(): num
    ret 0
log("not inside a function")
