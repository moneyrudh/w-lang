-- a parameter the function never uses is fine (the generated C compiles with -Wall -Wextra -Werror)
fun ignore(a: num, b: str): num
    ret 7

fun w(): num
    log("ignore 7 =>", ignore(1, "x"))
    ret 0
