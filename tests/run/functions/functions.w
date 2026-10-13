-- exit: 15
-- parameters, zil functions, calls as statements, nested calls

fun add(a: num, b: num): num
    ret a + b

fun mul(a: num, b: num): num
    ret a * b

fun greet(name: str): zil
    log("hello", name)
    ret

fun w(): num
    greet("W")
    dec total: num = add(add(1, 2), mul(3, 4))
    log("total 15 =>", total)
    ret total
