-- a zil call's result is a real type error, not a suppressed one
-- error: line 6: Type mismatch in initialization: cannot assign zil to num
fun nothing()
    ret
fun w(): num
    dec x: num = nothing()
    ret x
