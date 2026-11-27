-- variables declared without a value start at zero / false / empty text (expected => actual)
fun w(): num
    dec n: num
    dec r: real
    dec b: bool
    dec s: str
    log("0 0.000000 false [] =>", n, r, b, "[" + s + "]")
    ret 0
