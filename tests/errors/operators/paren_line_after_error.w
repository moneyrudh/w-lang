-- error: line 6: Expected '=' or '(' after identifier
-- the next statement starts with '(' and spans two lines; recovering from the error above
-- must not forget that bracket
fun w(): num
    dec a: num = 1
    a
    (a +
        2)
    ret a
