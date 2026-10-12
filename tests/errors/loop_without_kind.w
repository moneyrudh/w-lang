-- error: line 4: Expected 'while', 'until' or 'forever' after 'loop'
fun w(): num
    dec x: num = 1
    loop x < 5
        x = x + 1
    ret x
