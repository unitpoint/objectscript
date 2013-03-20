require "std"

typeof = typeOf
numberof = numberOf
stringof = stringOf
arrayof = arrayOf
objectof = objectOf
userdataof = userdataOf

function __get(name){
	print "global property \"${name}\" is not declared"
	print "back trace"
	printBackTrace(1)
	print ""
}

function Object.__get(name){
	print "object property \"${name}\" is not declared"
	print "back trace"
	printBackTrace(1) // skip current function
	print "======="
	print "target: ${this}"
	print ""
}

function unhandledException(e){
	if(e is CompilerException){
		echo "\nUnhandled exception: '${e.message}' in ${e.file}(${e.line},${e.pos}), token: ${e.token}\n${e.lineString.trim()}\n\n"
	}else{
		echo "\nUnhandled exception: '${e.message}'\n\n"
	}
	if('trace' in e)
	for(var i, t in e.trace){
		printf("#${i} ${t.file}%s: %s, args: ${t.arguments}\n",
			t.line > 0 ? "(${t.line},${t.pos})" : "",
			t.object && t.object !== _G ? "<${typeOf(t.object)}#${t.object.id}>.${t.name}" : t.name)
	}
}

function printBackTrace(skipNumFuncs){
	for(var i, t in debugBackTrace(skipNumFuncs + 1)){ // skip printBackTrace
		printf("#${i} ${t.file}%s: %s, args: ${t.arguments}\n",
			t.line > 0 ? "(${t.line},${t.pos})" : "",
			t.object && t.object !== _G ? "<${typeOf(t.object)}#${t.object.id}>.${t.name}" : t.name)
	}
}

var events = {}

function addEventListener(eventName, func, zOrder){
	functionOf(func) || return;
	if(!(eventName in events)){
		events[eventName] = {}
	}
	events[eventName][func] = zOrder || 0
	events[eventName].sort {|a, b| b <=> a}
	return [eventName, func]
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
	if(eventName in events){
		for(var func, zOrder in events[eventName]){
			func(params)
		}
	}
}

var timers = {}

function isCallable(f){ 
	var type = typeOf(f)
	return type == "function" || type == "object" || type == "userdata"
}

function setTimeout(func, delay, count, priority){
	count = count || 1
	count > 0 && functionOf(func) || return;
	timers[func] = {
		nextTime = app.timeSec + delay,
		delay = delay,
		func = func,
		count = count,
		priority = priority || 0,
	}
	timers.sort {|a, b| b.priority <=> a.priority }
	// timers.rsort "priority"
	return func
}

function clearTimeout(t){
	delete timers[t]
}

HIGH_PRIORITY = 999999

addEventListener("enterFrame", {||
	var time = app.timeSec
	for(var i, t in timers){
		if(t.nextTime <= time){
			t.nextTime = time + t.delay
			if(t.count === true){
				// t.func.call(null)
				(t.func)()
			}else{
				if(t.count <= 1){
					delete timers[i]
				}else{
					t.count = t.count - 1
				}
				// t.func.call(null)
				(t.func)()
			}
		}
	}
}, HIGH_PRIORITY+1)

function Object.deepClone(){
	var t = @clone()
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
