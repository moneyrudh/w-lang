-- fun w() without a return type, reaching the end of its body after a block that owns a str
-- (exits 0; the str is freed first)
fun w()
    say true
        dec s := "inner" + "!"
        log(s)
    log("end")
