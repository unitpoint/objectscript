#ifndef __OS_API_HELPER_H__
#define __OS_API_HELPER_H__

/******************************************************************************
* Copyright (C) 2012 Evgeniy Golovin (evgeniy.golovin@unitpoint.ru)
*
* Latest source code: https://github.com/unitpoint/objectscript
*
* Permission is hereby granted, free of charge, to any person obtaining
* a copy of this software and associated documentation files (the
* "Software"), to deal in the Software without restriction, including
* without limitation the rights to use, copy, modify, merge, publish,
* distribute, sublicense, and/or sell copies of the Software, and to
* permit persons to whom the Software is furnished to do so, subject to
* the following conditions:
*
* The above copyright notice and this permission notice shall be
* included in all copies or substantial portions of the Software.
*
* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
* EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
* MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
* IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY
* CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,
* TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE
* SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
******************************************************************************/

#include "objectscript.h"

namespace ObjectScript
{

// =====================================================================

template <class T> struct RemoveConst          { typedef T type; };
template <class T> struct RemoveConst<const T> { typedef T type; };
template <class T> struct RemoveConst<const T&> { typedef T type; };
template <class T> struct RemoveConst<const T*> { typedef T * type; };

template <class T> struct RemoveRef		{ typedef T type; };
template <class T> struct RemoveRef<T&> { typedef T type; };

template <class T> struct RemovePtr		{ typedef T type; };
template <class T> struct RemovePtr<T*> { typedef T type; };

template <class T> struct PlainType { typedef typename RemovePtr<typename RemoveRef<typename RemoveConst<T>::type>::type>::type type; };

// =====================================================================

template <class T> int getCtypeId(){ static int id = (int)&id; return id; }
template <class T> int getInstanceId(){ static int id = (int)&id; return id; }

template <class T> const char * getCtypeName();

// =====================================================================

#define OS_DECL_CTYPE(type) OS_DECL_CTYPE_NAME(type, #type)
#define OS_DECL_CTYPE_NAME(type, name) template <> const char * getCtypeName<type>(){ return name; }

// =====================================================================

template <class T>
struct CtypeValue
{
};

// =====================================================================

OS_DECL_CTYPE(bool);

template <>
struct CtypeValue<bool>
{
	typedef bool type;

	static bool isValid(bool){ return true; }

	static type to(bool val){ return (type)val; }
	static bool to(OS * os, int offs)
	{
		return os->toBool(offs);
	}

	static void push(OS * os, bool val)
	{
		os->pushBool(val);
	}
};

// template <> struct CtypeValue<const bool>: public CtypeValue<bool>{};
// template <> struct CtypeValue<const bool&>: public CtypeValue<bool>{};

// =====================================================================

template <class T>
struct CtypeNumber
{
	typedef typename RemoveConst<T>::type type;

	static bool isValid(type){ return true; }

	static type to(const type& val){ return (type)val; }
	static type to(OS * os, int offs)
	{
		return (type)os->toNumber(offs);
	}

