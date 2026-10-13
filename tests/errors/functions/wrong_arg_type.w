-- error: line 5: Argument 2 of 'add': cannot pass str as num
fun add(a: num, b: num): num
    ret a + b
fun w(): num
    ret add(1, "two")
