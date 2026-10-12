-- error: line 5: A condition can't be a call to a zil function
fun nothing(): zil
    ret
fun w(): num
    say nothing(), log("yes")
    ret 0
