fun add(a: num, b: num): num {
    ret a + b;
}

fun greet(name: str): zil {
    log("Hello", name);
}

fun w(): num {
    -- x is initialized here
    ---
    it is also possible that
    y is initialized here
    ---

    dec x: num = 42;
    dec y: num = 100;
    dec z: num = x + y;
 
    note x is incremented here
    note and if i wanted to do that
    note then i would too innit bruv

    z = z + 50;
 
    log("Test with comma:", x, "and", y);
    log("Test with plus: " + x);
    log("Mixed:" + x, "comma", y);
    log("x + y = " + z);
 
    note adding up results rn
    dec result: num = add(5, 10);
    greet("World");
 
    ret result + x + y;
}

fun main(): zil {
    log("Void func");
    ret;
}