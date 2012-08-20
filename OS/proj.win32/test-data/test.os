var a = {
	num: 12;
	__get@number: function(){
		return this.num;
	},
	__add: function(a, b){
		return a.number + b.number;
	}
};

var b = a + a;

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