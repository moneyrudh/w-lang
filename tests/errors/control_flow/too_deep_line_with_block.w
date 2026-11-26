-- error: line 7: Statements nested too deeply (more than 100 levels)
-- error: line 9: Expected a value, got ')'
-- error: line 10: Expected a value, got ')'
-- skipping the too-deep line and its block must not skip the statements after them
fun w(): num
    dec x: num = 1
    say true, say true, say true, say true, say true, say true, say true, say true, say true, say true, say true, say true, say true, say true, say true, say true, say true, say true, say true, say true, say true, say true, say true, say true, say true, say true, say true, say true, say true, say true, say true, say true, say true, say true, say true, say true, say true, say true, say true, say true, say true, say true, say true, say true, say true, say true, say true, say true, say true, say true, say true, say true, say true, say true, say true, say true, say true, say true, say true, say true, say true, say true, say true, say true, say true, say true, say true, say true, say true, say true, say true, say true, say true, say true, say true, say true, say true, say true, say true, say true, say true, say true, say true, say true, say true, say true, say true, say true, say true, say true, say true, say true, say true, say true, say true, say true, say true, say true, say true, say true, say true
        x = 2
    dec y: num = )
    dec z: num = )
    ret 0
