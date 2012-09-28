function __get(name){
	echo("global property \""name"\" is not declared\n")
	echo "back trace\n"
	printBackTrace(1)
	echo "\n"
}

function Object.__get(name, autoCreate){
	if(autoCreate) return;
	echo("object property \""name"\" is not declared\n")
	echo "back trace\n"
	printBackTrace(1) // skip current function
	echo("=======\ntarget "this"\n\n")
}

function Userdata.__set(name, value){
	echo("userdata property \""name"\" is not declared, set value "value"\n")
	echo "back trace\n"
	printBackTrace(1) // skip current function
	echo("=======\ntarget "this"\n\n")
}

function assert(a, message){
	if(!a){
		print(message || "assert failed")
		printBackTrace(1)
		terminate()
	}
}

setErrorHandler(function(code message file line){
	var type = "ERROR"
	if(code == E_WARNING)
		type = "WARNING"
	else if(code == E_NOTICE)
		type = "NOTICE"
	echo("["type"] "message"\n")
	echo "back trace\n"
	printBackTrace(1) // skip current function
})

function printBackTrace(skipNumFuncs){
	for(var i, t in debugBackTrace(skipNumFuncs + 1)){ // skip printBackTrace
		echo("======= ["i"]\n")
		// echo("  line: "t.line", pos: "t.pos", token: "t.token", file: "t.file"\n")
		echo("  line: "t.line", pos: "t.pos", file: "t.file"\n")
		echo("  function: "t.name", arguments: "t.arguments"\n")
		// print concat("  object: "(t.object === _G && "<<GLOBALS>>" || t.object)"\n")
	}
}

function eval(str, env){
	return compileText(str).applyEnv(env || _G, null, ...)
}

var events = {}

function addEventListener(eventName, func, zOrder){
	functionof func || return;
	events[eventName][func] = zOrder || 0
	events[eventName].rsort()
	return [eventName func]
}

function removeEventListener(eventName, func){
	if(arrayof eventName){
		eventName, func = eventName[0], eventName[1]
	}
	if(eventName in events){
		delete events[eventName][func]
	}
}

function triggerEvent(eventName, params){
	// print "core.triggerEvent: "..events
	for(var func, zOrder in events[eventName]){
		func(params)
	}
}

var timers = {}

function isCallable(f){ return typeof f === "function" || typeof f === "object" || typeof f === "userdata" }

function setTimeout(func, delay, count, priority){
	count = count || 1
	count > 0 && functionof func || return;
	var i = func // #timers
	timers[i] = {
		nextTime = app.timeSec + delay
		delay = delay
		func = func
		count = count
		priority = priority || 0
	}
	// timers.sort(function(a b){ return b.priority - a.priority })
	timers.rsort "priority"
	return i
}

function clearTimeout(t){
	delete timers[t]
}

HIGH_PRIORITY = 999999

addEventListener("enterFrame" function(){
	var time = app.timeSec
	for(var i, t in timers){
		if(t.nextTime <= time){
			t.nextTime = time + t.delay
			if(t.count === true){
				t.func.call(null)
			}else{
				if(t.count <= 1){
					delete timers[i]
				}else{
					t.count = t.count - 1
				}
				t.func.call(null)
			}
		}
	}
} HIGH_PRIORITY+1)

function toNumber(a){
	return numberof valueof a
}

function toString(a){
	return stringof valueof a
}

function toArray(a){
	var arr = arrayof a
	arr && return arr;
	var type = typeof a
	if(type == "number" || type == "string" || type == "boolean" || type == "userdata"){
		return [a]
	}
	if(type == "object"){
		arr = []
		for(var i, v in a){
			arr.push(v)
		}
		return arr
	}
	return null
}

function toObject(a){
	var object = objectof a
	object && return object;
	var type = typeof a
	if(type == "number" || type == "string" || type == "boolean" || type == "userdata"){
		return {a}
	}
	if(type == "array"){
		object = {}
		for(var i, v in a){
			object.push(v)
		}
		return object
	}
	return null
}

function deepClone(p){
	p = clone p
	for(var k, v in p){
		p[k] = deepClone(v)
	}
	return p
}
