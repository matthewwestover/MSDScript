## MSDScript User Guide
MSDScript can be run either by inputting a file or via the command line itself. 

**Note**: Command line execution expects a return character, so hitting ```Ctrl+D``` is necessary instead of just ```return``` is necessary to begin program execution.

### General Input
MSDScript is capable of parsing input that includes new lines and extra white space. There are several reserved keywords for execution, but any form of text can be used as a variable name. Reserved keywords are denoted with an underscore ```_``` character.

#### Keywords
* Numbers
	* Numbers should be input without commas, decimals, or spaces between characters within the number itself
	* Examples:
		* Good: ```1+1``` or ```1 + 1```
		* Bad: ```1,000 + 5.5``` or ```1 000 + 5```
* Variables
	* Variables can be any non-digit character or continuous string that is not preceded by an underscore ```_``` character. 
	* Examples:
		* Good: ```x``` or ```var```
		* Bad: ```1x``` or ```v a r```
* Booleans
	* Booleans are noted by the keywords ```_true``` or ```_false```
* Addition
	* All addition is noted by a plus ```+``` character.
* Multiplication
	* All multiplication is noted by an astricks ```*``` character.
* Comparison
	* Comparison between two values uses double equals ```==``` characters
* Variable Assignment
	* The ```_let``` keyword allows assignment to a variable name. 
	* It should be followed by an variable that equals what you would like to assign to the variable name. 
	* The ```_in``` keyword indicates where that variable assignment should be utlized
	* Examples:
		* ```_let x = 1 _ in x + 1``` or ```_let var = _true _in _if var _then this _else that```
* If Statements
	* Requires three parts. 1. Evaluation Section 2. then true branch 3. else false branch
	* They are comprised of the ```_if```, ```_then```, and ```_else``` keywords
	* Correct format:
	 
	```
	_if //this 
	_then //do this 
	_else //do that
	``` 
* Functions
	* Functions are noted by the ```_fun``` keyword.
	* Functions then include the input within parentheses ```( or )``` characters.
	* following the closing parenthesis the actual function body follows utilizing the other formats. 
	* Examples:
		* ```_fun (x) x + 1``` creates an add 1 to value x function.
* Function Calls
	* To execute a function, include the value you would like passed into the function after the function inside parentheses ```( or )``` characters.
	* Examples:
		* ```(_fun (x) x + 1) (2)``` will return the value ```3```


#### Executable Flags
To handle input optimization and segmentation/overflow errors there are two additional executable modes that can be input. These utilize input flags prior to the rest of the statement to execute. 

* ```--opt``` Will take the input statement and optimize it down to a simpler form, which will still produce the same end results after being interpreted.  
	* Examples:
		* ```1+1``` optimizes to ```2```
		* ```_let x = 5 _in x + y``` optimizes to ```5 + y```
* ```--step``` Will prevent segmentation faults for larger recursive calls. While technically this should be the "standard" for MSDScript execution, it has been left as a seperate flag to illustrate that it does work on inputs that fault without it. 