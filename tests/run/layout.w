-- exit: 6
-- indentation edge cases: blank lines, comment-only lines at odd indents,
-- expressions spanning lines inside brackets, deep nesting, no trailing newline

fun add(a: num,
        b: num): num
    ret a + b


fun w(): num
    dec x: num = add(1,
                     2)

        -- comment-only line indented deeper than the code
  -- and one indented less
    dec y: num = (x +
        x)
    ---
  block comment
          at odd indents
    ---
    say x is 3
        say y is 6
            say (x < y and
                 y > x)
                log("deep ok")
    ret y