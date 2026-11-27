-- error: line 4: Unexpected indentation
-- reported once; the parser doesn't get stuck on the end of the stray block
dec g: num = 1
    log(g)

fun w(): num
    ret 0
