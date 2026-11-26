-- error: line 6: Expected ':' or ':=' after variable name in declaration
-- error: line 7: Expected a value, got end of line
-- error: line 8: Expected a value, got ','
-- each broken statement reports its first error, then parsing continues on the next line
fun w(): num
    dec a num = 1
    dec b: num = 2 +
    say a > , log("y")
    ret 0
