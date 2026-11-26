-- error: line 4: not str: can't be used on str
fun w(): num
    dec s: str = "a"
    dec b: bool = not s
    ret 0
