-- x is still declared after the bad initializer, so its uses don't add "undefined" errors
-- error: line 4: Type mismatch in initialization: cannot assign str to num
fun w(): num
    dec x: num = "text"
    x = x + 1
    ret x
