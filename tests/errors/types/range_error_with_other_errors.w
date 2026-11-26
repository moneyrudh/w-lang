-- error: line 5: Type mismatch in initialization: cannot assign str to num
-- error: line 6: Number too large for num: the largest is 2147483647
-- a number out of range is a type error: it doesn't hide the file's other type errors
fun w(): num
    dec z: num = "s"
    ret 2147483648
