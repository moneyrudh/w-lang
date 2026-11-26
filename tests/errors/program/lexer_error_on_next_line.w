-- error: line 5: Expected a value, got end of line
-- error: line 6: Unexpected character '$'
-- the '$' is scanned while line 5 is still being parsed; it mustn't hide line 5's own error
fun w(): num
    dec a: num =
    $ dec b: num = 2
    ret 0
