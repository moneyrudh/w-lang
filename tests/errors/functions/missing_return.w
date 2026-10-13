-- error: line 2: Function 'f' with return type 'num' must return a value
fun f(): num
    log("no ret")
fun w(): num
    ret f()
