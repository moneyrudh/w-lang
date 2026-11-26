-- error: line 4: - bool: can't do arithmetic on bool
-- D9 applies to unary minus too
fun w(): num
    dec b: bool = -true
    ret 0
