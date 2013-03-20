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

