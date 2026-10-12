-- error: line 4: Cannot compare str with a non-str value
fun w(): num
    dec s: str = "a"
    dec b: bool = s < 5
    ret 0
