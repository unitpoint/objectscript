print 7 && 9
print 7 || 9

;{
	var i = 4
	var name, count, time = "Smith", 3 + 2/i, 1.23
	print concat("name: " name ", count: " count ", time: " time)
}

// ========================================================

function(a){
	var c = a * 2;
	{
		var c = a - 1
		print debuglocals
	}
}(10)

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
