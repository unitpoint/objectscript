var runJSStringResult = runJSStringResult
var index = 0

var function parseJSRet(i, ret){
	ret === "ok::true" && return true
	ret === "ok::false" && return false
	ret === "ok::null" && return null
	var prefix = ret.sub(0, 9)
	prefix === "ok::str::" && return ret.sub(9)
	prefix === "ok::num::" && return toNumber(ret.sub(9))
	prefix === "ok::obj::" && return JavaScriptObjectWrapper(toNumber(ret.sub(9)), "object")
	prefix === "ok::fun::" && return JavaScriptObjectWrapper(toNumber(ret.sub(9)), "function")
	throw ret
}

function JavaScriptObjectWrapper(i, type){
	var func = function(){
		// throw "JavaScriptObjectWrapper.call: ${i}"
		var args = []
		for(var i, value in arguments){
			if(functionOf(value) && "__javaScriptObjectIndex__" in value){
				var valueEncoded = "new JavaScriptValueWrapper(${value.__javaScriptObjectIndex__})"
			}else{
				var valueEncoded = json.encode(value)
			}
			args[i]	= valueEncoded
		}
		var next_i = index++
		var ret = runJSStringResult("ObjectScriptInterface.callValue(${i}, ${next_i}, ${args.json(',')})")
		// print "ObjectScriptInterface.callValue(${i}, ${next_i}, ${args.json(',')}): ${ret}"
		return parseJSRet(next_i, ret)
	}
	func.__javaScriptObjectIndex__ = i
	function func.valueOf(){
		throw "JavaScriptObjectWrapper.valueOf: ${i}"
	}	
	function func.__get(name){
		var next_i = index++
		var ret = runJSStringResult("ObjectScriptInterface.getIndirectValue(${i}, ${next_i}, ${json.encode(name)})")
		// print "ObjectScriptInterface.getIndirectValue(${i}, ${next_i}, ${json.encode(name)}): ${ret}"
		return parseJSRet(next_i, ret)
	}
	function func.__set(name, value){
		throw "set ${name} = ${value}"
	}
	return func
}

function __get(name){
	var i = index++
	var ret = runJSStringResult("ObjectScriptInterface.getRootValue(${i}, ${json.encode(name)})")
	// print "ObjectScriptInterface.getRootValue(${i}, ${json.encode(name)}): ${ret}"
	return parseJSRet(i, ret)
}

function __set(name, value){
	if(functionOf(value)){
		if("__javaScriptObjectIndex__" in value){
			var valueEncoded = "new JavaScriptValueWrapper(${value.__javaScriptObjectIndex__})"
		}else{
			// common function
			var valueEncoded = "ObjectScriptInterface.wrapOSFunction(${value.__id})"
		}
	}else{
		var valueEncoded = json.encode(value)
	}
	var ret = runJSStringResult("ObjectScriptInterface.setRootValue(${json.encode(name)}, ${valueEncoded})")
	// print "ObjectScriptInterface.setRootValue(${json.encode(name)}, ${json.encode(value)}): ${ret}"
	ret !== "" && throw ret
}
