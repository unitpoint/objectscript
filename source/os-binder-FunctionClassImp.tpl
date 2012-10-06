template <class R, class T OS_BIND_FUNC_PARMS_COMMA OS_BIND_FUNC_TEMPLATE_PARMS>
struct OS_BIND_FUNC_CLASS_NAME
{
	typedef R(OS_BIND_FUNC_CC T::*F)(OS_BIND_FUNC_PARMS){const};

	const char * name;
	F f;

	OS_BIND_FUNC_CLASS_NAME(const char * _name, F _f): name(_name), f(_f){}

	template <class R>
	static int run(ObjectScript::OS * os, int params, int, int, void * user_param)
	{
		OS_GET_TEMPLATE_SELF(T*);
		OS_BIND_FUNC_GET_ARGS;
		typedef typename RemoveConst<R>::type type;
		F& f = *(F*)user_param;
		// CtypeValue<type>::push(os, CtypeValue<type>::to((self->*f)(OS_BIND_FUNC_ARGS)));
		CtypeValue<type>::push(os, (self->*f)(OS_BIND_FUNC_ARGS));
		return 1;
	}

	template <>
	static int run<void>(ObjectScript::OS * os, int params, int, int, void * user_param)
	{
		OS_GET_TEMPLATE_SELF(T*);
		OS_BIND_FUNC_GET_ARGS;
		typedef typename RemoveConst<R>::type type;
		F& f = *(F*)user_param;
		(self->*f)(OS_BIND_FUNC_ARGS);
		return 0;
	}

	operator ObjectScript::OS::FuncDef() const 
	{ 
		ObjectScript::OS::FuncDef def = {name, run<R>, &(new OS_FunctionData<F>(f))->f}; 
		return def; 
	}
};

namespace ObjectScript {

template <class R, class T OS_BIND_FUNC_PARMS_COMMA OS_BIND_FUNC_TEMPLATE_PARMS> 
ObjectScript::OS::FuncDef def(const char * name, R(OS_BIND_FUNC_CC T::*f)(OS_BIND_FUNC_PARMS){const})
{
	typedef OS_BIND_FUNC_CLASS_NAME<R, T OS_BIND_FUNC_PARMS_COMMA OS_BIND_FUNC_PARMS> Func; 
	return Func(name, f);
}

} // namespace ObjectScript
