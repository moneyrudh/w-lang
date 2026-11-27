-- str values: joins anywhere, owned by variables, freed by the generated C (expected => actual)
dec title: str = "W"
dec empty_global: str

fun exclaim(s: str): str
    s = s + "!"
    ret s

fun repeat(s: str, times: num): str
    dec out: str
    dec i := 0
    loop while i < times
        out = out + s
        i = i + 1
    ret out

fun pick(n: num): str
    say n > 0, ret "positive"
    else, ret "not positive"

fun build(n: num): str
    -- recursion: every level returns a new string to the one above
    say n is 0, ret ""
    else, ret build(n - 1) + "x"

fun w(): num
    dec s := "a" + "b"
    log("join ab =>", s)
    s = s + s
    log("self join abab =>", s)
    log("call hey! =>", exclaim("hey"))
    log("repeat abcabcabc =>", repeat("abc", 3))
    log("pick positive, not positive =>", pick(1), pick(-1))
    log("recursion xxxxx =>", build(5))
    title = title + " lang"
    log("global W lang =>", title)
    log("empty global [] =>", "[" + empty_global + "]")

    -- a str made in a loop, with stop and skip leaving it early
    dec found := "none"
    dec i := 0
    loop forever
        dec label := "item" + repeat("!", i)
        i = i + 1
        say i is 2, skip
        say i > 3
            found = label
            stop
    log("loop item!!! =>", found)

    -- comparing joined strings
    log("compare true false =>", s is "ab" + "ab", "a" + "b" is "ba")

    -- shadowing reads the outer value
    say true
        dec s := s + "!"
        log("shadow abab! =>", s)
    log("outer abab =>", s)
    ret 0
