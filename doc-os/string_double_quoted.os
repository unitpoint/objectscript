print "this is a string"

// Outputs: three plus three is 6
print "three plus three is ${3+3}"

foobar = "blah"
// Outputs: the value of foobar is blah
print "the value of foobar is ${foobar}"

// Outputs: the value of foobar is ${foobar}
print "the value of foobar is \${foobar}"

function factorial(a){
	return a <= 1 ? 1 : a*factorial(a-1)
}
var p = 10
// Outputs: factorial of 10 is 3628800
print "factorial of 10 is ${factorial(p)}"