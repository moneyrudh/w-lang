-- error: line 5: Function 'add' takes 2 arguments, got 1
fun add(a: num, b: num): num
    ret a + b
fun w(): num
    ret add(1)
