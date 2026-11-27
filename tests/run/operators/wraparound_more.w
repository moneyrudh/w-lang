-- wrap-around in globals' values, with chr operands, and through arguments and returns (D11)
dec g_add: num = 2147483647 + 1
dec g_mul: num = 65536 * 65536
dec g_sub: num = -2147483648 - 1
dec g_div: num = -2147483648 / -1
dec g_mod: num = -2147483648 % -1
dec g_real: real = 2147483647 + 1
dec g_plain: num = 20 / 3 + 20 % 3

fun inc(n: num): num
    ret n + 1

fun w(): num
    log("globals -2147483648 0 2147483647 -2147483648 0 =>", g_add, g_mul, g_sub, g_div, g_mod)
    log("real from a wrapped num -2147483648.000000 =>", g_real)
    log("constant / and % still work 8 =>", g_plain)
    log("chr operand 'a' + 2147483647 -2147483552 =>", 'a' + 2147483647)
    log("through an argument and a return -2147483648 =>", inc(2147483647))
    ret 0
