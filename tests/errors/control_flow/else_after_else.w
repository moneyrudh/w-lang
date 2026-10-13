-- error: line 6: 'else' needs a 'say', 'when' or 'lest' before it
fun w(): num
    dec x: num = 1
    say x is 1, x = 2
    else, x = 3
    else, x = 4
    ret x
