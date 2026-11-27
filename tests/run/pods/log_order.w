-- log works out every value before it prints anything, also when one is a pod
pod P
    a: num

fun noisy(n: num): num
    log("noisy", n)
    ret n

fun make(): P
    log("making")
    ret P(a: 2)

fun w(): num
    log(noisy(1), make(), "x" + "y")
    ret 0
