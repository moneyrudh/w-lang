-- num arithmetic wraps around on overflow (D11), with no undefined behaviour (expected => actual)
fun w(): num
    dec big := 2147483647
    dec small := -2147483648
    log("2147483647 + 1 -2147483648 =>", big + 1)
    log("-2147483648 - 1 2147483647 =>", small - 1)
    log("65536 * 65536 0 =>", 65536 * 65536)
    log("-(-2147483648) -2147483648 =>", -small)
    log("-2147483648 / -1 -2147483648 =>", small / -1)
    log("-2147483648 % -1 0 =>", small % -1)
    ret 0
