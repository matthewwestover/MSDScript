## MSDScript API
### Important Files
#### Core
The core files needed to run MSDScript are as follows:  

* ```cont.cpp and cont.hpp```: Allow for step mode interpretation.  
* ```env.cpp and env.hpp```: Allow for quicker referencing of variable values.  
* ```expr.cpp and expr.hpp```: The main expression files.  
* ```step.cpp and step.hpp```: Allow for step mode interpretation.  
* ```value.cpp and value.hpp```: Allow for values to be stored and called on for function calls. 

#### Helpers
* ```macros.hpp```: MSDScript was initially built without shared pointers. This macros file allows to quickly switch between using the shared pointers or not. Required for usage. 
* ```main.cpp```: This file can be utilized for quick utilization of the parsing and interpreting methods. Not required for usage.
* ```parse.cpp and parse.hpp ```: Allow for parsing of input strings. Not needed if parsing will not be used. 

#### Testing
MSDScript has been built utilizing the Catch2 testing framework. Tests have been written directly into each ```.cpp``` file. The ```catch.hpp``` file should be included for this reason. 

### Important Terms
* Parse - Convert human readable input to a script usable expression
* Expr - An expression that can be utilizied by MSDScript. There are multiple types of Exprs dependant on the input. 
* Val - The value of sections of Exprs being interpreted. 
* Interp - MSDScript's function call to convert an Expr into an Val
* Optimize - Conversion of an Expr to its simpleist form that returns the same Val as the original more complex version
* lhs - left hand side of the expression
* rhs - right hand side of the expression

### MSDScript Grammar
```
<expr> = <comparg>
		| <comparg> == <expr>

<comparg> = <addend>
		| <addend> + <comparg>

<addend> = <multicand>
		| <multicand> * <addend>

<multicand> = <inner>
		| <multicand> ( <expr> )

<inner> = <number>
		| ( <expr> )
		| <variable>
		| _let <variable> = <expr> _in <expr>
		| _true
		| _false
		| _if <expr> _then <expr> _else <expr>
		| _fun ( <variable> ) <expr>
```

### Parse
MSDScript is capable of parsing formated types of input to perform basic calculations, variable assignment, comparisons, and function calls. This is done by utilizing the ```parse()``` function or its wrapper function.

```PTR(Expr) parse(std::istream &in)``` takes an istream input, parses it, and returns the entire input as an Expr. 

```static PTR(Expr) parse_str(std::string s)``` is a wrapper function for the ```parse()``` function. It takes an input string that is converted to an istream for ```parse()``` to use.

Parsing output is always an Expr. Further usage is dependant on the Expr class functions. 

### Interpreting Expressions
```interp()``` or ```interp_by_steps(Expr e)``` are the two functions for finding the value of an expression. 

```interp()``` returns a new Val which can be converted to a string. However it can result in a seg fault when large calculations are done.  
```interp_by_steps(Expr e)``` prevents excessive object creation in calculation. It returns a new value based on a passed in expression. 

### Expr
Exprs are expressions that store the input information that MSDScript can then use to perform calculations and operations on. There are multiple types of expressions, each with implemented functionality. 

#### Expr Sub-types
##### NumExpr
Constructor: ```NumExpr(int rep);```  
Member Variables: ```int rep```  
NumExprs are the Script representation of integer values. They cannot exceed INT_MAX in value.  
Comprised of: ```<NumExpr>```

##### VarExpr
Constructor: ```VarExpr(std::string name);```  
Member Variables: ```std::string name```  
VarExprs are the Script representation of variables. They must be at least one character in length and cannot begin with an underscore ```_``` character.  
Comprised of: ```<VarExpr>```

##### BoolExpr
Constructor: ```BoolExpr(bool rep);```  
Member Variables: ```bool rep```  
BoolExprs are the Script representation of booleans. They only can store ```true``` or ```false``` values.  
Comprised of: ```<BoolExpr>```

##### AddExpr
Constructor: ```AddExpr(PTR(Expr) lhs, PTR(Expr) rhs);```  
Member Variables: ```PTR(Expr) lhs, PTR(Expr) rhs```  
AddExprs are the Script representation of addition (+). They can store any form other Exprs as the left-hand (lhs) or right-hand(rhs) sides of the expression.  
Comprised of: ```<Expr> + <Expr>``` 

