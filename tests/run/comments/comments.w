-- exit: 11
-- all comment forms, and words that look like comment keywords
-- throwaway at top
---
multi throwaway
with -- and note and done inside
---
note Adds things.
note Even "quotes" and done and ; are fine.
note: colon form is just a single line note
note
fun w(): num
    dec notebook: num = 7  -- trailing throwaway
    dec notes: num = 3     note trailing doc works too
    dec done: num = 1
    note x is incremented here
    note and if i wanted to do that
    log("vals", notebook, notes, done)
    ret notebook + notes + done
