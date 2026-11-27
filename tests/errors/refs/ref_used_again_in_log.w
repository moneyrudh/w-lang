-- error: line 8: 's' is passed with ref, so it can't also be used elsewhere in the same statement
fun grow(s: ref str): str
    s = s + "!"
    ret s

fun w(): num
    dec s := "a"
    log(grow(ref s), s)
    ret 0