##### MultExpr
Constructor: ```MultExpr(PTR(Expr) lhs, PTR(Expr) rhs);```  
Member Variables: ```PTR(Expr) lhs, PTR(Expr) rhs```  
MultExprs are the Script representation of multiplication (*). They can store any form other Exprs as the left-hand (lhs) or right-hand(rhs) sides of the expression.  
Comprised of: ```<Expr> * <Expr>``` 

##### LetExpr
Constructor: ```LetExpr(std::string name, PTR(Expr) rhs, PTR(Expr) body);```  
Member Variables: ```std::string name, PTR(Expr) rhs, PTR(Expr) body```  
LetExprs are the Script representation of variable assignment. They store the variable string name, what it should be set to, and what it should be usable in.  This can be seen as "let x = 5 in x + y".  
Comprised of: ```_let <VarExpr> = <Expr> _in <Expr>``` 

##### EqualExpr
Constructor: ```EqualExpr(PTR(Expr) lhs, PTR(Expr) rhs);```  
Member Variables: ```PTR(Expr) lhs, PTR(Expr) rhs```  
EqualExprs are the Script representation of a comparison check. They store two parts to compare. This can be seen as "x == x".   
Comprised of: ```<Expr> == <Expr>``` 

##### IfExpr
Constructor: ```IfExpr(PTR(Expr) test_part, PTR(Expr) then_part, PTR(Expr) else_part);```  
Member Variables: ```PTR(Expr) test_part, PTR(Expr) then_part, PTR(Expr) else_part```  
IfExprs are the Script representation of conditional branching. They store a "test" condition that can be evaluated to determine if it will follow one path or the other. This can be seen as "if x = 1 then do_this else do_that"  
Comprised of: ```_if <Expr> _then <Expr> _else <Expr>``` 

##### FunExpr
Constructor: ```FunExpr(std::string arg, PTR(Expr) body);```  
Member Variables: ```std::string arg, PTR(Expr) body```  
FunExprs are the Script representation of a function definition. They store an argument to be passed to it, and a body in which the arguement can be applied.  
Comprised of: ```_fun (<Expr>) <Expr>)``` 

##### CallExpr
Constructor: ```CallExpr(PTR(Expr) to_be, PTR(Expr) actual);```  
Member Variables: ```PTR(Expr) to_be, PTR(Expr) actual```  
CallExprs are the Script representation of a function being called to execute. They store the function itself, and what would be passed into the functions arg.  

#### Expr Functions
Each Expr function has different implementation depending on the Expr it is applied to. They are as follows 

##### bool equals(PTR(Expr) other_expr);
```equals(PTR(Expr) other_expr)``` Always compares the current Expr to any other type of Expr. Any time an Expr is compared to an different Expr type this returns **false**.  

* NumExpr: If both NumExpr's rep values are the same returns **true**, otherwise **false**.  
* VarExpr: If both VarExpr's name values are the same, returns **true**, otherwise **false**.
* BoolExpr: If both BoolExpr's boolean values are the same, returns **true**, otherwise **false**.   
* AddExpr: If both AddExpr's lhs and rhs values are the same, returns **true**, otherwise **false**.    
* MultExpr: If both MultExpr's lhs and rhs values are the same, returns **true**, otherwise **false**.   
* LetExpr: If both LetExpr's name, rhs, and body values are the same, returns **true**, otherwise **false**.     
* EqualExpr: If both EqualExpr's lhs and rhs values are the same, returns **true**, otherwise **false**.     
* IfExpr: If both IfExpr's test\_part, then\_part, and else\_part values are the same, returns **true**, otherwise **false**.    
* FunExpr: If both FunExpr's arg and body values are the same, returns **true**, otherwise **false**.   
* CallExpr: If both CallExpr's to\_be and actual values are the same, returns **true**, otherwise **false**.  

##### bool has_var(); 
```bool has_var()``` Checks the Expr to see if it has any variables currently open within it. 
 
