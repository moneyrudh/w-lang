-- error: line 4: Number too large for real: the largest is about 3.4e38
-- exactly halfway between the largest float and the next power of two: rounds up, past the largest
fun w(): num
    dec r: real = 340282356779733661637539395458142568448.0
    ret 0
