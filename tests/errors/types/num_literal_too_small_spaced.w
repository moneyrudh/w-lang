-- error: line 4: Number too small for num: the smallest is -2147483648
-- a space between - and the number doesn't change it
fun w(): num
    dec n: num = - 2147483649
    ret 0
