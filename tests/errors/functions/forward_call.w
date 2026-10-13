-- known limitation (docs/LANGUAGE.md §1): delete this test once a signature pre-pass exists
-- error: line 4: Undefined function: 'later'
fun w(): num
    ret later()
fun later(): num
    ret 1
