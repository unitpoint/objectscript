_E = _G

function eval(str, env){
	return compileText(str).applyEnv(env || _G, null, ...)
}

var events = {}

function addEventListener(event_name, func, user_params){
	events[event_name][func] = user_params
}

function triggerEvent(event_name, params){
	for(var func, user_params in events[event_name]){
		func(params user_params)
	}
}

var timers = {}

function setTimeout(delay_ms, func, count){
	count = count || 1
	count > 0 && func || return;
	var i = #timers
	timers[i] = {
		next_time_ms = app.getTimeMS() + delay_ms
		delay_ms = delay_ms
		func = func
		count = count
	}
	return i
}

function clearTimeout(t){
	delete timers[t]
}

addEventListener("enterFrame", function(){
	var time_ms = app.getTimeMS()
	for(var i, t in timers){
		if(t.next_time_ms >= time_ms){
			t.func()
			t.count = t.count - 1
			if(t.count <= 0){
				delete timers[i]
				continue
			}
			t.next_time_ms = time_ms + t.delay_ms
		}
	}
})