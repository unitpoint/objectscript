print(5 " differences")

;{
	var i = 4
	var name, count, time = "Smith", 3 + 2/i, 1.23
	print concat("name: " name ", count: " count ", time: " time)
}

print {firstname="Ivan" lastname="Petrov"}

;{
	var a = {x=1, y=3; "zero" "one", "two" last:7,}
	print a[1]
	print a
	
	a = {[2+3]="five" y=3}
	print a[5]
	print a
	
	a = {x=1 y=2}
	var b = {[a]="powerful" 7="greate"}
	print b[a]	
	
	print [10, 20, 30, 40]
	print [10 20 30 40]
	
	var i, j, k = 0, 1, 3
	echo("i="i", j="j", k="k"\n")
	
	i, j = j, i
	echo("after swap: i="i", j="j"\n")
	
	var test = function(){ return 1, 2 }
	var a, b = test()
	print("results from function" "a="..a "b="..b)
	
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

print "7 && 9 = "..(7 && 9)
print "7 || 9 = "..(7 || 9)

// ========================================================

{
	var v1 = {x=1 y=2}
	var a = {[v1]="vv1" v2="vv2"}
	print( a[v1] a.v2 )
}
// ========================================================

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

var p = Person("James", "Bond")
p.walk()
print p

var IvanPerson = extends Person {
    __construct: function(){
        super.__construct.call(this, "Ivan", "Petrov")
    }
}
var p = IvanPerson()
p.walk()
print p

// ========================================================

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
	var a = {
		_color = "red"
		__get@color = function(){ return this._color }
		__set@color = function(v){ this._color = v }
	}
	
	print a["color"]
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
	
	print a.color
	a.color = "green"
	print a.color
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
	a[1, 2] = 5		// компилятор преобразует это в a.__setdim(5, 1, 2)
	print a[1, 2]	// print(a.__getdim(1, 2))
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

