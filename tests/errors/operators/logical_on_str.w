-- error: line 4: Logical operators (and, or) cannot be used on str
fun w(): num
    dec s: str = "a"
    dec b: bool = s and true
    ret 0
