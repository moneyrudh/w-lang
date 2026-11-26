-- error: line 5: num + bool: can't do arithmetic on bool
-- D9: true + 1 is almost always a mistake
fun w(): num
    dec flag: bool = true
    dec n: num = 1 + flag
    ret 0
