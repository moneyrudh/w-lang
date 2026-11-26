-- str + str inside log prints the pieces side by side; escapes in text are decoded
fun greet(): str
    ret "hey"

fun w(): num
    dec name: str = "World"
    log("Hello, " + name)
    log("a" + "b", "c" + greet() + "!")
    log("tab:\t|")
    log("two\nlines")
    log("quote: \"hi\"", "backslash: \\", "apostrophe: \'")
    dec s: str = "x\ty"
    log("in a variable:", s)
    ret 0
