-- the largest and smallest values each number type can be written as (expected => actual)
fun w(): num
    dec big: num = 2147483647
    dec small: num = -2147483648
    log("2147483647 -2147483648 =>", big, small)
    log("leading zeros 42 =>", 00042)
    log("- 5 and -5 are the same -5 -5 =>", - 5, -5)
    dec r: real = 340282346638528859811704183484516925440.0
    log("largest real 340282346638528859811704183484516925440.000000 =>", r)
    ret 0
