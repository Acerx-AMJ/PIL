# PIL
## About
PIL is a primitive interpreted esoteric programming language based off COBOL and assembly. It contains built-in instructions that the user can use to create their own, more complex instructions. Here's a factorial example:
```txt
factorial(n) define minus1, result  ; new function with parameter N. define two locals - minus1 and result
   leeq n, 1, $0                    ; check if N is smaller or equal to 1 and store the result in register 0
   jmp $0, factorial-end            ; if it is, jump to end, where we return 1

   sub n, 1, minus1                 ; subtract one from N and store it in minus1 variable
   call result, factorial, minus1   ; call factorial and store the result in the result variable
   mul result, n, result            ; multiply N by result
   return result                    ; return it
factorial-end:
   return 1

main()                              ; main program entry point
   factorial 5.0                    ; call factorial with the value 5
   printn R$0                       ; output 120. R$0 is the return register. you could also call and get the value that way
```
More examples can be found in the examples folder.

## ToC
- [Documentation](#documentation)
- - [Comments](#comments)
- - [Values](#values)
- - [Function Definitions](#function-definitions)
- - [Globals](#globals)
- - [Directives](#directives)
- - [Built-in Functions](#built-ins)
- [Usage](#usage)
- [Building](#building)
- [Credits](#credits)

## Documentation
PIL is an interpreted, loosely-typed language. It has 3 statement levels - file > functions > commands. There can be no commands in the file level and no functions in the command level. All statements must end with a newline.

### Comments
PIL only has line comments that start with a semicolon (;). Code after it is ignored.

### Values
PIL is loosely-typed, meaning a single container can contain different types of values. In PIL there are 4 value types:
- integer
- floating-point number
- character
- string

Values can be stored in 4 different places:
- local variables (function parameters, defines)
- global variables
- registers (accessed via $N syntax)
- return registers (accessed via R$N syntax)

To store a value into a container we use **move** or **set**:
```txt
move 20, $0                ; register $0 is now 20
move 'a', $1               ; register $1 is now 'a'
global my-var              ; declare 'my-var'
set my-var "Hello, World!" ; my-var is now "Hello, World!"
```
There are also functions that store the result in a container, like the [arithmetic](#arithmetic) functions.

### Function Definitions
Functions are user-made instructions just like the built-in **add**, **sub** and **jmp** and can be called with the same syntax. To define a function you need to provide a name with parentheses after it.
```txt
my-function()
   printn "Hello, World!"
```
And calling is the same as for built-in functions:
```txt
my-function
```
But this won't compile since function calls need to be in function level - inside a function body -, that's why to execute our code we have to use the reserved **main** function. It gets called once during the start of the program. Here's the full example:
```txt
my-function()
   printn "Hello, World!"

main() ; should not have any parameters. more on that later.
   my-function
```
And this should print "Hello, World!" to the console.

And we can pass arguments to user-made functions just the same as to native functions. But first we need to define our parameters list:
```txt
my-add(a, b)
   add a, b, $0 ; $0 = a + b

main()
   my-add 10, 5 ; $0 = 10 + 5
   my-add $0, 5 ; $0 = $0 + 5
   printn $0    ; 20
```
Functions can return multiple values and they don't require to be noted in the declaration. A function can even return different number of values based on certain conditions. By default a null value is returned.
```txt
my-func(condition)
   jmp condition, my-label
   return 1, 2, 3
my-label
   return "Hello!"
```
The values are stored in return registers (R$N). But to store the results elsewhere [call](#call) function can be used.

To define locals just like parameters, you must use the **define** keyword right after the function declaration. Note that you cannot do it on the next line or the interpreter will interpret it as a function call, which is totally valid.
```txt
my-add() define a, b
   set a 10
   set b 30
   add a, b, $0
   return $0
```

### Globals
Globals can be defined using the [global](#global) function and then edited and accessed from anywhere in the file.
```txt
increment-global()
   add g, 1, g

increment-and-print()
   increment-global
   printn g

main()
   global g 20
   increment-and-print
   increment-and-print
   increment-and-print
```

### Directives
PIL supports a few directives that configure the code pre parse time. They can be used anywhere in the file but it is recommended to use them at the top of the file.
#### include
```txt
include STRING
```
Reads the contents of the file and includes it in the source file.

#### register-count
```txt
register-count INTEGER
```
Sets the register count for the program. Only the last directive will take effect. Default is 16.

#### return-register-count
```txt
return-register-count INTEGER
```
Sets the return register count for the program. Only the last directive will take effect. Default is 4.
### Built-ins
PIL has built-in (native) functions. They can be called just like regular functions. There are 2 reserved functions that cannot be redefined, though it is not recommended to redefine any.
#### print
```txt
print ANY, ANY...
printn ANY, ANY...
printf STRING, ANY...
printfn STRING, ANY...
```
Output all values to the console. N variants will automatically output a newline. F variants will take a format string as the first argument and replace any '{}' with the value. F variants will not check format count. Non-F variants will not output spaces or any other character between values.
#### str
```txt
str ANY, ANY...
```
Concatenates all values and turns them into a string.
#### format
```txt
format STRING, ANY, ANY...
```
Formats the string by replacing all '{}' with values. Does not check format argument count.
#### arithmetic
```txt
add N1, N2..., DESTINATION
sub N1, N2..., DESTINATION
mul N1, N2..., DESTINATION
div N1, N2..., DESTINATION
mod N1, N2, DESTINATION
pow N1, N2, DESTINATION
neg N, DESTINATION
```
Do basic arithmetic and store the result in DESTINATION. add - addition, sub - subtraction, mul - multiplication, div - division, mod - modulus (remainder of division), pow - exponentiation, neg - negation. Division by 0 will not throw and instead return 0. It is user's responsibility to check for that.
#### square root
```txt
sqrt N, DESTINATION
cbrt N, DESTINATION
```
Calculate the square/cube root of the number and store in DESTINATION.
#### trigonometry
```txt
sin N, DESTINATION
cos N, DESTINATION
tan N, DESTINATION
asin N, DESTINATION
acos N, DESTINATION
atan N, DESTINATION
atan2 Y, X, DESINATION
asinh N, DESTINATION
acosh N, DESTINATION
atanh N, DESTINATION
sinh N, DESTINATION
cosh N, DESTINATION
tanh N, DESTINATION
```
Trigonometric functions. Store the result in DESTINATION.
#### abs
```txt
abs N, DESTINATION
```
Get the absolute value of a number and store it in DESTINATION.
#### min, max, clamp
```txt
min N1, N2..., DESTINATION
max N1, N2..., DESTINATION
clamp N, LO, HI, DESTINATION
```
Get the min value/max value/clamp the value in range [LO;HI] and store it in DESTINATION.
#### round
```txt
ceil N, DESTINATION
floor N, DESTINATION
round N, DESTINATION
```
Ceil/floor/round the number and store it in DESTINATION.
#### exp
```txt
exp N, DESTINATION
```
Exponentiate the number by base e and store it in DESTINATION.
#### log
```txt
ln N, DESTINATION
log N, BASE, DESTINATION
log2, N, DESTINATION
log10, N, DESTINATION
```
Get the logarithm of the number and store it in DESTINATION. ln uses base e, log - user-defined base, log2 - base 2 and log10 - base 10.
#### comparison
```txt
le N1, N2, DESTINATION
gr N1, N2, DESTINATION
leeq N1, N2, DESTINATION
greq N1, N2, DESTINATION
eq N1, N2, DESTINATION
neq N1, N2, DESTINATION
```
Compare the two values and store the result in DESTINATION. le - lesser, gr - greater, leeq - lesser equal, greq - greater equal, eq - equal, neq - not equal.
#### not
```txt
not N, DESTINATION
```
Return the opposite of value's truthiness and store it in DESTINATION. 0 - 1, anything else - 0.
#### control flow
```txt
goto LABEL
jmp CONDITION, LABEL
jmpn CONDITION, LABEL
```
Jump to label LABEL. goto - unconditional jump, jmp - only jump if condition is truthy, jmpn - only jump if condition is not thruthy.
#### move, set
```txt
move VALUE, DESTINATION
set DESTINATION, VALUE
```
Move a VALUE into DESTINATION. Set DESTINATION to VALUE.
#### global
```
global IDENTIFIER1, IDENTIFIER2..., VALUE?
```
Define all identifiers all initialized by optional VALUE or null value as global values.
#### constants
- **pi** - constant PI - 3.141592653589793238462 or 180 degrees in radians.
- **tau** - constant TAU - 2 * PI or 360 degrees in radians.
- **e** - constant E - 2.7182818284590452353602.

## Usage
Run a single file:
```bash
./pil FILE
```

## Building
This project uses C++20 and can be built with CMake:
```bash
cmake -B build
cmake --build build
```
After, the executable can be located in `build/pil`. Or simply build with your compiler:
```bash
g++ -std=c++20 -Iinclude source/*.cpp -o pil
```

## Credits
This project was made by Daniel Vishnevsky and is licensed under MIT License.
