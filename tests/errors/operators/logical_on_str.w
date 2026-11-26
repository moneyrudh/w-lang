-- error: line 4: str and bool: and / or can't be used on str
fun w(): num
    dec s: str = "a"
    dec b: bool = s and true
    ret 0
