typeof = typeOf
numberof = numberOf
stringof = stringOf
arrayof = arrayOf
objectof = objectOf
userdataof = userdataOf

function __get(name){
	echo("global property \""name"\" is not declared\n")
	echo "back trace\n"
	printBackTrace(1)
	echo "\n"
}

function Object.__get(name){
	echo("object property \""name"\" is not declared\n")
	echo "back trace\n"
	printBackTrace(1) // skip current function
	echo("=======\ntarget "this"\n\n")
}

function assert(a, message){
	if(!a){
		throw(message || "assert failed")
	}
}

function unhandledException(e){
	if("trace" in e){
		printf("Unhandled exception: '%s'\n", e.message);
		for(var i, t in e.trace){
			printf("#%d %s(%d): %s, args: %s\n", i, t.file, t.line, t.object ? "{obj-"..t.object.osValueId.."}."..t.name : t.name, t.arguments);
		}
	}else{
		printf("Unhandled exception: '%s' in %s(%d)\n", e.message, e.file, e.line);
	}
}

function printBackTrace(skipNumFuncs){
	for(var i, t in debugBackTrace(skipNumFuncs + 1)){ // skip printBackTrace
		printf("#%d %s(%d): %s, args: %s\n", i, t.file, t.line, t.object ? "{obj-"..t.object.osValueId.."}."..t.name : t.name, t.arguments);
	}
}

function eval(str, env){
	return compileText(str).applyEnv(env || _G, null, ...)
}

var events = {}

function addEventListener(eventName, func, zOrder){
	functionOf(func) || return;
	if(!(eventName in events)){
		events[eventName] = {}
	}
	events[eventName][func] = zOrder || 0
	events[eventName].rsort()
	return [eventName func]
}

function removeEventListener(eventName, func){
	if(arrayOf(eventName)){
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

function isCallable(f){ 
	var type = typeOf(f)
	return type === "function" || type === "object" || type === "userdata"
}

function setTimeout(func, delay, count, priority){
	count = count || 1
	count > 0 && functionOf(func) || return;
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

addEventListener("enterFrame" {||
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

function toArray(a){
	arrayOf(a) && return arr;
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
	objectOf(a) && return object;
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

function Object.deepClone(){
	var t = this.clone()
	for(var k, v in t){
		t[k] = v.deepClone()
	}
	return t
}

function Object.flip(){
	var r = {}
	for(var i, v in this){
		r[v] = i
	}
	return r
}

function math.clamp(a, min, max){
	return a < min ? min : a > max ? max : a
}
