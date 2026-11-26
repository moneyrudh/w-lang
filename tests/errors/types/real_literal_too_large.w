-- error: line 4: Number too large for real: the largest is about 3.4e38
-- real is a C float
fun w(): num
    dec r: real = 999999999999999999999999999999999999999999.0
    ret 0
