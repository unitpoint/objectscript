print({num = 123 num2: 321} "\n")
print("1 == " Math.min(5, "4", {1, 2}, 1, 3) "\n")

var a = {1,2,3,4,5}
var b = clone a
a[0] = 10
b[10] = b[0] * b[2] // 1 * 3
print(
	a "\n"
	b "\n"
)

/*
print ( {qwert:"asd", 10, 11, 12, 15:"df"} )

a, b[r], c.d, d[r,t] = 12*f, f() - b, c[b,c] + 8 * b
a[b, c], a.f.t[r] = c[b] + 8*b, c.b

print (3 * Math.min(5, "4", {1, 2}, 1, 3))

var a = {
	num: 12 + f(1 2 3) / a
	__get@number: function(){
		return this.num;
	}
	__add = function(a, b){
		return a.number + b.number;
	}
}

var b = extends a {
	num: 1;
}

print(
	" a: " a.number
)

print(
	"#a: " #a
	" a: " a.number
	" b: " b.number
	" sum: " a + b
	)


function()
{
	return a()
	return a[b]()
	return a[b,c]()
}

a.b = c
a[b] = c
a[b, c] = c
a[] = b

c = a.b
c = a[b]
c = a[b, c]
c = a[]

;{
	var a = clone b + c
	delete a.b
	delete a[b]
	delete a[b, c]
	delete a[]
}

var b2 = extends (extends c {
	num = 1
}).num c
*/

/*
Object.__clone = function(){
	var r = {};
	for(var k, v in this){
		r[k] = v;
	}
	return r;
}

Array.__clone = function(){
	va r = clone this.properties;
	r.prototype = this.prototype;
}

Object.__setdim = function(value){
	if(#... == 0){
		this.push(value);
		return;
	}
	throw "Error parameters number of [] operator";
	this[...[0]] = value;
}

Object.__getdim = function(){
	if(#... == 0){
		return this.lastProperty;
	}
	throw "Error parameters number of [] operator";
	return this[...[0]];
}

Array.each = function(f){
	for(var i = 0; i < #this; i++){
		if(f(i, this[i])){
			break;
		}
	}
}

Array.toObject = function(){
	var r = {};
	for(var k,v in this){
		r.push(v);
	}
	return r;
}

Array.__get@properties = function(){
	
}

Object.each = function(f){
	for(var k,v in this){
		if(f(k, v)){
			break;
		}
	}
}

Object.__get@values = function(){
	var r = [];
	for(var k,v in this){
		r.push(v);
	}
	return r;
}

Object.__get@keys = function(){
	var r = [];
	for(var k,v in this){
		r.push(k);
	}
	return r;
}

Object.merge = function(b){
	for(var k,v in b){
		if(typeof k === "number"){
			this.push(v);
		}else{
			this[k] = v;
		}
	}
	return this;
};

Object.__add = function(a, b, other){
	if(typeof other === "object"){
		return (clone a).merge(b);
	}
	if(typeof other === "array"){
		
	}
	if(typeof other !== "object" && typeof other !== "array"){
		return (clone this).push(other);
	}
	a = clone a;
	b.each(function(k, v){ a[k] = v; });
	for(var k, v in b){ a[k] = v; }
	return a;
};

Object.__mul = function(a, b, other){
	var iter = this.__iterator;
	var r = {};
	for(var k,v in this){
		r[k] = v * other;
	}
	{
		var k, v, valid, iter = this.__iterator;
		for(;;){
			k, v, valid = iter();
			if(!valid){
				break;
			}
		}
	}
	return r;
}

var a = {
	num: 12;
	__get@number: function(){
		return this.num;
	},
	__add: function(a, b){
		return a.number + b.number;
	}
};

var len = #a;
var e = 12;
var c = +a + ~e + !e + !!e + !!!e;

var b = extends a {
	num: 1;
};

var r = a + b;
*/

/*
var math = {
	__get: function(name){
		return function(){};
	},
	
	__get@round: function(){
		return function(a, b){
			return a + b;
		};
	},

};
	var b;
		b = math.round2(a, b);
		b = math.round(a, b);
		b = math[round](a, b);
		b = math.round[a, b];
		b = math[round][a, b];
		b = math[round, a, b];
		b = math[round, c](a, b);
		b = math[round, c][a, b][c];
		b = math[round, a, b][c];
		b = math[round][a][b](a);
*/
/*
var math = math;

var f = function(a, d)
{
	// return f * math["round"](2.3 * a / d);
	// return b / d;
	
	var b = f{a:"fgg" .. a};

	{
		var a = a * 2;
		b = math.round(a, b);
		b = math[round](a, b);
		b = math.round[a, b];
		b = math[round][a, b];
		b = math[round, a, b];
		// b = math->round(a);
		// b = math->round[a, b];
	}
	{
		var a = a * 2;
	}
	return b / d;
}

return f();

_ENV.A = 7;
A = 7;

_GLOBALS.A = 
{
	
}
*/