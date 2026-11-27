-- error: line 3: The value of global 'same' must be a constant: literals and operators, without variables, calls or text operations
-- comparing text is a strcmp call in C
dec same: bool = "a" is "b"

fun w(): num
    ret 0