* NumExpr: Always returns **false**.  
* VarExpr: Always returns **true**.  
* BoolExpr: Always returns **false**.  
* AddExpr: Checks both the lhs and rhs, returns **true** if either side has a variable, otherwise returns **false**.    
* MultExpr: Checks both the lhs and rhs, returns **true** if either side has a variable, otherwise returns **false**.  
* LetExpr: Checks the body and returns **true** if it has a variable, otherwise returns **false**.   
* EqualExpr: Checks both the lhs and rhs, returns **true** if either side has a variable, otherwise returns **false**.   
* IfExpr: Checks all three member variables and returns **true** if any of them has a variable, otherwise returns **false**.   
* FunExpr: Always returns **true**.  
* CallExpr: Always returns **true**.  

##### PTR(Val) interp(); 
```PTR(Val) interp()``` converts an Expr into a Val object. It attempts to simplify down as much as possible to a single value.  

* NumExpr: Returns a NumVal with the int value stored in it.   
* VarExpr: Attempts to return a value the Variable has been set to, if it cannot throws a run time error.  
* BoolExpr: Returns a BoolVal with the boolean value stored in it.  
* AddExpr: Takes the value of the lhs and adds it to the value of the rhs. Returns this new value.  
* MultExpr: Takes the value of the lhs and multiplies it to the value of the rhs. Returns this new value.   
* LetExpr: Takes the assigned variable's (name) value (rhs) and places it inside its body. Returns that body's value.  
* EqualExpr: Returns a BoolVal with a boolean value stored in it.  
* IfExpr: Evaluates the test\_part. If true, returns value of the then\_part. If false returns the value of the else\_part.   
* FunExpr: Returns a FunVal with the stored parameters to be called.   
* CallExpr: Returns a value that places the actual\_arg into the to\_be\_called function. 

##### void step_interp(); 
```step_interp()``` allows for the ```interp_by_steps(Expr e)``` method to be called. It uses a "step" methodology to interpret the values of a given expression.

##### PTR(Expr) optimize(); 
```optimize()``` takes an expression and simplifies it down to a more simple form that can still ```interp()``` to the same value.  

* NumExpr: Returns a new NumExpr with the same rep value.  
* VarExpr: Returns a new VarExpr with the same name value.  
* BoolExpr: Returns a new BoolExpr with the same boolean value.  
* AddExpr: Optimizes its lhs and rhs. If neither side has unassigned variables, adds them together and returns the combined value as an Expr. Otherwise it returns a new AddExpr with lhs and rhs both optmized.   
* MultExpr: Optimizes its lhs and rhs. If neither side has unassigned variables, multiplies them together and returns the value as an Expr. Otherwise it returns a new MultExpr with lhs and rhs both optmized.  
* LetExpr: If the body has a variable, attempts to place the rhs Expr into the body and return that body as a new Expr. Otherwise it returns a new LetExpr with rhs and body optmized.  
* EqualExpr: Optmizes the lhs and rhs. If there are no remaining variables inside them, returns a BoolExpr with the boolean result of the two sides. Otherwise it returns a new EqualExpr with the optmized lhs and rhs.   
* IfExpr: If the test\_part does not have a variable in it, it evaluates the test\_part and then returns either the then\_part or else\_part optimized depending on if the test\_part was true or not. Otherwise it returns a new IfExpr with all three components optimized.  
* FunExpr: Optimizes the body and returns a new FunExpr.  
* CallExpr: Returns a new CallExpr with optmized to\_be\_called and actual\_args.

##### std::string to_string(); 
```to_string()``` converts an expression back into the same readable format the parser could accept as an input.  
 
* NumExpr: Returns a string version of the stored int rep  
* VarExpr: Returns the variable  
* BoolExpr: Returns "\_true" or "\_false"  
* AddExpr: Returns lhs->to\_string " + " rhs->to\_string  
* MultExpr: Returns lhs->to\_string " * " rhs->to\_string  
* LetExpr: Returns "(\_let " + name + " = " + rhs->to\_string + " \_in " + body->to\_string + ")"  
* EqualExpr: Returns lhs->to\_string + " == " + rhs->to\_string  
* IfExpr: Returns "(\_if " + test\_part->to\_string() + " \_then " + then\_part->to\_string() + " \_else " + else\_part->to\_string() + ")"  
* FunExpr: Returns "(\_fun (" + formal\_arg + ") " + body->to\_string() + ")"  
* CallExpr: Returns ", (" + to\_be\_called->to\_string() + "(" + actual\_arg->to\_string() + "))"


