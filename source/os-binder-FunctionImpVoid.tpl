struct OS_BIND_FUNC_CLASS_NAME
{
	typedef void(OS_BIND_FUNC_CC *F)();

	const char * name;
	F f;

	OS_BIND_FUNC_CLASS_NAME(const char * _name, F _f): name(_name), f(_f){}

	static int run(ObjectScript::OS * os, int params, int, int, void * user_param)
	{
		F& f = *(F*)user_param;
		(*f)();
		return 0;
	}

	operator ObjectScript::OS::FuncDef() const 
	{ 
		ObjectScript::OS::FuncDef def = {name, run, &(new OS_FunctionData<F>(f))->f}; 
		return def; 
	}
};

namespace ObjectScript {

inline ObjectScript::OS::FuncDef def(const char * name, void(OS_BIND_FUNC_CC *f)())
{
	typedef OS_BIND_FUNC_CLASS_NAME Func; 
	return Func(name, f);
}

} // namespace ObjectScript
