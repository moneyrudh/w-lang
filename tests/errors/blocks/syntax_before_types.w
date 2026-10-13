-- a file with syntax errors isn't type-checked: the undefined 'y' below isn't reported
-- error: line 5: W doesn't use ';'
fun w(): num
    dec x: num = y
    x = 1;
    ret x
