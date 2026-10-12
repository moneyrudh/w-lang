-- error: line 3: Variable already declared in this scope
fun f(n: num): num
    dec n: num = 2
    ret n
fun w(): num
    ret f(1)
