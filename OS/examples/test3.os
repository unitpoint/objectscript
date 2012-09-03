// call function with arguments without comma
print(5 " differences")

;{
	var i = 4
	// multiple assignment
	var name, count, time = "Smith", 3 + 2/i, 1.23
	// call function with arguments without comma
	print concat("name: " name ", count: " count ", time: " time)
}

// auto call function with single argument, brackets are not used
print {firstname="Ivan" lastname="Petrov"}

;{
	// extended object constructor syntax
	var a = {x=1, y=3; "zero" "one", "two" last:7,}
	print a[1]
	print a
	
	// extended object constructor syntax
	a = {[2+3]="five" y=3}
	print a[5]
	print a
	
	a = {x=1 y=2}
	// any type could be used as index
	var b = {[a]="powerful" 7="greate"}
	print b[a]	
	
	print([10, 20, 30, 40])
	// extended array constructor syntax without comma
	print([10 20 30 40])
	
	// multiple assignment
	var i, j, k = 0, 1, 3
	echo("i="i", j="j", k="k"\n")
	
	// swap using multiple assignment
	i, j = j, i
	echo("after swap: i="i", j="j"\n")
	
	// multiple returns
	var test = function(){ return 1, 2 }
	var a, b = test()
	print("results from function" "a="..a "b="..b)
	
	// multiple returns
	var a, b, c = test()
	print("results from function" "a="..a "b="..b "c="..c)
}

;{
	print "Object iterator test"
	var obj = { null awesome=true 12 "excellent" }
	for(var k, v in obj){
		print( k " --> " v )
	}

	print "Object iterator test, only keys"
	obj = { null awesome=true 12 "excellent" }
	for(k in obj){
		print k
	}
	
	print "Array iterator test"
	Array.__iter = function(){
		var i, self = 0, this
		return function(){
			if(i < #self){
				print("... inside of array iter func: " i self[i])
				return true, i, self[i++]
			}
		}
	}	
	for(k, v in [10 20 30 40 50]){
		print( k " --> " v )
	}
	
	print("Test array prototype:", [1 2 3].prototype === Array)
	
	print "Test range iterator"
	var range = function(a, b){
		return function(){
			if(a <= b){
				return true, a++
			}
		}
	}
	for(var i in range(10, 13)){
		print "i = "..i
	}	
}

// logical operators
print "7 && 9 = "..(7 && 9)
print "7 || 9 = "..(7 || 9)

// ========================================================

{
	var v1 = {x=1 y=2}
	var a = {[v1]="vv1" v2="vv2"}
	print( a[v1] a.v2 )
}
// ========================================================

// class declaration
var Person = {
    __construct: function(firstname, lastname){
        this.firstname = firstname
        this.lastname = lastname
    }
    walk = function(){
        print this.fullname .. " is walking!"
    }
    __get@fullname = function(){
        return this.firstname .. " " .. this.lastname
    }
}
// create new instance of Person class
var p = Person("James", "Bond")
p.walk()
print p

// inherit class
var IvanPerson = extends Person {
    __construct: function(){
        super.__construct.call(this, "Ivan", "Petrov")
    }
}
// create new instance of IvanPerson class
var p = IvanPerson()
p.walk()
print p

// ========================================================
// new vector type class
var vec3 = {
	__construct = function(x, y, z){
		this.x = x
		this.y = y
		this.z = z
	}
	__add = function(a, b){
		return vec3(a.x + b.x, a.y + b.y, a.z + b.z)
	}
	__mul = function(a, b){
		return vec3(a.x * b.x, a.y * b.y, a.z * b.z)
	}
}

var v1 = vec3(10 20 30)
var v2 = vec3(1 2 3)
var v3 = v1 + v2 * v2
print v3

;{
	print "Test properties"

	var a = {
		_color = "red"
		__get@color = function(){ return this._color }
		__set@color = function(v){ this._color = v }
	}
	
	// get property
	print a["color"]
	// set property
	a.color = "blue"
	print a.color
	
	a = {
		_color = "white"
		__get = function(name){ 
			if(name == "color")
				return this._color 
		}
		__set = function(name, v){
			if(name == "color")
				this._color = v
		}
		__del = function(name){
			if(name == "color")
				delete this._color
		}
	}
	
	// get property
	print a.color
	// set property
	a.color = "green"
	print a.color
	// delete property
	delete a.color
	print a.color	
	
	print "Test multi dimensional properties"
	a = {
		_matrix = {}
		__getdim = function(x, y){
			return this._matrix[y*4 + x]
		}
		__setdim = function(value, x, y){
			this._matrix[y*4 + x] = value
		}
		__deldim = function(x, y){
			delete this._matrix[y*4 + x]
		}
	}
	// set property
	a[1, 2] = 5		// a.__setdim(5, 1, 2)
	// get property
	print a[1, 2]	// print(a.__getdim(1, 2))
	// delete property
	delete a[1, 2]	// a.__deldim(1, 2)
	print a[1, 2]	// print(a.__getdim(1, 2))	
}

print "Test function inline call"
print function(a b c){ return a + b * c }(1 2 3)

;{
	print "Test local vars scope"
	var i = 1;
	{
		var i = i
		i++
		print i
	}
	print i
}

print "Test debuglocals"
function(a){
	var c = a * 2;
	{
		var c = a - 1
		print debuglocals
	}
}(10)

print "Script environment"
print _E

print "Math functions"
var a = [math.random() math.random() math.random() math.random() math.random() math.random()]
print("random" a)

// remove debug print from Array.__iter
delete Array.__iter // delete our iterator, use default one

var transform = function(a f){
	var r = typeof a === "array" && [] || {}
	for(var i, v in a){
		r[i] = f(v)
	}
	return r
}
a = transform(a function(a){ return a*100 })
print("mult 100" a)
print("math.ceil" transform(a math.ceil))
print("math.floor" transform(a math.floor))
print("math.round(-1)" transform(a function(a){ return math.round(a, -1) }))
print("math.round(2)" transform(a function(a){ return math.round(a, 2) }))
print("math.sin" transform(a math.sin))
print("math.cos" transform(a math.cos))
print("math.tan" transform(a math.tan))

_E = extends _E math
print "Extend local environment to be able to use math module without namespace"
print("round(3)" transform(a function(a){ return round(a, 3) }))
print("PI" PI)

var core = require("core")
print("core.eval(math.round(13.5))" core.eval("return math.round(13.5)"))
print("eval(math.round(13.5))" eval("return math.round(13.5)"))
print("==============")
print("Next: math namespace is not used\neval executes code in global environment by default\nso eval(round(13.5))" eval("return round(13.5)"))
print("run eval in space of the current environment (round(13.5))" eval("return round(13.5)", _E))

print _G
