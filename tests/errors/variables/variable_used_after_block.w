-- error: line 6: Undefined variable: 'y'
fun w(): num
    dec x: num = 1
    say x is 1
        dec y: num = 5
    x = y
    ret x
