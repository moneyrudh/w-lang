-- error: line 2: Pod 'A' contains itself, so it would never end
pod A
    b: B

pod B
    a: A

fun w(): num
    ret 0
