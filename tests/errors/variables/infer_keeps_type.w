-- error: line 5: Type mismatch in assignment to 'n': cannot assign str to num
-- the inferred type is fixed: n is a num from here on
fun w(): num
    dec n := 5
    n = "five"
    ret 0
