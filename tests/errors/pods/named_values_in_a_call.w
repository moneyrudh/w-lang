-- error: line 10: Named values (name: value) are only for pod values, and 'f' isn't a pod
pod Vec
    x: real
    y: real

fun f(x: num): num
    ret x

fun w(): num
    ret f(x: 1)
