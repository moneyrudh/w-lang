-- error: line 4: The value of global 'b' must be a constant: literals and operators, without variables, calls or text operations
-- a global's value is set before any function runs
dec a: num = 1
dec b: num = a + 1

fun w(): num
    ret b
