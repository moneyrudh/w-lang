-- error: line 5: Pod 'Self' contains itself, so it would never end
-- error: line 8: Pod 'A' contains itself, so it would never end
-- error: line 21: Pod 'Outside' contains itself, so it would never end
-- error: line 28: Pod 'F' contains itself, so it would never end
pod Self
    me: Self

pod A
    b: B

pod B
    c: C

pod C
    a: A

-- reachable only through Entry; reported once, at Outside
pod Entry
    x: Outside

pod Outside
    y: Inside

pod Inside
    z: Outside

-- F closes two cycles (F -> G -> F and F -> G -> H -> F): one error
pod F
    g: G
    again: F

pod G
    f: F
    h: H

pod H
    f: F

fun w(): num
    ret 0
