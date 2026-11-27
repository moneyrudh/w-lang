-- error: line 7: The value of global 'g' must be a constant: literals and operators, without variables, calls or text operations
pod Vec
    x: real
    y: real

dec n: real = 1
dec g: Vec = Vec(x: n)

fun w(): num
    ret 0
