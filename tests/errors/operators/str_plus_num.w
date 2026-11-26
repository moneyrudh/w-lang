-- error: line 5: str + num: can't join str with a non-str value (to print them together, use log("text", value))
-- D8: text and numbers are printed together with commas, not joined with +
fun w(): num
    dec x: num = 1
    log("total: " + x)
    ret 0
