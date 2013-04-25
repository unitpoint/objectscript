function assert(a, message){
	a || throw(message || "assert failed")
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

function String.__get(i){		// declare [] read operator with one single argument
	return @sub(i, 1)
}

function String.__mul(i){		// declare * operator
	var buf = Buffer()
	for(; i >= 1; i--){
		buf << this
	}
	return toString(buf)
}

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
