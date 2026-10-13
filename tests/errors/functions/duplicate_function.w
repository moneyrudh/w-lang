-- error: line 4: Function 'f' already declared
fun f(): num
    ret 1
fun f(): num
    ret 2
fun w(): num
    ret f()
