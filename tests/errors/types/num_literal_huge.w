-- error: line 4: Number too large for num: the largest is 2147483647
-- far past what even a 64-bit number holds
fun w(): num
    dec n: num = 99999999999999999999999999
    ret 0
