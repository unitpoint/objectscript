function JavaScriptValueWrapper(i){
	this.i = i;
}

ObjectScriptInterface = {
	exportValues: {},
	
	_retValue: function(value, i, name){
		if(value === true) return "ok::true";
		if(value === false) return "ok::false";
		if(value === undefined || value === null) return "ok::null";
		switch(typeof value){
		case "number": return "ok::num::"+value;
		case "string": return "ok::str::"+value;
		/* case "function": 
			if("__javaScriptObjectIndex__" in value){
				i = value.__javaScriptObjectIndex__;
				if(this.exportValues[i].value !== value)
					throw "program corrupted";
				return "ok::fun::"+i;
			}
			value.__javaScriptObjectIndex__ = i;
			this.exportValues[i] = [name, value];
			return "ok::fun::"+i; */
		}
		if("__javaScriptObjectIndex__" in value){
			i = value.__javaScriptObjectIndex__;
			// console.log("found export object", value);
			if(this.exportValues[i].value !== value)
				throw "program corrupted";
			return "ok::obj::"+i;
		}
		value.__javaScriptObjectIndex__ = i;
		this.exportValues[i] = {name:name, value:value};
		return "ok::obj::"+i;
		// if($.isArray(value)){
	},
	
	getRootValue: function(i, name){
		var value = window[name];
		if(value === undefined && !(name in window)) return "unknown JS global property: "+name;
		return this._retValue(value, i, name);
	},
	
	getIndirectValue: function(i, next_i, name){
		var value = this.exportValues[i].value[name];
		// if(value === undefined) return "unknown JS property: "+this.exportValues[i].name+"."+name;
		return this._retValue(value, next_i, name);
	},
};

testNumVal = 123;
testStrVal = "my string";
testNullVal = null;
testUndefVal = undefined;
testTrueVal = true;
testFalseVal = false;
testObjVal = {user:"Ivan", age:21, item:{town:"Moscow"}};

