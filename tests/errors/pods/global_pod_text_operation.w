-- error: line 5: The value of global 'g' must be a constant: literals and operators, without variables, calls or text operations
pod Q
    f: bool

dec g: Q = Q(f: "a" is "b")

fun w(): num
    ret 0
