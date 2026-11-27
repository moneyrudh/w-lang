-- error: line 5: The value of global 'g' divides by zero
pod P
    a: num

dec g: P = P(a: 1 / 0)

fun w(): num
    ret 0
