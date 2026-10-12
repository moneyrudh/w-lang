-- error: line 4: Cannot apply 'not' to str
fun w(): num
    dec s: str = "a"
    dec b: bool = not s
    ret 0
