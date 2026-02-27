fun add(a: num, b: num): num {
    ret a + b;
}

fun greet(name: str): zil {
    log("Hello", name);
}

fun test(): zil {
    log("Void func");
    ret;
}

fun w(): num {
    dec x: num = 42;
    dec y: num = 100;
    dec z: num = x + y;
 
    z = z + 50;
 
    log("Test with comma:", x, "and", y);
    log("Test with plus: " + x);
    log("Mixed:" + x, "comma", y);
    log("x + y = " + z);
 
    dec result: num = add(5, 10);
    greet("World");
 
    ret result + x + y;
}