	static void push(OS * os, const type& val)
	{
		os->pushNumber((OS_NUMBER)val);
	}
};

#define OS_DECL_CTYPE_NUMBER(type) \
	OS_DECL_CTYPE(type); \
	template <> struct CtypeValue<type>: public CtypeNumber<type> {}
	// template <> struct CtypeValue<const type>: public CtypeNumber<type> {}; \
	// template <> struct CtypeValue<const type&>: public CtypeNumber<type> {}

OS_DECL_CTYPE_NUMBER(float);
OS_DECL_CTYPE_NUMBER(int);
OS_DECL_CTYPE_NUMBER(unsigned int);
OS_DECL_CTYPE_NUMBER(char);
OS_DECL_CTYPE_NUMBER(unsigned char);
OS_DECL_CTYPE_NUMBER(short);
OS_DECL_CTYPE_NUMBER(unsigned short);
OS_DECL_CTYPE_NUMBER(long);
OS_DECL_CTYPE_NUMBER(unsigned long);
OS_DECL_CTYPE_NUMBER(long long);
OS_DECL_CTYPE_NUMBER(unsigned long long);

// =====================================================================

template <class T> void pushCtypeValue(OS * os, T obj)
{
	typedef typename RemoveConst<T>::type type;
	CtypeValue<type>::push(os, CtypeValue<type>::to(obj));
}

// =====================================================================

template <class T> struct CtypeSimpleObject{};
template <class T> struct CtypeSimpleObject<T*>
{
	typedef typename RemoveConst<T>::type * type;

	static bool isValid(const type){ return true; }
	static type to(const type val){ return (type)val; }
	static type to(OS * os, int offs){ return (type)os->toUserdata(getInstanceId<typename RemoveConst<T>::type>(), offs);	}
	static void push(OS * os, const type val)
	{
		pushCtypeValue(os, val);
		os->pushStackValue();
		os->getGlobal(getCtypeName<T>());
		if(!os->isUserdata(getCtypeId<T>(), -1)){
			os->pop(2);
		}else{
			os->setPrototype(getInstanceId<T>());
		}
	}
};

#define OS_DECL_CTYPE_SIMPLE_OBJECT(type) \
	OS_DECL_CTYPE(type); \
	template <> struct CtypeValue<type*>: public CtypeSimpleObject<type*>{}

// =====================================================================

#define OS_GET_TEMPLATE_SELF(argType) \
	argType self = CtypeValue< typename RemoveConst<argType>::type >::to(os, -params-1); \
	if(!self){ \
		os->triggerError(OS_E_ERROR, OS::String(os, getCtypeName< typename PlainType<argType>::type >())+" this must not be null"); \
		return 0; \
	}

#define OS_GET_SELF(argType) \
	argType self = CtypeValue< RemoveConst<argType>::type >::to(os, -params-1); \
	if(!self){ \
		os->triggerError(OS_E_ERROR, OS::String(os, getCtypeName< PlainType<argType>::type >())+" this must not be null"); \
		return 0; \
	}

// =====================================================================

#define OS_GET_TEMPLATE_ARG(num, argType) \
	typename CtypeValue< typename RemoveConst<argType>::type >::type arg##num = CtypeValue< typename RemoveConst<argType>::type >::to(os, -params+num-1); \
	if(!CtypeValue< typename RemoveConst<argType>::type >::isValid(arg##num)){ \
		os->triggerError(OS_E_ERROR, OS::String(os, getCtypeName< typename PlainType<argType>::type >())+" expected"); \
		return 0; \
	}

#define OS_GET_ARG(num, argType) \
	CtypeValue< RemoveConst<argType>::type >::type arg##num = CtypeValue< RemoveConst<argType>::type >::to(os, -params+num-1); \
	if(!CtypeValue< RemoveConst<argType>::type >::isValid(arg##num)){ \
		os->triggerError(OS_E_ERROR, OS::String(os, getCtypeName< PlainType<argType>::type >())+" expected"); \
		return 0; \
	}

// =====================================================================

template <class T, class fieldType, class T2, fieldType T2::*field> 
int getField(OS * os, int params, int, int, void*)
{
	OS_GET_TEMPLATE_SELF(T*);
	typedef typename RemoveConst<fieldType>::type type;
	CtypeValue<type>::push(os, CtypeValue<type>::to(self->*field));
	return 1;
}

template <class T, class fieldType, class T2, fieldType T2::*field> 
int setField(OS * os, int params, int, int, void*)
{
	OS_GET_TEMPLATE_SELF(T*);
	OS_GET_TEMPLATE_ARG(1, fieldType);
	self->*field = arg1;
	return 0;
}

// =====================================================================

template <class T, class resType, class T2, resType(T2::*method)()const> 
int getFieldByMethod(OS * os, int params, int, int, void*)
{
	OS_GET_TEMPLATE_SELF(T*);
	typedef typename RemoveConst<resType>::type type;
	CtypeValue<type>::push(os, CtypeValue<type>::to((self->*method)()));
	return 1;
}

template <class T, class resType, class T2, resType(T2::*method)()> 
int getFieldByMethodNotConst(OS * os, int params, int, int, void*)
{
	OS_GET_TEMPLATE_SELF(T*);
	typedef typename RemoveConst<resType>::type type;
	CtypeValue<type>::push(os, CtypeValue<type>::to((self->*method)()));
	return 1;
}

// =====================================================================

template <class T, class resType, class argType1, class T2, resType(T2::*method)(argType1)const> 
int getFieldByMethod(OS * os, int params, int, int, void*)
{
	OS_GET_TEMPLATE_SELF(T*);
	OS_GET_TEMPLATE_ARG(1, argType1);
	typedef typename RemoveConst<resType>::type type;
	CtypeValue<type>::push(os, CtypeValue<type>::to((self->*method)(arg1)));
	return 1;
}

template <class T, class resType, class argType1, class T2, resType(T2::*method)(argType1)> 
int getFieldByMethodNotConst(OS * os, int params, int, int, void*)
{
	OS_GET_TEMPLATE_SELF(T*);
	OS_GET_TEMPLATE_ARG(1, argType1);
	typedef typename RemoveConst<resType>::type type;
	CtypeValue<resType>::push(os, CtypeValue<resType>::to((self->*method)(arg1)));
	return 1;
}

// =====================================================================

template <class T, class resType, class argType1, class argType2, class T2, resType(T2::*method)(argType1, argType2)const> 
int getFieldByMethod(OS * os, int params, int, int, void*)
{
	OS_GET_TEMPLATE_SELF(T*);
	OS_GET_TEMPLATE_ARG(1, argType1);
	OS_GET_TEMPLATE_ARG(2, argType2);
	typedef typename RemoveConst<resType>::type type;
	CtypeValue<type>::push(os, CtypeValue<type>::to((self->*method)(arg1, arg2)));
	return 1;
}

template <class T, class resType, class argType1, class argType2, class T2, resType(T2::*method)(argType1, argType2)> 
int getFieldByMethodNotConst(OS * os, int params, int, int, void*)
{
	OS_GET_TEMPLATE_SELF(T*);
	OS_GET_TEMPLATE_ARG(1, argType1);
	OS_GET_TEMPLATE_ARG(2, argType2);
	typedef typename RemoveConst<resType>::type type;
	CtypeValue<type>::push(os, CtypeValue<type>::to((self->*method)(arg1, arg2)));
	return 1;
}

// =====================================================================

template <class T, class T2, void(T2::*method)()> 
int voidMethod(OS * os, int params, int, int, void*)
{
	OS_GET_TEMPLATE_SELF(T*);
	(self->*method)();
	return 0;
}

template <class T, class argType1, class T2, void(T2::*method)(argType1)> 
int voidMethod(OS * os, int params, int, int, void*)
{
	OS_GET_TEMPLATE_SELF(T*);
	OS_GET_TEMPLATE_ARG(1, argType1);
	(self->*method)(arg1);
	return 0;
}

template <class T, class argType1, class argType2, class T2, void(T2::*method)(argType1, argType2)> 
int voidMethod(OS * os, int params, int, int, void*)
{
	OS_GET_TEMPLATE_SELF(T*);
	OS_GET_TEMPLATE_ARG(1, argType1);
	OS_GET_TEMPLATE_ARG(2, argType2);
	(self->*method)(arg1, arg2);
	return 0;
}

template <class T, class argType1, class argType2, class argType3, class T2, void(T2::*method)(argType1, argType2, argType3)> 
int voidMethod(OS * os, int params, int, int, void*)
{
	OS_GET_TEMPLATE_SELF(T*);
	OS_GET_TEMPLATE_ARG(1, argType1);
	OS_GET_TEMPLATE_ARG(2, argType2);
	OS_GET_TEMPLATE_ARG(3, argType3);
	(self->*method)(arg1, arg2, arg3);
	return 0;
}

template <class T, class argType1, class argType2, class argType3, class argType4, class T2, 
	void(T2::*method)(argType1, argType2, argType3, argType4)> 
int voidMethod(OS * os, int params, int, int, void*)
{
	OS_GET_TEMPLATE_SELF(T*);
	OS_GET_TEMPLATE_ARG(1, argType1);
	OS_GET_TEMPLATE_ARG(2, argType2);
	OS_GET_TEMPLATE_ARG(3, argType3);
	OS_GET_TEMPLATE_ARG(4, argType4);
	(self->*method)(arg1, arg2, arg3, arg4);
	return 0;
}

template <class T, class argType1, class argType2, class argType3, class argType4, class argType5, class T2, 
	void(T2::*method)(argType1, argType2, argType3, argType4, argType5)> 
int voidMethod(OS * os, int params, int, int, void*)
{
	OS_GET_TEMPLATE_SELF(T*);
	OS_GET_TEMPLATE_ARG(1, argType1);
	OS_GET_TEMPLATE_ARG(2, argType2);
	OS_GET_TEMPLATE_ARG(3, argType3);
	OS_GET_TEMPLATE_ARG(4, argType4);
	OS_GET_TEMPLATE_ARG(5, argType5);
	(self->*method)(arg1, arg2, arg3, arg4, arg5);
	return 0;
}

template <class T, class argType1, class argType2, class argType3, class argType4, class argType5, class argType6, class T2, 
	void(T2::*method)(argType1, argType2, argType3, argType4, argType5, argType6)> 
int voidMethod(OS * os, int params, int, int, void*)
{
	OS_GET_TEMPLATE_SELF(T*);
	OS_GET_TEMPLATE_ARG(1, argType1);
	OS_GET_TEMPLATE_ARG(2, argType2);
	OS_GET_TEMPLATE_ARG(3, argType3);
	OS_GET_TEMPLATE_ARG(4, argType4);
	OS_GET_TEMPLATE_ARG(5, argType5);
	OS_GET_TEMPLATE_ARG(6, argType6);
	(self->*method)(arg1, arg2, arg3, arg4, arg5, arg6);
	return 0;
}

template <class T, class argType1, class argType2, class argType3, class argType4, class argType5, class argType6, class argType7,
	class T2, void(T2::*method)(argType1, argType2, argType3, argType4, argType5, argType6, argType7)> 
int voidMethod(OS * os, int params, int, int, void*)
{
	OS_GET_TEMPLATE_SELF(T*);
	OS_GET_TEMPLATE_ARG(1, argType1);
	OS_GET_TEMPLATE_ARG(2, argType2);
	OS_GET_TEMPLATE_ARG(3, argType3);
	OS_GET_TEMPLATE_ARG(4, argType4);
	OS_GET_TEMPLATE_ARG(5, argType5);
	OS_GET_TEMPLATE_ARG(6, argType6);
	OS_GET_TEMPLATE_ARG(7, argType7);
	(self->*method)(arg1, arg2, arg3, arg4, arg5, arg6, arg7);
	return 0;
}

template <class T, class argType1, class argType2, class argType3, class argType4, class argType5, class argType6, class argType7, class argType8,
	class T2, void(T2::*method)(argType1, argType2, argType3, argType4, argType5, argType6, argType7, argType8)> 
int voidMethod(OS * os, int params, int, int, void*)
{
	OS_GET_TEMPLATE_SELF(T*);
	OS_GET_TEMPLATE_ARG(1, argType1);
	OS_GET_TEMPLATE_ARG(2, argType2);
	OS_GET_TEMPLATE_ARG(3, argType3);
	OS_GET_TEMPLATE_ARG(4, argType4);
	OS_GET_TEMPLATE_ARG(5, argType5);
	OS_GET_TEMPLATE_ARG(6, argType6);
	OS_GET_TEMPLATE_ARG(7, argType7);
	OS_GET_TEMPLATE_ARG(8, argType8);
	(self->*method)(arg1, arg2, arg3, arg4, arg5, arg6, arg7, arg8);
	return 0;
}

// =====================================================================

#define OS_GET_FIELD(T, fieldType, name, field)  {"__get@"#name, getField<T, fieldType, T, & T::field>}
#define OS_SET_FIELD(T, fieldType, name, field)  {"__set@"#name, getField<T, fieldType, T, & T::field>}

#define OS_GET_FIELD_SPEC(T, fieldType, name, T2, field)  {"__get@"#name, getField<T, fieldType, T2, & T2::field>}
#define OS_SET_FIELD_SPEC(T, fieldType, name, T2, field)  {"__set@"#name, getField<T, fieldType, T2, & T2::field>}

#define OS_GET_METHOD(T, resType, name, method)  {"__get@"#name, getFieldByMethod<T, resType, T, & T::method>}
#define OS_GET_METHOD_NOT_CONST(T, resType, name, method)  {"__get@"#name, getFieldByMethodNotConst<T, resType, T, & T::method>}
#define OS_GET_METHOD_SPEC(T, resType, name, T2, method)  {"__get@"#name, getFieldByMethod<T, resType, T2, & T2::method>}

#define OS_GET_METHOD_1(T, resType, name, method, argType1)  {"__get@"#name, getFieldByMethod<T, resType, argType1, T, & T::method>}
#define OS_GET_METHOD_1_NOT_CONST(T, resType, name, method, argType1)  {"__get@"#name, getFieldByMethodNotConst<T, resType, argType1, T, & T::method>}
#define OS_GET_METHOD_2_NOT_CONST(T, resType, name, method, argType1, argType2)  {"__get@"#name, getFieldByMethodNotConst<T, resType, argType1, argType2, T, & T::method>}

#define OS_SET_METHOD(T, resType, name, method)  {"__set@"#name, voidMethod<T, resType, T, & T::method>}
#define OS_SET_METHOD_SPEC(T, resType, name, T2, method)  {"__set@"#name, voidMethod<T, resType, T2, & T2::method>}

#define OS_VOID_METHOD(T, name, method)  {#name, voidMethod<T, T, & T::method>}
#define OS_VOID_METHOD_1(T, name, method, argType1)  {#name, voidMethod<T, argType1, T, & T::method>}
#define OS_VOID_METHOD_2(T, name, method, argType1, argType2)  {#name, voidMethod<T, argType1, argType2, T, & T::method>}
#define OS_VOID_METHOD_3(T, name, method, argType1, argType2, argType3)  {#name, voidMethod<T, argType1, argType2, argType3, T, & T::method>}
#define OS_VOID_METHOD_4(T, name, method, argType1, argType2, argType3, argType4)  {#name, voidMethod<T, argType1, argType2, argType3, argType4, T, & T::method>}
#define OS_VOID_METHOD_5(T, name, method, argType1, argType2, argType3, argType4, argType5)  {#name, voidMethod<T, argType1, argType2, argType3, argType4, argType5, T, & T::method>}
#define OS_VOID_METHOD_6(T, name, method, argType1, argType2, argType3, argType4, argType5, argType6)  {#name, voidMethod<T, argType1, argType2, argType3, argType4, argType5, argType6, T, & T::method>}
#define OS_VOID_METHOD_7(T, name, method, argType1, argType2, argType3, argType4, argType5, argType6, argType7)  {#name, voidMethod<T, argType1, argType2, argType3, argType4, argType5, argType6, argType7, T, & T::method>}

#define OS_VOID_METHOD_SPEC(T, name, T2, method)  {#name, voidMethod<T, T2, & T2::method>}
#define OS_VOID_METHOD_1_SPEC(T, name, T2, method, argType1)  {#name, voidMethod<T, argType1, T2, & T2::method>}
#define OS_VOID_METHOD_2_SPEC(T, name, T2, method, argType1, argType2)  {#name, voidMethod<T, argType1, argType2, T2, & T2::method>}
#define OS_VOID_METHOD_3_SPEC(T, name, T2, method, argType1, argType2, argType3)  {#name, voidMethod<T, argType1, argType2, argType3, T2, & T2::method>}
#define OS_VOID_METHOD_4_SPEC(T, name, T2, method, argType1, argType2, argType3, argType4)  {#name, voidMethod<T, argType1, argType2, argType3, argType4, T2, & T::method>}
#define OS_VOID_METHOD_5_SPEC(T, name, T2, method, argType1, argType2, argType3, argType4, argType5)  {#name, voidMethod<T, argType1, argType2, argType3, argType4, argType5, T2, & T::method>}
#define OS_VOID_METHOD_6_SPEC(T, name, T2, method, argType1, argType2, argType3, argType4, argType5, argType6)  {#name, voidMethod<T, argType1, argType2, argType3, argType4, argType5, argType6, T2, & T::method>}
#define OS_VOID_METHOD_7_SPEC(T, name, T2, method, argType1, argType2, argType3, argType4, argType5, argType6, argType7)  {#name, voidMethod<T, argType1, argType2, argType3, argType4, argType5, argType6, argType7, T2, & T2::method>}

// =====================================================================

template <class T>
void createCtypePrototype(OS * os, OS::FuncDef * list)
{
	os->pushGlobals();
	os->pushString(getCtypeName<T>());
	os->pushUserdata(getCtypeId<T>(), 0);
	os->setFuncs(list);
	os->setProperty();
}

// =====================================================================

};

#endif // __OS_API_HELPER_H__