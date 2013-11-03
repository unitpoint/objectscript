var modulesLoaded = {}
function __get(name){
	if(!(name in modulesLoaded)){
		modulesLoaded[name] = true
		require(name, false)
		if(name in this){
			return this[name]
		}
	}
	throw "unknown class or global property \"${name}\""
}

function assert(a, message){
	return a || throw(message || "assert failed")
}

function eval(str){
	return compileText(str).apply(null, ...)
}

function evalEnv(str, env){
	return compileText(str).applyEnv(env || _G, null, ...)
}

function toArray(a){
	arrayOf(a) && return a;
	var type = typeOf(a)
	if(type == "object"){
		var arr = []
		for(var i, v in a){
			arr.push(v)
		}
		return arr
	}
	if(type == "null"){
		return null
	}
	return [a]
}

function toObject(a){
	objectOf(a) && return a;
	var type = typeOf(a)
	if(type == "array"){
		var object = {}
		for(var i, v in a){
			object.push(v)
		}
		return object
	}
	if(type == "null"){
		return null
	}
	return {a}
}

function File.readContents(filename)
{
	var f = File(filename, "rb")
	var content = f.read()
	f.close()
	return content
}

function File.writeContents(filename, content)
{
	var f = File(filename, "wb")
	f.write(content)
	f.close()
}

Buffer.__lshift = Buffer.append	// make alias << operator to appent method

function Object.flip(){
	var r = {}
	for(var k, v in this){
		r[v] = k
	}
	return r
}

function Object.reverse(){
	var r = {}
	for(var k, v in @reverseIter()){
		r[k] = v
	}
	return r
}

function Array.reverse(){
	var r = []
	for(var k, v in @reverseIter()){
		r[] = v
	}
	return r
}

function DateTime.__add(b){
	// b is DateTime && throw "DateTime.__add requires Number"
	return DateTime {
		comdate = @comdate + (numberOf(b) || throw "DateTime.__add requires Number")
	}
}

function DateTime.__sub(b){
	b is DateTime && return @comdate - b.comdate;
	return DateTime {
		comdate = @comdate - (numberOf(b) || throw "DateTime.__sub requires DateTime or Number")
	}
}

var shutdownFunctions = []

function registerShutdownFunction(func){
	shutdownFunctions.push(functionOf(func) || throw "expect function")
}

function triggerShutdownFunctions(){
	var funcs = shutdownFunctions
	shutdownFunctions = []
	for(var _, func in funcs.reverseIter()){
		func()
	}
}

function String.__get(i){		// declare [] read operator with one single argument
	return @sub(i, 1)
}

function String.__mul(count){
	count == 1 && return this
	count <= 0 && return ""
	var buf = Buffer()
	for(; count >= 1; count--){
		buf.append(this)
	}
	if(count > 0){
		buf.append(@sub(0, #this * count))
	}
	return toString(buf)
}

function String.__div(count){
	return this * (1 / count)
}

function String.__add(b){
	return this .. b
}

function String.__radd(b){
	return b .. this
}

function String.flower(){
	return @sub(0, 1).upper() .. @sub(1)
}
