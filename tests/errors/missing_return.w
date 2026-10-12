-- error: must return a value
fun f(): num
    log("no ret")
fun w(): num
    ret f()
