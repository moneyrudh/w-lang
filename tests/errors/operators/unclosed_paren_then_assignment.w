-- error: line 6: Expected ')', got name
-- the next line could continue the expression, so the error is where it stops making sense;
-- the lines after it are parsed normally again
fun w(): num
    dec x: num = (1 + 2
    x = 3
    ret 0
