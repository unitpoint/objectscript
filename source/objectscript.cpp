#include "objectscript.h"
#include "os-binder.h"
#include <time.h>

using namespace ObjectScript;

#define HASH_GROW_SHIFT 0

#define OS_PTR_HASH(p) ((int)(intptr_t)(p) >> 2)

#define Instruction OS_U32

// #define OS_USE_OPCODE_VV

#ifndef OS_USE_OPCODE_VV

/*
** size and position of opcode arguments.
*/
#define OS_SIZE_C		9
#define OS_SIZE_B		9
#define OS_SIZE_Bx		(OS_SIZE_C + OS_SIZE_B)
#define OS_SIZE_A		6
#define OS_SIZE_Ax		(OS_SIZE_C + OS_SIZE_B + OS_SIZE_A)

// #define OS_SIZE_CC		2
#define OS_SIZE_OP		6

// #define OS_OPCODE_CONST_B	(1<<0)
// #define OS_OPCODE_CONST_C	(1<<1)

#define OS_POS_OP		0
#define OS_POS_A		(OS_POS_OP + OS_SIZE_OP)
#define OS_POS_C		(OS_POS_A + OS_SIZE_A)
#define OS_POS_B		(OS_POS_C + OS_SIZE_C)
#define OS_POS_Bx		OS_POS_C
#define OS_POS_Ax		OS_POS_A

#define OS_MAXARG_Bx        ((1<<OS_SIZE_Bx)-1)
#define OS_MAXARG_sBx        (OS_MAXARG_Bx>>1)
#define OS_MAXARG_Ax		((1<<OS_SIZE_Ax)-1)
#define OS_MAXARG_sAx		 (OS_MAXARG_Ax>>1)

#define OS_MAXARG_A        ((1<<OS_SIZE_A)-1)
#define OS_MAXARG_B        ((1<<(OS_SIZE_B-1))-1)
#define OS_MAXARG_C        ((1<<(OS_SIZE_C-1))-1)

#define OS_OPCODE_CONST_B	(1<<(OS_POS_B + OS_SIZE_B - 1))
#define OS_OPCODE_CONST_C	(1<<(OS_POS_C + OS_SIZE_C - 1))

#define OS_MASK1(n,p)	((~((~(Instruction)0)<<(n)))<<(p))
#define OS_MASK0(n,p)	(~OS_MASK1(n,p))

#define OS_GET_OPCODE_WITH_CC(i)	(((i)>>OS_POS_OP) & OS_MASK1(OS_SIZE_OP, 0))
#define OS_GET_OPCODE_NO_CC(i)	OS_GET_OPCODE_WITH_CC(i)
#define OS_GET_OPCODE_TYPE(i)	(((i)>>(OS_POS_OP)) & OS_MASK1(OS_SIZE_OP, 0))
#define OS_FROM_OPCODE_TYPE(i)	((i)<<0)
#define OS_TO_OPCODE_TYPE(i)	((i)>>0)

#define getarg(i,pos,size)		(((i)>>pos) & OS_MASK1(size, 0))
#define setarg(i,v,pos,size)	((i) = (((i)&OS_MASK0(size,pos)) | (((Instruction)(v))<<pos) & OS_MASK1(size, pos)))

#define OS_GETARG_A(i)		getarg(i, OS_POS_A, OS_SIZE_A)
#define OS_SETARG_A(i,v)	setarg(i, v, OS_POS_A, OS_SIZE_A)

#define OS_GETARG_B(i)		getarg(i, OS_POS_B, OS_SIZE_B-1)
#define OS_SETARG_B(i,v)	setarg(i, v, OS_POS_B, OS_SIZE_B-1)

#define OS_GETARG_C(i)		getarg(i, OS_POS_C, OS_SIZE_C-1)
#define OS_SETARG_C(i,v)	setarg(i, v, OS_POS_C, OS_SIZE_C-1)

#define OS_GETARG_Bx(i)		getarg(i, OS_POS_Bx, OS_SIZE_Bx)
#define OS_SETARG_Bx(i,v)	setarg(i, v, OS_POS_Bx, OS_SIZE_Bx)

#define OS_GETARG_Ax(i)		getarg(i, OS_POS_Ax, OS_SIZE_Ax)
#define OS_SETARG_Ax(i,v)	setarg(i, v, OS_POS_Ax, OS_SIZE_Ax)

#define OS_GETARG_sBx(i)	(OS_GETARG_Bx(i)-OS_MAXARG_sBx)
#define OS_SETARG_sBx(i,b)	OS_SETARG_Bx((i),((unsigned int)((b)+OS_MAXARG_sBx)))

#define OS_OPCODE_ABC(o,a,b,c)	((((Instruction)(o)) << OS_POS_OP) \
			| (((Instruction)(a)) << OS_POS_A) \
			| (((Instruction)(b)) << OS_POS_B) \
			| (((Instruction)(c)) << OS_POS_C))

#define OS_OPCODE_ABx(o,a,bc)	((((Instruction)(o)) << OS_POS_OP) \
			| (((Instruction)(a)) << OS_POS_A) \
			| (((Instruction)(bc)) << OS_POS_Bx))

#define OS_MAX_GENERIC_CONST_INDEX ((1<<(OS_SIZE_B-1))-1)

#define OS_GETARG_B_VALUE() ((instruction) & OS_OPCODE_CONST_B ? \
	(stack_func_prog_values[b]) \
	: (stack_func_locals[b]))

#define OS_GETARG_C_VALUE() ((instruction) & OS_OPCODE_CONST_C ? \
	(stack_func_prog_values[c]) \
	: (stack_func_locals[c]))

#define OS_CASE_OPCODE_ALL(opcode) case (opcode)
#define OS_CASE_OPCODE(opcode) case (opcode)

#else // OS_USE_OPCODE_VV

/*
** size and position of opcode arguments.
*/
#define OS_SIZE_C		8
#define OS_SIZE_B		8
#define OS_SIZE_Bx		(OS_SIZE_C + OS_SIZE_B)
#define OS_SIZE_A		8
#define OS_SIZE_Ax		(OS_SIZE_C + OS_SIZE_B + OS_SIZE_A)

// #define OS_SIZE_CC		2
#define OS_SIZE_OP		8

#define OS_OPCODE_CONST_B	(1<<0)
#define OS_OPCODE_CONST_C	(1<<1)

#define OS_POS_OP		0
#define OS_POS_A		(OS_POS_OP + OS_SIZE_OP)
#define OS_POS_B		(OS_POS_A + OS_SIZE_A)
#define OS_POS_C		(OS_POS_B + OS_SIZE_B)
#define OS_POS_Bx		OS_POS_B
#define OS_POS_Ax		OS_POS_A

#define OS_MAXARG_Bx        ((1<<OS_SIZE_Bx)-1)
#define OS_MAXARG_sBx        (OS_MAXARG_Bx>>1)
#define OS_MAXARG_Ax		((1<<OS_SIZE_Ax)-1)
#define OS_MAXARG_sAx		 (OS_MAXARG_Ax>>1)

#define OS_MAXARG_A        ((1<<OS_SIZE_A)-1)
#define OS_MAXARG_B        ((1<<OS_SIZE_B)-1)
#define OS_MAXARG_C        ((1<<OS_SIZE_C)-1)

#define OS_MASK1(n,p)	((~((~(Instruction)0)<<(n)))<<(p))
#define OS_MASK0(n,p)	(~OS_MASK1(n,p))

#define OS_GET_OPCODE_WITH_CC(i)	(((i)>>OS_POS_OP) & OS_MASK1(OS_SIZE_OP, 0))
#define OS_GET_OPCODE_NO_CC(i)	(((i)>>OS_POS_OP) & OS_MASK1(OS_SIZE_OP-2, 2))
#define OS_GET_OPCODE_TYPE(i)	(((i)>>(OS_POS_OP+2)) & OS_MASK1(OS_SIZE_OP-2, 0))
#define OS_FROM_OPCODE_TYPE(i)	((i)<<2)
#define OS_TO_OPCODE_TYPE(i)	((i)>>2)

#define getarg(i,pos,size)		(((i)>>pos) & OS_MASK1(size, 0))
#define setarg(i,v,pos,size)	((i) = (((i)&OS_MASK0(size,pos)) | (((Instruction)(v))<<pos) & OS_MASK1(size, pos)))

#define OS_GETARG_A(i)		getarg(i, OS_POS_A, OS_SIZE_A)
#define OS_SETARG_A(i,v)	setarg(i, v, OS_POS_A, OS_SIZE_A)

#define OS_GETARG_B(i)		getarg(i, OS_POS_B, OS_SIZE_B)
#define OS_SETARG_B(i,v)	setarg(i, v, OS_POS_B, OS_SIZE_B)

#define OS_GETARG_C(i)		getarg(i, OS_POS_C, OS_SIZE_C)
#define OS_SETARG_C(i,v)	setarg(i, v, OS_POS_C, OS_SIZE_C)

#define OS_GETARG_Bx(i)		getarg(i, OS_POS_Bx, OS_SIZE_Bx)
#define OS_SETARG_Bx(i,v)	setarg(i, v, OS_POS_Bx, OS_SIZE_Bx)

#define OS_GETARG_Ax(i)		getarg(i, OS_POS_Ax, OS_SIZE_Ax)
#define OS_SETARG_Ax(i,v)	setarg(i, v, OS_POS_Ax, OS_SIZE_Ax)

#define OS_GETARG_sBx(i)	(OS_GETARG_Bx(i)-OS_MAXARG_sBx)
#define OS_SETARG_sBx(i,b)	OS_SETARG_Bx((i),((unsigned int)((b)+OS_MAXARG_sBx)))

#define OS_OPCODE_ABC(o,a,b,c)	((((Instruction)(o)) << OS_POS_OP) \
			| (((Instruction)(a)) << OS_POS_A) \
			| (((Instruction)(b)) << OS_POS_B) \
			| (((Instruction)(c)) << OS_POS_C))

#define OS_OPCODE_ABx(o,a,bc)	((((Instruction)(o)) << OS_POS_OP) \
			| (((Instruction)(a)) << OS_POS_A) \
			| (((Instruction)(bc)) << OS_POS_Bx))

#define OS_MAX_GENERIC_CONST_INDEX ((1<<(OS_SIZE_B-1))-1)

#define OS_GETARG_B_VALUE() ((instruction) & OS_OPCODE_CONST_B ? \
	(stack_func_prog_values[b]) \
	: (stack_func_locals[b]))

#define OS_GETARG_C_VALUE() ((instruction) & OS_OPCODE_CONST_C ? \
	(stack_func_prog_values[c]) \
	: (stack_func_locals[c]))

#define OS_CASE_OPCODE_ALL(opcode) case ((opcode)<<2): case (((opcode)<<2)|OS_OPCODE_CONST_B): case (((opcode)<<2)|OS_OPCODE_CONST_C): case (((opcode)<<2)|OS_OPCODE_CONST_B|OS_OPCODE_CONST_C)
#define OS_CASE_OPCODE(opcode) case ((opcode)<<2)
#define OS_CASE_OPCODE_VV(opcode) case ((opcode)<<2)
#define OS_CASE_OPCODE_CV(opcode) case (((opcode)<<2)|OS_OPCODE_CONST_B)
#define OS_CASE_OPCODE_VC(opcode) case (((opcode)<<2)|OS_OPCODE_CONST_C)
#define OS_CASE_OPCODE_CC(opcode) case (((opcode)<<2)|OS_OPCODE_CONST_B|OS_OPCODE_CONST_C)

#endif // OS_USE_OPCODE_VV

// =====================================================================
// =====================================================================
// =====================================================================

#if defined __GNUC__ || defined IW_SDK

int OS_VSNPRINTF(OS_CHAR * str, size_t size, const OS_CHAR *format, va_list va)
{
	return vsnprintf(str, size, format, va);
}

#else

int OS_VSNPRINTF(OS_CHAR * str, size_t size, const OS_CHAR *format, va_list va)
{
	return vsnprintf_s(str, size, size/sizeof(OS_CHAR), format, va);
}

#endif

int OS_SNPRINTF(OS_CHAR * str, size_t size, const OS_CHAR *format, ...)
{

	va_list va;
	va_start(va, format);
	int ret = OS_VSNPRINTF(str, size, format, va);
	va_end(va);
	return ret;
}

/*
static bool OS_ISNAN(float a)
{
	volatile float b = a;
	return b != b;
}

static bool OS_ISNAN(double a)
{
	volatile double b = a;
	return b != b;
}
*/

#include <float.h>
#include <limits.h>

template <class T> T OS_getMaxValue();
template <> double OS_getMaxValue<double>(){ return DBL_MAX; }
template <> float OS_getMaxValue<float>(){ return FLT_MAX; }
template <> int OS_getMaxValue<int>(){ return INT_MAX; }

#define OS_MAX_NUMBER OS_getMaxValue<OS_NUMBER>()

#define CURRENT_BYTE_ORDER       (*(OS_INT32*)"\x01\x02\x03\x04")
#define LITTLE_ENDIAN_BYTE_ORDER 0x04030201
#define BIG_ENDIAN_BYTE_ORDER    0x01020304
#define PDP_ENDIAN_BYTE_ORDER    0x02010403

#define IS_LITTLE_ENDIAN (CURRENT_BYTE_ORDER == LITTLE_ENDIAN_BYTE_ORDER)
#define IS_BIG_ENDIAN    (CURRENT_BYTE_ORDER == BIG_ENDIAN_BYTE_ORDER)
#define IS_PDP_ENDIAN    (CURRENT_BYTE_ORDER == PDP_ENDIAN_BYTE_ORDER)

static inline OS_BYTE toLittleEndianByteOrder(OS_BYTE val)
{
	OS_ASSERT(sizeof(val) == sizeof(OS_BYTE)*1);
	return val;
}

static inline OS_INT8 toLittleEndianByteOrder(OS_INT8 val)
{
	OS_ASSERT(sizeof(val) == sizeof(OS_BYTE)*1);
	return val;
}

static inline OS_U16 toLittleEndianByteOrder(OS_U16 val)
{
	OS_ASSERT(sizeof(val) == sizeof(OS_BYTE)*2);
	if(IS_LITTLE_ENDIAN){
		return val;
	}
	OS_U16 r;
	((OS_BYTE*)&r)[0] = ((OS_BYTE*)&val)[1];
	((OS_BYTE*)&r)[1] = ((OS_BYTE*)&val)[0];
	return r;
}

static inline OS_INT16 toLittleEndianByteOrder(OS_INT16 val)
{
	OS_ASSERT(sizeof(val) == sizeof(OS_BYTE)*2);
	if(IS_LITTLE_ENDIAN){
		return val;
	}
	OS_INT16 r;
	((OS_BYTE*)&r)[0] = ((OS_BYTE*)&val)[1];
	((OS_BYTE*)&r)[1] = ((OS_BYTE*)&val)[0];
	return r;
}

static inline OS_INT32 toLittleEndianByteOrder(OS_INT32 val)
{
	OS_ASSERT(sizeof(val) == sizeof(OS_BYTE)*4);
	if(IS_LITTLE_ENDIAN){
		return val;
	}
	OS_INT32 r;
	((OS_BYTE*)&r)[0] = ((OS_BYTE*)&val)[3];
	((OS_BYTE*)&r)[1] = ((OS_BYTE*)&val)[2];
	((OS_BYTE*)&r)[2] = ((OS_BYTE*)&val)[1];
	((OS_BYTE*)&r)[3] = ((OS_BYTE*)&val)[0];
	return r;
}

static inline OS_INT64 toLittleEndianByteOrder(OS_INT64 val)
{
	OS_ASSERT(sizeof(val) == sizeof(OS_BYTE)*8);
	if(IS_LITTLE_ENDIAN){
		return val;
	}
	OS_INT64 r;
	((OS_BYTE*)&r)[0] = ((OS_BYTE*)&val)[7];
	((OS_BYTE*)&r)[1] = ((OS_BYTE*)&val)[6];
	((OS_BYTE*)&r)[2] = ((OS_BYTE*)&val)[5];
	((OS_BYTE*)&r)[3] = ((OS_BYTE*)&val)[4];
	((OS_BYTE*)&r)[4] = ((OS_BYTE*)&val)[3];
	((OS_BYTE*)&r)[5] = ((OS_BYTE*)&val)[2];
	((OS_BYTE*)&r)[6] = ((OS_BYTE*)&val)[1];
	((OS_BYTE*)&r)[7] = ((OS_BYTE*)&val)[0];
	return r;
}

static inline float toLittleEndianByteOrder(float val)
{
	OS_ASSERT(sizeof(val) == sizeof(OS_BYTE)*4);
	if(IS_LITTLE_ENDIAN){
		return val;
	}
	float r;
	((OS_BYTE*)&r)[0] = ((OS_BYTE*)&val)[3];
	((OS_BYTE*)&r)[1] = ((OS_BYTE*)&val)[2];
	((OS_BYTE*)&r)[2] = ((OS_BYTE*)&val)[1];
	((OS_BYTE*)&r)[3] = ((OS_BYTE*)&val)[0];
	return r;
}

static inline double toLittleEndianByteOrder(double val)
{
	OS_ASSERT(sizeof(val) == sizeof(OS_BYTE)*8);
	if(IS_LITTLE_ENDIAN){
		return val;
	}
	double r;
	((OS_BYTE*)&r)[0] = ((OS_BYTE*)&val)[7];
	((OS_BYTE*)&r)[1] = ((OS_BYTE*)&val)[6];
	((OS_BYTE*)&r)[2] = ((OS_BYTE*)&val)[5];
	((OS_BYTE*)&r)[3] = ((OS_BYTE*)&val)[4];
	((OS_BYTE*)&r)[4] = ((OS_BYTE*)&val)[3];
	((OS_BYTE*)&r)[5] = ((OS_BYTE*)&val)[2];
	((OS_BYTE*)&r)[6] = ((OS_BYTE*)&val)[1];
	((OS_BYTE*)&r)[7] = ((OS_BYTE*)&val)[0];
	return r;
}

#define fromLittleEndianByteOrder toLittleEndianByteOrder

// static const OS_INT32 nan_data = 0x7fc00000;
// static const float nan_float = fromLittleEndianByteOrder(*(float*)&nan_data);

static inline void parseSpaces(const OS_CHAR *& str)
{
	while(*str && OS_IS_SPACE(*str))
		str++;
}

template <class T>
static bool parseSimpleHex(const OS_CHAR *& p_str, T& p_val)
{
	T val = 0, prev_val = 0;
	const OS_CHAR * str = p_str;
	const OS_CHAR * start = str;
	for(;; str++){
		if(*str >= OS_TEXT('0') && *str <= OS_TEXT('9')){
			val = (val << 4) + (T)(*str - OS_TEXT('0'));
		}else if(*str >= OS_TEXT('a') && *str <= OS_TEXT('f')){
			val = (val << 4) + 10 + (T)(*str - OS_TEXT('a'));
		}else if(*str >= OS_TEXT('A') && *str <= OS_TEXT('F')){
			val = (val << 4) + 10 + (T)(*str - OS_TEXT('A'));
		}else{
			break;
		}
		if(prev_val > val){
			p_str = start;
			p_val = 0;
			return false;
		}
		prev_val = val;
	}
	p_val = val;
	p_str = str;
	return str > start;
}

template <class T>
static bool parseSimpleBin(const OS_CHAR *& p_str, T& p_val)
{
	T val = 0, prev_val = 0;
	const OS_CHAR * str = p_str;
	const OS_CHAR * start = str;
	for(; *str >= OS_TEXT('0') && *str <= OS_TEXT('1'); str++){
		val = (val << 1) + (T)(*str - OS_TEXT('0'));
		if(prev_val > val){
			p_str = start;
			p_val = 0;
			return false;
		}
		prev_val = val;
	}
	p_val = val;
	p_str = str;
	return str > start;
}

template <class T>
static bool parseSimpleOctal(const OS_CHAR *& p_str, T& p_val)
{
	T val = 0, prev_val = 0;
	const OS_CHAR * str = p_str;
	const OS_CHAR * start = str;
	for(; *str >= OS_TEXT('0') && *str <= OS_TEXT('7'); str++)
	{
		val = (val << 3) + (T)(*str - OS_TEXT('0'));
		if(prev_val > val){
			p_str = start;
			p_val = 0;
			return false;
		}
		prev_val = val;
	}
	p_val = val;
	p_str = str;
	return str > start;
}

template <class T>
static bool parseSimpleDec(const OS_CHAR *& p_str, T& p_val)
{
	T val = 0, prev_val = 0;
	const OS_CHAR * str = p_str;
	const OS_CHAR * start = str;
	for(; *str >= OS_TEXT('0') && *str <= OS_TEXT('9'); str++){
		val = val * 10 + (T)(*str - OS_TEXT('0'));
		if(prev_val > val){
			p_str = start;
			p_val = 0;
			return false;
		}
		prev_val = val;
	}
	p_val = val;
	p_str = str;
	return str > start;
}

template <class T>
static bool parseSimpleFloat(const OS_CHAR *& p_str, T& p_val)
{
	T val = 0;
	const OS_CHAR * str = p_str;
	const OS_CHAR * start = str;
	for(; *str >= OS_TEXT('0') && *str <= OS_TEXT('9'); str++){
		val = val * 10 + (*str - OS_TEXT('0'));
	}
	p_val = val;
	p_str = str;
	return str > start;
}

bool OS::Utils::parseFloat(const OS_CHAR *& str, OS_FLOAT& result)
{
	const OS_CHAR * start_str = str;
	int sign = 1;
	if(*str == OS_TEXT('-')){
		str++;
		start_str++;
		sign = -1;
	}else if(*str == OS_TEXT('+')){
		str++;
		start_str++;
	}

	if(str[0] == OS_TEXT('0') && str[1] != OS_TEXT('.')){
		bool is_valid, is_octal = false;
		OS_INT int_val;
		if(str[1] == OS_TEXT('x') || str[1] == OS_TEXT('X')){ // parse hex
			str += 2;
			is_valid = parseSimpleHex(str, int_val);
		}else if(str[1] == OS_TEXT('b') || str[1] == OS_TEXT('B')){ // parse hex
			str += 2;
			is_valid = parseSimpleBin(str, int_val);
		}else{ // parse octal
			is_octal = true;
			is_valid = parseSimpleOctal(str, int_val);
		}
		if(!is_valid || (start_str+1 == str && !is_octal)){
			result = 0;
			return false;
		}
		if((OS_INT)(OS_FLOAT)int_val != int_val){
			result = 0;
			return false;
		}
		result = (OS_FLOAT)int_val;
		return true;
	}

	OS_FLOAT float_val;
	if(!parseSimpleFloat(str, float_val)){
		result = 0;
		return false;
	}

	if(*str == OS_TEXT('.')){ // parse float
		// parse 1.#INF ...
		if(sign == 1 && start_str+1 == str && *start_str == OS_TEXT('1') && str[1] == OS_TEXT('#')){
			const OS_CHAR * spec[] = {OS_TEXT("INF"), OS_TEXT("IND"), OS_TEXT("QNAN"), NULL};
			int i = 0;
			for(; spec[i]; i++){
				if(OS_STRCMP(str, spec[i]) != 0)
					continue;

				size_t specLen = OS_STRLEN(spec[i]);
				str += specLen;
				if(!*str || OS_IS_SPACE(*str) || OS_STRCHR(OS_TEXT("!@#$%^&*()-+={}[]\\|;:'\",<.>/?`~"), *str)){
					OS_INT32 spec_val;
					switch(i){
					case 0:
						spec_val = 0x7f800000;
						break;

					case 1:
						spec_val = 0xffc00000;
						break;

					default:
						OS_ASSERT(false);
						// no break

					case 2:
						spec_val = 0x7fc00000;
						break;
					}
					result = (OS_FLOAT)fromLittleEndianByteOrder(*(float*)&spec_val);
					return true;
				}            
			}
			result = 0;
			return false;
		}

		OS_FLOAT m = (OS_FLOAT)0.1;
		for(str++; *str >= OS_TEXT('0') && *str <= OS_TEXT('9'); str++, m *= (OS_FLOAT)0.1){
			float_val += (OS_FLOAT)(*str - OS_TEXT('0')) * m;
		}
		if(start_str == str){
			result = 0;
			return false;
		}
		if(*str == OS_TEXT('e') || *str == OS_TEXT('E')){
			str++;
			bool div = false; // + for default
			if(*str == OS_TEXT('-')){
				div = true;
				str++;
			}else if(*str == OS_TEXT('+')){
				// div = false;
				str++;
			}
			int pow;
			if(!parseSimpleDec(str, pow)){
				result = 0;
				return false;
			}
			m = (OS_FLOAT)1.0;
			for(int i = 0; i < pow; i++){
				m *= (OS_FLOAT)10.0;
			}
			if(div){
				float_val /= m;
			}else{
				float_val *= m;
			}
		}
		result = sign > 0 ? float_val : -float_val;
		return true;
	}
	if(start_str == str){
		result = 0;
		return false;
	}
	result = sign > 0 ? float_val : -float_val;
	return true;
}

OS_CHAR * OS::Utils::numToStr(OS_CHAR * dst, OS_INT32 a)
{
	OS_SNPRINTF(dst, sizeof(OS_CHAR)*63, OS_TEXT("%i"), a);
	return dst;
}

OS_CHAR * OS::Utils::numToStr(OS_CHAR * dst, OS_INT64 a)
{
	OS_SNPRINTF(dst, sizeof(OS_CHAR)*63, OS_TEXT("%li"), (long int)a);
	return dst;
}

OS_CHAR * OS::Utils::numToStr(OS_CHAR * dst, float a, int precision)
{
	return numToStr(dst, (double)a, precision);
}

OS_CHAR * OS::Utils::numToStr(OS_CHAR * dst, double a, int precision)
{
	if(precision <= 0) {
		if(precision < 0) {
			OS_FLOAT p = (OS_FLOAT)10.0;
			for(int i = -precision-1; i > 0; i--){
				p *= (OS_FLOAT)10.0;
			}
			a = ::floor(a / p + (OS_FLOAT)0.5) * p;
		}
		OS_SNPRINTF(dst, sizeof(OS_CHAR)*127, OS_TEXT("%.f"), a);
		return dst;
	}
	if(precision == OS_AUTO_PRECISION){
		/* %G already handles removing trailing zeros from the fractional part, yay */ 
#if 1
		OS_SNPRINTF(dst, sizeof(OS_CHAR)*127, OS_TEXT("%G"), a);
#else
		OS_SNPRINTF(dst, sizeof(OS_CHAR)*127, OS_TEXT("%.*G"), 17, a);
#endif
		return dst;
	}
	int n = OS_SNPRINTF(dst, sizeof(OS_CHAR)*127, OS_TEXT("%.*f"), precision, a);
	OS_ASSERT(n >= 1 && !OS_STRSTR(dst, OS_TEXT(".")) || dst[n-1] != '0'); (void)n;
	return dst;
}

OS_INT OS::Utils::strToInt(const OS_CHAR * str)
{
	return (OS_INT)strToFloat(str);
}

OS_FLOAT OS::Utils::strToFloat(const OS_CHAR* str)
{
	OS_FLOAT fval;
	if(parseFloat(str, fval) && (!*str || (*str==OS_TEXT('f') && !str[1]))){
		return fval;
	}
	return 0;
}

#define OS_STR_HASH_START_VALUE 5381
#define OS_ADD_STR_HASH_VALUE hash = ((hash << 5) + hash) + *buf

#define OS_STR_HASH_LIMIT_SHIFT 5

int OS::Utils::keyToHash(const void * p_buf, int size)
{
	OS_ASSERT(size >= 0);
	int step = (size >> OS_STR_HASH_LIMIT_SHIFT) + 1;
	const OS_BYTE * buf = (const OS_BYTE*)p_buf;
	const OS_BYTE * end = buf + size;
	int hash = OS_STR_HASH_START_VALUE;
	for(; buf < end; buf += step){
		OS_ADD_STR_HASH_VALUE;
	}
	return hash;
}

int OS::Utils::keyToHash(const void * buf1, int size1, const void * buf2, int size2)
{
	OS_ASSERT(size1 >= 0 && size2 >= 0);
	if(size2 > 0){
		int size = size1 + size2;
		int step = (size >> OS_STR_HASH_LIMIT_SHIFT) + 1;
		const OS_BYTE * buf = (const OS_BYTE*)buf1;
		const OS_BYTE * end = buf + size1;
		int hash = OS_STR_HASH_START_VALUE;
		for(; buf < end; buf += step){
			OS_ADD_STR_HASH_VALUE;
		}
		buf = (const OS_BYTE*)buf2 + (buf - end);
		end = buf + size2;
		for(; buf < end; buf += step){
			OS_ADD_STR_HASH_VALUE;
		}
		return hash;
	}
	return keyToHash(buf1, size1);
}

int OS::Utils::cmp(const void * buf1, int len1, const void * buf2, int len2)
{
	int len = len1 < len2 ? len1 : len2;
	int cmp = OS_MEMCMP(buf1, buf2, len);
	return cmp ? cmp : len1 - len2;
}

// =====================================================================
// =====================================================================
// =====================================================================

OS::Core::String::String(OS * os)
{
	string = os->core->newStringValue((void*)NULL, 0);
	string->external_ref_count++;
#ifdef OS_DEBUG
	this->str = string->toChar();
#endif
}

OS::Core::String::String(GCStringValue * s)
{
	string = s;
	string->external_ref_count++;
#ifdef OS_DEBUG
	this->str = string->toChar();
#endif
}

OS::Core::String::String(const String& s)
{
	string = s.string;
	string->external_ref_count++;
#ifdef OS_DEBUG
	this->str = string->toChar();
#endif
}

OS::Core::String::String(OS * os, const String& a, const String& b)
{
	string = os->core->newStringValue(a, b);
	string->external_ref_count++;
#ifdef OS_DEBUG
	this->str = string->toChar();
#endif
}

OS::Core::String::String(OS * os, const OS_CHAR * str)
{
	string = os->core->newStringValue(str);
	string->external_ref_count++;
#ifdef OS_DEBUG
	this->str = string->toChar();
#endif
}

OS::Core::String::String(OS * os, const OS_CHAR * str, int len)
{
	string = os->core->newStringValue(str, len);
	string->external_ref_count++;
#ifdef OS_DEBUG
	this->str = string->toChar();
#endif
}

OS::Core::String::String(OS * os, const OS_CHAR * str, int len, const OS_CHAR * str2, int len2)
{
	string = os->core->newStringValue(str, len, str2, len2);
	string->external_ref_count++;
#ifdef OS_DEBUG
	this->str = string->toChar();
#endif
}

OS::Core::String::String(OS * os, const OS_CHAR * str, int len, bool trim_left, bool trim_right)
{
	string = os->core->newStringValue(str, len, trim_left, trim_right);
	string->external_ref_count++;
#ifdef OS_DEBUG
	this->str = string->toChar();
#endif
}

OS::Core::String::String(OS * os, const void * buf, int size)
{
	string = os->core->newStringValue(buf, size);
	string->external_ref_count++;
#ifdef OS_DEBUG
	this->str = string->toChar();
#endif
}

OS::Core::String::String(OS * os, const void * buf1, int size1, const void * buf2, int size2)
{
	string = os->core->newStringValue(buf1, size1, buf2, size2);
	string->external_ref_count++;
#ifdef OS_DEBUG
	this->str = string->toChar();
#endif
}

OS::Core::String::String(OS * os, const void * buf1, int size1, const void * buf2, int size2, const void * buf3, int size3)
{
	string = os->core->newStringValue(buf1, size1, buf2, size2, buf3, size3);
	string->external_ref_count++;
#ifdef OS_DEBUG
	this->str = string->toChar();
#endif
}

OS::Core::String::String(OS * os, OS_INT value)
{
	string = os->core->newStringValue(value);
	string->external_ref_count++;
#ifdef OS_DEBUG
	this->str = string->toChar();
#endif
}

OS::Core::String::String(OS * os, OS_FLOAT value, int precision)
{
	string = os->core->newStringValue(value, precision);
	string->external_ref_count++;
#ifdef OS_DEBUG
	this->str = string->toChar();
#endif
}

OS::Core::String::~String()
{
	if(string){ // can be cleared by OS::~String
		OS_ASSERT(string->external_ref_count > 0);
		string->external_ref_count--;
		if(string->gc_color == GC_WHITE){
			string->gc_color = GC_BLACK;
		}
	}
}

struct OS_VaListDtor
{
	va_list * va;

	OS_VaListDtor(va_list * p_va){ va = p_va; }
	~OS_VaListDtor(){ va_end(*va); }
};

OS::Core::String OS::Core::String::format(OS * allocator, int temp_buf_len, const OS_CHAR * fmt, ...)
{
	va_list va;
	va_start(va, fmt);
	OS_VaListDtor va_dtor(&va);
	return String(allocator->core->newStringValueVa(temp_buf_len, fmt, va));
}

OS::Core::String OS::Core::String::formatVa(OS * allocator, int temp_buf_len, const OS_CHAR * fmt, va_list va)
{
	return String(allocator->core->newStringValueVa(temp_buf_len, fmt, va));
}

OS::Core::String OS::Core::String::format(OS * allocator, const OS_CHAR * fmt, ...)
{
	va_list va;
	va_start(va, fmt);
	OS_VaListDtor va_dtor(&va);
	return String(allocator->core->newStringValueVa(OS_DEF_FMT_BUF_LEN, fmt, va));
}

OS::Core::String OS::Core::String::formatVa(OS * allocator, const OS_CHAR * fmt, va_list va)
{
	return String(allocator->core->newStringValueVa(OS_DEF_FMT_BUF_LEN, fmt, va));
}

OS::Core::String& OS::Core::String::operator=(const String& b)
{
	if(string != b.string){
		OS_ASSERT(string->external_ref_count > 0);
		string->external_ref_count--;
		if(string->gc_color == GC_WHITE){
			string->gc_color = GC_BLACK;
		}
		string = b.string;
		string->external_ref_count++;
#ifdef OS_DEBUG
		this->str = string->toChar();
#endif
	}
	return *this;
}

bool OS::Core::String::operator==(const String& b) const
{
	return string == b.string;
}

bool OS::Core::String::operator==(const OS_CHAR * b) const
{
	return cmp(b) == 0;
}

bool OS::Core::String::operator==(GCStringValue * b) const
{
	return string == b;
}

bool OS::Core::String::operator!=(const String& b) const
{
	return string != b.string;
}

bool OS::Core::String::operator!=(const OS_CHAR * b) const
{
	return cmp(b) != 0;
}

bool OS::Core::String::operator!=(GCStringValue * b) const
{
	return string != b;
}

bool OS::Core::String::operator<=(const String& b) const
{
	return cmp(b) <= 0;
}

bool OS::Core::String::operator<=(const OS_CHAR * b) const
{
	return cmp(b) <= 0;
}

bool OS::Core::String::operator<(const String& b) const
{
	return cmp(b) < 0;
}

bool OS::Core::String::operator<(const OS_CHAR * b) const
{
	return cmp(b) < 0;
}

bool OS::Core::String::operator>=(const String& b) const
{
	return cmp(b) >= 0;
}

bool OS::Core::String::operator>=(const OS_CHAR * b) const
{
	return cmp(b) >= 0;
}

bool OS::Core::String::operator>(const String& b) const
{
	return cmp(b) > 0;
}

bool OS::Core::String::operator>(const OS_CHAR * b) const
{
	return cmp(b) > 0;
}

int OS::Core::String::cmp(const String& b) const
{
	if(string == b.string){
		return 0;
	}
	return Utils::cmp(string->toChar(), string->data_size, b.string->toChar(), b.string->data_size);
}

int OS::Core::String::cmp(const OS_CHAR * b) const
{
	return Utils::cmp(string->toChar(), string->data_size, b, OS_STRLEN(b));
}

int OS::Core::String::getHash() const
{
	return string->hash;
}

OS_NUMBER OS::Core::String::toNumber() const
{
	return string->toNumber();
}

// =====================================================================

OS::Core::Buffer::Buffer(OS * p_allocator): MemStreamWriter(p_allocator)
{
	cacheStr = NULL;
}

OS::Core::Buffer::Buffer(const Buffer& buf): MemStreamWriter(buf.allocator)
{
	cacheStr = NULL;
	append(buf);
}

OS::Core::Buffer::~Buffer()
{
	freeCacheStr();
}

OS::Core::Buffer& OS::Core::Buffer::append(OS_CHAR c)
{
	return append((const void*)&c, sizeof(c));
}

OS::Core::Buffer& OS::Core::Buffer::append(const OS_CHAR * str)
{
	return append(str, OS_STRLEN(str));
}

OS::Core::Buffer& OS::Core::Buffer::append(const OS_CHAR * str, int len)
{
	return append((const void*)str, len * sizeof(OS_CHAR));
}

OS::Core::Buffer& OS::Core::Buffer::append(const void * buf, int size)
{
	freeCacheStr();
	// allocator->vectorReserveCapacity(*this, count + size OS_DBG_FILEPOS);
	writeBytes(buf, size);
	return *this;
}

OS::Core::Buffer& OS::Core::Buffer::append(const Core::String& str)
{
	return append((void*)str.toChar(), str.getDataSize());
}

OS::Core::Buffer& OS::Core::Buffer::append(const Buffer& buf)
{
	return append((void*)buf.buffer.buf, buf.buffer.count);
}

OS::Core::Buffer& OS::Core::Buffer::operator+=(const Core::String& str)
{
	return append(str);
}

OS::Core::Buffer& OS::Core::Buffer::operator+=(const OS_CHAR * str)
{
	return append(str);
}

OS::Core::Buffer::operator OS::Core::String()
{
	return toString();
}

OS::Core::String OS::Core::Buffer::toString()
{
	return Core::String(toGCStringValue());
}

OS::String OS::Core::Buffer::toStringOS()
{
	return OS::String(allocator, toGCStringValue());
}

void OS::Core::Buffer::freeCacheStr()
{
	if(cacheStr){
		cacheStr->external_ref_count--;
		cacheStr = NULL;
	}
}

OS::Core::GCStringValue * OS::Core::Buffer::toGCStringValue()
{
	if(!cacheStr){
		cacheStr = allocator->core->newStringValue((void*)buffer.buf, buffer.count);
		cacheStr->external_ref_count++;
	}
	return cacheStr;
}

// =====================================================================

OS::String::String(OS * allocator): super(allocator)
{
	this->allocator = allocator->retain();
}

OS::String::String(const String& str): super(str)
{
	allocator = str.allocator->retain();
}

OS::String::String(OS * allocator, Core::GCStringValue * str): super(str)
{
	this->allocator = allocator->retain();
}

OS::String::String(OS * allocator, const Core::String& str): super(str)
{
	this->allocator = allocator->retain();
}

OS::String::String(OS * allocator, const OS_CHAR * str): super(allocator, str)
{
	this->allocator = allocator->retain();
}

OS::String::String(OS * allocator, const OS_CHAR * str1, int len1, const OS_CHAR * str2, int len2): super(allocator, str1, len1, str2, len2)
{
	this->allocator = allocator->retain();
}

OS::String::String(OS * allocator, const OS_CHAR * str, int len): super(allocator, str, len)
{
	this->allocator = allocator->retain();
}

OS::String::String(OS * allocator, const OS_CHAR * str, int len, bool trim_left, bool trim_right): super(allocator, str, len, trim_left, trim_right)
{
	this->allocator = allocator->retain();
}

OS::String::String(OS * allocator, const void * buf, int size): super(allocator, buf, size)
{
	this->allocator = allocator->retain();
}

OS::String::String(OS * allocator, const void * buf1, int size1, const void * buf2, int size2): super(allocator, buf1, size1, buf2, size2)
{
	this->allocator = allocator->retain();
}

OS::String::String(OS * allocator, OS_INT value): super(allocator, value)
{
	this->allocator = allocator->retain();
}

OS::String::String(OS * allocator, OS_FLOAT value, int precision): super(allocator, value, precision)
{
	this->allocator = allocator->retain();
}

OS::String::~String()
{
	OS_ASSERT(string->external_ref_count > 0);
	string->external_ref_count--;
	if(string->gc_color == Core::GC_WHITE){
		string->gc_color = Core::GC_BLACK;
	}
	string = NULL;
	allocator->release();
}

OS::String& OS::String::operator=(const Core::String& str)
{
	if(string != str.string){
		OS_ASSERT(string->external_ref_count > 0);
		string->external_ref_count--;
		if(string->gc_color == Core::GC_WHITE){
			string->gc_color = Core::GC_BLACK;
		}
		string = str.string;
		string->external_ref_count++;
#ifdef OS_DEBUG
		this->str = string->toChar();
#endif
	}
	return *this;
}

OS::String& OS::String::operator=(const String& str)
{
	OS_ASSERT(allocator == str.allocator);
	if(string != str.string){
		OS_ASSERT(string->external_ref_count > 0);
		string->external_ref_count--;
		if(string->gc_color == Core::GC_WHITE){
			string->gc_color = Core::GC_BLACK;
		}
		string = str.string;
		string->external_ref_count++;
#ifdef OS_DEBUG
		this->str = string->toChar();
#endif
	}
	return *this;
}

OS::String& OS::String::operator+=(const String& str)
{
	return *this = allocator->core->newStringValue(*this, str);
}

OS::String& OS::String::operator+=(const OS_CHAR * str)
{
	return *this = allocator->core->newStringValue(toChar(), getDataSize(), str, OS_STRLEN(str)*sizeof(OS_CHAR));
}

OS::String OS::String::operator+(const String& str) const
{
	return String(allocator, allocator->core->newStringValue(*this, str));
}

OS::String OS::String::operator+(const OS_CHAR * str) const
{
	return String(allocator, allocator->core->newStringValue(toChar(), getDataSize(), str, OS_STRLEN(str)*sizeof(OS_CHAR)));
}

OS::String OS::String::trim(bool trim_left, bool trim_right) const
{
	return String(allocator, allocator->core->newStringValue(*this, trim_left, trim_right));
}

// =====================================================================
// =====================================================================
// =====================================================================

const OS_CHAR * OS::Core::Tokenizer::getTokenTypeName(TokenType token_type)
{
	switch(token_type){
	case NOTHING: return OS_TEXT("NOTHING");

	case BEGIN_CODE_BLOCK:    return OS_TEXT("BEGIN_CODE_BLOCK");
	case END_CODE_BLOCK:      return OS_TEXT("END_CODE_BLOCK");

	case BEGIN_BRACKET_BLOCK: return OS_TEXT("BEGIN_BRACKET_BLOCK");
	case END_BRACKET_BLOCK:   return OS_TEXT("END_BRACKET_BLOCK");

	case BEGIN_ARRAY_BLOCK:   return OS_TEXT("BEGIN_ARRAY_BLOCK");
	case END_ARRAY_BLOCK:     return OS_TEXT("END_ARRAY_BLOCK");

	case CODE_SEPARATOR:      return OS_TEXT("CODE_SEPARATOR");
	case PARAM_SEPARATOR:     return OS_TEXT("PARAM_SEPARATOR");

	case COMMENT_LINE:        return OS_TEXT("COMMENT_LINE");
	case COMMENT_MULTI_LINE:  return OS_TEXT("COMMENT_MULTI_LINE");

	case NAME:      return OS_TEXT("NAME");
	case STRING:    return OS_TEXT("STRING");

	case NUMBER:   return OS_TEXT("NUMBER");

	case OPERATOR:        return OS_TEXT("OPERATOR");
	case BINARY_OPERATOR: return OS_TEXT("BINARY_OPERATOR");

	case OPERATOR_INDIRECT: return OS_TEXT("OPERATOR_INDIRECT");
	case OPERATOR_CONCAT:  return OS_TEXT("OPERATOR_CONCAT");

	case OPERATOR_THIS: return OS_TEXT("OPERATOR_THIS");
	case OPERATOR_LOGIC_AND:  return OS_TEXT("OPERATOR_LOGIC_AND");
	case OPERATOR_LOGIC_OR:   return OS_TEXT("OPERATOR_LOGIC_OR");
	case OPERATOR_LOGIC_PTR_EQ:   return OS_TEXT("OPERATOR_LOGIC_PTR_EQ");
	case OPERATOR_LOGIC_PTR_NE:   return OS_TEXT("OPERATOR_LOGIC_PTR_NE");
	case OPERATOR_LOGIC_EQ:   return OS_TEXT("OPERATOR_LOGIC_EQ");
	case OPERATOR_LOGIC_NE:   return OS_TEXT("OPERATOR_LOGIC_NE");
	case OPERATOR_LOGIC_GE:   return OS_TEXT("OPERATOR_LOGIC_GE");
	case OPERATOR_LOGIC_LE:   return OS_TEXT("OPERATOR_LOGIC_LE");
	case OPERATOR_LOGIC_GREATER:  return OS_TEXT("OPERATOR_LOGIC_GREATER");
	case OPERATOR_LOGIC_LESS: return OS_TEXT("OPERATOR_LOGIC_LESS");
	case OPERATOR_LOGIC_NOT:  return OS_TEXT("OPERATOR_LOGIC_NOT");

	case OPERATOR_INC:  return OS_TEXT("OPERATOR_INC");
	case OPERATOR_DEC:  return OS_TEXT("OPERATOR_DEC");

	case OPERATOR_QUESTION: return OS_TEXT("OPERATOR_QUESTION");
	case OPERATOR_COLON:    return OS_TEXT("OPERATOR_COLON");

	case OPERATOR_BIT_AND:  return OS_TEXT("OPERATOR_BIT_AND");
	case OPERATOR_BIT_OR:   return OS_TEXT("OPERATOR_BIT_OR");
	case OPERATOR_BIT_XOR:  return OS_TEXT("OPERATOR_BIT_XOR");
	case OPERATOR_BIT_NOT:  return OS_TEXT("OPERATOR_BIT_NOT");
	case OPERATOR_COMPARE:  return OS_TEXT("OPERATOR_COMPARE");
	case OPERATOR_ADD:      return OS_TEXT("OPERATOR_ADD");
	case OPERATOR_SUB:      return OS_TEXT("OPERATOR_SUB");
	case OPERATOR_MUL:      return OS_TEXT("OPERATOR_MUL");
	case OPERATOR_DIV:      return OS_TEXT("OPERATOR_DIV");
	case OPERATOR_MOD:      return OS_TEXT("OPERATOR_MOD");
	case OPERATOR_LSHIFT:  return OS_TEXT("OPERATOR_LSHIFT");
	case OPERATOR_RSHIFT:  return OS_TEXT("OPERATOR_RSHIFT");
	case OPERATOR_POW:      return OS_TEXT("OPERATOR_POW");

	case OPERATOR_BIT_AND_ASSIGN: return OS_TEXT("OPERATOR_BIT_AND_ASSIGN");
	case OPERATOR_BIT_OR_ASSIGN:  return OS_TEXT("OPERATOR_BIT_OR_ASSIGN");
	case OPERATOR_BIT_XOR_ASSIGN: return OS_TEXT("OPERATOR_BIT_XOR_ASSIGN");
	case OPERATOR_BIT_NOT_ASSIGN: return OS_TEXT("OPERATOR_BIT_NOT_ASSIGN");
	case OPERATOR_ADD_ASSIGN:     return OS_TEXT("OPERATOR_ADD_ASSIGN");
	case OPERATOR_SUB_ASSIGN:     return OS_TEXT("OPERATOR_SUB_ASSIGN");
	case OPERATOR_MUL_ASSIGN:     return OS_TEXT("OPERATOR_MUL_ASSIGN");
	case OPERATOR_DIV_ASSIGN:     return OS_TEXT("OPERATOR_DIV_ASSIGN");
	case OPERATOR_MOD_ASSIGN:     return OS_TEXT("OPERATOR_MOD_ASSIGN");
	case OPERATOR_LSHIFT_ASSIGN: return OS_TEXT("OPERATOR_LSHIFT_ASSIGN");
	case OPERATOR_RSHIFT_ASSIGN: return OS_TEXT("OPERATOR_RSHIFT_ASSIGN");
	case OPERATOR_POW_ASSIGN:     return OS_TEXT("OPERATOR_POW_ASSIGN");

	case OPERATOR_ASSIGN: return OS_TEXT("OPERATOR_ASSIGN");

	case OPERATOR_RESERVED: return OS_TEXT("OPERATOR_RESERVED");

	case OPERATOR_END:  return OS_TEXT("OPERATOR_END");

	case ERROR_TOKEN:   return OS_TEXT("ERROR_TOKEN");
	}
	return OS_TEXT("UNKNOWN_TOKENTYPE");
}

OS::Core::Tokenizer::TokenData::TokenData(TextData * p_text_data, const String& p_str, TokenType p_type, int p_line, int p_pos): str(p_str)
{
	text_data = p_text_data->retain();
	ref_count = 1;
	type = p_type;
	line = p_line;
	pos = p_pos;
}

OS * OS::Core::Tokenizer::TokenData::getAllocator() const
{
	return text_data->allocator;
}

OS::Core::Tokenizer::TokenData::~TokenData()
{
	OS_ASSERT(ref_count == 0);
	text_data->release();
}

OS::Core::Tokenizer::TokenData * OS::Core::Tokenizer::TokenData::retain()
{
	ref_count++;
	return this;
}

void OS::Core::Tokenizer::TokenData::release()
{
	if(--ref_count <= 0){
		OS_ASSERT(ref_count == 0);
		OS * allocator = getAllocator();
		this->~TokenData();
		allocator->free(this);
	}
}

OS_FLOAT OS::Core::Tokenizer::TokenData::getFloat() const
{
	return float_value;
}

bool OS::Core::Tokenizer::TokenData::isTypeOf(TokenType token_type) const
{
	if(type == token_type){
		return true;
	}
	if(token_type == OS::Core::Tokenizer::SEPARATOR){
		switch(type)
		{
		case OS::Core::Tokenizer::BEGIN_CODE_BLOCK:  // {
		case OS::Core::Tokenizer::END_CODE_BLOCK:    // }

		case OS::Core::Tokenizer::BEGIN_BRACKET_BLOCK:  // (
		case OS::Core::Tokenizer::END_BRACKET_BLOCK:    // )

		case OS::Core::Tokenizer::BEGIN_ARRAY_BLOCK:  // [
		case OS::Core::Tokenizer::END_ARRAY_BLOCK:    // ]

		case OS::Core::Tokenizer::CODE_SEPARATOR:     // ;
		case OS::Core::Tokenizer::PARAM_SEPARATOR:    // ,
			return true;
		}
		return false;
	}
	if(token_type == BINARY_OPERATOR){
		switch(type)
		{
		case OS::Core::Tokenizer::PARAM_SEPARATOR:
		case OS::Core::Tokenizer::OPERATOR_QUESTION:

		case OS::Core::Tokenizer::OPERATOR_INDIRECT:  // .
		case OS::Core::Tokenizer::OPERATOR_CONCAT:	// ..
		case OS::Core::Tokenizer::OPERATOR_IN:		// in
		case OS::Core::Tokenizer::OPERATOR_ISPROTOTYPEOF:
		case OS::Core::Tokenizer::OPERATOR_IS:

		case OS::Core::Tokenizer::OPERATOR_LOGIC_AND: // &&
		case OS::Core::Tokenizer::OPERATOR_LOGIC_OR:  // ||
		case OS::Core::Tokenizer::OPERATOR_LOGIC_PTR_EQ:  // ===
		case OS::Core::Tokenizer::OPERATOR_LOGIC_PTR_NE:  // !==
		case OS::Core::Tokenizer::OPERATOR_LOGIC_EQ:  // ==
		case OS::Core::Tokenizer::OPERATOR_LOGIC_NE:  // !=
		case OS::Core::Tokenizer::OPERATOR_LOGIC_GE:  // >=
		case OS::Core::Tokenizer::OPERATOR_LOGIC_LE:  // <=
		case OS::Core::Tokenizer::OPERATOR_LOGIC_GREATER: // >
		case OS::Core::Tokenizer::OPERATOR_LOGIC_LESS:    // <

		case OS::Core::Tokenizer::OPERATOR_BIT_AND: // &
		case OS::Core::Tokenizer::OPERATOR_BIT_OR:  // |
		case OS::Core::Tokenizer::OPERATOR_BIT_XOR: // ^
		case OS::Core::Tokenizer::OPERATOR_BIT_NOT: // ~
		case OS::Core::Tokenizer::OPERATOR_COMPARE: // <=>
		case OS::Core::Tokenizer::OPERATOR_ADD: // +
		case OS::Core::Tokenizer::OPERATOR_SUB: // -
		case OS::Core::Tokenizer::OPERATOR_MUL: // *
		case OS::Core::Tokenizer::OPERATOR_DIV: // /
		case OS::Core::Tokenizer::OPERATOR_MOD: // %
		case OS::Core::Tokenizer::OPERATOR_LSHIFT: // <<
		case OS::Core::Tokenizer::OPERATOR_RSHIFT: // >>
		case OS::Core::Tokenizer::OPERATOR_POW: // **

		case OS::Core::Tokenizer::OPERATOR_BIT_AND_ASSIGN: // &=
		case OS::Core::Tokenizer::OPERATOR_BIT_OR_ASSIGN:  // |=
		case OS::Core::Tokenizer::OPERATOR_BIT_XOR_ASSIGN: // ^=
		case OS::Core::Tokenizer::OPERATOR_BIT_NOT_ASSIGN: // ~=
		case OS::Core::Tokenizer::OPERATOR_ADD_ASSIGN: // +=
		case OS::Core::Tokenizer::OPERATOR_SUB_ASSIGN: // -=
		case OS::Core::Tokenizer::OPERATOR_MUL_ASSIGN: // *=
		case OS::Core::Tokenizer::OPERATOR_DIV_ASSIGN: // /=
		case OS::Core::Tokenizer::OPERATOR_MOD_ASSIGN: // %=
		case OS::Core::Tokenizer::OPERATOR_LSHIFT_ASSIGN: // <<=
		case OS::Core::Tokenizer::OPERATOR_RSHIFT_ASSIGN: // >>=
		case OS::Core::Tokenizer::OPERATOR_POW_ASSIGN: // **=

		case OS::Core::Tokenizer::OPERATOR_ASSIGN: // =
			return true;
		}
		return false;
	}
	return false;
}


bool OS::Core::Tokenizer::operator_initialized = false;
OS::Core::Tokenizer::OperatorDesc OS::Core::Tokenizer::operator_desc[] = 
{
	{ OPERATOR_INDIRECT, OS_TEXT(".") },
	{ OPERATOR_CONCAT, OS_TEXT("..") },
	{ REST_ARGUMENTS, OS_TEXT("...") },

	{ OPERATOR_RESERVED, OS_TEXT("->") },
	{ OPERATOR_RESERVED, OS_TEXT("::") },

	{ OPERATOR_THIS, OS_TEXT("@") },

	{ OPERATOR_LOGIC_AND, OS_TEXT("&&") },
	{ OPERATOR_LOGIC_OR,  OS_TEXT("||") },
	{ OPERATOR_LOGIC_PTR_EQ, OS_TEXT("===") },
	{ OPERATOR_LOGIC_PTR_NE, OS_TEXT("!==") },
	{ OPERATOR_LOGIC_EQ,  OS_TEXT("==") },
	{ OPERATOR_LOGIC_NE,  OS_TEXT("!=") },
	{ OPERATOR_LOGIC_GE,  OS_TEXT(">=") },
	{ OPERATOR_LOGIC_LE,  OS_TEXT("<=") },
	{ OPERATOR_LOGIC_GREATER, OS_TEXT(">") },
	{ OPERATOR_LOGIC_LESS,    OS_TEXT("<") },
	{ OPERATOR_LOGIC_NOT,     OS_TEXT("!") },

	{ OPERATOR_INC,     OS_TEXT("++") },
	{ OPERATOR_DEC,     OS_TEXT("--") },

	{ OPERATOR_QUESTION,  OS_TEXT("?") },
	{ OPERATOR_COLON,     OS_TEXT(":") },

	{ OPERATOR_LENGTH,    OS_TEXT("#") },

	{ OPERATOR_BIT_AND, OS_TEXT("&") },
	{ OPERATOR_BIT_OR,  OS_TEXT("|") },
	{ OPERATOR_BIT_XOR, OS_TEXT("^") },
	{ OPERATOR_BIT_NOT, OS_TEXT("~") },
	{ OPERATOR_CONCAT, OS_TEXT("..") },
	{ OPERATOR_COMPARE, OS_TEXT("<=>") },
	{ OPERATOR_ADD, OS_TEXT("+") },
	{ OPERATOR_SUB, OS_TEXT("-") },
	{ OPERATOR_MUL, OS_TEXT("*") },
	{ OPERATOR_DIV, OS_TEXT("/") },
	{ OPERATOR_MOD, OS_TEXT("%") },
	{ OPERATOR_LSHIFT, OS_TEXT("<<") },
	{ OPERATOR_RSHIFT, OS_TEXT(">>") },
	{ OPERATOR_POW, OS_TEXT("**") },

	{ OPERATOR_BIT_AND_ASSIGN, OS_TEXT("&=") },
	{ OPERATOR_BIT_OR_ASSIGN,  OS_TEXT("|=") },
	{ OPERATOR_BIT_XOR_ASSIGN, OS_TEXT("^=") },
	{ OPERATOR_BIT_NOT_ASSIGN, OS_TEXT("~=") },
	{ OPERATOR_ADD_ASSIGN, OS_TEXT("+=") },
	{ OPERATOR_SUB_ASSIGN, OS_TEXT("-=") },
	{ OPERATOR_MUL_ASSIGN, OS_TEXT("*=") },
	{ OPERATOR_DIV_ASSIGN, OS_TEXT("/=") },
	{ OPERATOR_MOD_ASSIGN, OS_TEXT("%=") },
	{ OPERATOR_LSHIFT_ASSIGN, OS_TEXT("<<=") },
	{ OPERATOR_RSHIFT_ASSIGN, OS_TEXT(">>=") },
	{ OPERATOR_POW_ASSIGN, OS_TEXT("**=") },

	{ OPERATOR_ASSIGN, OS_TEXT("=") },

	{ BEGIN_CODE_BLOCK, OS_TEXT("{") },
	{ END_CODE_BLOCK, OS_TEXT("}") },

	{ BEGIN_BRACKET_BLOCK, OS_TEXT("(") },
	{ END_BRACKET_BLOCK, OS_TEXT(")") },

	{ BEGIN_ARRAY_BLOCK, OS_TEXT("[") },
	{ END_ARRAY_BLOCK, OS_TEXT("]") },

	{ CODE_SEPARATOR, OS_TEXT(";") },
	{ PARAM_SEPARATOR, OS_TEXT(",") }
};

const int OS::Core::Tokenizer::operator_count = sizeof(operator_desc) / sizeof(operator_desc[0]);

int OS::Core::Tokenizer::compareOperatorDesc(const void * a, const void * b) 
{
	const OperatorDesc * op0 = (const OperatorDesc*)a;
	const OperatorDesc * op1 = (const OperatorDesc*)b;
	return (int)OS_STRLEN(op1->name) - (int)OS_STRLEN(op0->name);
}

void OS::Core::Tokenizer::initOperatorsTable()
{
	if(!operator_initialized){
		::qsort(operator_desc, operator_count, sizeof(operator_desc[0]), Tokenizer::compareOperatorDesc);
		operator_initialized = true;
	}
}

OS::Core::Tokenizer::TextData::TextData(OS * p_allocator): filename(p_allocator)
{
	allocator = p_allocator;
	ref_count = 1;
}

OS::Core::Tokenizer::TextData::~TextData()
{
	OS_ASSERT(!ref_count);
}

OS::Core::Tokenizer::TextData * OS::Core::Tokenizer::TextData::retain()
{
	ref_count++;
	return this;
}

void OS::Core::Tokenizer::TextData::release()
{
	if(--ref_count <= 0){
		OS_ASSERT(!ref_count);
		OS * allocator = this->allocator;
		allocator->vectorClear(lines);
		this->~TextData();
		allocator->free(this);
	}
}

OS::Core::Tokenizer::Tokenizer(OS * p_allocator)
{
	allocator = p_allocator;
	initOperatorsTable();
	settings.save_comments = false;
	error = ERROR_NOTHING;
	cur_line = 0;
	cur_pos = 0;

	text_data = new (allocator->malloc(sizeof(TextData) OS_DBG_FILEPOS)) TextData(allocator);
}

OS * OS::Core::Tokenizer::getAllocator()
{
	return allocator;
}

OS::Core::Tokenizer::~Tokenizer()
{
	OS * allocator = getAllocator();
	for(int i = 0; i < tokens.count; i++){
		TokenData * token = tokens[i];
		token->release();
	}
	allocator->vectorClear(tokens);
	text_data->release();
}

OS::Core::Tokenizer::TokenData * OS::Core::Tokenizer::removeToken(int i)
{
	TokenData * token = getToken(i);
	getAllocator()->vectorRemoveAtIndex(tokens, i);
	return token;
}

void OS::Core::Tokenizer::insertToken(int i, TokenData * token OS_DBG_FILEPOS_DECL)
{
	getAllocator()->vectorInsertAtIndex(tokens, i, token OS_DBG_FILEPOS_PARAM);
}

bool OS::Core::Tokenizer::parseText(const OS_CHAR * text, int len, const String& filename, OS_ESourceCodeType source_code_type, bool check_utf8_bom)
{
	OS_ASSERT(text_data->lines.count == 0);

	OS * allocator = getAllocator();

	text_data->filename = filename;

	const OS_CHAR * str = text;
	const OS_CHAR * str_end = str + len;
	while(str < str_end)
	{
#if 0
		const OS_CHAR * line_end = OS_STRCHR(str, OS_TEXT('\n'));
		if(line_end){
			allocator->vectorAddItem(text_data->lines, String(allocator, str, line_end - str, false, true) OS_DBG_FILEPOS);
			str = line_end+1;
		}else{
			allocator->vectorAddItem(text_data->lines, String(allocator, str, str_end - str, false, true) OS_DBG_FILEPOS);
			break;
		}
#else
		const OS_CHAR * line_end = str;
		for(; line_end < str_end && *line_end != OS_TEXT('\n'); line_end++);
		allocator->vectorAddItem(text_data->lines, String(allocator, str, line_end - str) OS_DBG_FILEPOS);
		str = line_end+1;
#endif
	}
	return parseLines(source_code_type, check_utf8_bom);
}

void OS::Core::Tokenizer::TokenData::setFloat(OS_FLOAT value)
{
	float_value = value;
}

OS::Core::Tokenizer::TokenData * OS::Core::Tokenizer::addToken(const String& str, TokenType type, int line, int pos OS_DBG_FILEPOS_DECL)
{
	OS * allocator = getAllocator();
	TokenData * token = new (allocator->malloc(sizeof(TokenData) OS_DBG_FILEPOS_PARAM)) TokenData(text_data, str, type, line, pos);
	allocator->vectorAddItem(tokens, token OS_DBG_FILEPOS);
	return token;
}

static bool isValidCharAfterNumber(const OS_CHAR * str)
{
	return !*str || OS_IS_SPACE(*str) || OS_STRCHR(OS_TEXT("!@#$%^&*()-+={}[]\\|;:'\",<.>/?`~"), *str);
}

bool OS::Core::Tokenizer::parseFloat(const OS_CHAR *& str, OS_FLOAT& fval, bool parse_end_spaces)
{
	if(Utils::parseFloat(str, fval)){
		if(isValidCharAfterNumber(str)){
			if(parse_end_spaces){
				parseSpaces(str);
			}
			return true;
		}
		if(*str == OS_TEXT('f') && isValidCharAfterNumber(str+1)){
			str++;
			if(parse_end_spaces){
				parseSpaces(str);
			}
			return true;
		}
	}
	return false;
}

bool OS::Core::Tokenizer::parseLines(OS_ESourceCodeType source_code_type, bool check_utf8_bom)
{
	OS * allocator = getAllocator();
	cur_line = cur_pos = 0;
	bool template_enabled = source_code_type == OS_SOURCECODE_TEMPLATE;
	bool is_template = template_enabled;
	for(; cur_line < text_data->lines.count; cur_line++){
		// parse line
		const OS_CHAR * line_start = text_data->lines[cur_line].toChar();
		const OS_CHAR * str = line_start;

		cur_pos = 0;
		if(!cur_line){
			if(check_utf8_bom){
				OS_ASSERT(sizeof(OS_CHAR) == sizeof(char));
				if(str[0] == '\xef' && str[1] == '\xbb' && str[2] == '\xbf'){
					line_start += 3;
					str = line_start;
				}
			}
			if(source_code_type == OS_SOURCECODE_AUTO){
				if(str[0] == OS_TEXT('<') && str[1] == OS_TEXT('%')){
					source_code_type = OS_SOURCECODE_TEMPLATE;
					template_enabled = true;
					is_template = false;
					str += 2;
				}
			}
		}

		for(;;){
			if(template_enabled && is_template){
				Buffer s(allocator);
				for(;;){
					const OS_CHAR * line_pos = str;
					const OS_CHAR * open_os_tag = OS_STRSTR(str, OS_TEXT("<%"));
					if(open_os_tag){
						s.append(line_pos, open_os_tag - line_pos);
						if(s.getSize() > 0){
							addToken(s, OUTPUT_STRING, cur_line, str - line_start OS_DBG_FILEPOS);
						}
						str = open_os_tag + 2;
						is_template = false;

						if(str[0] == OS_TEXT('=')){
							addToken(String(allocator, str, 1), OUTPUT_NEXT_VALUE, cur_line, str - line_pos OS_DBG_FILEPOS);
							str++;
						}
						break;
					}
					s.append(line_pos);
					s.append(OS_TEXT("\n"));
					if(cur_line >= text_data->lines.count){
						if(s.getSize() > 0){
							addToken(s, OUTPUT_STRING, cur_line, str - line_pos OS_DBG_FILEPOS);
						}
						return true;
					}
					str = line_start = text_data->lines[++cur_line].toChar();
				}
			}

			// skip spaces
			parseSpaces(str);
			if(!*str){
				break;
			}

			if(template_enabled && !is_template && str[0] == OS_TEXT('%') && str[1] == OS_TEXT('>')){
				str += 2;
				is_template = true;
				continue;
			}

			if(*str == OS_TEXT('"') || *str == OS_TEXT('\'')){ // begin string
				Buffer s(allocator);
				OS_CHAR closeChar = *str;
				const OS_CHAR * token_start = str;
				for(str++; *str && *str != closeChar;){
					OS_CHAR c = *str++;
					if(c == OS_TEXT('\\')){
						switch(*str){
						case OS_TEXT('r'): c = OS_TEXT('\r'); str++; break;
						case OS_TEXT('n'): c = OS_TEXT('\n'); str++; break;
						case OS_TEXT('t'): c = OS_TEXT('\t'); str++; break;
						case OS_TEXT('\"'): c = OS_TEXT('\"'); str++; break;
						case OS_TEXT('\''): c = OS_TEXT('\''); str++; break;
						case OS_TEXT('\\'): c = OS_TEXT('\\'); str++; break;
							//case OS_TEXT('x'): 
						default:
							{
								OS_INT val;
								int maxVal = sizeof(OS_CHAR) == 2 ? 0xFFFF : 0xFF;

								if(*str == OS_TEXT('x') || *str == OS_TEXT('X')){ // parse hex
									str++;
									if(!parseSimpleHex(str, val)){
										cur_pos = str - line_start;
										error = ERROR_CONST_STRING_ESCAPE_CHAR;
										return false;
									}
								}else if(*str == OS_TEXT('0')){ // octal
									if(!parseSimpleOctal(str, val)){
										cur_pos = str - line_start;
										error = ERROR_CONST_STRING_ESCAPE_CHAR;
										return false;
									}
								}else if(*str >= OS_TEXT('1') && *str <= OS_TEXT('9')){
									if(!parseSimpleDec(str, val)){
										cur_pos = str - line_start;
										error = ERROR_CONST_STRING_ESCAPE_CHAR;
										return false;
									}
								}else{
									val = c;
								}
								c = (OS_CHAR)(val <= maxVal ? val : maxVal);
							}
							break;
						}
					}
					s.append(c);
				}
				if(*str != closeChar){
					cur_pos = str - line_start;
					error = ERROR_CONST_STRING;
					return false;
				}
				str++;
				addToken(s, STRING, cur_line, token_start - line_start OS_DBG_FILEPOS);
				continue;
			}

			if(*str == OS_TEXT('/')){
				if(str[1] == OS_TEXT('/')){ // begin line comment
					if(settings.save_comments){
						addToken(String(allocator, str), COMMENT_LINE, cur_line, str - line_start OS_DBG_FILEPOS);
					}
					break;
				}
				if(str[1] == OS_TEXT('*')){ // begin multi line comment
					Buffer comment(allocator);
					comment.append(str, 2);
					int startLine = cur_line;
					int startPos = str - line_start;
					for(str += 2;;){
						const OS_CHAR * end = OS_STRSTR(str, OS_TEXT("*/"));
						if(end){
							if(settings.save_comments){
								comment.append(str, (int)(end+2 - str));
								addToken(comment, COMMENT_MULTI_LINE, startLine, startPos OS_DBG_FILEPOS);
							}
							str = end + 2;
							break;
						}
						if(cur_line >= text_data->lines.count){
							error = ERROR_MULTI_LINE_COMMENT;
							cur_pos = str - line_start;
							return false;
						}
						if(settings.save_comments){
							comment.append(str);
							comment.append(OS_TEXT("\n")); // OS_TEXT("\r\n"));
						}
						str = line_start = text_data->lines[++cur_line].toChar();
					}
					continue;
				}
			}

			if(*str == OS_TEXT('_') || *str == OS_TEXT('$') // || *str == OS_TEXT('@') 
				|| (*str >= OS_TEXT('a') && *str <= OS_TEXT('z'))
				|| (*str >= OS_TEXT('A') && *str <= OS_TEXT('Z')) )
			{ // parse name
				const OS_CHAR * name_start = str;
				for(str++; *str; str++){
					if(*str == OS_TEXT('_') || *str == OS_TEXT('$') || *str == OS_TEXT('@')
						|| (*str >= OS_TEXT('a') && *str <= OS_TEXT('z'))
						|| (*str >= OS_TEXT('A') && *str <= OS_TEXT('Z'))
						|| (*str >= OS_TEXT('0') && *str <= OS_TEXT('9')) )
					{
						continue;
					}
					break;
				}
				String name = String(allocator, name_start, str - name_start);
				TokenType type = NAME;
				addToken(name, type, cur_line, name_start - line_start OS_DBG_FILEPOS);
				continue;
			}
			// parse operator
			if(0 && (*str == OS_TEXT('-') || *str == OS_TEXT('+')) && (str[1] >= OS_TEXT('0') && str[1] <= OS_TEXT('9'))){
				// int i = 0;
			}else{
				int i;
				for(i = 0; i < operator_count; i++){
					size_t len = OS_STRLEN(operator_desc[i].name);
					if(OS_STRNCMP(str, operator_desc[i].name, len) == 0){
						addToken(String(allocator, str, (int)len), operator_desc[i].type, cur_line, str - line_start OS_DBG_FILEPOS);
						str += len;
						break;
					}
				}
				if(i < operator_count){
					continue;
				}
			}
			{
				OS_FLOAT fval;
				const OS_CHAR * token_start = str;
				if(parseFloat(str, fval, true)){
					TokenData * token = addToken(String(allocator, token_start, str - token_start, false, true), NUMBER, cur_line, token_start - line_start OS_DBG_FILEPOS);
					token->setFloat(fval);
					continue;
				}
			}

			error = ERROR_SYNTAX;
			cur_pos = str - line_start;
			return false;
		}
	}
	// PrintTokens();
	return true;
}

// =====================================================================
// =====================================================================
// =====================================================================

OS::Core::Compiler::ExpressionList::ExpressionList(OS * p_allocator)
{
	allocator = p_allocator;
}

OS::Core::Compiler::ExpressionList::~ExpressionList()
{
	allocator->vectorDeleteItems(*this);
	allocator->vectorClear(*this);
}

bool OS::Core::Compiler::ExpressionList::isValue() const
{
	return count > 0 && buf[count-1]->isValue();
}

bool OS::Core::Compiler::ExpressionList::isClear() const
{
	return count <= 0 || buf[count-1]->isClear();
}

bool OS::Core::Compiler::ExpressionList::isWriteable() const
{
	return count > 0 && buf[count-1]->isWriteable();
}

OS::Core::Compiler::Expression * OS::Core::Compiler::ExpressionList::add(Expression * exp OS_DBG_FILEPOS_DECL)
{
	allocator->vectorAddItem(*this, exp OS_DBG_FILEPOS_PARAM);
	return exp;
}

OS::Core::Compiler::Expression * OS::Core::Compiler::ExpressionList::removeIndex(int i)
{
	Expression * exp = (*this)[i];
	allocator->vectorRemoveAtIndex(*this, i);
	return exp;
}

OS::Core::Compiler::Expression * OS::Core::Compiler::ExpressionList::removeLast()
{
	return removeIndex(count-1);
}

void OS::Core::Compiler::ExpressionList::swap(ExpressionList& list)
{
	OS_ASSERT(allocator == list.allocator);

	Expression ** save_buf = buf;
	int save_count = count;
	int save_capacity = capacity;

	buf = list.buf;
	count = list.count;
	capacity = list.capacity;

	list.buf = save_buf;
	list.count = save_count;
	list.capacity = save_capacity;
}

// =====================================================================

OS::Core::Compiler::LocalVarDesc::LocalVarDesc()
{
	up_count = 0;
	up_scope_count = 0;
	index = 0;
	type = LOCAL_GENERIC;
}

OS::Core::Compiler::Expression::Expression(ExpressionType p_type, TokenData * p_token): list(p_token->getAllocator())
{
	token = p_token->retain();
	type = p_type;
	ret_values = 0;
	active_locals = 0;
	slots.a = slots.b = slots.c = 0;
}

OS::Core::Compiler::Expression::Expression(ExpressionType p_type, TokenData * p_token, Expression * e1 OS_DBG_FILEPOS_DECL): list(p_token->getAllocator())
{
	token = p_token->retain();
	type = p_type;
	list.add(e1 OS_DBG_FILEPOS_PARAM);
	ret_values = 0;
	active_locals = 0;
	slots.a = slots.b = slots.c = 0;
}

OS::Core::Compiler::Expression::Expression(ExpressionType p_type, TokenData * p_token, Expression * e1, Expression * e2 OS_DBG_FILEPOS_DECL): list(p_token->getAllocator())
{
	token = p_token->retain();
	type = p_type;
	list.add(e1 OS_DBG_FILEPOS_PARAM);
	list.add(e2 OS_DBG_FILEPOS_PARAM);
	ret_values = 0;
	active_locals = 0;
	slots.a = slots.b = slots.c = 0;
}

OS::Core::Compiler::Expression::Expression(ExpressionType p_type, TokenData * p_token, Expression * e1, Expression * e2, Expression * e3 OS_DBG_FILEPOS_DECL): list(p_token->getAllocator())
{
	token = p_token->retain();
	type = p_type;
	list.add(e1 OS_DBG_FILEPOS_PARAM);
	list.add(e2 OS_DBG_FILEPOS_PARAM);
	list.add(e3 OS_DBG_FILEPOS_PARAM);
	ret_values = 0;
	active_locals = 0;
	slots.a = slots.b = slots.c = 0;
}


OS::Core::Compiler::Expression::~Expression()
{
	token->release();
}

bool OS::Core::Compiler::Expression::isConstValue() const
{
	switch(type){
	case EXP_TYPE_CONST_STRING:
	case EXP_TYPE_CONST_NUMBER:
	case EXP_TYPE_CONST_NULL:
	case EXP_TYPE_CONST_TRUE:
	case EXP_TYPE_CONST_FALSE:
		OS_ASSERT(ret_values == 1);
		return true;
	}
	return false;
}

bool OS::Core::Compiler::Expression::isValue() const
{
	return ret_values > 0;
}

bool OS::Core::Compiler::Expression::isClear() const
{
	return ret_values == 0;
}

bool OS::Core::Compiler::Expression::isWriteable() const
{
	switch(type){
	case EXP_TYPE_NAME:
	case EXP_TYPE_INDIRECT:
	case EXP_TYPE_CALL_DIM:
	case EXP_TYPE_CALL_METHOD:
		return true;

	case EXP_TYPE_PARAMS:
		for(int i = 0; i < list.count; i++){
			if(list[i]->type == EXP_TYPE_PARAMS || !list[i]->isWriteable()){
				return false;
			}
		}
		return true;
	}
	return false;
}

bool OS::Core::Compiler::Expression::isOperator() const
{
	return isBinaryOperator() || isUnaryOperator();
}

bool OS::Core::Compiler::Expression::isUnaryOperator() const
{
	switch(type){
	case EXP_TYPE_LOGIC_BOOL:	// !!
	case EXP_TYPE_LOGIC_NOT:	// !
	case EXP_TYPE_PLUS:			// +
	case EXP_TYPE_NEG:			// -
	case EXP_TYPE_LENGTH:		// #
	case EXP_TYPE_PRE_INC:		// ++
	case EXP_TYPE_PRE_DEC:		// --
	case EXP_TYPE_POST_INC:		// ++
	case EXP_TYPE_POST_DEC:		// --
	case EXP_TYPE_BIT_NOT:		// ~
		return true;
	}
	return false;
}

bool OS::Core::Compiler::Expression::isLogicOperator() const
{
	switch(type){
	case EXP_TYPE_LOGIC_BOOL:	// !!
	case EXP_TYPE_LOGIC_NOT:	// !

	case EXP_TYPE_LOGIC_AND: // &&
	case EXP_TYPE_LOGIC_OR:  // ||

	case EXP_TYPE_LOGIC_PTR_EQ:  // ===
	case EXP_TYPE_LOGIC_PTR_NE:  // !==
	case EXP_TYPE_LOGIC_EQ:  // ==
	case EXP_TYPE_LOGIC_NE:  // !=
	case EXP_TYPE_LOGIC_GE:  // >=
	case EXP_TYPE_LOGIC_LE:  // <=
	case EXP_TYPE_LOGIC_GREATER: // >
	case EXP_TYPE_LOGIC_LESS:    // <
		return true;
	}
	return false;
}

bool OS::Core::Compiler::Expression::isBinaryOperator() const
{
	switch(type){
	case EXP_TYPE_INDIRECT:

	case EXP_TYPE_ASSIGN:

	case EXP_TYPE_PARAMS:
	case EXP_TYPE_QUESTION:
	case EXP_TYPE_IN:
	case EXP_TYPE_ISPROTOTYPEOF:
	case EXP_TYPE_IS:
	case EXP_TYPE_CONCAT: // ..

	case EXP_TYPE_LOGIC_AND: // &&
	case EXP_TYPE_LOGIC_OR:  // ||

	case EXP_TYPE_LOGIC_PTR_EQ:  // ===
	case EXP_TYPE_LOGIC_PTR_NE:  // !==
	case EXP_TYPE_LOGIC_EQ:  // ==
	case EXP_TYPE_LOGIC_NE:  // !=
	case EXP_TYPE_LOGIC_GE:  // >=
	case EXP_TYPE_LOGIC_LE:  // <=
	case EXP_TYPE_LOGIC_GREATER: // >
	case EXP_TYPE_LOGIC_LESS:    // <

	case EXP_TYPE_BIT_AND: // &
	case EXP_TYPE_BIT_OR:  // |
	case EXP_TYPE_BIT_XOR: // ^

	case EXP_TYPE_BIT_AND_ASSIGN: // &=
	case EXP_TYPE_BIT_OR_ASSIGN:  // |=
	case EXP_TYPE_BIT_XOR_ASSIGN: // ^=
	case EXP_TYPE_BIT_NOT_ASSIGN: // ~=

	case EXP_TYPE_COMPARE:    // <=>
	case EXP_TYPE_ADD: // +
	case EXP_TYPE_SUB: // -
	case EXP_TYPE_MUL: // *
	case EXP_TYPE_DIV: // /
	case EXP_TYPE_MOD: // %
	case EXP_TYPE_LSHIFT: // <<
	case EXP_TYPE_RSHIFT: // >>
	case EXP_TYPE_POW: // **

	case EXP_TYPE_ADD_ASSIGN: // +=
	case EXP_TYPE_SUB_ASSIGN: // -=
	case EXP_TYPE_MUL_ASSIGN: // *=
	case EXP_TYPE_DIV_ASSIGN: // /=
	case EXP_TYPE_MOD_ASSIGN: // %=
	case EXP_TYPE_LSHIFT_ASSIGN: // <<=
	case EXP_TYPE_RSHIFT_ASSIGN: // >>=
	case EXP_TYPE_POW_ASSIGN: // **=
		return true;
	}
	return isAssignOperator();
}

bool OS::Core::Compiler::Expression::isAssignOperator() const
{
	switch(type){
	case EXP_TYPE_ASSIGN: // =

	case EXP_TYPE_BIT_AND_ASSIGN: // &=
	case EXP_TYPE_BIT_OR_ASSIGN:  // |=
	case EXP_TYPE_BIT_XOR_ASSIGN: // ^=
	case EXP_TYPE_BIT_NOT_ASSIGN: // ~=

	case EXP_TYPE_ADD_ASSIGN: // +=
	case EXP_TYPE_SUB_ASSIGN: // -=
	case EXP_TYPE_MUL_ASSIGN: // *=
	case EXP_TYPE_DIV_ASSIGN: // /=
	case EXP_TYPE_MOD_ASSIGN: // %=
	case EXP_TYPE_LSHIFT_ASSIGN: // <<=
	case EXP_TYPE_RSHIFT_ASSIGN: // >>=
	case EXP_TYPE_POW_ASSIGN: // **=
		return true;
	}
	return false;
}

OS::Core::String OS::Core::Compiler::Expression::getSlotStr(OS::Core::Compiler * compiler, Scope * scope, int slot_num, int up_count)
{
	// OS_ASSERT(slot_num);
	OS * allocator = compiler->allocator;
	for(; up_count > 0; up_count--){
		scope = scope->function->parent;
	}
	if(slot_num < 0){
		slot_num = -slot_num-1;
		if(slot_num == CONST_NULL){
			return String(allocator, OS_TEXT("const null"));
		}
		if(slot_num == CONST_FALSE){
			return String(allocator, OS_TEXT("const false"));
		}
		if(slot_num == CONST_TRUE){
			return String(allocator, OS_TEXT("const true"));
		}
		slot_num -= CONST_STD_VALUES;
		if(slot_num < compiler->prog_numbers.count){
			return String::format(allocator, OS_TEXT("const number %g"), compiler->prog_numbers[slot_num]);
		}
		slot_num -= compiler->prog_numbers.count;
		return String::format(allocator, OS_TEXT("const string \"%s\""), compiler->prog_strings[slot_num].toChar());
	}
	if(slot_num >= scope->function->num_locals){
		return allocator->core->strings->var_temp_prefix;
	}
	for(;;){
		for(int i = scope->locals.count-1; i >= 0; i--){
			const Scope::LocalVar& local_var = scope->locals[i];
			if(local_var.index == slot_num){
				if(local_var.name == allocator->core->strings->var_temp_prefix){
					OS_ASSERT(false);
					return allocator->core->strings->var_temp_prefix;
				}
				return String::format(allocator, slot_num < scope->function->num_params ? OS_TEXT("param %s") : OS_TEXT("var %s"), local_var.name.toChar());
			}
		}
		if(scope == scope->function){
			OS_ASSERT(false);
			break;
		}
		scope = scope->parent;
	}
	return allocator->core->strings->var_temp_prefix; // shut up compiler
}

void OS::Core::Compiler::Expression::debugPrint(Buffer& out, OS::Core::Compiler * compiler, Scope * scope, int depth)
{
	OS * allocator = getAllocator();
	compiler->debugPrintSourceLine(out, token);

	int i;
	OS_CHAR * spaces = (OS_CHAR*)alloca(sizeof(OS_CHAR)*(depth*2+1));
	for(i = 0; i < depth*2; i++){
		spaces[i] = OS_TEXT(' ');
	}
	spaces[i] = OS_TEXT('\0');

	const OS_CHAR * type_name;
	switch(type){
	default:
		OS_ASSERT(false);
		break;

	case EXP_TYPE_NOP:
		for(i = 0; i < list.count; i++){
			list[i]->debugPrint(out, compiler, scope, depth);
		}
		break;

	case EXP_TYPE_CODE_LIST:
		type_name = OS::Core::Compiler::getExpName(type);
		for(i = 0; i < list.count; i++){
			list[i]->debugPrint(out, compiler, scope, depth);
		}
		break;

	case EXP_TYPE_IF:
		OS_ASSERT(list.count == 2 || list.count == 3);
		out += String::format(allocator, OS_TEXT("%sbegin if\n"), spaces);
		out += String::format(allocator, OS_TEXT("%s  begin bool exp\n"), spaces);
		list[0]->debugPrint(out, compiler, scope, depth+2);
		out += String::format(allocator, OS_TEXT("%s  end bool exp\n"), spaces);
		out += String::format(allocator, OS_TEXT("%s  begin then\n"), spaces);
		list[1]->debugPrint(out, compiler, scope, depth+2);
		out += String::format(allocator, OS_TEXT("%s  end then\n"), spaces);
		if(list.count == 3){
			out += String::format(allocator, OS_TEXT("%s  begin else\n"), spaces);
			list[2]->debugPrint(out, compiler, scope, depth+2);
			out += String::format(allocator, OS_TEXT("%s  end else\n"), spaces);
		}
		out += String::format(allocator, OS_TEXT("%send if ret values %d\n"), spaces, ret_values);
		break;

	case EXP_TYPE_QUESTION:
		OS_ASSERT(list.count == 3);
		out += String::format(allocator, OS_TEXT("%sbegin question\n"), spaces);
		out += String::format(allocator, OS_TEXT("%s  begin bool exp\n"), spaces);
		list[0]->debugPrint(out, compiler, scope, depth+2);
		out += String::format(allocator, OS_TEXT("%s  end bool exp\n"), spaces);
		out += String::format(allocator, OS_TEXT("%s  begin then value\n"), spaces);
		list[1]->debugPrint(out, compiler, scope, depth+2);
		out += String::format(allocator, OS_TEXT("%s  end then value\n"), spaces);
		out += String::format(allocator, OS_TEXT("%s  begin else value\n"), spaces);
		list[2]->debugPrint(out, compiler, scope, depth+2);
		out += String::format(allocator, OS_TEXT("%s  end else value\n"), spaces);
		out += String::format(allocator, OS_TEXT("%send question ret values %d\n"), spaces, ret_values);
		break;

	case EXP_TYPE_PARAMS:
		for(i = 0; i < list.count; i++){
			list[i]->debugPrint(out, compiler, scope, depth);
		}
		break;

	case EXP_TYPE_ARRAY:
		out += String::format(allocator, OS_TEXT("%snew array %d: %s (%d)\n"), spaces, list.count,
			getSlotStr(compiler, scope, slots.a).toChar(), slots.a);
		for(i = 0; i < list.count; i++){
			list[i]->debugPrint(out, compiler, scope, depth);
		}
		break;

	case EXP_TYPE_OBJECT:
		out += String::format(allocator, OS_TEXT("%snew object %d: %s (%d)\n"), spaces, list.count,
			getSlotStr(compiler, scope, slots.a).toChar(), slots.a);
		for(i = 0; i < list.count; i++){
			list[i]->debugPrint(out, compiler, scope, depth);
		}
		break;

	case EXP_TYPE_FUNCTION:
		{
			Scope * scope = dynamic_cast<Scope*>(this);
			OS_ASSERT(scope);
			out += String::format(allocator, OS_TEXT("%sbegin function\n"), spaces);
			if(scope->num_locals > 0){
				out += String::format(allocator, OS_TEXT("%s  begin locals %d, stack %d\n"), spaces, scope->num_locals, scope->stack_size);
				for(i = 0; i < scope->locals.count; i++){
					if(scope->locals[i].name == allocator->core->strings->var_temp_prefix){
						continue;
					}
					out += String::format(allocator, OS_TEXT("%s    %d %s%s\n"), spaces, 
						scope->locals[i].index,
						scope->locals[i].name.toChar(),
						i < scope->num_params ? OS_TEXT(" (param)") : OS_TEXT("")
						);
				}
				out += String::format(allocator, OS_TEXT("%s  end locals\n"), spaces);
			}
			for(i = 0; i < list.count; i++){
				if(i > 0){
					out += OS_TEXT("\n");
				}
				list[i]->debugPrint(out, compiler, scope, depth+1);
			}
			out += String::format(allocator, OS_TEXT("%send function: %s (%d), index %d\n"), spaces, 
				scope->function->parent ? scope->function->parent->getSlotStr(compiler, scope->function->parent, slots.a).toChar() : OS_TEXT("<<->>"), 
				slots.a, slots.b);
			break;
		}

	case EXP_TYPE_SCOPE:
	case EXP_TYPE_LOOP_SCOPE:
		{
			Scope * scope = dynamic_cast<Scope*>(this);
			OS_ASSERT(scope);
			const OS_CHAR * exp_name = OS::Core::Compiler::getExpName(type);
			out += String::format(allocator, OS_TEXT("%sbegin %s\n"), spaces, exp_name);
			if(scope->locals.count > 0){
				out += String::format(allocator, OS_TEXT("%s  begin locals %d\n"), spaces, scope->locals.count);
				for(i = 0; i < scope->locals.count; i++){
					if(scope->locals[i].name == allocator->core->strings->var_temp_prefix){
						continue;
					}
					out += String::format(allocator, OS_TEXT("%s    %d %s%s\n"), spaces, 
						scope->locals[i].index,
						scope->locals[i].name.toChar(),
						i < scope->num_params ? OS_TEXT(" (param)") : OS_TEXT("")
						);
				}
				out += String::format(allocator, OS_TEXT("%s  end locals\n"), spaces);
			}
			for(i = 0; i < list.count; i++){
				if(i > 0){
					out += OS_TEXT("\n");
				}
				list[i]->debugPrint(out, compiler, scope, depth+1);
			}
			out += String::format(allocator, OS_TEXT("%send %s ret values %d\n"), spaces, exp_name, ret_values);
			break;
		}

	case EXP_TYPE_RETURN:
		if(list.count > 0){
			for(i = 0; i < list.count; i++){
				list[i]->debugPrint(out, compiler, scope, depth+1);
			}
		}
		out += String::format(allocator, OS_TEXT("%sreturn: %s (%d), count %d\n"), spaces, 
			slots.a ? getSlotStr(compiler, scope, slots.a).toChar() : OS_TEXT("<<->>"), 
			slots.a, slots.b);
		break;

	case EXP_TYPE_GET_UPVALUE:
		out += String::format(allocator, OS_TEXT("%sget upvalue: %s (%d) = %s (%d %d)\n"), spaces,
			getSlotStr(compiler, scope, slots.a).toChar(), slots.a, 
			getSlotStr(compiler, scope, slots.b, slots.c).toChar(), slots.b, slots.c);
		break;

	case EXP_TYPE_SET_UPVALUE:
		OS_ASSERT(list.count == 1);
		list[0]->debugPrint(out, compiler, scope, depth);
		out += String::format(allocator, OS_TEXT("%sset upvalue: %s (%d %d) = %s (%d)\n"), spaces,
			getSlotStr(compiler, scope, slots.a, slots.c).toChar(), slots.a, slots.c, 
			getSlotStr(compiler, scope, slots.b).toChar(), slots.b);
		break;
					
	case EXP_TYPE_MOVE:
	case EXP_TYPE_GET_XCONST:
		{
			for(i = 0; i < list.count; i++){
				list[i]->debugPrint(out, compiler, scope, depth);
			}
			if(slots.a != slots.b && slots.a){
				OS_ASSERT(slots.a > 0);
				const OS_CHAR * exp_name = OS::Core::Compiler::getExpName(type);
				out += String::format(allocator, OS_TEXT("%s%s: %s (%d) = %s (%d)\n"), spaces, exp_name,
					getSlotStr(compiler, scope, slots.a).toChar(), slots.a, 
					getSlotStr(compiler, scope, slots.b).toChar(), slots.b);
			}
			break;
		}

	case EXP_TYPE_BREAK:
		OS_ASSERT(list.count == 0);
		out += String::format(allocator, OS_TEXT("%sbreak\n"), spaces);
		break;

	case EXP_TYPE_CONTINUE:
		OS_ASSERT(list.count == 0);
		out += String::format(allocator, OS_TEXT("%scontinue\n"), spaces);
		break;

	case EXP_TYPE_DEBUGGER:
		OS_ASSERT(list.count == 0);
		out += String::format(allocator, OS_TEXT("%sdebugger\n"), spaces);
		break;

	case EXP_TYPE_TAIL_CALL:
		OS_ASSERT(list.count == 2);
		out += String::format(allocator, OS_TEXT("%sbegin %s\n"), spaces, OS::Core::Compiler::getExpName(type));
		list[0]->debugPrint(out, compiler, scope, depth+1);
		list[1]->debugPrint(out, compiler, scope, depth+1);
		out += String::format(allocator, OS_TEXT("%send %s\n"), spaces, OS::Core::Compiler::getExpName(type));
		break;

	case EXP_TYPE_TAIL_CALL_METHOD:
		OS_ASSERT(list.count == 2);
		out += String::format(allocator, OS_TEXT("%sbegin %s\n"), spaces, OS::Core::Compiler::getExpName(type));
		list[0]->debugPrint(out, compiler, scope, depth+1);
		list[1]->debugPrint(out, compiler, scope, depth+1);
		out += String::format(allocator, OS_TEXT("%send %s\n"), spaces, OS::Core::Compiler::getExpName(type));
		break;

	case EXP_TYPE_CALL_DIM:
	case EXP_TYPE_GET_THIS_PROPERTY_BY_STRING:
	case EXP_TYPE_GET_PROPERTY_BY_LOCALS:
	case EXP_TYPE_GET_PROPERTY_BY_LOCAL_AND_NUMBER:
	case EXP_TYPE_GET_PROPERTY_AUTO_CREATE:
		OS_ASSERT(list.count == 2);
		out += String::format(allocator, OS_TEXT("%sbegin %s\n"), spaces, OS::Core::Compiler::getExpName(type));
		list[0]->debugPrint(out, compiler, scope, depth+1);
		list[1]->debugPrint(out, compiler, scope, depth+1);
		out += String::format(allocator, OS_TEXT("%send %s ret values %d\n"), spaces, OS::Core::Compiler::getExpName(type), ret_values);
		break;

	case EXP_TYPE_GET_PROPERTY:
		OS_ASSERT(list.count == 0 || list.count == 2);
		if(list.count == 2){
			list[0]->debugPrint(out, compiler, scope, depth);
			list[1]->debugPrint(out, compiler, scope, depth);
		}
		out += String::format(allocator, OS_TEXT("%s%s: %s (%d) = %s (%d) [%s (%d)]\n"), spaces, OS::Core::Compiler::getExpName(type), 
					getSlotStr(compiler, scope, slots.a).toChar(), slots.a, 
					getSlotStr(compiler, scope, slots.b).toChar(), slots.b, 
					getSlotStr(compiler, scope, slots.c).toChar(), slots.c);
		break;

	case EXP_TYPE_VALUE:
		OS_ASSERT(list.count == 1);
		out += String::format(allocator, OS_TEXT("%sbegin %s\n"), spaces, OS::Core::Compiler::getExpName(type));
		list[0]->debugPrint(out, compiler, scope, depth+1);
		out += String::format(allocator, OS_TEXT("%send %s\n"), spaces, OS::Core::Compiler::getExpName(type));
		break;

	case EXP_TYPE_POP_VALUE:
		OS_ASSERT(list.count == 1);
		out += String::format(allocator, OS_TEXT("%sbegin %s\n"), spaces, OS::Core::Compiler::getExpName(type));
		list[0]->debugPrint(out, compiler, scope, depth+1);
		out += String::format(allocator, OS_TEXT("%send %s ret values %d\n"), spaces, OS::Core::Compiler::getExpName(type), ret_values);
		break;

	case EXP_TYPE_SUPER:
		OS_ASSERT(list.count == 0);
		out += String::format(allocator, OS_TEXT("%ssuper\n"), spaces);
		break;

	case EXP_TYPE_PLUS:			// +
	case EXP_TYPE_NEG:			// -
	case EXP_TYPE_LOGIC_BOOL:	// !!
	case EXP_TYPE_LOGIC_NOT:	// !
	case EXP_TYPE_BIT_NOT:		// ~
		{
			OS_ASSERT(list.count == 1);
			const OS_CHAR * exp_name = OS::Core::Compiler::getExpName(type);
			list[0]->debugPrint(out, compiler, scope, depth);
			out += String::format(allocator, OS_TEXT("%s%s (%d) = [%s] %s (%d)\n"), spaces,
				getSlotStr(compiler, scope, slots.a).toChar(), slots.a, exp_name,
				getSlotStr(compiler, scope, slots.b).toChar(), slots.b);
			break;
		}

	case EXP_TYPE_INDIRECT:
	case EXP_TYPE_ASSIGN:
	case EXP_TYPE_CONCAT: // ..
	case EXP_TYPE_IN:
	case EXP_TYPE_ISPROTOTYPEOF:
	case EXP_TYPE_IS:
	case EXP_TYPE_BIT_AND: // &
	case EXP_TYPE_BIT_OR:  // |
	case EXP_TYPE_BIT_XOR: // ^
	case EXP_TYPE_BIT_AND_ASSIGN: // &=
	case EXP_TYPE_BIT_OR_ASSIGN:  // |=
	case EXP_TYPE_BIT_XOR_ASSIGN: // ^=
	case EXP_TYPE_BIT_NOT_ASSIGN: // ~=
	case EXP_TYPE_COMPARE: // <=>
	case EXP_TYPE_ADD: // +
	case EXP_TYPE_SUB: // -
	case EXP_TYPE_MUL: // *
	case EXP_TYPE_DIV: // /
	case EXP_TYPE_MOD: // %
	case EXP_TYPE_LSHIFT: // <<
	case EXP_TYPE_RSHIFT: // >>
	case EXP_TYPE_POW: // **

	case EXP_TYPE_ADD_ASSIGN: // +=
	case EXP_TYPE_SUB_ASSIGN: // -=
	case EXP_TYPE_MUL_ASSIGN: // *=
	case EXP_TYPE_DIV_ASSIGN: // /=
	case EXP_TYPE_MOD_ASSIGN: // %=
	case EXP_TYPE_LSHIFT_ASSIGN: // <<=
	case EXP_TYPE_RSHIFT_ASSIGN: // >>=
	case EXP_TYPE_POW_ASSIGN: // **=
		{
			OS_ASSERT(list.count == 2);
			const OS_CHAR * exp_name = OS::Core::Compiler::getExpName(type);
			list[0]->debugPrint(out, compiler, scope, depth);
			list[1]->debugPrint(out, compiler, scope, depth);
			out += String::format(allocator, OS_TEXT("%s%s (%d) = %s (%d) [%s] %s (%d)\n"), spaces,  
				getSlotStr(compiler, scope, slots.a).toChar(), slots.a, 
				getSlotStr(compiler, scope, slots.b).toChar(), slots.b, exp_name,
				getSlotStr(compiler, scope, slots.c).toChar(), slots.c);
			break;
		}

	case EXP_TYPE_LOGIC_PTR_EQ:  // ===
	case EXP_TYPE_LOGIC_PTR_NE:  // !==
	case EXP_TYPE_LOGIC_EQ:  // ==
	case EXP_TYPE_LOGIC_NE:  // !=
	case EXP_TYPE_LOGIC_GE:  // >=
	case EXP_TYPE_LOGIC_LE:  // <=
	case EXP_TYPE_LOGIC_GREATER: // >
	case EXP_TYPE_LOGIC_LESS:    // <
		{
			OS_ASSERT(list.count == 2);
			list[0]->debugPrint(out, compiler, scope, depth);
			list[1]->debugPrint(out, compiler, scope, depth);
			out += String::format(allocator, OS_TEXT("%s%s (%d) = %s (%d) [%s] %s (%d)\n"), spaces,  
				getSlotStr(compiler, scope, slots.a).toChar(), slots.a, 
				getSlotStr(compiler, scope, slots.a).toChar(), slots.a, OS::Core::Compiler::getExpName(type),
				getSlotStr(compiler, scope, slots.a+1).toChar(), slots.a+1);
			break;
		}

	case EXP_TYPE_LOGIC_AND: // &&
	case EXP_TYPE_LOGIC_OR:  // ||
		{
			OS_ASSERT(list.count == 2);
			const OS_CHAR * exp_name = OS::Core::Compiler::getExpName(type);
			list[0]->debugPrint(out, compiler, scope, depth);
			out += String::format(allocator, OS_TEXT("%s%s: %s (%d)\n"), spaces, exp_name,
				getSlotStr(compiler, scope, slots.a).toChar(), slots.a);
			list[1]->debugPrint(out, compiler, scope, depth + 1);
			break;
		}

	case EXP_TYPE_CALL_METHOD:
	case EXP_TYPE_SUPER_CALL:
	case EXP_TYPE_CALL:
	case EXP_TYPE_CALL_AUTO_PARAM:
		{
			OS_ASSERT(list.count == 2);
			const OS_CHAR * exp_name = OS::Core::Compiler::getExpName(type);
			out += String::format(allocator, OS_TEXT("%sbegin %s\n"), spaces, exp_name);
			list[0]->debugPrint(out, compiler, scope, depth+1);
			list[1]->debugPrint(out, compiler, scope, depth+1);
			out += String::format(allocator, OS_TEXT("%send %s: start %d, params %d, ret values %d\n"), spaces, exp_name, slots.a, slots.b, slots.c);
			break;
		}

	case EXP_TYPE_NEW_LOCAL_VAR:
		{
			break;
		}

	case EXP_TYPE_GET_THIS:
	case EXP_TYPE_GET_ARGUMENTS:
	case EXP_TYPE_GET_REST_ARGUMENTS:
		{
			OS_ASSERT(list.count == 0);
			const OS_CHAR * exp_name = OS::Core::Compiler::getExpName(type);
			out += String::format(allocator, OS_TEXT("%s%s: %s (%d)\n"), spaces, exp_name, getSlotStr(compiler, scope, slots.a).toChar(), slots.a);
			break;
		}

	case EXP_TYPE_GET_LOCAL_VAR:
	case EXP_TYPE_GET_LOCAL_VAR_AUTO_CREATE:
		{
			OS_ASSERT(list.count == 0);
			const OS_CHAR * exp_name = OS::Core::Compiler::getExpName(type);
			String info = String::format(allocator, OS_TEXT("(%d %d%s)"),
				local_var.index, local_var.up_count, 
				local_var.type == LOCAL_PARAM ? OS_TEXT(" param") : (local_var.type == LOCAL_TEMP ? OS_TEXT(" temp") : OS_TEXT("")));
			out += String::format(allocator, OS_TEXT("%s%s %s %s\n"), spaces, exp_name, token->str.toChar(), info.toChar());
			break;
		}

	case EXP_TYPE_GET_ENV_VAR:
	case EXP_TYPE_GET_ENV_VAR_AUTO_CREATE:
		{
			OS_ASSERT(list.count == 0);
			const OS_CHAR * exp_name = OS::Core::Compiler::getExpName(type);
			out += String::format(allocator, OS_TEXT("%s%s %s\n"), spaces, exp_name, token->str.toChar());
			break;
		}

	case EXP_TYPE_SET_PROPERTY:
		{
			OS_ASSERT(list.count >= 1 && list.count <= 3);
			const OS_CHAR * exp_name = OS::Core::Compiler::getExpName(type);
			for(i = 0; i < list.count; i++){
				list[i]->debugPrint(out, compiler, scope, depth);
			}
			out += String::format(allocator, OS_TEXT("%s%s: %s (%d) [%s (%d)] = %s (%d)\n"), spaces, exp_name, 
				getSlotStr(compiler, scope, slots.a).toChar(), slots.a,
				getSlotStr(compiler, scope, slots.b).toChar(), slots.b,
				getSlotStr(compiler, scope, slots.c).toChar(), slots.c);
			break;
		}
	}
}

// =====================================================================

int OS::Core::Compiler::cacheString(Table * strings_table, Vector<String>& strings, const String& str)
{
	PropertyIndex index(str, PropertyIndex::KeepStringIndex());
	Property * prop = strings_table->get(index);
	if(prop){
		OS_ASSERT(OS_IS_VALUE_NUMBER(prop->value));
		return (int)OS_VALUE_NUMBER(prop->value);
	}
	prop = new (malloc(sizeof(Property) OS_DBG_FILEPOS)) Property(index);
	prop->value = Value(strings_table->count);
	allocator->core->addTableProperty(strings_table, prop);
	allocator->vectorAddItem(strings, str OS_DBG_FILEPOS);
	OS_ASSERT(strings_table->count == strings.count);
	return strings_table->count-1;
}

int OS::Core::Compiler::cacheString(const String& str)
{
	return cacheString(prog_strings_table, prog_strings, str);
}

int OS::Core::Compiler::cacheDebugString(const String& str)
{
	return cacheString(prog_debug_strings_table, prog_debug_strings, str);
}

int OS::Core::Compiler::cacheNumber(OS_NUMBER num)
{
	PropertyIndex index(num);
	Property * prop = prog_numbers_table->get(index);
	if(prop){
		OS_ASSERT(OS_IS_VALUE_NUMBER(prop->value));
		return (int)OS_VALUE_NUMBER(prop->value);
	}
	prop = new (malloc(sizeof(Property) OS_DBG_FILEPOS)) Property(index);
	prop->value = Value(prog_numbers_table->count);
	allocator->core->addTableProperty(prog_numbers_table, prop);
	allocator->vectorAddItem(prog_numbers, num OS_DBG_FILEPOS);
	OS_ASSERT(prog_numbers_table->count == prog_numbers.count);
	return prog_numbers_table->count-1;
}

void OS::Core::Compiler::writeDebugInfo(Expression * exp)
{
	if(prog_debug_info){
		prog_num_debug_infos++;
		prog_debug_info->writeUVariable(getOpcodePos());
		prog_debug_info->writeUVariable(exp->token->line+1);
		prog_debug_info->writeUVariable(exp->token->pos+1);
		prog_debug_info->writeUVariable(cacheDebugString(exp->token->str));
	}
}

void OS::Core::Compiler::writeJumpOpcode(int offs)
{
	offs += 1;
	Instruction instruction = OS_FROM_OPCODE_TYPE(OP_JUMP);
	OS_SETARG_sBx(instruction, offs);
	writeOpcode(instruction);
}

void OS::Core::Compiler::fixJumpOpcode(int offs, int pos)
{
	Instruction instruction = prog_opcodes[pos];
#ifdef OS_DEBUG
	OpcodeType opcode = (OpcodeType)OS_GET_OPCODE_TYPE(instruction);
	OS_ASSERT(opcode == OP_JUMP);
#endif
	OS_SETARG_sBx(instruction, offs);
	prog_opcodes[pos] = instruction;
}

bool OS::Core::Compiler::writeOpcodes(Scope * scope, ExpressionList& list, bool optimization_enabled)
{
	int start = prog_opcodes.count + 2;
	for(int i = 0; i < list.count; i++){
		if(!writeOpcodes(scope, list[i])){
			return false;
		}
		if(optimization_enabled && prog_opcodes.count >= start){
			Instruction prev = prog_opcodes[prog_opcodes.count - 2];
			if(OS_GET_OPCODE_TYPE(prev) == OP_MOVE){
				Instruction cur = prog_opcodes[prog_opcodes.count - 1];
				if(OS_GET_OPCODE_TYPE(cur) == OP_MOVE){
					int prev_a = OS_GETARG_A(prev);
					int cur_a = OS_GETARG_A(cur);
					if(prev_a+1 == cur_a){
						Instruction instruction = OS_FROM_OPCODE_TYPE(OP_MOVE2);
						OS_SETARG_A(instruction, prev_a);
						int prev_b = OS_GETARG_B(prev & ~OS_OPCODE_CONST_B);
						OS_SETARG_B(instruction, prev_b);
						int cur_b = OS_GETARG_B(cur & ~OS_OPCODE_CONST_B);
						OS_SETARG_C(instruction, cur_b);
						if(prev & OS_OPCODE_CONST_B){
							instruction |= OS_OPCODE_CONST_B;
						}
						if(cur & OS_OPCODE_CONST_B){
							instruction |= OS_OPCODE_CONST_C;
						}
						prog_opcodes[prog_opcodes.count - 2] = instruction;
						start = --prog_opcodes.count;
					}
				}
			}
		}
	}
	return true;
}

int OS::Core::Compiler::getOpcodePos()
{
	return prog_opcodes.count;
}

int OS::Core::Compiler::writeOpcode(OS_U32 opcode)
{
	int i = prog_opcodes.count;
	allocator->vectorAddItem(prog_opcodes, opcode OS_DBG_FILEPOS);
	return i;
}

int OS::Core::Compiler::writeOpcode(OpcodeType opcode)
{
	return writeOpcode(OS_FROM_OPCODE_TYPE(opcode));
}

int OS::Core::Compiler::writeOpcodeABC(OpcodeType p_opcode, int a, int b, int c)
{
	int opcode = OS_FROM_OPCODE_TYPE(p_opcode);
	OS_ASSERT(a >= 0 && a <= OS_MAXARG_A);
	if(b < 0){
		b = -1-b;
		OS_ASSERT(b <= OS_MAXARG_B);
		opcode |= OS_OPCODE_CONST_B;
	}else{
		OS_ASSERT(b <= OS_MAXARG_B);
	}
	if(c < 0){
		c = -1-c;
		OS_ASSERT(c <= OS_MAXARG_C);
		opcode |= OS_OPCODE_CONST_C;
	}else{
		OS_ASSERT(c <= OS_MAXARG_C);
	}
	return writeOpcode(OS_OPCODE_ABC(opcode, a, b, c));
}

int OS::Core::Compiler::writeOpcodeABx(OpcodeType opcode, int a, int b)
{
	OS_ASSERT(a >= 0 && a <= OS_MAXARG_A);
	OS_ASSERT(b >= 0 && b <= OS_MAXARG_Bx);
	Instruction instruction = OS_FROM_OPCODE_TYPE(opcode);
	return writeOpcode(OS_OPCODE_ABx(instruction, a, b));
}

void OS::Core::Compiler::writeOpcodeAt(OS_U32 opcode, int pos)
{
	prog_opcodes[pos] = opcode;
}

bool OS::Core::Compiler::writeOpcodes(Scope * scope, Expression * exp)
{
#if 1
	int i;
	switch(exp->type){
	default:
		{
			ExpressionType exp_type = exp->type;
			OS_ASSERT(false); (void)exp_type;
			return false;;
		}

	case EXP_TYPE_NOP:
	case EXP_TYPE_NEW_LOCAL_VAR:
	case EXP_TYPE_VALUE:
	case EXP_TYPE_CODE_LIST:
	case EXP_TYPE_PARAMS:
	case EXP_TYPE_POP_VALUE:
		if(!writeOpcodes(scope, exp->list, exp->type == EXP_TYPE_CODE_LIST || exp->type == EXP_TYPE_PARAMS)){
			return false;
		}
		break;

	case EXP_TYPE_FUNCTION:
		{
			Scope * scope = dynamic_cast<Scope*>(exp);
			OS_ASSERT(scope);
			writeDebugInfo(exp);
			
			int prog_func_index = scope->prog_func_index; // prog_functions.indexOf(scope);
			OS_ASSERT(prog_func_index >= 0);
			
			int pos = writeOpcodeABC(OP_NEW_FUNCTION, exp->slots.a, prog_func_index, 0); (void)pos;

			allocator->vectorReserveCapacity(scope->locals_compiled, scope->num_locals OS_DBG_FILEPOS);
			scope->locals_compiled.count = scope->num_locals;

			scope->opcodes_pos = getOpcodePos();
			if(!writeOpcodes(scope, exp->list, true)){
				return false;
			}
			writeOpcodeABC(OP_RETURN, 0, 0, 1); // return auto
			scope->opcodes_size = getOpcodePos() - scope->opcodes_pos;

			for(i = 0; i < scope->locals.count; i++){
				Scope::LocalVar& var = scope->locals[i];
				Scope::LocalVarCompiled& var_scope = scope->locals_compiled[var.index];
				var_scope.cached_name_index = cacheString(var.name);
				var_scope.start_code_pos = scope->opcodes_pos;
				var_scope.end_code_pos = getOpcodePos();
			}
			break;
		}

	case EXP_TYPE_SCOPE:
	case EXP_TYPE_LOOP_SCOPE:
		{
			Scope * scope = dynamic_cast<Scope*>(exp);
			OS_ASSERT(scope);
			int start_code_pos = getOpcodePos();
			if(!writeOpcodes(scope, exp->list, true)){
				return false;
			}
			if(exp->type == EXP_TYPE_LOOP_SCOPE){
				writeJumpOpcode(start_code_pos - getOpcodePos() - 2);
				scope->fixLoopBreaks(this, start_code_pos, getOpcodePos());
			}else{
				OS_ASSERT(scope->loop_breaks.count == 0);
			}
			for(i = 0; i < scope->locals.count; i++){
				Scope::LocalVar& var = scope->locals[i];
				Scope::LocalVarCompiled& var_scope = scope->function->locals_compiled[var.index];
				var_scope.cached_name_index = cacheString(var.name);
				var_scope.start_code_pos = start_code_pos;
				var_scope.end_code_pos = getOpcodePos();
			}
			break;
		}

	case EXP_TYPE_IF:
	case EXP_TYPE_QUESTION:
		{
			OS_ASSERT(exp->list.count == 2 || exp->list.count == 3);
			Expression * exp_compare = exp->list[0];
			bool inverse = exp_compare->type == EXP_TYPE_LOGIC_NOT;
			if(inverse){
				OS_ASSERT(exp_compare->list.count == 1);
				switch(exp_compare->list[0]->type){
				case EXP_TYPE_LOGIC_PTR_EQ:
				case EXP_TYPE_LOGIC_PTR_NE:
				case EXP_TYPE_LOGIC_EQ:
				case EXP_TYPE_LOGIC_NE:
				case EXP_TYPE_LOGIC_LE:
				case EXP_TYPE_LOGIC_GREATER:
				case EXP_TYPE_LOGIC_GE:
				case EXP_TYPE_LOGIC_LESS:
					exp_compare = exp_compare->list[0];
					break;

				default:
					OS_ASSERT(exp_compare->slots.a == exp_compare->slots.b);
					break;
				}
			}
			
			OpcodeType opcode;
			OS_ASSERT(exp_compare->slots.a >= scope->function->num_locals);
			switch(exp_compare->type){
			case EXP_TYPE_LOGIC_PTR_EQ:
			case EXP_TYPE_LOGIC_PTR_NE:
				if(!writeOpcodes(scope, exp_compare->list, true)){
					return false;
				}
				opcode = OP_LOGIC_PTR_EQ;
				inverse ^= exp_compare->type != EXP_TYPE_LOGIC_PTR_EQ;
				break;

			case EXP_TYPE_LOGIC_EQ:
			case EXP_TYPE_LOGIC_NE:
				if(!writeOpcodes(scope, exp_compare->list, true)){
					return false;
				}
				opcode = OP_LOGIC_EQ;
				inverse ^= exp_compare->type != EXP_TYPE_LOGIC_EQ;
				break;

			case EXP_TYPE_LOGIC_LE:
			case EXP_TYPE_LOGIC_GREATER:
				if(!writeOpcodes(scope, exp_compare->list, true)){
					return false;
				}
				opcode = OP_LOGIC_GREATER;
				inverse ^= exp_compare->type != EXP_TYPE_LOGIC_GREATER;
				break;

			case EXP_TYPE_LOGIC_GE:
			case EXP_TYPE_LOGIC_LESS:
				if(!writeOpcodes(scope, exp_compare->list, true)){
					return false;
				}
				opcode = OP_LOGIC_GE;
				inverse ^= exp_compare->type != EXP_TYPE_LOGIC_GE;
				break;

			default:
				if(exp_compare->type == EXP_TYPE_LOGIC_NOT || exp_compare->type == EXP_TYPE_LOGIC_BOOL){
					if(!writeOpcodes(scope, exp_compare->list)){
						return false;
					}
				}else{
					if(!writeOpcodes(scope, exp_compare)){
						return false;
					}
				}
				opcode = OP_LOGIC_BOOL;
				break;
			}
			writeDebugInfo(exp);

			writeOpcodeABC(opcode, exp_compare->slots.a, inverse, 1);
			int if_jump_pos = writeOpcode(OP_JUMP);

			if(!writeOpcodes(scope, exp->list[1])){
				return false;
			}
			int if_jump_to = getOpcodePos();
			if(exp->list.count == 3){ // && exp->list[2]->list.count > 0){
				int jump_pos = writeOpcode(OP_JUMP);
				
				if_jump_to = getOpcodePos();
				if(!writeOpcodes(scope, exp->list[2])){
					return false;
				}
				fixJumpOpcode(getOpcodePos() - jump_pos - 1, jump_pos);
			}
			fixJumpOpcode(if_jump_to - if_jump_pos - 1, if_jump_pos);
			break;
		}

	case EXP_TYPE_LOGIC_AND: // &&
	case EXP_TYPE_LOGIC_OR:  // ||
		{
			OS_ASSERT(exp->list.count == 2);
			if(!writeOpcodes(scope, exp->list[0])){
				return false;
			}
			writeDebugInfo(exp);

			// Expression * exp_compare = exp->list[0];
			bool inverse = exp->type == EXP_TYPE_LOGIC_OR;
			writeOpcodeABC(OP_LOGIC_BOOL, exp->slots.a, inverse, 1);
			int op_jump_pos = writeOpcode(OP_JUMP);

			if(!writeOpcodes(scope, exp->list[1])){
				return false;
			}

			int op_jump_to = getOpcodePos();
			fixJumpOpcode(op_jump_to - op_jump_pos - 1, op_jump_pos);
			break;
		}

	case EXP_TYPE_ARRAY:
	case EXP_TYPE_OBJECT:
		writeDebugInfo(exp);
		writeOpcodeABC(exp->type == EXP_TYPE_OBJECT ? OP_NEW_OBJECT : OP_NEW_ARRAY, exp->slots.a, exp->list.count);
		if(!writeOpcodes(scope, exp->list)){
			return false;
		}
		break;

	case EXP_TYPE_GET_ARGUMENTS:
	case EXP_TYPE_GET_REST_ARGUMENTS:
		OS_ASSERT(exp->list.count == 0);
		writeDebugInfo(exp);
		writeOpcodeABC(OP_MULTI, exp->slots.a, 0, exp->type == EXP_TYPE_GET_ARGUMENTS ? OP_MULTI_GET_ARGUMENTS : OP_MULTI_GET_REST_ARGUMENTS);
		break;

	case EXP_TYPE_SUPER:
		OS_ASSERT(exp->list.count == 0);
		writeDebugInfo(exp);
		writeOpcodeABC(OP_MULTI, exp->slots.a, 0, OP_MULTI_SUPER);
		break;

	case EXP_TYPE_DEBUGGER:
		OS_ASSERT(exp->list.count == 0);
		writeDebugInfo(exp);
		writeOpcodeABC(OP_MULTI, 0, 0, OP_MULTI_DEBUGGER);
		break;

	case EXP_TYPE_LOGIC_BOOL:
	case EXP_TYPE_LOGIC_NOT:
		OS_ASSERT(exp->list.count == 1);
		if(!writeOpcodes(scope, exp->list)){
			return false;
		}
		writeDebugInfo(exp);
		writeOpcodeABC(OP_LOGIC_BOOL, exp->slots.a, exp->type == EXP_TYPE_LOGIC_NOT, 0);
		break;

	case EXP_TYPE_LOGIC_PTR_EQ:
	case EXP_TYPE_LOGIC_PTR_NE:
		OS_ASSERT(exp->list.count == 2);
		if(!writeOpcodes(scope, exp->list, true)){
			return false;
		}
		writeDebugInfo(exp);
		writeOpcodeABC(OP_LOGIC_PTR_EQ, exp->slots.a, exp->type == EXP_TYPE_LOGIC_PTR_NE, 0);
		break;

	case EXP_TYPE_LOGIC_EQ:
	case EXP_TYPE_LOGIC_NE:
		OS_ASSERT(exp->list.count == 2);
		if(!writeOpcodes(scope, exp->list, true)){
			return false;
		}
		writeDebugInfo(exp);
		writeOpcodeABC(OP_LOGIC_EQ, exp->slots.a, exp->type == EXP_TYPE_LOGIC_NE, 0);
		break;

	case EXP_TYPE_LOGIC_LE:
	case EXP_TYPE_LOGIC_GREATER:
		OS_ASSERT(exp->list.count == 2);
		if(!writeOpcodes(scope, exp->list, true)){
			return false;
		}
		writeDebugInfo(exp);
		writeOpcodeABC(OP_LOGIC_GREATER, exp->slots.a, exp->type == EXP_TYPE_LOGIC_LE, 0);
		break;

	case EXP_TYPE_LOGIC_GE:
	case EXP_TYPE_LOGIC_LESS:
		OS_ASSERT(exp->list.count == 2);
		if(!writeOpcodes(scope, exp->list, true)){
			return false;
		}
		writeDebugInfo(exp);
		writeOpcodeABC(OP_LOGIC_GE, exp->slots.a, exp->type == EXP_TYPE_LOGIC_LESS, 0);
		break;

	case EXP_TYPE_SUPER_CALL:
	case EXP_TYPE_CALL:
	case EXP_TYPE_CALL_AUTO_PARAM:
	case EXP_TYPE_CALL_METHOD:
	case EXP_TYPE_TAIL_CALL:
	case EXP_TYPE_TAIL_CALL_METHOD:

	case EXP_TYPE_GET_PROPERTY:
	case EXP_TYPE_SET_PROPERTY:

	case EXP_TYPE_GET_UPVALUE:
	case EXP_TYPE_SET_UPVALUE:

	case EXP_TYPE_MOVE:
	case EXP_TYPE_RETURN:

	case EXP_TYPE_BIT_NOT:
	case EXP_TYPE_PLUS:
	case EXP_TYPE_NEG:

	case EXP_TYPE_BIT_AND:
	case EXP_TYPE_BIT_OR:
	case EXP_TYPE_BIT_XOR:

	// case EXP_TYPE_CONCAT:
	case EXP_TYPE_COMPARE:
	case EXP_TYPE_ADD:
	case EXP_TYPE_SUB:
	case EXP_TYPE_MUL:
	case EXP_TYPE_DIV:
	case EXP_TYPE_MOD:
	case EXP_TYPE_LSHIFT:
	case EXP_TYPE_RSHIFT:
	case EXP_TYPE_POW:
		if(!writeOpcodes(scope, exp->list)){
			return false;
		}
		writeDebugInfo(exp);
		writeOpcodeABC(Program::getOpcodeType(exp->type), exp->slots.a, exp->slots.b, exp->slots.c);
		break;

	case EXP_TYPE_GET_XCONST:
		if(!writeOpcodes(scope, exp->list)){
			return false;
		}
		writeDebugInfo(exp);
		OS_ASSERT(exp->slots.b < 0);
		writeOpcodeABx(OP_GET_XCONST, exp->slots.a, -1-exp->slots.b);
		break;

	case EXP_TYPE_BREAK:
	case EXP_TYPE_CONTINUE:
		OS_ASSERT(exp->list.count == 0);
		writeDebugInfo(exp);
		scope->addLoopBreak(writeOpcode(OP_JUMP), exp->type == EXP_TYPE_BREAK ? Scope::LOOP_BREAK : Scope::LOOP_CONTINUE);
		break;

	}
#endif
	return true;
}

// =====================================================================

OS::Core::Compiler::Scope::Scope(Scope * p_parent, ExpressionType type, TokenData * token): Expression(type, token)
{
	OS_ASSERT(type == EXP_TYPE_FUNCTION || type == EXP_TYPE_SCOPE || type == EXP_TYPE_LOOP_SCOPE);
	parent = p_parent;
	function = type == EXP_TYPE_FUNCTION ? this : parent->function;
	num_params = 0;
	num_locals = 0;
	max_up_count = 0;
	func_depth = 0;
	func_index = 0;
	num_local_funcs = 0;
	prog_func_index = -1;
	parser_started = false;
	stack_size = 0;
	stack_cur_size = 0;
}

OS::Core::Compiler::Scope::~Scope()
{
	getAllocator()->vectorClear(locals);
	getAllocator()->vectorClear(locals_compiled);
	getAllocator()->vectorClear(loop_breaks);
}

OS::Core::Compiler::Scope::LocalVar::LocalVar(const String& p_name, int p_index): name(p_name)
{
	index = p_index;
}

OS::Core::Compiler::Scope::LocalVarCompiled::LocalVarCompiled()
{
	cached_name_index = -1;
	start_code_pos = -1;
	end_code_pos = -1;
}

bool OS::Core::Compiler::Scope::addLoopBreak(int pos, ELoopBreakType type)
{
	Scope * scope = this;
	for(; scope; scope = scope->parent){
		if(scope->type == EXP_TYPE_LOOP_SCOPE){
			break;
		}
	}
	if(!scope){
		return false;
	}
	LoopBreak loop_break;
	loop_break.pos = pos;
	loop_break.type = type;
	getAllocator()->vectorAddItem(scope->loop_breaks, loop_break OS_DBG_FILEPOS);
	return true;
}

void OS::Core::Compiler::Scope::fixLoopBreaks(Compiler * compiler, int scope_start_pos, int scope_end_pos)
{
	for(int i = 0; i < loop_breaks.count; i++){
		LoopBreak& loop_break = loop_breaks[i];
		if(loop_break.type == LOOP_BREAK){
			int offs = scope_end_pos - loop_break.pos - 1;
			compiler->fixJumpOpcode(offs, loop_break.pos);
		}else{
			int offs = scope_start_pos - loop_break.pos - 1;
			compiler->fixJumpOpcode(offs, loop_break.pos);
		}
	}
}

void OS::Core::Compiler::Scope::addPreVars()
{
	Core::Strings * strings = getAllocator()->core->strings;
	// don't change following order
	OS_ASSERT(PRE_VAR_FUNC == 0);
	addLocalVar(strings->var_func);
	OS_ASSERT(PRE_VAR_THIS == 1);
	addLocalVar(strings->var_this);
}

void OS::Core::Compiler::Scope::addPostVars()
{
	Core::Strings * strings = getAllocator()->core->strings;
	// don't change following order
	OS_ASSERT(POST_VAR_ENV == 0);
	addLocalVar(strings->var_env);
#ifdef OS_GLOBAL_VAR_ENABLED
	OS_ASSERT(POST_VAR_GLOBALS == 1);
	addLocalVar(strings->var_globals);
#endif
}

void OS::Core::Compiler::Scope::addLocalVar(const String& name)
{
	OS_ASSERT(function->stack_size == function->num_locals);
	OS_ASSERT(function->stack_cur_size == function->num_locals);
	OS * allocator = getAllocator();
	LocalVar local_var(name, function->num_locals);
	allocator->vectorAddItem(locals, local_var OS_DBG_FILEPOS);
	function->stack_size = function->stack_cur_size = ++function->num_locals;
}

void OS::Core::Compiler::Scope::addLocalVar(const String& name, LocalVarDesc& local_var)
{
	local_var.index = function->num_locals;
	local_var.up_count = 0;
	local_var.type = LOCAL_GENERIC;
	addLocalVar(name);
}

int OS::Core::Compiler::Scope::allocTempVar()
{
	if(++function->stack_cur_size > function->stack_size){
		OS_ASSERT(function->stack_cur_size == function->stack_size+1);
		function->stack_size = function->stack_cur_size;
	}
	return function->stack_cur_size-1;
}

void OS::Core::Compiler::Scope::popTempVar(int count)
{
	function->stack_cur_size -= count;
	OS_ASSERT(function->stack_cur_size >= function->num_locals);
}

// =====================================================================

OS::Core::Compiler::Compiler(Tokenizer * p_tokenizer)
	: expect_token(p_tokenizer->getAllocator())
{
	allocator = p_tokenizer->getAllocator();
	tokenizer = p_tokenizer;

	error = ERROR_NOTHING;
	error_token = NULL;
	expect_token_type = Tokenizer::NOTHING;

	recent_token = NULL;
	next_token_index = 0;

	recent_printed_text_data = NULL;
	recent_printed_line = 0;

	// prog = NULL;
	prog_strings_table = NULL;
	prog_debug_strings_table = NULL;
	prog_numbers_table = NULL;
	prog_debug_info = NULL;
	prog_num_debug_infos = 0;
	prog_max_up_count = 0;
}

OS::Core::Compiler::~Compiler()
{
	if(recent_printed_text_data){
		recent_printed_text_data->release();
	}
	if(prog_numbers_table){
		allocator->core->deleteTable(prog_numbers_table);
		prog_numbers_table = NULL;
	}
	if(prog_strings_table){
		allocator->core->deleteTable(prog_strings_table);
		prog_strings_table = NULL;
	}
	if(prog_debug_strings_table){
		allocator->core->deleteTable(prog_debug_strings_table);
		prog_debug_strings_table = NULL;
	}
	allocator->vectorClear(prog_numbers);
	allocator->vectorClear(prog_strings);
	allocator->vectorClear(prog_debug_strings);
	allocator->vectorClear(prog_functions);
	allocator->vectorClear(prog_opcodes);
	allocator->deleteObj(prog_debug_info);
	// allocator->deleteObj(tokenizer);
}

bool OS::Core::Compiler::compile()
{
	OS_ASSERT(!prog_strings_table && !prog_debug_strings_table && !prog_numbers_table);
	OS_ASSERT(!prog_functions.count && !prog_numbers.count && !prog_strings.count);

	Scope * scope = NULL;
	if(tokenizer->isError()){
		setError(ERROR_SYNTAX, NULL);
	}else if(!readToken()){
		setError(ERROR_EXPECT_TOKEN, recent_token);
	}else{
		scope = expectTextExpression();
	}
	if(scope){
		prog_strings_table = allocator->core->newTable(OS_DBG_FILEPOS_START);
		prog_numbers_table = allocator->core->newTable(OS_DBG_FILEPOS_START);
		
		Expression * exp = postCompileExpression(scope, scope);
		OS_ASSERT(exp->type == EXP_TYPE_FUNCTION);

		OS::String filename(allocator, tokenizer->getTextData()->filename);
		bool is_eval = filename.getDataSize() == 0;

		if((!is_eval || allocator->core->settings.create_debug_eval_opcodes) && 
			allocator->core->settings.create_debug_opcodes)
		{
			Buffer dump(allocator);
			OS_ASSERT(dynamic_cast<Scope*>(exp));
			exp->debugPrint(dump, this, dynamic_cast<Scope*>(exp), 0);
			OS::String dump_filename = allocator->getDebugOpcodesFilename(filename);
			FileStreamWriter(allocator, dump_filename).writeBytes(dump.buffer.buf, dump.buffer.count);
		}
		prog_debug_strings_table = allocator->core->newTable(OS_DBG_FILEPOS_START);
		prog_debug_info = new (malloc(sizeof(MemStreamWriter) OS_DBG_FILEPOS)) MemStreamWriter(allocator);

		if(!writeOpcodes(scope, exp)){
			// TODO:
		}

		MemStreamWriter mem_writer(allocator);
		MemStreamWriter debuginfo_mem_writer(allocator);
		saveToStream(&mem_writer, &debuginfo_mem_writer);

		if(!is_eval && allocator->core->settings.create_compiled_file){
			OS::String compiled_filename = allocator->getCompiledFilename(filename);
			FileStreamWriter(allocator, compiled_filename).writeBytes(mem_writer.buffer.buf, mem_writer.buffer.count);
			if(allocator->core->settings.create_debug_info){
				OS::String debug_info_filename = allocator->getDebugInfoFilename(filename);
				FileStreamWriter(allocator, debug_info_filename).writeBytes(debuginfo_mem_writer.buffer.buf, debuginfo_mem_writer.buffer.count);
			}
		}

		Program * prog = new (malloc(sizeof(Program) OS_DBG_FILEPOS)) Program(allocator);
		prog->filename = tokenizer->getTextData()->filename;

		MemStreamReader mem_reader(NULL, mem_writer.buffer.buf, mem_writer.buffer.count);
		MemStreamReader debuginfo_mem_reader(NULL, debuginfo_mem_writer.buffer.buf, debuginfo_mem_writer.buffer.count);
		prog->loadFromStream(&mem_reader, &debuginfo_mem_reader);

		prog->pushStartFunction();
		prog->release();

		allocator->deleteObj(exp);

		return true;
	}else{
		Buffer dump(allocator);
		dump += OS_TEXT("Error");
		switch(error){
		default:
			dump += OS_TEXT(" unknown");
			break;

		case ERROR_SYNTAX:
			dump += OS_TEXT(" SYNTAX");
			break;

		case ERROR_NESTED_ROOT_BLOCK:
			dump += OS_TEXT(" NESTED_ROOT_BLOCK");
			break;

		case ERROR_LOCAL_VAL_NOT_DECLARED:
			dump += OS_TEXT(" LOCAL_VAL_NOT_DECLARED");
			break;

		case ERROR_VAR_ALREADY_EXIST:
			dump += OS_TEXT(" VAR_ALREADY_EXIST");
			break;

		case ERROR_VAR_NAME:
			dump += OS_TEXT(" VAR_NAME");
			break;

		case ERROR_EXPECT_TOKEN_TYPE:
			dump += OS_TEXT(" EXPECT_TOKEN_TYPE ");
			dump += Tokenizer::getTokenTypeName(expect_token_type);
			break;

		case ERROR_EXPECT_TOKEN_STR:
			dump += OS_TEXT(" EXPECT_TOKEN_STR ");
			dump += expect_token;
			break;

		case ERROR_EXPECT_TOKEN:
			dump += OS_TEXT(" EXPECT_TOKEN");
			break;

		case ERROR_EXPECT_VALUE:
			dump += OS_TEXT(" EXPECT_VALUE");
			break;

		case ERROR_EXPECT_WRITEABLE:
			dump += OS_TEXT(" EXPECT_WRITEABLE");
			break;

		case ERROR_EXPECT_GET_OR_SET:
			dump += OS_TEXT(" EXPECT_GET_OR_SET");
			break;

		case ERROR_EXPECT_EXPRESSION:
			dump += OS_TEXT(" EXPECT_EXPRESSION");
			break;

		case ERROR_EXPECT_FUNCTION_SCOPE:
			dump += OS_TEXT(" EXPECT_FUNCTION_SCOPE");
			break;

		case ERROR_EXPECT_CODE_SEP_BEFORE_NESTED_BLOCK:
			dump += OS_TEXT(" EXPECT_CODE_SEP_BEFORE_NESTED_BLOCK");
			break;

		case ERROR_EXPECT_SWITCH_SCOPE:
			dump += OS_TEXT(" EXPECT_SWITCH_SCOPE");
			break;

		case ERROR_FINISH_BINARY_OP:
			dump += OS_TEXT(" FINISH_BINARY_OP");
			break;

		case ERROR_FINISH_UNARY_OP:
			dump += OS_TEXT(" FINISH_UNARY_OP");
			break;
		}
		dump += OS_TEXT("\n");
		if(error_token){
			if(error_token->text_data->filename.getDataSize() > 0){
				dump += OS::Core::String::format(allocator, "filename %s\n", error_token->text_data->filename.toChar());
			}
			dump += OS::Core::String::format(allocator, "[%d] %s\n", error_token->line+1, error_token->text_data->lines[error_token->line].toChar());
			dump += OS::Core::String::format(allocator, "pos %d, token: %s\n", error_token->pos+1, error_token->str.toChar());
		}else if(tokenizer->isError()){
			if(tokenizer->getFilename().getDataSize() > 0){
				dump += OS::Core::String::format(allocator, "filename %s\n", tokenizer->getFilename().toChar());
			}
			dump += OS::Core::String::format(allocator, "[%d] %s\n", tokenizer->getErrorLine()+1, tokenizer->getLineString(tokenizer->getErrorLine()).toChar());
			dump += OS::Core::String::format(allocator, "pos %d\n", tokenizer->getErrorPos()+1);
		}
		allocator->echo(dump.toString().toChar());
		// FileStreamWriter(allocator, "test-data/debug-exp-dump.txt").writeBytes(dump.toChar(), dump.getDataSize());

		allocator->pushNull();
	}
	return false;
}

void * OS::Core::Compiler::malloc(int size OS_DBG_FILEPOS_DECL)
{
	return allocator->malloc(size OS_DBG_FILEPOS_PARAM);
}

void OS::Core::Compiler::resetError()
{
	error = ERROR_NOTHING;
	error_token = NULL;
	expect_token_type = Tokenizer::NOTHING;
}

void OS::Core::Compiler::setError(ErrorType value, TokenData * error_token)
{
	OS_ASSERT(!isError());
	error = value;
	this->error_token = error_token;
	expect_token_type = Tokenizer::NOTHING;
}

void OS::Core::Compiler::setError(TokenType expect_token_type, TokenData * error_token)
{
	OS_ASSERT(!isError());
	error = ERROR_EXPECT_TOKEN_TYPE;
	this->error_token = error_token;
	this->expect_token_type = expect_token_type;
}

void OS::Core::Compiler::setError(const String& str, TokenData * error_token)
{
	OS_ASSERT(!isError());
	error = ERROR_EXPECT_TOKEN_STR;
	this->error_token = error_token;
	expect_token_type = Tokenizer::NOTHING;
	expect_token = str;
}

bool OS::Core::Compiler::isError()
{
	return error != ERROR_NOTHING;
}

OS::Core::Compiler::ExpressionType OS::Core::Compiler::getUnaryExpressionType(TokenType token_type)
{
	switch(token_type){
	case Tokenizer::OPERATOR_LENGTH: return EXP_TYPE_LENGTH;
	case Tokenizer::OPERATOR_BIT_NOT: return EXP_TYPE_BIT_NOT;
	case Tokenizer::OPERATOR_ADD: return EXP_TYPE_PLUS;
	case Tokenizer::OPERATOR_SUB: return EXP_TYPE_NEG;
	case Tokenizer::OPERATOR_LOGIC_NOT: return EXP_TYPE_LOGIC_NOT;
	}
	return EXP_TYPE_UNKNOWN;
}

OS::Core::Compiler::ExpressionType OS::Core::Compiler::getExpressionType(TokenType token_type)
{
	switch(token_type){
	case Tokenizer::PARAM_SEPARATOR: return EXP_TYPE_PARAMS;

	case Tokenizer::OPERATOR_INDIRECT: return EXP_TYPE_INDIRECT;

	case Tokenizer::OPERATOR_CONCAT: return EXP_TYPE_CONCAT;
	case Tokenizer::OPERATOR_LENGTH: return EXP_TYPE_LENGTH;

	case Tokenizer::OPERATOR_LOGIC_AND: return EXP_TYPE_LOGIC_AND;
	case Tokenizer::OPERATOR_LOGIC_OR: return EXP_TYPE_LOGIC_OR;
	case Tokenizer::OPERATOR_LOGIC_PTR_EQ: return EXP_TYPE_LOGIC_PTR_EQ;
	case Tokenizer::OPERATOR_LOGIC_PTR_NE: return EXP_TYPE_LOGIC_PTR_NE;
	case Tokenizer::OPERATOR_LOGIC_EQ: return EXP_TYPE_LOGIC_EQ;
	case Tokenizer::OPERATOR_LOGIC_NE: return EXP_TYPE_LOGIC_NE;
	case Tokenizer::OPERATOR_LOGIC_GE: return EXP_TYPE_LOGIC_GE;
	case Tokenizer::OPERATOR_LOGIC_LE: return EXP_TYPE_LOGIC_LE;
	case Tokenizer::OPERATOR_LOGIC_GREATER: return EXP_TYPE_LOGIC_GREATER;
	case Tokenizer::OPERATOR_LOGIC_LESS: return EXP_TYPE_LOGIC_LESS;
	case Tokenizer::OPERATOR_LOGIC_NOT: return EXP_TYPE_LOGIC_NOT;

	case Tokenizer::OPERATOR_QUESTION: return EXP_TYPE_QUESTION;
	case Tokenizer::OPERATOR_IN: return EXP_TYPE_IN;
	case Tokenizer::OPERATOR_ISPROTOTYPEOF: return EXP_TYPE_ISPROTOTYPEOF;
	case Tokenizer::OPERATOR_IS: return EXP_TYPE_IS;

	case Tokenizer::OPERATOR_BIT_AND: return EXP_TYPE_BIT_AND;
	case Tokenizer::OPERATOR_BIT_OR: return EXP_TYPE_BIT_OR;
	case Tokenizer::OPERATOR_BIT_XOR: return EXP_TYPE_BIT_XOR;
	case Tokenizer::OPERATOR_BIT_NOT: return EXP_TYPE_BIT_NOT;
	case Tokenizer::OPERATOR_COMPARE: return EXP_TYPE_COMPARE;
	case Tokenizer::OPERATOR_ADD: return EXP_TYPE_ADD;
	case Tokenizer::OPERATOR_SUB: return EXP_TYPE_SUB;
	case Tokenizer::OPERATOR_MUL: return EXP_TYPE_MUL;
	case Tokenizer::OPERATOR_DIV: return EXP_TYPE_DIV;
	case Tokenizer::OPERATOR_MOD: return EXP_TYPE_MOD;
	case Tokenizer::OPERATOR_LSHIFT: return EXP_TYPE_LSHIFT;
	case Tokenizer::OPERATOR_RSHIFT: return EXP_TYPE_RSHIFT;
	case Tokenizer::OPERATOR_POW: return EXP_TYPE_POW;

	case Tokenizer::OPERATOR_BIT_AND_ASSIGN: return EXP_TYPE_BIT_AND_ASSIGN;
	case Tokenizer::OPERATOR_BIT_OR_ASSIGN: return EXP_TYPE_BIT_OR_ASSIGN;
	case Tokenizer::OPERATOR_BIT_XOR_ASSIGN: return EXP_TYPE_BIT_XOR_ASSIGN;
	case Tokenizer::OPERATOR_BIT_NOT_ASSIGN: return EXP_TYPE_BIT_NOT_ASSIGN;
	case Tokenizer::OPERATOR_ADD_ASSIGN: return EXP_TYPE_ADD_ASSIGN;
	case Tokenizer::OPERATOR_SUB_ASSIGN: return EXP_TYPE_SUB_ASSIGN;
	case Tokenizer::OPERATOR_MUL_ASSIGN: return EXP_TYPE_MUL_ASSIGN;
	case Tokenizer::OPERATOR_DIV_ASSIGN: return EXP_TYPE_DIV_ASSIGN;
	case Tokenizer::OPERATOR_MOD_ASSIGN: return EXP_TYPE_MOD_ASSIGN;
	case Tokenizer::OPERATOR_LSHIFT_ASSIGN: return EXP_TYPE_LSHIFT_ASSIGN;
	case Tokenizer::OPERATOR_RSHIFT_ASSIGN: return EXP_TYPE_RSHIFT_ASSIGN;
	case Tokenizer::OPERATOR_POW_ASSIGN: return EXP_TYPE_POW_ASSIGN;

	case Tokenizer::OPERATOR_ASSIGN: return EXP_TYPE_ASSIGN;
	}
	return EXP_TYPE_UNKNOWN;
}

OS::Core::Compiler::OpcodeLevel OS::Core::Compiler::getOpcodeLevel(ExpressionType exp_type)
{
	switch(exp_type){
	case EXP_TYPE_ASSIGN:	// =
	case EXP_TYPE_BIT_AND_ASSIGN: // &=
	case EXP_TYPE_BIT_OR_ASSIGN:  // |=
	case EXP_TYPE_BIT_XOR_ASSIGN: // ^=
	case EXP_TYPE_BIT_NOT_ASSIGN: // ~=
	case EXP_TYPE_ADD_ASSIGN: // +=
	case EXP_TYPE_SUB_ASSIGN: // -=
	case EXP_TYPE_MUL_ASSIGN: // *=
	case EXP_TYPE_DIV_ASSIGN: // /=
	case EXP_TYPE_MOD_ASSIGN: // %=
	case EXP_TYPE_LSHIFT_ASSIGN: // <<=
	case EXP_TYPE_RSHIFT_ASSIGN: // >>=
	case EXP_TYPE_POW_ASSIGN: // **=
		return OP_LEVEL_1;

	case EXP_TYPE_PARAMS:	// ,
		return OP_LEVEL_1_1;

	case EXP_TYPE_QUESTION:    // ? :
		return OP_LEVEL_2;

	case EXP_TYPE_LOGIC_OR:  // ||
		return OP_LEVEL_3;

	case EXP_TYPE_LOGIC_AND: // &&
		return OP_LEVEL_4;

	case EXP_TYPE_CONCAT: // ..
		return OP_LEVEL_5;

	case EXP_TYPE_LOGIC_PTR_EQ:  // ===
	case EXP_TYPE_LOGIC_PTR_NE:  // !==
	case EXP_TYPE_LOGIC_EQ:  // ==
	case EXP_TYPE_LOGIC_NE:  // !=
		return OP_LEVEL_6;

	case EXP_TYPE_LOGIC_GE:  // >=
	case EXP_TYPE_LOGIC_LE:  // <=
	case EXP_TYPE_LOGIC_GREATER: // >
	case EXP_TYPE_LOGIC_LESS:    // <
	case EXP_TYPE_COMPARE:	 // <=>
		return OP_LEVEL_7;

	case EXP_TYPE_BIT_OR:  // |
		return OP_LEVEL_8;

	case EXP_TYPE_BIT_AND: // &
	case EXP_TYPE_BIT_XOR: // ^
		return OP_LEVEL_9;

	case EXP_TYPE_LSHIFT: // <<
	case EXP_TYPE_RSHIFT: // >>
		return OP_LEVEL_10;

	case EXP_TYPE_ADD: // +
	case EXP_TYPE_SUB: // -
		return OP_LEVEL_11;

	case EXP_TYPE_MUL: // *
	case EXP_TYPE_DIV: // /
	case EXP_TYPE_MOD: // %
		return OP_LEVEL_12;

	case EXP_TYPE_POW: // **
	case EXP_TYPE_IN:
	case EXP_TYPE_ISPROTOTYPEOF:
	case EXP_TYPE_IS:
		return OP_LEVEL_13;

	case EXP_TYPE_PRE_INC:     // ++
	case EXP_TYPE_PRE_DEC:     // --
	case EXP_TYPE_POST_INC:    // ++
	case EXP_TYPE_POST_DEC:    // --
		return OP_LEVEL_14;

	case EXP_TYPE_LOGIC_BOOL:	// !!
	case EXP_TYPE_LOGIC_NOT:    // !
	case EXP_TYPE_PLUS:			// +
	case EXP_TYPE_NEG:			// -
	case EXP_TYPE_LENGTH:		// #
	case EXP_TYPE_BIT_NOT:		// ~
		return OP_LEVEL_15;

	case EXP_TYPE_INDIRECT:
		return OP_LEVEL_16;
	}
	return OP_LEVEL_0;
}

OS::Core::Tokenizer::TokenData * OS::Core::Compiler::getPrevToken()
{
	int i = next_token_index-2;
	return i >= 0 ? tokenizer->getToken(i) : NULL;
}

OS::Core::Tokenizer::TokenData * OS::Core::Compiler::readToken()
{
	if(isError()){
		return NULL;
	}
	if(next_token_index < tokenizer->getNumTokens()){
		return recent_token = tokenizer->getToken(next_token_index++);
	}
	return recent_token = NULL;
}

OS::Core::Tokenizer::TokenData * OS::Core::Compiler::setNextTokenIndex(int i)
{
	OS_ASSERT(tokenizer && i >= 0 && i <= tokenizer->getNumTokens());
	next_token_index = i;
	return recent_token = next_token_index > 0 ? tokenizer->getToken(next_token_index-1) : NULL;
}

OS::Core::Tokenizer::TokenData * OS::Core::Compiler::setNextToken(TokenData * token)
{
	OS_ASSERT(tokenizer);
	int i, count = tokenizer->getNumTokens();
	for(i = next_token_index - 1; i >= 0; i--){
		if(tokenizer->getToken(i) == token)
			break;
	}
	if(i < 0){
		for(i = next_token_index; i < count; i++){
			if(tokenizer->getToken(i) == token)
				break;
		}
	}
	if(i >= 0 && i < count){
		next_token_index = i;
		return recent_token = next_token_index > 0 ? tokenizer->getToken(next_token_index-1) : NULL;
	}
	OS_ASSERT(false);
	return NULL;
}

OS::Core::Tokenizer::TokenData * OS::Core::Compiler::putNextTokenType(TokenType token_type)
{
	if(token_type == Tokenizer::CODE_SEPARATOR && recent_token && recent_token->type == token_type){
		return ungetToken();
	}
	TokenData * token = recent_token;
	if(readToken()){
		if(recent_token->type == token_type){
			return ungetToken();
		}
		ungetToken();
		token = recent_token;
	}
	if(!token){
		if(next_token_index > 0){
			token = tokenizer->getToken(next_token_index-1);
		}
	}
	if(token){
		token = new (malloc(sizeof(TokenData) OS_DBG_FILEPOS)) TokenData(token->text_data, String(allocator), token_type, token->line, token->pos);
	}else{
		token = new (malloc(sizeof(TokenData) OS_DBG_FILEPOS)) TokenData(tokenizer->getTextData(), String(allocator), token_type, 0, 0);
	}
	tokenizer->insertToken(next_token_index, token OS_DBG_FILEPOS);
	return token;
}

OS::Core::Tokenizer::TokenData * OS::Core::Compiler::ungetToken()
{
	return setNextTokenIndex(next_token_index-1);
}

bool OS::Core::Compiler::isNextTokens(TokenType * list, int count)
{
	bool ok = true;
	int save_next_token_index = next_token_index;
	for(int i = 0; i < count; i++){
		if(!readToken() || !recent_token->isTypeOf(list[i])){
			ok = false;
			break;
		}
	}
	setNextTokenIndex(save_next_token_index);
	return ok;
}

bool OS::Core::Compiler::isNextToken(TokenType t0)
{
	return isNextTokens(&t0, 1);
}

bool OS::Core::Compiler::isNextTokens(TokenType t0, TokenType t1)
{
	TokenType list[] = {t0, t1};
	return isNextTokens(list, sizeof(list)/sizeof(list[0]));
}

bool OS::Core::Compiler::isNextTokens(TokenType t0, TokenType t1, TokenType t2)
{
	TokenType list[] = {t0, t1, t2};
	return isNextTokens(list, sizeof(list)/sizeof(list[0]));
}

bool OS::Core::Compiler::isNextTokens(TokenType t0, TokenType t1, TokenType t2, TokenType t3)
{
	TokenType list[] = {t0, t1, t2, t3};
	return isNextTokens(list, sizeof(list)/sizeof(list[0]));
}

void OS::Core::Compiler::deleteNops(ExpressionList& list)
{
	for(int i = 0; i < list.count; i++){
		Expression * exp = list[i];
		switch(exp->type){
		case EXP_TYPE_NOP:
			allocator->deleteObj(exp);
			list.removeIndex(i--);
			break;
		}
	}
}

OS::Core::Tokenizer::TokenData * OS::Core::Compiler::expectToken(TokenType type)
{
	if(isError()){
		return NULL;
	}
	if(!readToken() || recent_token->type != type){
		setError(type, recent_token);
		return NULL;
	}
	return recent_token;
}

OS::Core::Tokenizer::TokenData * OS::Core::Compiler::expectToken()
{
	if(isError()){
		return NULL;
	}
	if(!readToken()){
		setError(ERROR_EXPECT_TOKEN, recent_token);
		return NULL;
	}
	return recent_token;
}

OS::Core::Compiler::Expression * OS::Core::Compiler::expectExpressionValues(Expression * exp, int ret_values, bool auto_no_values)
{
	if(exp->ret_values == ret_values || ret_values < 0){
		return exp;
	}
	switch(exp->type){
	case EXP_TYPE_CALL:
	case EXP_TYPE_CALL_AUTO_PARAM:
	case EXP_TYPE_CALL_DIM:
	case EXP_TYPE_CALL_METHOD:
	case EXP_TYPE_GET_PROPERTY:
	case EXP_TYPE_GET_THIS_PROPERTY_BY_STRING:
	case EXP_TYPE_GET_PROPERTY_BY_LOCALS:
	case EXP_TYPE_GET_PROPERTY_BY_LOCAL_AND_NUMBER:
	case EXP_TYPE_GET_PROPERTY_AUTO_CREATE:
	case EXP_TYPE_INDIRECT:
	case EXP_TYPE_TAIL_CALL: // ret values are not used for tail call
	case EXP_TYPE_TAIL_CALL_METHOD: // ret values are not used for tail call
		exp->ret_values = ret_values;
		return exp;

	case EXP_TYPE_CODE_LIST:
		if(exp->list.count > 0){
			Expression * last_exp = exp->list[exp->list.count-1];
			switch(last_exp->type){
			case EXP_TYPE_CALL:
			case EXP_TYPE_CALL_AUTO_PARAM:
			case EXP_TYPE_CALL_DIM:
			case EXP_TYPE_CALL_METHOD:
			case EXP_TYPE_GET_PROPERTY:
			case EXP_TYPE_GET_THIS_PROPERTY_BY_STRING:
			case EXP_TYPE_GET_PROPERTY_BY_LOCALS:
			case EXP_TYPE_GET_PROPERTY_BY_LOCAL_AND_NUMBER:
			case EXP_TYPE_GET_PROPERTY_AUTO_CREATE:
			case EXP_TYPE_INDIRECT:
			case EXP_TYPE_TAIL_CALL: // ret values are not used for tail call
			case EXP_TYPE_TAIL_CALL_METHOD: // ret values are not used for tail call
				last_exp->ret_values = ret_values;
				exp->ret_values = ret_values;
				return exp;

			case EXP_TYPE_RETURN:
				last_exp = new (malloc(sizeof(Expression) OS_DBG_FILEPOS)) Expression(EXP_TYPE_CODE_LIST, last_exp->token, last_exp OS_DBG_FILEPOS);
				exp->list[exp->list.count-1] = last_exp;
				last_exp->ret_values = ret_values;
				exp->ret_values = ret_values;
				return exp;
			}
		}
		break;

	case EXP_TYPE_RETURN:
		exp = new (malloc(sizeof(Expression) OS_DBG_FILEPOS)) Expression(EXP_TYPE_CODE_LIST, exp->token, exp OS_DBG_FILEPOS);
		exp->ret_values = ret_values;		
		return exp;

	case EXP_TYPE_PARAMS:
		if(exp->ret_values > ret_values){
			for(int i = exp->list.count-1; exp->ret_values > ret_values && i >= 0; i--){
				Expression * param_exp = exp->list[i];
				if(param_exp->type == EXP_TYPE_PARAMS){
					break;
				}
				OS_ASSERT(param_exp->type != EXP_TYPE_PARAMS);
				OS_ASSERT(param_exp->type != EXP_TYPE_RETURN);
				OS_ASSERT(param_exp->type != EXP_TYPE_CODE_LIST);
				if(param_exp->isConstValue()){
					exp->list.removeIndex(i);
					exp->ret_values--;
					allocator->deleteObj(param_exp);
					continue;
				}
				switch(param_exp->type){
				case EXP_TYPE_CALL:
				case EXP_TYPE_CALL_AUTO_PARAM:
				case EXP_TYPE_CALL_DIM:
				case EXP_TYPE_CALL_METHOD:
				case EXP_TYPE_GET_PROPERTY:
				case EXP_TYPE_GET_THIS_PROPERTY_BY_STRING:
				case EXP_TYPE_GET_PROPERTY_BY_LOCALS:
				case EXP_TYPE_GET_PROPERTY_BY_LOCAL_AND_NUMBER:
				case EXP_TYPE_GET_PROPERTY_AUTO_CREATE:
				case EXP_TYPE_INDIRECT:
					if(exp->ret_values <= param_exp->ret_values){
						param_exp->ret_values -= exp->ret_values;
						exp->ret_values = 0;
					}else{
						exp->ret_values -= param_exp->ret_values;
						param_exp->ret_values = 0;
					}
					continue;
				}
				break;
			}
		}
		break;

	case EXP_TYPE_PRE_INC:
	case EXP_TYPE_PRE_DEC:
	case EXP_TYPE_POST_INC:
	case EXP_TYPE_POST_DEC:
		OS_ASSERT(exp->ret_values == 1);
		if(!ret_values){
			exp->ret_values = 0;
			return exp;
		}
		break;
	}
	while(exp->ret_values > ret_values){
		int new_ret_values = exp->ret_values-1;
		exp = new (malloc(sizeof(Expression) OS_DBG_FILEPOS)) Expression(EXP_TYPE_POP_VALUE, exp->token, exp OS_DBG_FILEPOS);
		exp->ret_values = new_ret_values;
	}
	if(exp->ret_values < ret_values){
		if(exp->type != EXP_TYPE_PARAMS){
			exp = new (malloc(sizeof(Expression) OS_DBG_FILEPOS)) Expression(EXP_TYPE_PARAMS, exp->token, exp OS_DBG_FILEPOS);
			exp->ret_values = exp->list[0]->ret_values;
		}
		while(exp->ret_values < ret_values){
			if(auto_no_values && exp->ret_values+1 == ret_values){
				break;
			}
			Expression * null_exp = new (malloc(sizeof(Expression) OS_DBG_FILEPOS)) Expression(EXP_TYPE_CONST_NULL, exp->token);
			null_exp->ret_values = 1;
			exp->list.add(null_exp OS_DBG_FILEPOS);
			exp->ret_values++;
		}
	}
	return exp;
}

OS::Core::Compiler::Expression * OS::Core::Compiler::newSingleValueExpression(Expression * exp)
{
	exp = expectExpressionValues(exp, 1);
	switch(exp->type){
	case EXP_TYPE_CALL:
	case EXP_TYPE_CALL_AUTO_PARAM:
	case EXP_TYPE_CALL_DIM:
	case EXP_TYPE_CALL_METHOD:
	case EXP_TYPE_GET_PROPERTY:
	case EXP_TYPE_GET_THIS_PROPERTY_BY_STRING:
	case EXP_TYPE_GET_PROPERTY_BY_LOCALS:
	case EXP_TYPE_GET_PROPERTY_BY_LOCAL_AND_NUMBER:
	case EXP_TYPE_GET_PROPERTY_AUTO_CREATE:
	case EXP_TYPE_INDIRECT:
		{
			exp = new (malloc(sizeof(Expression) OS_DBG_FILEPOS)) Expression(EXP_TYPE_VALUE, exp->token, exp OS_DBG_FILEPOS);
			exp->ret_values = 1;
			break;
		}
	}
	return exp;
}

OS::Core::Compiler::Expression * OS::Core::Compiler::newExpressionFromList(ExpressionList& list, int ret_values, bool auto_no_values)
{
	Expression * exp;
	if(list.count == 1){
		exp = list[0];
		list.removeIndex(0);
	}else if(list.count == 0){
		TokenData * cur_token = ungetToken();
		readToken();
		exp = new (malloc(sizeof(Expression) OS_DBG_FILEPOS)) Expression(EXP_TYPE_CODE_LIST, cur_token);
	}else{
		int i;
		for(i = 0; i < list.count-1; i++){
			OS_ASSERT(list[i]->type != EXP_TYPE_CODE_LIST);
			list[i] = expectExpressionValues(list[i], 0);
		}
		exp = new (malloc(sizeof(Expression) OS_DBG_FILEPOS)) Expression(EXP_TYPE_CODE_LIST, list[0]->token);
		exp->list.swap(list);
		exp->ret_values = exp->list[exp->list.count-1]->ret_values;
	}
	return expectExpressionValues(exp, ret_values, auto_no_values);
}

OS::Core::Compiler::Expression * OS::Core::Compiler::postCompilePass2(Scope * scope, Expression * exp)
{
	switch(exp->type){
	case EXP_TYPE_FUNCTION:
		{
			Scope * new_scope = dynamic_cast<Scope*>(exp);
			OS_ASSERT(new_scope && (new_scope->parent == scope || (!new_scope->parent && new_scope->type == EXP_TYPE_FUNCTION)));
			if(new_scope != scope){
				new_scope->func_index = scope->function->num_local_funcs++;
				new_scope->func_depth = scope->function->func_depth + 1;
			}
			scope = new_scope;
			OS_ASSERT(prog_functions.indexOf(scope) < 0);
			scope->prog_func_index = prog_functions.count;
			allocator->vectorAddItem(prog_functions, scope OS_DBG_FILEPOS);
			break;
		}

	case EXP_TYPE_SCOPE:
	case EXP_TYPE_LOOP_SCOPE:
		{
			Scope * new_scope = dynamic_cast<Scope*>(exp);
			OS_ASSERT(new_scope && (new_scope->parent == scope || (!new_scope->parent && new_scope->type == EXP_TYPE_FUNCTION)));
			scope = new_scope;
			break;
		}

	case EXP_TYPE_DEBUG_LOCALS:
		if(exp->list.count == 0){
			Expression * obj_exp = new (malloc(sizeof(Expression) OS_DBG_FILEPOS)) Expression(EXP_TYPE_OBJECT, exp->token);

			Vector<String> vars;

			// skip std vars
			allocator->vectorAddItem(vars, allocator->core->strings->var_func OS_DBG_FILEPOS);
			allocator->vectorAddItem(vars, allocator->core->strings->var_this OS_DBG_FILEPOS);
			allocator->vectorAddItem(vars, allocator->core->strings->var_env OS_DBG_FILEPOS);
#ifdef OS_GLOBAL_VAR_ENABLED
			allocator->vectorAddItem(vars, allocator->core->strings->var_globals OS_DBG_FILEPOS);
#endif

			Scope * start_scope = scope;
			for(; scope; scope = scope->parent){
				for(int i = scope->locals.count-1; i >= 0; i--){
					const Scope::LocalVar& local_var = scope->locals[i];
					if(local_var.name.toChar()[0] == OS_TEXT('#')){
						continue;
					}
					bool found = false;
					for(int j = 0; j < vars.count; j++){
						if(vars[j] == local_var.name){
							found = true;
							break;
						}
					}
					if(found){
						continue;
					}
					allocator->vectorAddItem(vars, local_var.name OS_DBG_FILEPOS);

					TokenData * name_token = new (malloc(sizeof(TokenData) OS_DBG_FILEPOS)) TokenData(tokenizer->getTextData(), local_var.name, 
						Tokenizer::NAME, exp->token->line, exp->token->pos);

					Expression * var_exp = new (malloc(sizeof(Expression) OS_DBG_FILEPOS)) Expression(EXP_TYPE_GET_LOCAL_VAR, name_token);
					OS_ASSERT(scope->function);
					var_exp->active_locals = scope->function->num_locals;
					var_exp->ret_values = 1;
					found = findLocalVar(var_exp->local_var, start_scope, local_var.name, start_scope->function->num_locals, true);
					OS_ASSERT(found); // && var_exp->local_var.index == local_var.index);
					if(start_scope->function->max_up_count < var_exp->local_var.up_count){
						start_scope->function->max_up_count = var_exp->local_var.up_count;
					}

					Expression * obj_item_exp = new (malloc(sizeof(Expression) OS_DBG_FILEPOS)) Expression(EXP_TYPE_OBJECT_SET_BY_NAME, name_token, var_exp OS_DBG_FILEPOS);
					allocator->vectorInsertAtIndex(obj_exp->list, 0, obj_item_exp OS_DBG_FILEPOS);

					name_token->release();
				}
			}
			allocator->vectorClear(vars);
			obj_exp->ret_values = 1;

			scope = start_scope;
			exp->list.add(obj_exp OS_DBG_FILEPOS);
		}
		break;

	case EXP_TYPE_PARAMS:
		{
			for(int i = exp->list.count-1; i >= 0; i--){
				Expression * sub_exp = postCompilePass2(scope, exp->list[i]);
				if(sub_exp->type == EXP_TYPE_PARAMS){
					// OS_ASSERT(false);
					ExpressionList list(allocator);
					int j;
					for(j = 0; j < i; j++){
						list.add(exp->list[j] OS_DBG_FILEPOS);
					}
					for(j = 0; j < sub_exp->list.count; j++){
						list.add(sub_exp->list[j] OS_DBG_FILEPOS);
					}
					for(j = i+1; j < exp->list.count; j++){
						list.add(exp->list[j] OS_DBG_FILEPOS);
					}
					exp->ret_values += sub_exp->ret_values;
					list.swap(exp->list);
					allocator->vectorClear(list);
					allocator->vectorClear(sub_exp->list);
					allocator->deleteObj(sub_exp);
				}else{
					exp->list[i] = sub_exp;
				}
			}
			return exp;
		}

	case EXP_TYPE_POST_INC:
	case EXP_TYPE_POST_DEC:
		OS_ASSERT(exp->list.count == 1);
		if(exp->ret_values > 0){
			OS_ASSERT(exp->ret_values == 1);
			exp->list[0] = postCompilePass2(scope, exp->list[0]);

			Expression * var_exp = exp->list[0];
			OS_ASSERT(var_exp->type == EXP_TYPE_GET_LOCAL_VAR);

			String temp_var_name = String(allocator, OS_TEXT("#temp")); // + String(allocator, (OS_INT)scope->function->num_locals+1);
			TokenData * temp_var_token = new (malloc(sizeof(TokenData) OS_DBG_FILEPOS)) TokenData(tokenizer->getTextData(), temp_var_name, Tokenizer::NAME, exp->token->line, exp->token->pos);

			TokenData * num_token = new (malloc(sizeof(TokenData) OS_DBG_FILEPOS)) TokenData(tokenizer->getTextData(), String(allocator, OS_TEXT("1")), Tokenizer::NUMBER, exp->token->line, exp->token->pos);
			num_token->setFloat(1);

			Expression * cur_var_exp = new (malloc(sizeof(Expression) OS_DBG_FILEPOS)) Expression(EXP_TYPE_GET_LOCAL_VAR, var_exp->token);
			cur_var_exp->ret_values = 1;
			cur_var_exp->local_var = var_exp->local_var;

			Expression * result_exp = new (malloc(sizeof(Expression) OS_DBG_FILEPOS)) Expression(EXP_TYPE_CODE_LIST, exp->token);
			Expression * copy_exp = new (malloc(sizeof(Expression) OS_DBG_FILEPOS)) Expression(EXP_TYPE_SET_LOCAL_VAR, temp_var_token, cur_var_exp OS_DBG_FILEPOS);
			OS_ASSERT(!findLocalVar(copy_exp->local_var, scope, temp_var_name, scope->function->num_locals, false));
			scope->addLocalVar(temp_var_name, copy_exp->local_var);
			result_exp->list.add(copy_exp OS_DBG_FILEPOS);

			cur_var_exp = new (malloc(sizeof(Expression) OS_DBG_FILEPOS)) Expression(EXP_TYPE_GET_LOCAL_VAR, var_exp->token);
			cur_var_exp->ret_values = 1;
			cur_var_exp->local_var = var_exp->local_var;

			Expression * num_exp = new (malloc(sizeof(Expression) OS_DBG_FILEPOS)) Expression(EXP_TYPE_CONST_NUMBER, num_token);
			num_exp->ret_values = 1;

			Expression * op_exp = new (malloc(sizeof(Expression) OS_DBG_FILEPOS)) Expression(exp->type == EXP_TYPE_POST_INC ? EXP_TYPE_ADD : EXP_TYPE_SUB, exp->token, cur_var_exp, num_exp OS_DBG_FILEPOS);
			op_exp->ret_values = 1;

			Expression * set_exp = new (malloc(sizeof(Expression) OS_DBG_FILEPOS)) Expression(EXP_TYPE_SET_LOCAL_VAR, var_exp->token, op_exp OS_DBG_FILEPOS);
			set_exp->local_var = var_exp->local_var;

			result_exp->list.add(set_exp OS_DBG_FILEPOS);

			Expression * get_temp_var_exp = new (malloc(sizeof(Expression) OS_DBG_FILEPOS)) Expression(EXP_TYPE_GET_LOCAL_VAR, temp_var_token);
			get_temp_var_exp->ret_values = 1;
			get_temp_var_exp->local_var = copy_exp->local_var;

			result_exp->list.add(get_temp_var_exp OS_DBG_FILEPOS);
			result_exp->ret_values = 1;

			temp_var_token->release();
			num_token->release();

			allocator->deleteObj(exp);
			return postCompilePass2(scope, result_exp);
		}
		exp->type = exp->type == EXP_TYPE_POST_INC ? EXP_TYPE_PRE_INC : EXP_TYPE_PRE_DEC;
		// no break

	case EXP_TYPE_PRE_INC:
	case EXP_TYPE_PRE_DEC:
		{
			OS_ASSERT(exp->list.count == 1);
			exp->list[0] = postCompilePass2(scope, exp->list[0]);

			Expression * var_exp = exp->list[0];
			OS_ASSERT(var_exp->type == EXP_TYPE_GET_LOCAL_VAR);

			TokenData * num_token = new (malloc(sizeof(TokenData) OS_DBG_FILEPOS)) TokenData(tokenizer->getTextData(), String(allocator, OS_TEXT("1")), Tokenizer::NUMBER, exp->token->line, exp->token->pos);
			num_token->setFloat(1);

			Expression * cur_var_exp = new (malloc(sizeof(Expression) OS_DBG_FILEPOS)) Expression(EXP_TYPE_GET_LOCAL_VAR, var_exp->token);
			cur_var_exp->ret_values = 1;
			cur_var_exp->local_var = var_exp->local_var;

			Expression * num_exp = new (malloc(sizeof(Expression) OS_DBG_FILEPOS)) Expression(EXP_TYPE_CONST_NUMBER, num_token);
			num_exp->ret_values = 1;

			Expression * op_exp = new (malloc(sizeof(Expression) OS_DBG_FILEPOS)) Expression(exp->type == EXP_TYPE_PRE_INC ? EXP_TYPE_ADD : EXP_TYPE_SUB, exp->token, cur_var_exp, num_exp OS_DBG_FILEPOS);
			op_exp->ret_values = 1;

			Expression * set_exp = new (malloc(sizeof(Expression) OS_DBG_FILEPOS)) Expression(EXP_TYPE_SET_LOCAL_VAR, var_exp->token, op_exp OS_DBG_FILEPOS);
			set_exp->local_var = var_exp->local_var;

			Expression * result_exp = new (malloc(sizeof(Expression) OS_DBG_FILEPOS)) Expression(EXP_TYPE_CODE_LIST, exp->token);
			result_exp->list.add(set_exp OS_DBG_FILEPOS);

			if(exp->ret_values > 0){
				OS_ASSERT(exp->ret_values == 1);

				cur_var_exp = new (malloc(sizeof(Expression) OS_DBG_FILEPOS)) Expression(EXP_TYPE_GET_LOCAL_VAR, var_exp->token);
				cur_var_exp->ret_values = 1;
				cur_var_exp->local_var = var_exp->local_var;

				result_exp->list.add(cur_var_exp OS_DBG_FILEPOS);
				result_exp->ret_values = 1;
			}
			allocator->deleteObj(exp);
			num_token->release();
			return postCompilePass2(scope, result_exp);
		}

	case EXP_TYPE_NAME:
		if(findLocalVar(exp->local_var, scope, exp->token->str, exp->active_locals, true)){
			exp->type = EXP_TYPE_GET_LOCAL_VAR;
			if(scope->function->max_up_count < exp->local_var.up_count){
				scope->function->max_up_count = exp->local_var.up_count;
			}
		}else{
			exp->type = EXP_TYPE_GET_ENV_VAR;
		}
		break;

	case EXP_TYPE_RETURN:
#ifdef OS_TAIL_CALL_ENABLED
		if(exp->list.count == 1){
			Expression * sub_exp = exp->list[0] = postCompilePass2(scope, exp->list[0]);
			switch(sub_exp->type){
			case EXP_TYPE_CALL:
			case EXP_TYPE_CALL_AUTO_PARAM:
				sub_exp->type = EXP_TYPE_TAIL_CALL;
				allocator->vectorClear(exp->list);
				allocator->deleteObj(exp);
				return sub_exp;

			case EXP_TYPE_CALL_METHOD:
				sub_exp->type = EXP_TYPE_TAIL_CALL_METHOD;
				allocator->vectorClear(exp->list);
				allocator->deleteObj(exp);
				return sub_exp;
			}
			return exp;
		}
#endif
		break;

	case EXP_TYPE_CALL:
	case EXP_TYPE_CALL_AUTO_PARAM:
		{
			OS_ASSERT(exp->list.count == 2);
			exp->list[0] = postCompilePass2(scope, exp->list[0]);
			exp->list[1] = postCompilePass2(scope, exp->list[1]);
			Expression * left_exp = exp->list[0];
			Expression * right_exp = exp->list[1];
			if(left_exp->type == EXP_TYPE_GET_PROPERTY){
				OS_ASSERT(left_exp->list.count == 2);
				OS_ASSERT(right_exp->type == EXP_TYPE_PARAMS);
				allocator->vectorInsertAtIndex(right_exp->list, 0, left_exp->list[1] OS_DBG_FILEPOS);
				right_exp->ret_values += left_exp->list[1]->ret_values;
				left_exp->list[1] = right_exp;
				left_exp->type = EXP_TYPE_CALL_METHOD;
				left_exp->ret_values = exp->ret_values;
				allocator->vectorClear(exp->list);
				allocator->deleteObj(exp);
				return left_exp;
			}
			if(left_exp->type == EXP_TYPE_GET_ENV_VAR){
				OS_ASSERT(left_exp->list.count == 0);
				OS_ASSERT(right_exp->type == EXP_TYPE_PARAMS);
				left_exp->type = EXP_TYPE_CONST_STRING;
				allocator->vectorInsertAtIndex(right_exp->list, 0, left_exp OS_DBG_FILEPOS);
				right_exp->ret_values++;

				TokenData * name_token = new (malloc(sizeof(TokenData) OS_DBG_FILEPOS)) TokenData(tokenizer->getTextData(), 
					allocator->core->strings->var_env, 
					Tokenizer::NAME, left_exp->token->line, left_exp->token->pos);

				left_exp = new (malloc(sizeof(Expression) OS_DBG_FILEPOS)) Expression(EXP_TYPE_GET_LOCAL_VAR, name_token);
				left_exp->ret_values = 1;
				if(!findLocalVar(left_exp->local_var, scope, name_token->str, scope->function->num_params + POST_VAR_ENV+1, true)){
					OS_ASSERT(false);
				};
				if(scope->function->max_up_count < left_exp->local_var.up_count){
					scope->function->max_up_count = left_exp->local_var.up_count;
				}
				exp->list[0] = left_exp;

				name_token->release();

				exp->type = EXP_TYPE_CALL_METHOD;
				return exp;
			}
			return exp;
		}

	case EXP_TYPE_SET_DIM:
		{
			OS_ASSERT(exp->list.count == 3);
			exp->list[0] = postCompilePass2(scope, exp->list[0]);
			exp->list[1] = postCompilePass2(scope, exp->list[1]);
			exp->list[2] = postCompilePass2(scope, exp->list[2]);
			Expression * params = exp->list[2];
			if(params->list.count == 1){
				exp->list[2] = params->list[0];
				allocator->vectorClear(params->list);
				allocator->deleteObj(params);
				exp->type = EXP_TYPE_SET_PROPERTY;
				for(Expression * get_exp = exp->list[1];;){
					switch(get_exp->type){
					case EXP_TYPE_GET_PROPERTY:
						OS_ASSERT(get_exp->list.count == 2);
						get_exp->type = EXP_TYPE_GET_PROPERTY_AUTO_CREATE;
						get_exp = get_exp->list[0];
						continue;

					case EXP_TYPE_GET_LOCAL_VAR:
						get_exp->type = EXP_TYPE_GET_LOCAL_VAR_AUTO_CREATE;
						break;

					case EXP_TYPE_GET_ENV_VAR:
						get_exp->type = EXP_TYPE_GET_ENV_VAR_AUTO_CREATE;
						break;
					}
					break;
				}
				return exp;
			}
			if(params->list.count == 0){
				// nop
			}
			break;
		}

	case EXP_TYPE_SET_PROPERTY:
		{
			OS_ASSERT(exp->list.count == 3);
			exp->list[0] = postCompilePass2(scope, exp->list[0]);
			exp->list[1] = postCompilePass2(scope, exp->list[1]);
			exp->list[2] = postCompilePass2(scope, exp->list[2]);
			for(Expression * get_exp = exp->list[1];;){
				switch(get_exp->type){
				case EXP_TYPE_GET_PROPERTY:
					OS_ASSERT(get_exp->list.count == 2);
					get_exp->type = EXP_TYPE_GET_PROPERTY_AUTO_CREATE;
					get_exp = get_exp->list[0];
					continue;

				case EXP_TYPE_GET_LOCAL_VAR:
					get_exp->type = EXP_TYPE_GET_LOCAL_VAR_AUTO_CREATE;
					break;

				case EXP_TYPE_GET_ENV_VAR:
					get_exp->type = EXP_TYPE_GET_ENV_VAR_AUTO_CREATE;
					break;
				}
				break;
			}
			return exp;
		}

	case EXP_TYPE_CALL_DIM:
		{
			OS_ASSERT(exp->list.count == 2);
			exp->list[0] = postCompilePass2(scope, exp->list[0]);
			exp->list[1] = postCompilePass2(scope, exp->list[1]);
			Expression * name_exp = exp->list[0];
			Expression * params = exp->list[1];
			OS_ASSERT(params->type == EXP_TYPE_PARAMS);
			if(params->list.count == 1){
				exp->list[1] = params->list[0];
				allocator->vectorClear(params->list);
				allocator->deleteObj(params);
				exp->type = EXP_TYPE_GET_PROPERTY;
			}else{
				// exp->type = EXP_TYPE_GET_DIM;
				String method_name = !params->list.count ? allocator->core->strings->__getempty : allocator->core->strings->__getdim;
				TokenData * token = new (malloc(sizeof(TokenData) OS_DBG_FILEPOS)) TokenData(tokenizer->getTextData(), method_name, Tokenizer::NAME, name_exp->token->line, name_exp->token->pos);
				Expression * exp_method_name = new (malloc(sizeof(Expression) OS_DBG_FILEPOS)) Expression(EXP_TYPE_CONST_STRING, token);
				exp_method_name->ret_values = 1;
				token->release();

				allocator->vectorInsertAtIndex(params->list, 0, exp_method_name OS_DBG_FILEPOS);
				params->ret_values++;

				exp->type = EXP_TYPE_CALL_METHOD;
			}
			return exp;
		}

	case EXP_TYPE_INDIRECT:
		{
			OS_ASSERT(exp->list.count == 2);
			exp->list[0] = expectExpressionValues(exp->list[0], 1);
			exp->list[1] = expectExpressionValues(exp->list[1], 1);
			// Expression * left_exp = exp->list[0];
			Expression * right_exp = exp->list[1];
			ExpressionType exp_type = EXP_TYPE_GET_PROPERTY;
			switch(right_exp->type){
			case EXP_TYPE_NAME:
				right_exp->type = EXP_TYPE_CONST_STRING;
				break;

			case EXP_TYPE_CALL:
			case EXP_TYPE_CALL_AUTO_PARAM:
				right_exp->type = EXP_TYPE_PARAMS;
				exp_type = EXP_TYPE_CALL_METHOD;
				OS_ASSERT(right_exp->list.count == 2);
				if(right_exp->list[0]->type == EXP_TYPE_NAME){
					OS_ASSERT(right_exp->list[0]->ret_values == 1);
					right_exp->list[0]->type = EXP_TYPE_CONST_STRING;
				}
				break;
			}
			exp->type = exp_type;
			break;
		}
	}
	for(int i = 0; i < exp->list.count; i++){
		exp->list[i] = postCompilePass2(scope, exp->list[i]);
	}
	return exp;
}

OS::Core::Compiler::Expression * OS::Core::Compiler::postCompileExpression(Scope * scope, Expression * exp)
{
	exp = postCompilePass2(scope, exp);
#if 0
	return exp;
#elif 1
	OS_ASSERT(scope->type == EXP_TYPE_FUNCTION);
	exp = postCompilePass3(scope, exp);
	exp = postCompileNewVM(scope, exp);
	OS_ASSERT(scope->function->stack_cur_size == scope->function->num_locals || scope->function->stack_cur_size == scope->function->num_locals+1);
	return exp;
#else
	OS_ASSERT(scope->type == EXP_TYPE_FUNCTION);
	// prog_stack_size = 0;
	return postCompilePass3(scope, exp);
#endif
}

OS::Core::Compiler::Expression * OS::Core::Compiler::postCompilePass3(Scope * scope, Expression * exp)
{
	struct Lib {
		static Expression * processList(Compiler * compiler, Scope * scope, Expression * exp)
		{
			for(int i = 0; i < exp->list.count; i++){
				exp->list[i] = compiler->postCompilePass3(scope, exp->list[i]);
			}
			return exp;
		}
	};

	switch(exp->type){
	case EXP_TYPE_FUNCTION:
		{
			Scope * new_scope = dynamic_cast<Scope*>(exp);
			OS_ASSERT(new_scope && (new_scope->parent == scope || (!new_scope->parent && new_scope->type == EXP_TYPE_FUNCTION)));
			scope = new_scope;
			break;
		}

	case EXP_TYPE_SCOPE:
	case EXP_TYPE_LOOP_SCOPE:
		{
			Scope * new_scope = dynamic_cast<Scope*>(exp);
			OS_ASSERT(new_scope && (new_scope->parent == scope || (!new_scope->parent && new_scope->type == EXP_TYPE_FUNCTION)));
			scope = new_scope;
			break;
		}

	case EXP_TYPE_CALL:
	case EXP_TYPE_CALL_AUTO_PARAM:
	case EXP_TYPE_TAIL_CALL:
		{
			OS_ASSERT(exp->list.count == 2);
			Expression * exp_params = exp->list[1];
			OS_ASSERT(exp_params->type == EXP_TYPE_PARAMS);
			Expression * exp_this = new (malloc(sizeof(Expression) OS_DBG_FILEPOS)) Expression(EXP_TYPE_CONST_NULL, exp_params->token);
			exp_this->ret_values = 1;
			allocator->vectorInsertAtIndex(exp_params->list, 0, exp_this OS_DBG_FILEPOS);
			exp_params->ret_values++;
			return Lib::processList(this, scope, exp);
		}

	case EXP_TYPE_GET_ENV_VAR:
	case EXP_TYPE_GET_ENV_VAR_AUTO_CREATE:
		OS_ASSERT(exp->list.count == 0);
		exp->slots.b = cacheString(exp->token->str);
		break;

	case EXP_TYPE_SET_ENV_VAR:
		OS_ASSERT(exp->list.count == 1);
		exp->slots.b = cacheString(exp->token->str);
		break;

	case EXP_TYPE_DELETE:
		OS_ASSERT(exp->list.count == 2);
		exp->slots.b = cacheString(allocator->core->strings->func_delete);
		break;

	case EXP_TYPE_CONCAT:
		OS_ASSERT(exp->list.count == 2);
		exp->slots.b = cacheString(allocator->core->strings->func_concat);
		break;

	case EXP_TYPE_SET_DIM:
		OS_ASSERT(exp->list.count == 3);
		OS_ASSERT(exp->list[2]->type == EXP_TYPE_PARAMS);
		exp->slots.b = cacheString(exp->list[2]->ret_values > 0 ? allocator->core->strings->__setdim : allocator->core->strings->__setempty);
		break;

	case EXP_TYPE_LENGTH:
		OS_ASSERT(exp->list.count == 1);
		exp->slots.b = cacheString(allocator->core->strings->__len);
		break;

	case EXP_TYPE_OBJECT_SET_BY_NAME:
		OS_ASSERT(exp->list.count == 1);
		exp->slots.b = cacheString(exp->token->str);
		break;

	case EXP_TYPE_OBJECT_SET_BY_INDEX:
		OS_ASSERT(exp->list.count == 1);
		exp->slots.b = cacheNumber((OS_NUMBER)exp->token->getFloat());
		break;

	case EXP_TYPE_OBJECT_SET_BY_AUTO_INDEX:
		OS_ASSERT(exp->list.count == 1);
		exp->slots.b = cacheString(allocator->core->strings->func_push);
		break;

	case EXP_TYPE_EXTENDS:
		OS_ASSERT(exp->list.count == 2);
		exp->slots.b = cacheString(allocator->core->strings->func_extends);
		break;

	case EXP_TYPE_IN:
		OS_ASSERT(exp->list.count == 2);
		exp->slots.b = cacheString(allocator->core->strings->func_in);
		break;

	case EXP_TYPE_IS:
		OS_ASSERT(exp->list.count == 2);
		exp->slots.b = cacheString(allocator->core->strings->func_is);
		break;

	case EXP_TYPE_ISPROTOTYPEOF:
		OS_ASSERT(exp->list.count == 2);
		exp->slots.b = cacheString(allocator->core->strings->func_isprototypeof);
		break;

	case EXP_TYPE_CONST_NUMBER:
		OS_ASSERT(exp->list.count == 0);
		exp->slots.b = cacheNumber((OS_NUMBER)exp->token->getFloat());
		break;

	case EXP_TYPE_CONST_STRING:
		OS_ASSERT(exp->list.count == 0);
		exp->slots.b = cacheString(exp->token->str);
		break;

	case EXP_TYPE_CONST_NULL:
		OS_ASSERT(exp->list.count == 0);
		exp->slots.b = CONST_NULL;
		break;

	case EXP_TYPE_CONST_FALSE:
		OS_ASSERT(exp->list.count == 0);
		exp->slots.b = CONST_FALSE;
		break;

	case EXP_TYPE_CONST_TRUE:
		OS_ASSERT(exp->list.count == 0);
		exp->slots.b = CONST_TRUE;
		break;
	}
	return Lib::processList(this, scope, exp);
}
OS::Core::Compiler::Expression * OS::Core::Compiler::postCompileNewVM(Scope * scope, Expression * exp)
{
	struct Lib {
		static Expression * processList(Compiler * compiler, Scope * scope, Expression * exp)
		{
			for(int i = 0; i < exp->list.count; i++){
				exp->list[i] = compiler->postCompileNewVM(scope, exp->list[i]);
			}
			return exp;
		}
		
		static bool allowOverrideOpcodeResult(Expression * exp)
		{
			switch(exp->type){
			case EXP_TYPE_FUNCTION:
			case EXP_TYPE_MOVE:
			case EXP_TYPE_CONST_NUMBER:
			case EXP_TYPE_CONST_STRING:
			case EXP_TYPE_CONST_NULL:
			case EXP_TYPE_CONST_FALSE:
			case EXP_TYPE_CONST_TRUE:
			case EXP_TYPE_CONCAT:
			case EXP_TYPE_LOGIC_PTR_EQ:
			case EXP_TYPE_LOGIC_PTR_NE:
			case EXP_TYPE_LOGIC_EQ:
			case EXP_TYPE_LOGIC_NE:
			case EXP_TYPE_LOGIC_GE:
			case EXP_TYPE_LOGIC_LE:
			case EXP_TYPE_LOGIC_GREATER:
			case EXP_TYPE_LOGIC_LESS:
			case EXP_TYPE_BIT_AND:
			case EXP_TYPE_BIT_OR:
			case EXP_TYPE_BIT_XOR:
			case EXP_TYPE_COMPARE: // <=>
			case EXP_TYPE_ADD: // +
			case EXP_TYPE_SUB: // -
			case EXP_TYPE_MUL: // *
			case EXP_TYPE_DIV: // /
			case EXP_TYPE_MOD: // %
			case EXP_TYPE_LSHIFT: // <<
			case EXP_TYPE_RSHIFT: // >>
			case EXP_TYPE_POW: // **
			case EXP_TYPE_GET_UPVALUE:
			case EXP_TYPE_GET_PROPERTY:
				return true;
			}
			return false;
		}

		static Expression * addXconst(Compiler * compiler, Expression * exp, Expression * xconst)
		{
			if(xconst){
				return new (compiler->malloc(sizeof(Expression) OS_DBG_FILEPOS)) Expression(EXP_TYPE_CODE_LIST, exp->token, xconst, exp OS_DBG_FILEPOS);
			}
			return exp;
		}
	};
	Expression * exp1, * exp2, * exp_xconst;
	int stack_pos, b;
	switch(exp->type){
	default:
		OS_ASSERT(false);
		break;

	case EXP_TYPE_DEBUGGER:
		break;

	case EXP_TYPE_FUNCTION:
		{
			Scope * new_scope = dynamic_cast<Scope*>(exp);
			OS_ASSERT(new_scope && (new_scope->parent == scope || (!new_scope->parent && new_scope->type == EXP_TYPE_FUNCTION)));
			scope = new_scope;
			scope->slots.a = scope->parent ? scope->parent->allocTempVar() : 0;
			scope->slots.b = scope->prog_func_index;
			break;
		}

	case EXP_TYPE_SCOPE:
	case EXP_TYPE_LOOP_SCOPE:
		{
			Scope * new_scope = dynamic_cast<Scope*>(exp);
			OS_ASSERT(new_scope && (new_scope->parent == scope || (!new_scope->parent && new_scope->type == EXP_TYPE_FUNCTION)));
			scope = new_scope;
			break;
		}

	case EXP_TYPE_PARAMS:
	case EXP_TYPE_MOVE:
	case EXP_TYPE_NEW_LOCAL_VAR:
	case EXP_TYPE_BREAK:
	case EXP_TYPE_CONTINUE:
	case EXP_TYPE_NOP:
		break;

	case EXP_TYPE_CODE_LIST:
		{
			stack_pos = scope->function->stack_cur_size;
			exp = Lib::processList(this, scope, exp);
			while(stack_pos + exp->ret_values > scope->function->stack_cur_size){
				scope->allocTempVar();
			}
			return exp;
		}

	case EXP_TYPE_POP_VALUE:
		exp = Lib::processList(this, scope, exp);
		scope->popTempVar();
		return exp;

	case EXP_TYPE_VALUE:
		break;

	case EXP_TYPE_RETURN:
		stack_pos = scope->function->stack_cur_size;
		exp = Lib::processList(this, scope, exp);
		exp->slots.a = stack_pos;
		exp->slots.b = scope->function->stack_cur_size - stack_pos;
		exp->slots.c = 0;
		OS_ASSERT(exp->slots.b == exp->ret_values);
		scope->function->stack_cur_size = stack_pos;
		if(exp->slots.b == 1){
			OS_ASSERT(exp->list.count == 1);
			exp1 = exp->list[0];
			if(exp1->type == EXP_TYPE_MOVE && exp1->slots.b >= 0){
				exp->slots.a = exp1->slots.b;
				exp1->type = EXP_TYPE_NOP;
			}
		}
		return exp;

	case EXP_TYPE_CALL:
	case EXP_TYPE_CALL_AUTO_PARAM:
	case EXP_TYPE_TAIL_CALL:
		OS_ASSERT(exp->list.count == 2);
		OS_ASSERT(exp->list[1]->type == EXP_TYPE_PARAMS);
		exp1 = exp->list[0];
		if(exp1->type == EXP_TYPE_SUPER){
			exp1->type = EXP_TYPE_CONST_NULL;
			exp->type = EXP_TYPE_SUPER_CALL;
		}
		// no break

	case EXP_TYPE_CALL_METHOD:
	case EXP_TYPE_TAIL_CALL_METHOD:
		stack_pos = scope->function->stack_cur_size;
		exp = Lib::processList(this, scope, exp);
		exp->slots.a = stack_pos;
		exp->slots.b = scope->function->stack_cur_size - stack_pos;
		exp->slots.c = exp->ret_values;
		scope->function->stack_cur_size = stack_pos + exp->ret_values;
		if(scope->function->stack_size < scope->function->stack_cur_size){
			scope->function->stack_size = scope->function->stack_cur_size;
		}
		return exp;

	case EXP_TYPE_IF:
		OS_ASSERT(exp->list.count == 2 || exp->list.count == 3);
		stack_pos = scope->function->stack_cur_size;
		exp->list[0] = exp1 = postCompileNewVM(scope, exp->list[0]);
		OS_ASSERT(stack_pos+1 == scope->function->stack_cur_size);
		scope->popTempVar();
		exp->list[1] = exp1 = postCompileNewVM(scope, exp->list[1]);
		OS_ASSERT(stack_pos == scope->function->stack_cur_size);
		if(exp->list.count == 3){
			exp->list[2] = exp1 = postCompileNewVM(scope, exp->list[2]);
			OS_ASSERT(stack_pos == scope->function->stack_cur_size);
		}
		OS_ASSERT(stack_pos == scope->function->stack_cur_size);
		return exp;

	case EXP_TYPE_QUESTION:
		OS_ASSERT(exp->list.count == 3);
		stack_pos = scope->function->stack_cur_size;
		
		exp->list[0] = exp1 = postCompileNewVM(scope, exp->list[0]);
		OS_ASSERT(stack_pos+1 == scope->function->stack_cur_size);
		scope->popTempVar();
		
		exp->list[1] = exp1 = postCompileNewVM(scope, exp->list[1]);
		OS_ASSERT(stack_pos+1 == scope->function->stack_cur_size);
		scope->popTempVar();
		
		exp->list[2] = exp1 = postCompileNewVM(scope, exp->list[2]);
		OS_ASSERT(stack_pos+1 == scope->function->stack_cur_size);
		return exp;

	case EXP_TYPE_DEBUG_LOCALS:
		OS_ASSERT(exp->list.count == 1);
		exp1 = postCompileNewVM(scope, exp->list[0]);
		allocator->vectorClear(exp->list);
		allocator->deleteObj(exp);
		return exp1;

	case EXP_TYPE_ARRAY:
	case EXP_TYPE_OBJECT:
		exp->slots.a = scope->allocTempVar();
		break;

	case EXP_TYPE_OBJECT_SET_BY_NAME:
		OS_ASSERT(exp->list.count == 1);

		stack_pos = scope->allocTempVar();
		b = -1 - exp->slots.b - prog_numbers.count - CONST_STD_VALUES; // const string
		if(b < -OS_MAX_GENERIC_CONST_INDEX){
			exp_xconst = new (malloc(sizeof(Expression) OS_DBG_FILEPOS)) Expression(EXP_TYPE_GET_XCONST, exp->token);
			exp_xconst->slots.b = b;
			exp_xconst->slots.a = b = stack_pos;
		}else exp_xconst = NULL;

		exp->type = EXP_TYPE_SET_PROPERTY;
		OS_ASSERT(scope->function->stack_cur_size > scope->function->num_locals);
		exp->slots.a = scope->function->stack_cur_size-2;
		exp->slots.b = b;
		exp->list[0] = exp1 = postCompileNewVM(scope, exp->list[0]);
		exp->slots.c = exp1->slots.a;
		scope->popTempVar(2);
		if(exp1->type == EXP_TYPE_MOVE){
			exp->slots.c = exp1->slots.b;
			exp1->type = EXP_TYPE_NOP;
		}
		return Lib::addXconst(this, exp, exp_xconst);

	case EXP_TYPE_OBJECT_SET_BY_INDEX:
		OS_ASSERT(exp->list.count == 1);

		stack_pos = scope->allocTempVar();
		b = -1 - exp->slots.b - CONST_STD_VALUES; // const index
		if(b < -OS_MAX_GENERIC_CONST_INDEX){
			exp_xconst = new (malloc(sizeof(Expression) OS_DBG_FILEPOS)) Expression(EXP_TYPE_GET_XCONST, exp->token);
			exp_xconst->slots.b = b;
			exp_xconst->slots.a = b = stack_pos;
		}else exp_xconst = NULL;

		exp->type = EXP_TYPE_SET_PROPERTY;
		OS_ASSERT(scope->function->stack_cur_size > scope->function->num_locals);
		exp->slots.a = scope->function->stack_cur_size-2;
		exp->slots.b = b;
		exp->list[0] = exp1 = postCompileNewVM(scope, exp->list[0]);
		exp->slots.c = exp1->slots.a;
		scope->popTempVar(2);
		if(exp1->type == EXP_TYPE_MOVE){
			exp->slots.c = exp1->slots.b;
			exp1->type = EXP_TYPE_NOP;
		}
		return Lib::addXconst(this, exp, exp_xconst);

	case EXP_TYPE_OBJECT_SET_BY_EXP:
		OS_ASSERT(exp->list.count == 2);
		exp->type = EXP_TYPE_SET_PROPERTY;
		OS_ASSERT(scope->function->stack_cur_size > scope->function->num_locals);
		exp->slots.a = scope->function->stack_cur_size-1;
		exp->list[0] = exp1 = postCompileNewVM(scope, exp->list[0]);
		exp->list[1] = exp2 = postCompileNewVM(scope, exp->list[1]);
		exp->slots.b = exp1->slots.a;
		exp->slots.c = exp2->slots.a;
		scope->popTempVar(2);
		if(exp1->type == EXP_TYPE_MOVE){
			exp->slots.b = exp1->slots.b;
			exp1->type = EXP_TYPE_NOP;
		}
		if(exp2->type == EXP_TYPE_MOVE){
			exp->slots.c = exp2->slots.b;
			exp2->type = EXP_TYPE_NOP;
		}
		return exp;

	case EXP_TYPE_OBJECT_SET_BY_AUTO_INDEX:
		stack_pos = scope->function->stack_cur_size;

		exp->type = EXP_TYPE_CALL_METHOD;
		exp->ret_values = 0;

		exp1 = new (malloc(sizeof(Expression) OS_DBG_FILEPOS)) Expression(EXP_TYPE_PARAMS, exp->token);
		exp1->list.swap(exp->list);
		exp1->ret_values = 1;

		exp2 = new (malloc(sizeof(Expression) OS_DBG_FILEPOS)) Expression(EXP_TYPE_MOVE, exp->token);
		exp2->slots.a = scope->allocTempVar();
		exp2->slots.b = scope->function->stack_cur_size-2;
		exp->list.add(exp2 OS_DBG_FILEPOS);
		exp->list.add(exp1 OS_DBG_FILEPOS);

		exp2 = new (malloc(sizeof(Expression) OS_DBG_FILEPOS)) Expression(EXP_TYPE_MOVE, exp->token);
		exp2->slots.a = scope->allocTempVar();
		exp2->slots.b = -1 - exp->slots.b - prog_numbers.count - CONST_STD_VALUES; // const string
		allocator->vectorInsertAtIndex(exp1->list, 0, exp2 OS_DBG_FILEPOS);

		if(exp2->slots.b < -OS_MAX_GENERIC_CONST_INDEX){
			exp2->type = EXP_TYPE_GET_XCONST;
		}

		exp1->list[1] = postCompileNewVM(scope, exp1->list[1]);
		OS_ASSERT(scope->function->stack_cur_size - stack_pos == 3);
		exp->slots.a = stack_pos;
		exp->slots.b = 3;
		exp->slots.c = 0;
		scope->function->stack_cur_size = stack_pos;
		return exp;

	case EXP_TYPE_CONST_NUMBER:
		OS_ASSERT(exp->list.count == 0);
		exp->slots.a = scope->allocTempVar();
		exp->slots.b = -1 - exp->slots.b - CONST_STD_VALUES;
		exp->type = exp->slots.b < -OS_MAX_GENERIC_CONST_INDEX ? EXP_TYPE_GET_XCONST : EXP_TYPE_MOVE;
		return exp;

	case EXP_TYPE_CONST_STRING:
		OS_ASSERT(exp->list.count == 0);
		exp->slots.a = scope->allocTempVar();
		exp->slots.b = -1 - exp->slots.b - prog_numbers.count - CONST_STD_VALUES;
		exp->type = exp->slots.b < -OS_MAX_GENERIC_CONST_INDEX ? EXP_TYPE_GET_XCONST : EXP_TYPE_MOVE;
		return exp;

	case EXP_TYPE_CONST_NULL:
		OS_ASSERT(exp->list.count == 0);
		exp->slots.a = scope->allocTempVar();
		exp->slots.b = -1 - CONST_NULL;
		exp->type = EXP_TYPE_MOVE;
		return exp;

	case EXP_TYPE_CONST_FALSE:
		OS_ASSERT(exp->list.count == 0);
		exp->slots.a = scope->allocTempVar();
		exp->slots.b = -1 - CONST_FALSE;
		exp->type = EXP_TYPE_MOVE;
		return exp;

	case EXP_TYPE_CONST_TRUE:
		OS_ASSERT(exp->list.count == 0);
		exp->slots.a = scope->allocTempVar();
		exp->slots.b = -1 - CONST_TRUE;
		exp->type = EXP_TYPE_MOVE;
		return exp;

	case EXP_TYPE_GET_THIS:
		OS_ASSERT(exp->list.count == 0);
		exp->slots.a = scope->allocTempVar();
		exp->slots.b = PRE_VAR_THIS;
		exp->type = EXP_TYPE_MOVE;
		return exp;

	case EXP_TYPE_GET_ARGUMENTS:
	case EXP_TYPE_GET_REST_ARGUMENTS:
		OS_ASSERT(exp->list.count == 0);
		exp->slots.a = scope->allocTempVar();
		return exp;

	case EXP_TYPE_LOGIC_NOT: // !
		OS_ASSERT(exp->list.count == 1);
		stack_pos = scope->function->stack_cur_size;
		exp1 = postCompileNewVM(scope, exp->list[0]);
		OS_ASSERT(stack_pos+1 == scope->function->stack_cur_size);
		if(exp1->type == EXP_TYPE_LOGIC_NOT){
			OS_ASSERT(exp1->list.count == 1);
			exp->type = EXP_TYPE_LOGIC_BOOL;
			exp->list[0] = exp1->list[0];
			allocator->vectorRemoveAtIndex(exp1->list, 0);
			allocator->deleteObj(exp1);
			return exp;
		}
		if(exp1->type == EXP_TYPE_LOGIC_BOOL){
			OS_ASSERT(exp1->list.count == 1);
			exp->list[0] = exp1->list[0];
			allocator->vectorRemoveAtIndex(exp1->list, 0);
			allocator->deleteObj(exp1);
			return exp;
		}
		exp->list[0] = exp1;
		OS_ASSERT(exp1->ret_values == 1);
		exp->slots.a = stack_pos;
		exp->slots.b = stack_pos;
		return exp;

	case EXP_TYPE_LOGIC_BOOL: // !
		OS_ASSERT(exp->list.count == 1);
		stack_pos = scope->function->stack_cur_size;
		exp1 = postCompileNewVM(scope, exp->list[0]);
		OS_ASSERT(stack_pos+1 == scope->function->stack_cur_size);
		if(exp1->type == EXP_TYPE_LOGIC_NOT || exp1->type == EXP_TYPE_LOGIC_BOOL){
			allocator->vectorRemoveAtIndex(exp->list, 0);
			allocator->deleteObj(exp);
			return exp1;
		}
		exp->list[0] = exp1;
		OS_ASSERT(exp1->ret_values == 1);
		exp->slots.a = stack_pos;
		exp->slots.b = stack_pos;
		return exp;

	case EXP_TYPE_CONCAT:
		OS_ASSERT(exp->list.count == 2 && exp->ret_values == 1);
		if(exp->list[0]->type == EXP_TYPE_CONCAT){
			stack_pos = scope->function->stack_cur_size;
			exp1 = postCompileNewVM(scope, exp->list[0]);
			OS_ASSERT(stack_pos+1 == scope->function->stack_cur_size);
			OS_ASSERT(exp1->type == EXP_TYPE_CALL_METHOD);
			OS_ASSERT(exp1->list.count == 2);
			scope->function->stack_cur_size = exp1->slots.a + exp1->slots.b;
  			exp2 = postCompileNewVM(scope, exp->list[1]);
			OS_ASSERT(exp2->ret_values == 1);
			OS_ASSERT(exp1->list[1]->type == EXP_TYPE_PARAMS);
			exp1->list[1]->list.add(exp2 OS_DBG_FILEPOS);
			exp1->slots.b += exp2->ret_values;
			scope->function->stack_cur_size = stack_pos+1;
			allocator->vectorClear(exp->list);
			allocator->deleteObj(exp);
			return exp1;
		}
		// no break

	case EXP_TYPE_EXTENDS:
	case EXP_TYPE_IN:
	case EXP_TYPE_IS:
	case EXP_TYPE_ISPROTOTYPEOF:
	case EXP_TYPE_DELETE:
		OS_ASSERT(exp->list.count == 2 && (exp->ret_values == 1 || (exp->ret_values == 0 && exp->type == EXP_TYPE_DELETE)));
		stack_pos = scope->function->stack_cur_size;

		exp1 = new (malloc(sizeof(Expression) OS_DBG_FILEPOS)) Expression(EXP_TYPE_PARAMS, exp->token);
		exp1->list.swap(exp->list);
		exp1->ret_values = 2;

		exp2 = new (malloc(sizeof(Expression) OS_DBG_FILEPOS)) Expression(EXP_TYPE_MOVE, exp->token);
		exp2->slots.a = scope->allocTempVar();
		exp2->slots.b = scope->function->num_params + POST_VAR_GLOBALS;
		exp2->ret_values = 1;
		exp->list.add(exp2 OS_DBG_FILEPOS);

		exp2 = new (malloc(sizeof(Expression) OS_DBG_FILEPOS)) Expression(EXP_TYPE_MOVE, exp->token);
		exp2->slots.a = scope->allocTempVar();
		exp2->slots.b = -1 - exp->slots.b - prog_numbers.count - CONST_STD_VALUES;
		exp2->ret_values = 1;
		allocator->vectorInsertAtIndex(exp1->list, 0, exp2 OS_DBG_FILEPOS);

		if(exp2->slots.b < -OS_MAX_GENERIC_CONST_INDEX){
			exp2->type = EXP_TYPE_GET_XCONST;
		}

		OS_ASSERT(exp1->list.count == 3);
		exp1->list[1] = postCompileNewVM(scope, exp1->list[1]);
		exp1->list[2] = postCompileNewVM(scope, exp1->list[2]);
		OS_ASSERT(stack_pos+4 == scope->function->stack_cur_size);

		exp->list.add(exp1 OS_DBG_FILEPOS); // params

		exp->type = EXP_TYPE_CALL_METHOD;
		exp->slots.a = stack_pos;
		exp->slots.b = scope->function->stack_cur_size - stack_pos;
		exp->slots.c = exp->ret_values;
		scope->function->stack_cur_size = stack_pos + exp->ret_values;
		return exp;

	case EXP_TYPE_SET_DIM:
		{
			OS_ASSERT(exp->list.count == 3);
			stack_pos = scope->function->stack_cur_size;
			exp->list[0] = postCompileNewVM(scope, exp->list[0]);
			OS_ASSERT(stack_pos+1 <= scope->function->stack_cur_size);

			stack_pos = scope->function->stack_cur_size;
			exp->list[1] = postCompileNewVM(scope, exp->list[1]);
			OS_ASSERT(stack_pos+1 <= scope->function->stack_cur_size);

			exp1 = new (malloc(sizeof(Expression) OS_DBG_FILEPOS)) Expression(EXP_TYPE_CODE_LIST, exp->token);
			exp1->list.swap(exp->list);
			exp->list.add(exp1 OS_DBG_FILEPOS);

			exp2 = exp1->list[2];
			OS_ASSERT(exp2->type == EXP_TYPE_PARAMS);
			allocator->vectorRemoveAtIndex(exp1->list, 2);
			exp->list.add(exp2 OS_DBG_FILEPOS);

			exp1 = new (malloc(sizeof(Expression) OS_DBG_FILEPOS)) Expression(EXP_TYPE_MOVE, exp->token);
			exp1->slots.a = scope->allocTempVar();
			exp1->slots.b = -1 - exp->slots.b - prog_numbers.count - CONST_STD_VALUES;
			exp1->ret_values = 1;
			allocator->vectorInsertAtIndex(exp2->list, 0, exp1 OS_DBG_FILEPOS);

			if(exp2->slots.b < -OS_MAX_GENERIC_CONST_INDEX){
				exp2->type = EXP_TYPE_GET_XCONST;
			}

			exp1 = new (malloc(sizeof(Expression) OS_DBG_FILEPOS)) Expression(EXP_TYPE_MOVE, exp->token);
			exp1->slots.a = scope->allocTempVar();
			exp1->slots.b = scope->function->stack_cur_size - 4;
			exp1->ret_values = 1;
			allocator->vectorInsertAtIndex(exp2->list, 1, exp1 OS_DBG_FILEPOS);

			for(int i = 2; i < exp2->list.count; i++){
				exp2->list[i] = postCompileNewVM(scope, exp2->list[i]);
			}
			exp->type = EXP_TYPE_CALL_METHOD;
			exp->slots.a = stack_pos;
			exp->slots.b = scope->function->stack_cur_size - stack_pos;
			exp->slots.c = 0;
			scope->function->stack_cur_size = stack_pos - 1;
			return exp;
		}

	case EXP_TYPE_LENGTH: // #
		OS_ASSERT(exp->list.count == 1);
		stack_pos = scope->function->stack_cur_size;
		exp = Lib::processList(this, scope, exp);
		OS_ASSERT(stack_pos+1 == scope->function->stack_cur_size);

		exp1 = new (malloc(sizeof(Expression) OS_DBG_FILEPOS)) Expression(EXP_TYPE_PARAMS, exp->token);
		exp1->ret_values = 1;
		
		exp2 = new (malloc(sizeof(Expression) OS_DBG_FILEPOS)) Expression(EXP_TYPE_MOVE, exp->token);
		exp2->slots.a = scope->allocTempVar();
		exp2->slots.b = -1 - exp->slots.b - prog_numbers.count - CONST_STD_VALUES;
		exp2->ret_values = 1;
		exp1->list.add(exp2 OS_DBG_FILEPOS);
		exp->list.add(exp1 OS_DBG_FILEPOS);

		if(exp2->slots.b < -OS_MAX_GENERIC_CONST_INDEX){
			exp2->type = EXP_TYPE_GET_XCONST;
		}

		exp->type = EXP_TYPE_CALL_METHOD;
		exp->slots.a = stack_pos;
		exp->slots.b = scope->function->stack_cur_size - stack_pos;
		exp->slots.c = exp->ret_values;
		scope->function->stack_cur_size = stack_pos + exp->ret_values;
		return exp;

	case EXP_TYPE_BIT_NOT:		// ~
	case EXP_TYPE_PLUS:			// +
	case EXP_TYPE_NEG:			// -
		OS_ASSERT(exp->list.count == 1);
		stack_pos = scope->function->stack_cur_size;
		exp = Lib::processList(this, scope, exp);
		OS_ASSERT(stack_pos+1 == scope->function->stack_cur_size);
		exp->slots.a = stack_pos;
		exp->slots.b = stack_pos;
		exp1 = exp->list[0];
		if(exp1->type == EXP_TYPE_MOVE){
			exp->slots.b = exp1->slots.b;
			exp1->type = EXP_TYPE_NOP;
		}		
		return exp;

	case EXP_TYPE_LOGIC_AND:
	case EXP_TYPE_LOGIC_OR:
		OS_ASSERT(exp->list.count == 2);
		stack_pos = scope->function->stack_cur_size;
		
		exp->list[0] = exp1 = postCompileNewVM(scope, exp->list[0]);
		OS_ASSERT(stack_pos+1 == scope->function->stack_cur_size);
		scope->popTempVar();

		exp->list[1] = exp2 = postCompileNewVM(scope, exp->list[1]);
		scope->function->stack_cur_size = stack_pos+1;
		
		exp->slots.a = stack_pos;
		return exp;

	// case EXP_TYPE_CONCAT:
	case EXP_TYPE_BIT_AND:
	case EXP_TYPE_BIT_OR:
	case EXP_TYPE_BIT_XOR:
	case EXP_TYPE_COMPARE:
	case EXP_TYPE_ADD: // +
	case EXP_TYPE_SUB: // -
	case EXP_TYPE_MUL: // *
	case EXP_TYPE_DIV: // /
	case EXP_TYPE_MOD: // %
	case EXP_TYPE_LSHIFT: // <<
	case EXP_TYPE_RSHIFT: // >>
	case EXP_TYPE_POW: // **
		OS_ASSERT(exp->list.count == 2);
		stack_pos = scope->function->stack_cur_size;
		exp = Lib::processList(this, scope, exp);
		exp1 = exp->list[0];
		exp2 = exp->list[1];
		OS_ASSERT(stack_pos+2 == scope->function->stack_cur_size);
		exp->slots.a = stack_pos;
		exp->slots.b = stack_pos;
		exp->slots.c = stack_pos+1;
		scope->popTempVar();
		if(exp1->type == EXP_TYPE_MOVE){
			exp->slots.b = exp1->slots.b;
			exp1->type = EXP_TYPE_NOP;
		}
		if(exp2->type == EXP_TYPE_MOVE){
			exp->slots.c = exp2->slots.b;
			exp2->type = EXP_TYPE_NOP;
		}
		return exp;

	case EXP_TYPE_LOGIC_PTR_EQ:
	case EXP_TYPE_LOGIC_PTR_NE:
	case EXP_TYPE_LOGIC_EQ:
	case EXP_TYPE_LOGIC_NE:
	case EXP_TYPE_LOGIC_GE:
	case EXP_TYPE_LOGIC_LE:
	case EXP_TYPE_LOGIC_GREATER:
	case EXP_TYPE_LOGIC_LESS:
		OS_ASSERT(exp->list.count == 2);
		stack_pos = scope->function->stack_cur_size;
		exp = Lib::processList(this, scope, exp);
		OS_ASSERT(stack_pos+2 == scope->function->stack_cur_size);
		exp->slots.a = stack_pos;
		exp->slots.b = 0;
		exp->slots.c = 0;
		scope->popTempVar();
		return exp;

	case EXP_TYPE_GET_LOCAL_VAR:
	case EXP_TYPE_GET_LOCAL_VAR_AUTO_CREATE:
		OS_ASSERT(exp->list.count == 0);
		if(exp->local_var.up_count){
			exp->type = EXP_TYPE_GET_UPVALUE;
			exp->slots.a = scope->allocTempVar();
			exp->slots.b = exp->local_var.index;
			exp->slots.c = exp->local_var.up_count;
		}else{
			exp->type = EXP_TYPE_MOVE;
			exp->slots.a = scope->allocTempVar();
			exp->slots.b = exp->local_var.index;
		}
		return exp;

	case EXP_TYPE_SET_LOCAL_VAR:
		OS_ASSERT(exp->list.count == 1);
		stack_pos = scope->function->stack_cur_size;
		exp = Lib::processList(this, scope, exp);
		OS_ASSERT(stack_pos < scope->function->stack_cur_size);
		if(exp->local_var.up_count){
			exp->type = EXP_TYPE_SET_UPVALUE;
			exp->slots.a = exp->local_var.index;
			exp->slots.b = --scope->function->stack_cur_size;
			exp->slots.c = exp->local_var.up_count;
		}else{
			exp->type = EXP_TYPE_MOVE;
			exp->slots.a = exp->local_var.index;
			exp->slots.b = --scope->function->stack_cur_size;
		}
		exp1 = exp->list[0];
		if(exp1->type == EXP_TYPE_PARAMS){
			OS_ASSERT(exp1->list.count > 0);
			exp1 = exp1->list.lastElement();
		}
		if(exp1->type == EXP_TYPE_MOVE && exp1->slots.a >= scope->function->num_locals){ // stack_cur_size is already decremented
			exp->slots.b = exp1->slots.b;
			exp1->type = EXP_TYPE_NOP;
			return exp;
		}
		if(exp->type == EXP_TYPE_MOVE 
			&& Lib::allowOverrideOpcodeResult(exp1)
			&& exp1->slots.a >= scope->function->num_locals
			)
		{
			exp1->slots.a = exp->slots.a;
			exp->type = EXP_TYPE_NOP;
		}
		return exp;

	case EXP_TYPE_SET_PROPERTY:
		OS_ASSERT(exp->list.count == 3);
		stack_pos = scope->function->stack_cur_size;
		exp = Lib::processList(this, scope, exp);
		OS_ASSERT(stack_pos+3 <= scope->function->stack_cur_size);
		stack_pos = scope->function->stack_cur_size - 3;
		exp->slots.a = stack_pos + 1;
		exp->slots.b = stack_pos + 2;
		exp->slots.c = stack_pos;
		scope->function->stack_cur_size = stack_pos;
		exp1 = exp->list[0];
		if(exp1->type == EXP_TYPE_MOVE){
			exp->slots.c = exp1->slots.b;
			exp1->type = EXP_TYPE_NOP;
		}
		exp1 = exp->list[1];
		if(exp1->type == EXP_TYPE_MOVE){
			exp->slots.a = exp1->slots.b;
			exp1->type = EXP_TYPE_NOP;
		}
		exp1 = exp->list[2];
		if(exp1->type == EXP_TYPE_MOVE){
			exp->slots.b = exp1->slots.b;
			exp1->type = EXP_TYPE_NOP;
		}
		return exp;

	case EXP_TYPE_GET_ENV_VAR:
	case EXP_TYPE_GET_ENV_VAR_AUTO_CREATE:
		OS_ASSERT(exp->list.count == 0);
		exp->slots.a = scope->allocTempVar();
		exp->slots.c = -1 - exp->slots.b - prog_numbers.count - CONST_STD_VALUES;
		exp->slots.b = scope->function->num_params + POST_VAR_ENV;
		exp->type = EXP_TYPE_GET_PROPERTY;
		
		if(exp->slots.c < -OS_MAX_GENERIC_CONST_INDEX){
			exp_xconst = new (malloc(sizeof(Expression) OS_DBG_FILEPOS)) Expression(EXP_TYPE_GET_XCONST, exp->token);
			exp_xconst->slots.b = exp->slots.c;
			exp_xconst->slots.a = exp->slots.c = exp->slots.a;
			return Lib::addXconst(this, exp, exp_xconst);
		}
		return exp;

	case EXP_TYPE_SET_ENV_VAR:
		OS_ASSERT(exp->list.count == 1);
		stack_pos = scope->function->stack_cur_size;
		exp = Lib::processList(this, scope, exp);
		OS_ASSERT(stack_pos < scope->function->stack_cur_size);
		exp->slots.a = scope->function->num_params + POST_VAR_ENV;
		exp->slots.b = -1 - exp->slots.b - prog_numbers.count - CONST_STD_VALUES;
		exp->slots.c = --scope->function->stack_cur_size;
		exp->type = EXP_TYPE_SET_PROPERTY;

		if(exp->slots.b < -OS_MAX_GENERIC_CONST_INDEX){
			exp_xconst = new (malloc(sizeof(Expression) OS_DBG_FILEPOS)) Expression(EXP_TYPE_GET_XCONST, exp->token);
			exp_xconst->slots.b = exp->slots.b;
			exp_xconst->slots.a = exp->slots.b = exp->slots.c + 1;
			if(exp_xconst->slots.a >= scope->function->stack_size){
				scope->function->stack_size = exp_xconst->slots.a + 1;
			}
			exp->list.add(exp_xconst OS_DBG_FILEPOS);
		}
		exp1 = exp->list[0];
		return exp;

	case EXP_TYPE_GET_PROPERTY_AUTO_CREATE:
		exp->type = EXP_TYPE_GET_PROPERTY;
		// no break

	case EXP_TYPE_GET_PROPERTY:
		OS_ASSERT(exp->list.count == 2);
		stack_pos = scope->function->stack_cur_size;
		exp = Lib::processList(this, scope, exp);
		OS_ASSERT(stack_pos+2 == scope->function->stack_cur_size);
		exp1 = exp->list[0];
		exp2 = exp->list[1];
		exp->slots.a = stack_pos;
		exp->slots.b = stack_pos; // exp1->slots.a;
		exp->slots.c = stack_pos + 1; // exp2->slots.a;
		scope->popTempVar();
		if(exp1->type == EXP_TYPE_MOVE){
			exp->slots.b = exp1->slots.b;
			exp1->type = EXP_TYPE_NOP;
		}
		if(exp2->type == EXP_TYPE_MOVE){
			exp->slots.c = exp2->slots.b;
			exp2->type = EXP_TYPE_NOP;
		}
		return exp;
	}
	return Lib::processList(this, scope, exp);
}

OS::Core::Compiler::Scope * OS::Core::Compiler::expectTextExpression()
{
	OS_ASSERT(recent_token);

	Scope * scope = new (malloc(sizeof(Scope) OS_DBG_FILEPOS)) Scope(NULL, EXP_TYPE_FUNCTION, recent_token);
	scope->parser_started = true;
	scope->ret_values = 1;
	scope->addPreVars();
	scope->num_params = scope->num_locals;
	scope->addPostVars();

	Params p = Params()
		.setAllowAssign(true)
		.setAllowAutoCall(true)
		.setAllowBinaryOperator(true)
		.setAllowParams(true)
		.setAllowRootBlocks(true);

	Expression * exp;
	ExpressionList list(allocator);

	while(!isError()){
		exp = expectSingleExpression(scope, p);
		if(isError()){
			break;
		}
		if(exp){
			list.add(exp OS_DBG_FILEPOS);
		}
		if(!recent_token){
			break;
		}
		TokenType token_type = recent_token->type;
		if(token_type == Tokenizer::CODE_SEPARATOR){
			if(!readToken()){
				break;
			}
			token_type = recent_token->type;
		}
		if(token_type == Tokenizer::END_ARRAY_BLOCK 
			|| token_type == Tokenizer::END_BRACKET_BLOCK
			|| token_type == Tokenizer::END_CODE_BLOCK)
		{
			break;
		}
	}
	if(isError()){
		allocator->deleteObj(scope);
		return NULL;
	}
	if(recent_token){
		setError(ERROR_SYNTAX, recent_token);
		allocator->deleteObj(scope);
		return NULL;
	}
	if(list.count == 0){
		return scope;
	}

	bool ret_eval_value = false;
	int ret_values = list.count == 1 && list[0]->ret_values > 0 && list[0]->type == EXP_TYPE_FUNCTION ? 1 : 0;
	if(!ret_values){
		bool is_eval = tokenizer->getTextData()->filename.getDataSize() == 0;
		if(is_eval){
			exp = newExpressionFromList(list, 1, true);
			ret_eval_value = true;
		}else{		
			putNextTokenType(Tokenizer::CODE_SEPARATOR);
			readToken();

			TokenData * name_token = new (malloc(sizeof(TokenData) OS_DBG_FILEPOS)) TokenData(tokenizer->getTextData(), 
				allocator->core->strings->var_env, 
				Tokenizer::NAME, recent_token->line, recent_token->pos);

			ExpressionList& func_exp_list = ret_values == 1 ? list[0]->list : list;
			Expression * name_exp = new (malloc(sizeof(Expression) OS_DBG_FILEPOS)) Expression(EXP_TYPE_GET_LOCAL_VAR, name_token);
			name_exp->ret_values = 1;
			if(!findLocalVar(name_exp->local_var, scope, allocator->core->strings->var_env, scope->num_locals, false)){
				OS_ASSERT(false);
			}
			OS_ASSERT(name_exp->local_var.up_count == 0);
			Expression * ret_exp = new (malloc(sizeof(Expression) OS_DBG_FILEPOS)) Expression(EXP_TYPE_RETURN, recent_token, name_exp OS_DBG_FILEPOS);
			ret_exp->ret_values = 1;
			func_exp_list.add(ret_exp OS_DBG_FILEPOS);

			name_token->release();

			exp = newExpressionFromList(list, ret_values);
		}
	}else{
		exp = newExpressionFromList(list, ret_values);
	}
	switch(exp->type){
	case EXP_TYPE_CODE_LIST:
		if(!ret_eval_value && exp->list.count == 1 && exp->list[0]->type == EXP_TYPE_FUNCTION){
			allocator->deleteObj(scope);
			scope = dynamic_cast<Scope*>(exp->list[0]);
			allocator->vectorClear(exp->list);
			allocator->deleteObj(exp);
			return scope;
		}
		// exp = expectExpressionValues(exp, 0);
		scope->list.swap(exp->list);
		allocator->deleteObj(exp);
		break;

	case EXP_TYPE_FUNCTION:
		if(!ret_eval_value){
			OS_ASSERT(scope->num_locals == 0);
			allocator->deleteObj(scope);
			scope = dynamic_cast<Scope*>(exp);
			OS_ASSERT(scope);
			scope->parent = NULL;
			return scope;
		}
		// no break

	default:
		scope->list.add(exp OS_DBG_FILEPOS);
	}
	if(ret_eval_value && scope->list.count > 0){
		Expression * last_exp = scope->list.lastElement();
		if(last_exp->ret_values == 1 && last_exp->type != EXP_TYPE_RETURN){
			scope->list.lastElement() = new (malloc(sizeof(Expression) OS_DBG_FILEPOS)) Expression(EXP_TYPE_RETURN, last_exp->token, last_exp OS_DBG_FILEPOS);
			scope->list.lastElement()->ret_values = 1;
		}
	}
	return scope;
}

OS::Core::Compiler::Scope * OS::Core::Compiler::expectCodeExpression(Scope * parent)
{
	OS_ASSERT(recent_token && recent_token->type == Tokenizer::BEGIN_CODE_BLOCK);
	if(!expectToken()){
		allocator->deleteObj(parent);
		return NULL;
	}

	Scope * scope;
	bool is_new_func;
	if(parent->type == EXP_TYPE_FUNCTION && !parent->parser_started){
		scope = parent;
		is_new_func = true;
		parent->parser_started = true;
	}else{
		scope = new (malloc(sizeof(Scope) OS_DBG_FILEPOS)) Scope(parent, EXP_TYPE_SCOPE, recent_token);
		is_new_func = false;
	}

	Params p = Params()
		.setAllowAssign(true)
		.setAllowAutoCall(true)
		.setAllowBinaryOperator(true)
		.setAllowParams(true)
		.setAllowRootBlocks(true);

	Expression * exp;
	ExpressionList list(allocator);
	while(!isError()){
		exp = expectSingleExpression(scope, p);
		if(isError()){
			break;
		}
		if(exp){
			list.add(exp OS_DBG_FILEPOS);
		}
		TokenType token_type = recent_token->type;
		if(token_type == Tokenizer::CODE_SEPARATOR){
			if(!readToken()){
				break;
			}
			token_type = recent_token->type;
		}
		if(token_type == Tokenizer::END_ARRAY_BLOCK 
			|| token_type == Tokenizer::END_BRACKET_BLOCK
			|| token_type == Tokenizer::END_CODE_BLOCK)
		{
			break;
		}
	}
	if(isError()){
		allocator->deleteObj(scope);
		return NULL;
	}
	if(!recent_token || recent_token->type != Tokenizer::END_CODE_BLOCK){
		setError(Tokenizer::END_CODE_BLOCK, recent_token);
		allocator->deleteObj(scope);
		return NULL;
	}
	readToken();

	if(list.count == 0){
		return scope;
	}
	if(is_new_func){
		exp = newExpressionFromList(list, 1, true);
	}else{
		exp = newExpressionFromList(list, 0);
	}
	switch(exp->type){
	case EXP_TYPE_CODE_LIST:
		{
			scope->list.swap(exp->list);
			allocator->deleteObj(exp);
			break;
		}
		// no break

	default:
		scope->list.add(exp OS_DBG_FILEPOS);
	}
	if(is_new_func && scope->list.count > 0){
		Expression * last_exp = scope->list.lastElement();
		if(last_exp->ret_values == 1 && last_exp->type != EXP_TYPE_RETURN){
			scope->list.lastElement() = new (malloc(sizeof(Expression) OS_DBG_FILEPOS)) Expression(EXP_TYPE_RETURN, last_exp->token, last_exp OS_DBG_FILEPOS);
			scope->list.lastElement()->ret_values = 1;
		}
	}
	return scope;
}

OS::Core::Compiler::Expression * OS::Core::Compiler::expectObjectOrFunctionExpression(Scope * scope, const Params& org_p, bool allow_finish_exp)
{
	OS_ASSERT(recent_token && recent_token->type == Tokenizer::BEGIN_CODE_BLOCK);
	if(!expectToken()){
		return NULL;
	}
	if(recent_token->type == Tokenizer::OPERATOR_BIT_OR || recent_token->type == Tokenizer::OPERATOR_LOGIC_OR){ // {|| - no params
		Expression * exp = expectFunctionSugarExpression(scope); // {|x, x2| x*x2}
		// TODO: process org_p, allow_finish_exp ?
		return allow_finish_exp ? finishValueExpression(scope, exp, org_p) : exp;
		// return exp;
	}
	ungetToken();
	struct Lib {
		Compiler * compiler;
		Expression * obj_exp;

		Expression * finishValue(Scope * scope, const Params& p, bool allow_finish_exp)
		{
			if(!allow_finish_exp){
				return obj_exp;
			}
			return compiler->finishValueExpression(scope, obj_exp, Params(p).setAllowAssign(false).setAllowAutoCall(false));
		}

		void * malloc(int size OS_DBG_FILEPOS_DECL)
		{
			return compiler->malloc(size OS_DBG_FILEPOS_PARAM);
		}

		Lib(Compiler * p_compiler, int active_locals)
		{
			compiler = p_compiler;
			obj_exp = new (malloc(sizeof(Expression) OS_DBG_FILEPOS)) Expression(EXP_TYPE_OBJECT, compiler->recent_token);
			obj_exp->ret_values = 1;
		}

		Expression * error()
		{
			compiler->allocator->deleteObj(obj_exp);
			return NULL;
		}

		Expression * error(ErrorType err, TokenData * token)
		{
			compiler->setError(err, token);
			return error();
		}

		Expression * error(TokenType err, TokenData * token)
		{
			compiler->setError(err, token);
			return error();
		}

	} lib(this, scope->function->num_locals);

	Params p = Params().setAllowBinaryOperator(true);

	for(readToken();;){
		Expression * exp = NULL;
		if(!recent_token){
			return lib.error(ERROR_SYNTAX, recent_token);
		}
		if(recent_token->type == Tokenizer::END_CODE_BLOCK){
			readToken();
			return lib.finishValue(scope, org_p, allow_finish_exp);
		}
		TokenData * name_token = recent_token;
		if(name_token->type == Tokenizer::BEGIN_ARRAY_BLOCK){
			readToken();
			TokenData * save_token = recent_token;
			exp = expectSingleExpression(scope, p);
			if(!exp){
				return lib.error();
			}
			if(exp->ret_values < 1){
				allocator->deleteObj(exp);
				return lib.error(ERROR_EXPECT_VALUE, save_token);
			}
			exp = expectExpressionValues(exp, 1);
			if(!recent_token || recent_token->type != Tokenizer::END_ARRAY_BLOCK){
				allocator->deleteObj(exp);
				return lib.error(Tokenizer::END_ARRAY_BLOCK, recent_token);
			}
			if(!readToken() || (recent_token->type != Tokenizer::OPERATOR_COLON && recent_token->type != Tokenizer::OPERATOR_ASSIGN)){
				allocator->deleteObj(exp);
				return lib.error(Tokenizer::OPERATOR_COLON, recent_token);
			}
			save_token = readToken();
			Expression * exp2 = expectSingleExpression(scope, p);
			if(!exp2){
				return isError() ? lib.error() : lib.error(ERROR_EXPECT_EXPRESSION, save_token);
			}
			exp2 = expectExpressionValues(exp2, 1);
			exp = new (malloc(sizeof(Expression) OS_DBG_FILEPOS)) Expression(EXP_TYPE_OBJECT_SET_BY_EXP, name_token, exp, exp2 OS_DBG_FILEPOS);
		}else if(isNextToken(Tokenizer::OPERATOR_COLON) || isNextToken(Tokenizer::OPERATOR_ASSIGN)){
			ExpressionType exp_type = EXP_TYPE_OBJECT_SET_BY_NAME;
			switch(name_token->type){
			case Tokenizer::STRING:
			case Tokenizer::NAME:
				break;

			case Tokenizer::NUMBER:
				if(name_token->getFloat() != (OS_FLOAT)(OS_INT)name_token->getFloat()){
					// use it as EXP_TYPE_OBJECT_SET_BY_NAME
					break;
				}
				exp_type = EXP_TYPE_OBJECT_SET_BY_INDEX;
				break;

			default:
				return lib.error(ERROR_SYNTAX, name_token);
			}
			readToken(); // skip OPERATOR_COLON
			TokenData * save_token = readToken();
			exp = expectSingleExpression(scope, p);
			if(!exp){
				return isError() ? lib.error() : lib.error(ERROR_EXPECT_EXPRESSION, save_token);
			}
			exp = expectExpressionValues(exp, 1);
			exp = new (malloc(sizeof(Expression) OS_DBG_FILEPOS)) Expression(exp_type, name_token, exp OS_DBG_FILEPOS);
		}else{
			exp = expectSingleExpression(scope, p);
			if(!exp){
				return isError() ? lib.error() : lib.error(ERROR_EXPECT_EXPRESSION, name_token);
			}
			exp = expectExpressionValues(exp, 1);
			exp = new (malloc(sizeof(Expression) OS_DBG_FILEPOS)) Expression(EXP_TYPE_OBJECT_SET_BY_AUTO_INDEX, name_token, exp OS_DBG_FILEPOS);
		}
		OS_ASSERT(exp);
		lib.obj_exp->list.add(exp OS_DBG_FILEPOS);
		if(recent_token && recent_token->type == Tokenizer::END_CODE_BLOCK){
			readToken();
			return lib.finishValue(scope, org_p, allow_finish_exp);
		}
#if 11
		if(!recent_token){
			return lib.error(Tokenizer::END_CODE_BLOCK, recent_token);
		}
		switch(recent_token->type){
		case Tokenizer::PARAM_SEPARATOR:
		case Tokenizer::CODE_SEPARATOR:
			readToken();
		}
#else
		if(!recent_token || (recent_token->type != Tokenizer::PARAM_SEPARATOR
			&& recent_token->type != Tokenizer::CODE_SEPARATOR)){
				return lib.error(Tokenizer::PARAM_SEPARATOR, recent_token);
		}
		readToken();
#endif
	}
	return NULL; // shut up compiler
}

OS::Core::Compiler::Expression * OS::Core::Compiler::expectArrayExpression(Scope * scope, const Params& __p)
{
	Params next_p = Params(__p).setAllowAssign(false).setAllowAutoCall(false);
	Expression * params = new (malloc(sizeof(Expression) OS_DBG_FILEPOS)) Expression(EXP_TYPE_ARRAY, recent_token);
	params->ret_values = 1;
	readToken();
	if(recent_token && recent_token->type == Tokenizer::END_ARRAY_BLOCK){
		readToken();
		return finishValueExpression(scope, params, next_p);
	}
	Params p = Params().setAllowBinaryOperator(true);
	for(;;){
		Expression * exp = expectSingleExpression(scope, p);
		if(!exp){
			if(isError()){
				allocator->deleteObj(params);
				return NULL;
			}
			if(!recent_token || recent_token->type != Tokenizer::END_ARRAY_BLOCK){
				setError(Tokenizer::END_ARRAY_BLOCK, recent_token);
				allocator->deleteObj(params);
				return NULL;
			}
			readToken();
			return finishValueExpression(scope, params, next_p);
		}
		exp = expectExpressionValues(exp, 1);
		exp = new (malloc(sizeof(Expression) OS_DBG_FILEPOS)) Expression(EXP_TYPE_OBJECT_SET_BY_AUTO_INDEX, exp->token, exp OS_DBG_FILEPOS);
		params->list.add(exp OS_DBG_FILEPOS);
		if(recent_token && recent_token->type == Tokenizer::END_ARRAY_BLOCK){
			readToken();
			return finishValueExpression(scope, params, next_p);
		}
#if 11
		if(!recent_token){
			setError(Tokenizer::END_ARRAY_BLOCK, recent_token);
			allocator->deleteObj(params);
			return NULL;
		}
		switch(recent_token->type){
		case Tokenizer::PARAM_SEPARATOR:
		case Tokenizer::CODE_SEPARATOR:
			readToken();
		}
#else
		if(!recent_token || (recent_token->type != Tokenizer::PARAM_SEPARATOR
			&& recent_token->type != Tokenizer::CODE_SEPARATOR)){
				setError(Tokenizer::PARAM_SEPARATOR, recent_token);
				allocator->deleteObj(params);
				return NULL;
		}
		readToken();
#endif
	}
	return NULL; // shut up compiler
}

OS::Core::Compiler::Expression * OS::Core::Compiler::expectParamsExpression(Scope * scope)
{
	struct Lib 
	{
		static Expression * calcParamsExpression(Compiler * compiler, Scope * scope, Expression * params)
		{
			if(params->list.count > 1){
				for(int i = 0; i < params->list.count; i++){
					params->list[i] = compiler->expectExpressionValues(params->list[i], 1);
				}
				params->ret_values = params->list.count;
			}else if(params->list.count == 1){
				params->ret_values = params->list[0]->ret_values;
			}
			return params;
		}
	};

	Expression * params = new (malloc(sizeof(Expression) OS_DBG_FILEPOS)) Expression(EXP_TYPE_PARAMS, recent_token);
	bool is_dim = recent_token->type == Tokenizer::BEGIN_ARRAY_BLOCK;
	TokenType end_exp_type = is_dim ? Tokenizer::END_ARRAY_BLOCK : Tokenizer::END_BRACKET_BLOCK;
	readToken();
	if(recent_token && recent_token->type == end_exp_type){
		readToken();
		return Lib::calcParamsExpression(this, scope, params);
	}
	Params p = Params().setAllowBinaryOperator(true);
	for(;;){
		Expression * exp = expectSingleExpression(scope, p);
		if(!exp){
			if(isError()){
				allocator->deleteObj(params);
				return NULL;
			}
			if(!recent_token || recent_token->type != end_exp_type){
				setError(end_exp_type, recent_token);
				allocator->deleteObj(params);
				return NULL;
			}
			readToken();
			return Lib::calcParamsExpression(this, scope, params);
		}
		params->list.add(exp OS_DBG_FILEPOS);
		if(recent_token && (recent_token->type == Tokenizer::PARAM_SEPARATOR || recent_token->type == Tokenizer::CODE_SEPARATOR)){
			readToken();
		}
		if(recent_token && recent_token->type == end_exp_type){
			readToken();
			return Lib::calcParamsExpression(this, scope, params);
		}
		if(!recent_token){
			setError(end_exp_type, recent_token);
			allocator->deleteObj(params);
			return NULL;
		}
	}
	return NULL; // shut up compiler
}

OS::Core::Compiler::Expression * OS::Core::Compiler::expectBracketExpression(Scope * scope, const Params& p)
{
	OS_ASSERT(recent_token && recent_token->type == Tokenizer::BEGIN_BRACKET_BLOCK);
	readToken();
	Expression * exp = expectSingleExpression(scope, Params()
		.setAllowBinaryOperator(true)
		.setAllowCall(true)
		.setAllowAutoCall(true));
	if(!exp){
		return NULL;
	}
	exp = newSingleValueExpression(exp);
	OS_ASSERT(exp->ret_values == 1);
	if(!recent_token){
		setError(Tokenizer::END_BRACKET_BLOCK, recent_token);
		allocator->deleteObj(exp);
		return NULL;
	}
	switch(recent_token->type){
	case Tokenizer::END_BRACKET_BLOCK:
		readToken();
		return finishValueExpression(scope, exp, p);
	}
	setError(Tokenizer::END_BRACKET_BLOCK, recent_token);
	allocator->deleteObj(exp);
	return NULL;
}

OS::Core::Compiler::Expression * OS::Core::Compiler::expectExtendsExpression(Scope * scope)
{
	OS_ASSERT(recent_token && recent_token->str == allocator->core->strings->syntax_extends);
	TokenData * save_token = recent_token;
	if(!expectToken()){
		return NULL;
	}
	Params p;
	Expression * exp = expectSingleExpression(scope, p);
	if(!exp){
		return NULL;
	}
	if(exp->type == EXP_TYPE_CALL_AUTO_PARAM){
		OS_ASSERT(exp->list.count == 2);
		Expression * params = exp->list[1];
		OS_ASSERT(params->type == EXP_TYPE_PARAMS && params->list.count == 1);
		exp->list[1] = params->list[0];
		allocator->vectorClear(params->list);
		allocator->deleteObj(params);
		exp->type = EXP_TYPE_EXTENDS;
		exp->ret_values = 1;
		return exp;
	}
	Expression * exp2 = expectSingleExpression(scope, p);
	if(!exp2){
		allocator->deleteObj(exp);
		return NULL;
	}
	exp = expectExpressionValues(exp, 1);
	exp2 = expectExpressionValues(exp2, 1);
	exp = new (malloc(sizeof(Expression) OS_DBG_FILEPOS)) Expression(EXP_TYPE_EXTENDS, save_token, exp, exp2 OS_DBG_FILEPOS);
	exp->ret_values = 1;
	return exp;
}

OS::Core::Compiler::Expression * OS::Core::Compiler::finishQuestionOperator(Scope * scope, TokenData * token, Expression * exp, Expression * exp2)
{
	ungetToken();
	if(!expectToken(Tokenizer::OPERATOR_COLON)){
		allocator->deleteObj(exp);
		allocator->deleteObj(exp2);
		return NULL;
	}
	if(!expectToken()){
		return NULL;
	}
	Expression * exp3 = expectSingleExpression(scope, Params().setAllowBinaryOperator(true));
	if(!exp3){
		allocator->deleteObj(exp);
		allocator->deleteObj(exp2);
		return NULL;
	}
	exp = expectExpressionValues(exp, 1);
	exp2 = expectExpressionValues(exp2, 1);
	exp3 = expectExpressionValues(exp3, 1);
	exp = new (malloc(sizeof(Expression) OS_DBG_FILEPOS)) Expression(EXP_TYPE_QUESTION, token, exp, exp2, exp3 OS_DBG_FILEPOS);
	exp->ret_values = 1;
	return exp;
}

OS::Core::Compiler::Expression * OS::Core::Compiler::expectDeleteExpression(Scope * scope)
{
	OS_ASSERT(recent_token && recent_token->str == allocator->core->strings->syntax_delete);
	if(!expectToken()){
		return NULL;
	}
	Expression * exp = expectSingleExpression(scope, Params());
	if(!exp){
		return NULL;
	}
	if(exp->type == EXP_TYPE_INDIRECT){
		OS_ASSERT(exp->list.count == 2);
		Expression * field = exp->list[1];
		if(field->type == EXP_TYPE_NAME){
			field->type = EXP_TYPE_CONST_STRING;
		}
		exp->type = EXP_TYPE_DELETE;
		exp->ret_values = 0;
		return exp;
	}
	if(exp->type == EXP_TYPE_CALL_DIM){
		OS_ASSERT(exp->list.count == 2);
		Expression * params = exp->list[1];
		if(params->list.count == 1){
			exp->list[1] = params->list[0];
			allocator->vectorClear(params->list);
			allocator->deleteObj(params);
			exp->type = EXP_TYPE_DELETE;
			exp->ret_values = 0;
			return exp;
		}
		Expression * object = exp->list[0];

		String method_name = !params->list.count ? allocator->core->strings->__delempty : allocator->core->strings->__deldim;
		TokenData * token = new (malloc(sizeof(TokenData) OS_DBG_FILEPOS)) TokenData(tokenizer->getTextData(), method_name, Tokenizer::NAME, object->token->line, object->token->pos);
		Expression * exp_method_name = new (malloc(sizeof(Expression) OS_DBG_FILEPOS)) Expression(EXP_TYPE_CONST_STRING, token);
		exp_method_name->ret_values = 1;
		token->release();

		Expression * indirect = new (malloc(sizeof(Expression) OS_DBG_FILEPOS)) Expression(EXP_TYPE_INDIRECT, object->token, object, exp_method_name OS_DBG_FILEPOS);
		exp->list[0] = indirect;
		exp->type = EXP_TYPE_CALL;
		exp->ret_values = 1;
		return exp;
	}
	setError(ERROR_SYNTAX, exp->token);
	allocator->deleteObj(exp);
	return NULL;
}

OS::Core::Compiler::Expression * OS::Core::Compiler::expectFunctionExpression(Scope * parent)
{
	Scope * scope = new (malloc(sizeof(Scope) OS_DBG_FILEPOS)) Scope(parent, EXP_TYPE_FUNCTION, recent_token);
	scope->function = scope;
	scope->ret_values = 1;
	Expression * name_exp = NULL;
	if(isNextToken(Tokenizer::NAME)){
		TokenData * token = readToken();
		if(isNextToken(Tokenizer::NAME)){
			String prefix(allocator);
			if(token->str == allocator->core->strings->syntax_get){
				prefix = allocator->core->strings->__getAt;
			}else if(token->str == allocator->core->strings->syntax_set){
				prefix = allocator->core->strings->__setAt;
			}else{
				setError(ERROR_EXPECT_GET_OR_SET, token);
				allocator->deleteObj(name_exp);
				allocator->deleteObj(scope);
				return NULL;
			}
			token = readToken();
			token->str = String(allocator, prefix, token->str);
			name_exp = new (malloc(sizeof(Expression) OS_DBG_FILEPOS)) Expression(EXP_TYPE_NAME, token);
		}else{
			name_exp = expectSingleExpression(parent, Params().setAllowCall(false));
			if(!name_exp || !name_exp->isWriteable()){
				setError(ERROR_EXPECT_WRITEABLE, token);
				allocator->deleteObj(name_exp);
				allocator->deleteObj(scope);
				return NULL;
			}
			ungetToken();
		}
	}
	if(!expectToken(Tokenizer::BEGIN_BRACKET_BLOCK)){
		allocator->deleteObj(scope);
		return NULL;
	}
	scope->addPreVars();
	for(;;){
		if(!readToken()){
			setError(ERROR_SYNTAX, recent_token);
			allocator->deleteObj(scope);
			return NULL;
		}
		switch(recent_token->type){
		case Tokenizer::END_BRACKET_BLOCK:
			break;

		case Tokenizer::NAME:
			scope->addLocalVar(recent_token->str);
			if(!readToken()){
				setError(ERROR_SYNTAX, recent_token);
				allocator->deleteObj(scope);
				return NULL;
			}
			if(recent_token->type == Tokenizer::END_BRACKET_BLOCK){
				break;
			}
			if(recent_token->type == Tokenizer::PARAM_SEPARATOR){
				continue;
			}
#if 11
			ungetToken();
			continue;
#else
			setError(ERROR_SYNTAX, recent_token);
			allocator->deleteObj(scope);
			return NULL;
#endif

		default:
			setError(ERROR_SYNTAX, recent_token);
			allocator->deleteObj(scope);
			return NULL;
		}
		break;
	}
	scope->num_params = scope->num_locals;
	OS_ASSERT(recent_token && recent_token->type == Tokenizer::END_BRACKET_BLOCK);
	if(!expectToken(Tokenizer::BEGIN_CODE_BLOCK)){
		allocator->deleteObj(scope);
		return NULL;
	}
	scope->addPostVars();
	scope = expectCodeExpression(scope);
	if(!scope || !name_exp){
		return scope;
	}
	return newBinaryExpression(parent, EXP_TYPE_ASSIGN, name_exp->token, name_exp, scope);
}

OS::Core::Compiler::Expression * OS::Core::Compiler::expectFunctionSugarExpression(Scope * parent)
{
	OS_ASSERT(recent_token && (recent_token->type == Tokenizer::OPERATOR_BIT_OR || recent_token->type == Tokenizer::OPERATOR_LOGIC_OR));
	Scope * scope = new (malloc(sizeof(Scope) OS_DBG_FILEPOS)) Scope(parent, EXP_TYPE_FUNCTION, recent_token);
	scope->function = scope;
	scope->ret_values = 1;
	scope->addPreVars();
	if(recent_token->type == Tokenizer::OPERATOR_BIT_OR)
	for(;;){
		if(!readToken()){
			setError(ERROR_SYNTAX, recent_token);
			allocator->deleteObj(scope);
			return NULL;
		}
		switch(recent_token->type){
		case Tokenizer::OPERATOR_BIT_OR:
			break;

		case Tokenizer::NAME:
			scope->addLocalVar(recent_token->str);
			if(!readToken()){
				setError(ERROR_SYNTAX, recent_token);
				allocator->deleteObj(scope);
				return NULL;
			}
			if(recent_token->type == Tokenizer::OPERATOR_BIT_OR){
				break;
			}
			if(recent_token->type == Tokenizer::PARAM_SEPARATOR){
				continue;
			}
#if 11
			ungetToken();
			continue;
#else
			setError(ERROR_SYNTAX, recent_token);
			allocator->deleteObj(scope);
			return NULL;
#endif

		default:
			setError(ERROR_SYNTAX, recent_token);
			allocator->deleteObj(scope);
			return NULL;
		}
		break;
	}
	scope->num_params = scope->num_locals;
	OS_ASSERT(recent_token && (recent_token->type == Tokenizer::OPERATOR_BIT_OR || recent_token->type == Tokenizer::OPERATOR_LOGIC_OR));
	scope->addPostVars();

	if(!expectToken()){
		allocator->deleteObj(scope);
		return NULL;
	}
	scope->parser_started = true;

	Params p = Params()
		.setAllowAssign(true)
		.setAllowAutoCall(true)
		.setAllowBinaryOperator(true)
		.setAllowParams(true)
		.setAllowRootBlocks(true);

	Expression * exp;
	ExpressionList list(allocator);
	while(!isError()){
		exp = expectSingleExpression(scope, p);
		if(isError()){
			break;
		}
		if(exp){
			list.add(exp OS_DBG_FILEPOS);
		}
		TokenType token_type = recent_token->type;
		if(token_type == Tokenizer::CODE_SEPARATOR){
			if(!readToken()){
				break;
			}
			token_type = recent_token->type;
		}
		if(token_type == Tokenizer::END_ARRAY_BLOCK 
			|| token_type == Tokenizer::END_BRACKET_BLOCK
			|| token_type == Tokenizer::END_CODE_BLOCK)
		{
			break;
		}
	}
	if(isError()){
		allocator->deleteObj(scope);
		return NULL;
	}
	if(!recent_token || recent_token->type != Tokenizer::END_CODE_BLOCK){
		setError(Tokenizer::END_CODE_BLOCK, recent_token);
		allocator->deleteObj(scope);
		return NULL;
	}
	readToken();

	if(list.count == 0){
		return scope;
	}
	exp = newExpressionFromList(list, 1, true);
	switch(exp->type){
	case EXP_TYPE_CODE_LIST:
		scope->list.swap(exp->list);
		allocator->deleteObj(exp);
		break;

	default:
		scope->list.add(exp OS_DBG_FILEPOS);
	}
	if(scope->list.count > 0){
		Expression * last_exp = scope->list.lastElement();
		if(last_exp->ret_values == 1 && last_exp->type != EXP_TYPE_RETURN){
			scope->list.lastElement() = new (malloc(sizeof(Expression) OS_DBG_FILEPOS)) Expression(EXP_TYPE_RETURN, last_exp->token, last_exp OS_DBG_FILEPOS);
			scope->list.lastElement()->ret_values = 1;
		}
	}
	return scope;
}

OS::Core::Compiler::Expression * OS::Core::Compiler::expectVarExpression(Scope * scope)
{
	OS_ASSERT(recent_token && recent_token->str == allocator->core->strings->syntax_var);
	if(!expectToken(Tokenizer::NAME)){
		return NULL;
	}
	Expression * name_exp;
	Expression * exp = new (malloc(sizeof(Expression) OS_DBG_FILEPOS)) Expression(EXP_TYPE_PARAMS, recent_token);
	exp->ret_values = 1;
	if(recent_token->str == allocator->core->strings->syntax_function){
		if(!expectToken(Tokenizer::NAME)){
			allocator->deleteObj(exp);
			return NULL;
		}
		TokenData * name_token;
		if(recent_token->str == allocator->core->strings->syntax_get || recent_token->str == allocator->core->strings->syntax_set){
			bool is_getter = recent_token->str == allocator->core->strings->syntax_get;
			if(!expectToken(Tokenizer::NAME)){
				allocator->deleteObj(exp);
				return NULL;
			}
			if(!isVarNameValid(recent_token->str)){
				setError(ERROR_VAR_NAME, recent_token);
				allocator->deleteObj(exp);
				return NULL;
			}
			if(!expectToken(Tokenizer::BEGIN_BRACKET_BLOCK)){
				allocator->deleteObj(exp);
				return NULL;
			}
			ungetToken();
			ungetToken();

			name_token = tokenizer->removeToken(next_token_index-1); name_token->release();
			name_token = tokenizer->removeToken(next_token_index-1);
			if(is_getter){
				name_token->str = String(allocator, allocator->core->strings->__getAt, name_token->str);
			}else{
				name_token->str = String(allocator, allocator->core->strings->__setAt, name_token->str);
			}
		}else{
			if(!isVarNameValid(recent_token->str)){
				setError(ERROR_VAR_NAME, recent_token);
				allocator->deleteObj(exp);
				return NULL;
			}
			if(!expectToken(Tokenizer::BEGIN_BRACKET_BLOCK)){
				allocator->deleteObj(exp);
				return NULL;
			}
			ungetToken();

			name_token = tokenizer->removeToken(next_token_index-1);
		}

		name_exp = new (malloc(sizeof(Expression) OS_DBG_FILEPOS)) Expression(EXP_TYPE_NAME, name_token);
		name_exp->ret_values = 1;
		name_token->release();

		allocator->deleteObj(exp);

		ungetToken(); // return to function

		Expression * func_exp = expectFunctionExpression(scope);
		if(!func_exp){
			allocator->deleteObj(exp);
			return NULL;
		}
		OS_ASSERT(func_exp->type == EXP_TYPE_FUNCTION);
		exp = newBinaryExpression(scope, EXP_TYPE_ASSIGN, name_exp->token, name_exp, func_exp);
	}else{
		for(;;){
			if(!isVarNameValid(recent_token->str)){
				setError(ERROR_VAR_NAME, recent_token);
				allocator->deleteObj(exp);
				return NULL;
			}
			name_exp = new (malloc(sizeof(Expression) OS_DBG_FILEPOS)) Expression(EXP_TYPE_NAME, recent_token);
			name_exp->ret_values = 1;

			exp->list.add(name_exp OS_DBG_FILEPOS);
			exp->ret_values++;

			if(!readToken() || recent_token->type != Tokenizer::PARAM_SEPARATOR){
				break;
			}
			if(!expectToken(Tokenizer::NAME)){
				allocator->deleteObj(exp);
				return NULL;
			}
		}

		if(recent_token && recent_token->type == Tokenizer::OPERATOR_ASSIGN){
			bool is_finished;
			exp = finishBinaryOperator(scope, getOpcodeLevel(exp->type), exp, Params().setAllowParams(true).setAllowInOperator(true), is_finished);
			OS_ASSERT(is_finished);
		}
	}
	Expression * ret_exp = exp;
	while(exp){
		switch(exp->type){
		case EXP_TYPE_PARAMS:
			{
				Expression * params = exp;
				for(int i = 0; i < params->list.count; i++){
					exp = params->list[i];
					OS_ASSERT(exp->type == EXP_TYPE_NAME);
					if(exp->type == EXP_TYPE_NAME){
						if(findLocalVar(exp->local_var, scope, exp->token->str, exp->active_locals, false)){
						}else{
							scope->addLocalVar(exp->token->str, exp->local_var);
						}
						OS_ASSERT(exp->local_var.up_count == 0);
						exp->type = EXP_TYPE_NEW_LOCAL_VAR;
						exp->ret_values = 0;
					}
				}
				params->ret_values = 0;
				return params;
			}

		case EXP_TYPE_SET_LOCAL_VAR:
			for(;;){
				if(exp->local_var.up_scope_count == 0){
				}else{
					OS_ASSERT(!findLocalVar(exp->local_var, scope, exp->token->str, exp->active_locals, false));
					scope->addLocalVar(exp->token->str, exp->local_var);
				}
				OS_ASSERT(exp->list.count == 1);
				exp = exp->list[0];
				switch(exp->type){
				case EXP_TYPE_SET_ENV_VAR:
				case EXP_TYPE_SET_LOCAL_VAR:
					break;

				default:
					return ret_exp;
				}
				break;
			}
			break;

		case EXP_TYPE_SET_ENV_VAR:
			for(;;){
				OS_ASSERT(!findLocalVar(exp->local_var, scope, exp->token->str, exp->active_locals, false));
				scope->addLocalVar(exp->token->str, exp->local_var);
				exp->type = EXP_TYPE_SET_LOCAL_VAR;
				OS_ASSERT(exp->list.count == 1);
				exp = exp->list[0];
				switch(exp->type){
				case EXP_TYPE_SET_ENV_VAR:
				case EXP_TYPE_SET_LOCAL_VAR:
					break;

				default:
					return ret_exp;
				}
				break;
			}
			break;

		case EXP_TYPE_NAME:
			if(findLocalVar(exp->local_var, scope, exp->token->str, exp->active_locals, false)){
			}else{
				scope->addLocalVar(exp->token->str, exp->local_var);
			}
			OS_ASSERT(exp->local_var.up_count == 0);
			exp->type = EXP_TYPE_NEW_LOCAL_VAR;
			exp->ret_values = 0;
			return ret_exp;

		default:
			return ret_exp;
		}
	}
	return ret_exp;
}

OS::Core::Compiler::Expression * OS::Core::Compiler::expectForExpression(Scope * parent)
{
	OS_ASSERT(recent_token && recent_token->str == allocator->core->strings->syntax_for);

	Scope * scope = new (malloc(sizeof(Scope) OS_DBG_FILEPOS)) Scope(parent, EXP_TYPE_SCOPE, recent_token);
	if(!expectToken(Tokenizer::BEGIN_BRACKET_BLOCK) || !expectToken()){
		allocator->deleteObj(scope);
		return NULL;
	}
	Expression * exp = expectSingleExpression(scope, Params()
		.setAllowAssign(true)
		.setAllowAutoCall(true)
		.setAllowBinaryOperator(true)
		.setAllowParams(true)
		.setAllowVarDecl(true)
		.setAllowNopResult(true)
		.setAllowInOperator(false));

	if(!exp){
		allocator->deleteObj(scope);
		return NULL;
	}
	if(!recent_token){
		setError(ERROR_EXPECT_TOKEN, recent_token);
		allocator->deleteObj(scope);
		allocator->deleteObj(exp);
		return NULL;
	}	
	if(recent_token->type == Tokenizer::NAME && (exp->type == EXP_TYPE_PARAMS || exp->type == EXP_TYPE_NEW_LOCAL_VAR || exp->type == EXP_TYPE_NAME)){
		if(recent_token->str != allocator->core->strings->syntax_in){
			setError(allocator->core->strings->syntax_in, recent_token);
			allocator->deleteObj(scope);
			allocator->deleteObj(exp);
			return NULL;
		}
		ExpressionList vars(allocator);
		if(exp->type == EXP_TYPE_PARAMS){
			vars.swap(exp->list);
			allocator->deleteObj(exp);
		}else{
			vars.add(exp OS_DBG_FILEPOS);
		}
		exp = NULL;
		for(int i = 0; i < vars.count; i++){
			OS_ASSERT(vars[i]->type == EXP_TYPE_NAME || vars[i]->type == EXP_TYPE_NEW_LOCAL_VAR);
			Expression * name_exp = vars[i];
			if(name_exp->type == EXP_TYPE_NAME){
				name_exp->type = EXP_TYPE_NOP;
			}
		}
		if(!expectToken()){
			allocator->deleteObj(scope);
			return NULL;
		}
		exp = expectSingleExpression(scope, Params().setAllowBinaryOperator(true).setAllowAutoCall(true)); // true, false, false, false, true);
		if(!recent_token || recent_token->type != Tokenizer::END_BRACKET_BLOCK){
			setError(Tokenizer::END_BRACKET_BLOCK, recent_token);
			allocator->deleteObj(scope);
			allocator->deleteObj(exp);
			return NULL;
		}
		if(!exp->ret_values){
			setError(ERROR_EXPECT_VALUE, exp->token);
			allocator->deleteObj(scope);
			allocator->deleteObj(exp);
			return NULL;
		}
		exp = expectExpressionValues(exp, 1);
		if(!expectToken()){
			allocator->deleteObj(scope);
			allocator->deleteObj(exp);
			return NULL;
		}
		Scope * loop_scope = new (malloc(sizeof(Scope) OS_DBG_FILEPOS)) Scope(scope, EXP_TYPE_LOOP_SCOPE, recent_token);
		Expression * body_exp = expectSingleExpression(loop_scope, true, true);
		if(!body_exp){
			allocator->deleteObj(scope);
			allocator->deleteObj(exp);
			allocator->deleteObj(loop_scope);
			return NULL;
		}
		body_exp = expectExpressionValues(body_exp, 0);

		exp = new (malloc(sizeof(Expression) OS_DBG_FILEPOS)) Expression(EXP_TYPE_CALL_METHOD, exp->token, exp OS_DBG_FILEPOS);
		{
			Expression * params = new (malloc(sizeof(Expression) OS_DBG_FILEPOS)) Expression(EXP_TYPE_PARAMS, exp->token);

			String method_name = allocator->core->strings->__iter;
			TokenData * token = new (malloc(sizeof(TokenData) OS_DBG_FILEPOS)) TokenData(tokenizer->getTextData(), method_name, Tokenizer::NAME, exp->token->line, exp->token->pos);
			Expression * exp_method_name = new (malloc(sizeof(Expression) OS_DBG_FILEPOS)) Expression(EXP_TYPE_CONST_STRING, token);
			exp_method_name->ret_values = 1;
			token->release();

			params->list.add(exp_method_name OS_DBG_FILEPOS);
			params->ret_values = 1;
			exp->list.add(params OS_DBG_FILEPOS);
		}
		exp = expectExpressionValues(exp, vars.count + 1);
		int num_locals = vars.count;

		const int temp_count = 2;
		const OS_CHAR * temp_names[temp_count] = {
			OS_TEXT("#func"), OS_TEXT("#valid")
		};
		for(int i = 0; i < temp_count; i++){
			String name(allocator, temp_names[i]);
			TokenData * token = new (malloc(sizeof(TokenData) OS_DBG_FILEPOS)) TokenData(tokenizer->getTextData(), name, Tokenizer::NAME, exp->token->line, exp->token->pos);
			Expression * name_exp = new (malloc(sizeof(Expression) OS_DBG_FILEPOS)) Expression(EXP_TYPE_NEW_LOCAL_VAR, token);
			vars.add(name_exp OS_DBG_FILEPOS);
			token->release();

			scope->addLocalVar(name, name_exp->local_var);
			OS_ASSERT(scope->function);
			name_exp->active_locals = scope->function->num_locals;
			name_exp->local_var.type = LOCAL_TEMP;
		}

		ExpressionList list(allocator);

		// OS: var func, state, state2 = (in_exp).__iter()
		{
			Expression * params = new (malloc(sizeof(Expression) OS_DBG_FILEPOS)) Expression(EXP_TYPE_PARAMS, exp->token);
			for(int i = num_locals; i < vars.count-1; i++){
				Expression * var_exp = vars[i];
				Expression * name_exp = new (malloc(sizeof(Expression) OS_DBG_FILEPOS)) Expression(EXP_TYPE_NAME, var_exp->token);
				OS_ASSERT(scope->function);
				name_exp->active_locals = scope->function->num_locals;
				name_exp->ret_values = 1;
				params->list.add(name_exp OS_DBG_FILEPOS);
			}
			params->ret_values = params->list.count;

			String assing_operator(allocator, OS_TEXT("="));
			TokenData * assign_token = new (malloc(sizeof(TokenData) OS_DBG_FILEPOS)) TokenData(tokenizer->getTextData(), assing_operator, Tokenizer::OPERATOR_ASSIGN, exp->token->line, exp->token->pos);
			exp = newBinaryExpression(scope, EXP_TYPE_ASSIGN, assign_token, params, exp);
			OS_ASSERT(exp && exp->type == EXP_TYPE_SET_LOCAL_VAR && !exp->ret_values);
			assign_token->release();

			list.add(exp OS_DBG_FILEPOS); exp = NULL;
		}
		/* OS:
		begin loop
		var valid, k, v = func(state, state2)
		if(!valid) break

		body_exp

		end loop
		*/
		list.add(loop_scope OS_DBG_FILEPOS);
		{
			// OS: var valid, k, v
			Expression * params = new (malloc(sizeof(Expression) OS_DBG_FILEPOS)) Expression(EXP_TYPE_PARAMS, loop_scope->token);
			for(int i = 0; i < num_locals+1; i++){
				Expression * var_exp = !i ? vars.lastElement() : vars[i-1];
				Expression * name_exp = new (malloc(sizeof(Expression) OS_DBG_FILEPOS)) Expression(EXP_TYPE_NAME, var_exp->token);
				OS_ASSERT(scope->function);
				name_exp->active_locals = scope->function->num_locals;
				name_exp->ret_values = 1;
				params->list.add(name_exp OS_DBG_FILEPOS);
			}
			params->ret_values = params->list.count;

			// OS: func(state, state2)
			Expression * call_exp = new (malloc(sizeof(Expression) OS_DBG_FILEPOS)) Expression(EXP_TYPE_CALL, loop_scope->token);
			{
				Expression * var_exp = vars[num_locals]; // func
				Expression * name_exp = new (malloc(sizeof(Expression) OS_DBG_FILEPOS)) Expression(EXP_TYPE_NAME, var_exp->token);
				OS_ASSERT(scope->function);
				name_exp->active_locals = scope->function->num_locals;
				name_exp->ret_values = 1;
				call_exp->list.add(name_exp OS_DBG_FILEPOS);

				Expression * params = new (malloc(sizeof(Expression) OS_DBG_FILEPOS)) Expression(EXP_TYPE_PARAMS, loop_scope->token);
				for(int i = num_locals+1; i < vars.count-1; i++){
					Expression * var_exp = vars[i];
					Expression * name_exp = new (malloc(sizeof(Expression) OS_DBG_FILEPOS)) Expression(EXP_TYPE_NAME, var_exp->token);
					OS_ASSERT(scope->function);
					name_exp->active_locals = scope->function->num_locals;
					name_exp->ret_values = 1;
					params->list.add(name_exp OS_DBG_FILEPOS);
				}
				params->ret_values = params->list.count;
				call_exp->list.add(params OS_DBG_FILEPOS);
			}
			call_exp->ret_values = params->list.count;

			// OS: var valid, k, v = func(state, state2)
			String assing_operator(allocator, OS_TEXT("="));
			TokenData * assign_token = new (malloc(sizeof(TokenData) OS_DBG_FILEPOS)) TokenData(tokenizer->getTextData(), assing_operator, 
				Tokenizer::OPERATOR_ASSIGN, loop_scope->token->line, loop_scope->token->pos);
			exp = newBinaryExpression(scope, EXP_TYPE_ASSIGN, assign_token, params, call_exp);
			OS_ASSERT(exp && exp->type == EXP_TYPE_SET_LOCAL_VAR && !exp->ret_values);
			assign_token->release();

			loop_scope->list.add(exp OS_DBG_FILEPOS); exp = NULL;
		}

		// OS: if(!valid) break
		{
			Expression * var_exp = vars.lastElement(); // valid var
			Expression * name_exp = new (malloc(sizeof(Expression) OS_DBG_FILEPOS)) Expression(EXP_TYPE_NAME, var_exp->token);
			OS_ASSERT(scope->function);
			name_exp->active_locals = scope->function->num_locals;
			name_exp->ret_values = 1;

			Expression * not_exp = new (malloc(sizeof(Expression) OS_DBG_FILEPOS)) Expression(EXP_TYPE_LOGIC_NOT, loop_scope->token, name_exp OS_DBG_FILEPOS);
			not_exp->ret_values = 1;

			Expression * break_exp = new (malloc(sizeof(Expression) OS_DBG_FILEPOS)) Expression(EXP_TYPE_BREAK, loop_scope->token);
			Expression * if_exp = new (malloc(sizeof(Expression) OS_DBG_FILEPOS)) Expression(EXP_TYPE_IF, loop_scope->token, not_exp, break_exp OS_DBG_FILEPOS);
			loop_scope->list.add(if_exp OS_DBG_FILEPOS);
		}
		loop_scope->list.add(body_exp OS_DBG_FILEPOS);

		// assemble all exps
		scope->list.swap(vars);
		scope->list.add(newExpressionFromList(list, 0) OS_DBG_FILEPOS);
		return scope;
	}
	Expression * pre_exp = exp;
	if(recent_token->type != Tokenizer::CODE_SEPARATOR){
		setError(Tokenizer::CODE_SEPARATOR, recent_token);
		allocator->deleteObj(scope);
		allocator->deleteObj(pre_exp);
		return NULL;
	}
	readToken();
	Expression * bool_exp;
	if(recent_token->type == Tokenizer::CODE_SEPARATOR){
		bool_exp = NULL;
	}else{
		bool_exp = expectSingleExpression(scope, Params().setAllowAutoCall(true).setAllowBinaryOperator(true));
		if(!bool_exp){
			allocator->deleteObj(scope);
			allocator->deleteObj(pre_exp);
			return NULL;
		}
	}
	if(bool_exp && !bool_exp->ret_values){
		setError(ERROR_EXPECT_VALUE, bool_exp->token);
		allocator->deleteObj(scope);
		allocator->deleteObj(pre_exp);
		allocator->deleteObj(bool_exp);
		return NULL;
	}
	if(recent_token->type != Tokenizer::CODE_SEPARATOR){
		setError(Tokenizer::CODE_SEPARATOR, recent_token);
		allocator->deleteObj(scope);
		allocator->deleteObj(pre_exp);
		allocator->deleteObj(bool_exp);
		return NULL;
	}
	readToken();
	Expression * post_exp = expectSingleExpression(scope, Params()
		.setAllowAssign(true)
		.setAllowAutoCall(true)
		.setAllowBinaryOperator(true)
		.setAllowNopResult(true));
	if(!post_exp){
		allocator->deleteObj(scope);
		allocator->deleteObj(pre_exp);
		allocator->deleteObj(bool_exp);
		return NULL;
	}
	if(recent_token->type != Tokenizer::END_BRACKET_BLOCK){
		setError(Tokenizer::END_BRACKET_BLOCK, recent_token);
		allocator->deleteObj(scope);
		allocator->deleteObj(pre_exp);
		allocator->deleteObj(bool_exp);
		allocator->deleteObj(post_exp);
		return NULL;
	}
	readToken();

	Scope * loop_scope = new (malloc(sizeof(Scope) OS_DBG_FILEPOS)) Scope(scope, EXP_TYPE_LOOP_SCOPE, recent_token);
	Expression * body_exp = expectSingleExpression(loop_scope, true, true);
	if(!body_exp){
		allocator->deleteObj(scope);
		allocator->deleteObj(pre_exp);
		allocator->deleteObj(bool_exp);
		allocator->deleteObj(post_exp);
		allocator->deleteObj(loop_scope);
		return NULL;
	}
	if(bool_exp){
		bool_exp = expectExpressionValues(bool_exp, 1);
		Expression * not_exp = new (malloc(sizeof(Expression) OS_DBG_FILEPOS)) Expression(EXP_TYPE_LOGIC_NOT, bool_exp->token, bool_exp OS_DBG_FILEPOS);
		not_exp->ret_values = 1;

		Expression * break_exp = new (malloc(sizeof(Expression) OS_DBG_FILEPOS)) Expression(EXP_TYPE_BREAK, bool_exp->token);
		Expression * if_exp = new (malloc(sizeof(Expression) OS_DBG_FILEPOS)) Expression(EXP_TYPE_IF, bool_exp->token, not_exp, break_exp OS_DBG_FILEPOS);

		loop_scope->list.add(if_exp OS_DBG_FILEPOS);
	}
	body_exp = expectExpressionValues(body_exp, 0);
	loop_scope->list.add(body_exp OS_DBG_FILEPOS);

	post_exp = expectExpressionValues(post_exp, 0);
	loop_scope->list.add(post_exp OS_DBG_FILEPOS);

	scope->list.add(pre_exp OS_DBG_FILEPOS);
	scope->list.add(loop_scope OS_DBG_FILEPOS);
	return scope;
}

OS::Core::Compiler::Expression * OS::Core::Compiler::expectDebugLocalsExpression(Scope * scope)
{
	OS_ASSERT(recent_token && recent_token->str == allocator->core->strings->syntax_debuglocals);

	Expression * exp = new (malloc(sizeof(Expression) OS_DBG_FILEPOS)) Expression(EXP_TYPE_DEBUG_LOCALS, recent_token);
	exp->ret_values = 1;
	readToken();
	return exp;
}

OS::Core::Compiler::Expression * OS::Core::Compiler::expectIfExpression(Scope * scope)
{
	OS_ASSERT(recent_token && (recent_token->str == allocator->core->strings->syntax_if 
		|| recent_token->str == allocator->core->strings->syntax_elseif));
	if(!expectToken(Tokenizer::BEGIN_BRACKET_BLOCK) || !expectToken()){
		return NULL;
	}
	TokenData * token = recent_token;
	Expression * if_exp = expectSingleExpression(scope, Params().setAllowBinaryOperator(true).setAllowNopResult(true));
	if(!if_exp){
		return NULL;
	}
	if(if_exp->ret_values < 1){
		setError(ERROR_EXPECT_VALUE, token);
		allocator->deleteObj(if_exp);
		return NULL;
	}
	if_exp = expectExpressionValues(if_exp, 1);
	if(!recent_token || recent_token->type != Tokenizer::END_BRACKET_BLOCK){
		setError(Tokenizer::END_BRACKET_BLOCK, recent_token);
		allocator->deleteObj(if_exp);
		return NULL;
	}
	if(!expectToken()){
		allocator->deleteObj(if_exp);
		return NULL;
	}
	if(!recent_token){
		setError(ERROR_EXPECT_TOKEN, recent_token);
		allocator->deleteObj(if_exp);
		return NULL;
	}
	token = recent_token;
	Expression * then_exp;
	if(recent_token->type == Tokenizer::BEGIN_CODE_BLOCK){
		then_exp = expectCodeExpression(scope);
	}else{
		then_exp = expectSingleExpression(scope, true);
		if(recent_token && recent_token->type == Tokenizer::CODE_SEPARATOR){
			readToken();
		}
	}
	if(!then_exp){
		allocator->deleteObj(if_exp);
		return NULL;
	}
	then_exp = expectExpressionValues(then_exp, 0);
	if(recent_token && recent_token->type == Tokenizer::NAME){
		Expression * else_exp = NULL;
		if(recent_token->str == allocator->core->strings->syntax_elseif){
			if(!expectToken()){
				allocator->deleteObj(if_exp);
				allocator->deleteObj(then_exp);
				return NULL;
			}
			else_exp = expectIfExpression(scope);
		}else if(recent_token->str == allocator->core->strings->syntax_else){
			if(!expectToken()){
				allocator->deleteObj(if_exp);
				allocator->deleteObj(then_exp);
				return NULL;
			}
			token = recent_token;
			else_exp = expectSingleExpression(scope, true, true);
		}else{
			return new (malloc(sizeof(Expression) OS_DBG_FILEPOS)) Expression(EXP_TYPE_IF, if_exp->token, if_exp, then_exp OS_DBG_FILEPOS);
		}
		if(!else_exp){
			allocator->deleteObj(if_exp);
			allocator->deleteObj(then_exp);
			return NULL;
		}
		else_exp = expectExpressionValues(else_exp, 0);
		return new (malloc(sizeof(Expression) OS_DBG_FILEPOS)) Expression(EXP_TYPE_IF, if_exp->token, if_exp, then_exp, else_exp OS_DBG_FILEPOS);
	}
	return new (malloc(sizeof(Expression) OS_DBG_FILEPOS)) Expression(EXP_TYPE_IF, if_exp->token, if_exp, then_exp OS_DBG_FILEPOS);
}

OS::Core::Compiler::Expression * OS::Core::Compiler::expectReturnExpression(Scope * scope)
{
	OS_ASSERT(recent_token && recent_token->str == allocator->core->strings->syntax_return);
	Expression * ret_exp = new (malloc(sizeof(Expression) OS_DBG_FILEPOS)) Expression(EXP_TYPE_RETURN, recent_token);
	if(!readToken()){
		setError(ERROR_SYNTAX, recent_token);
		allocator->deleteObj(ret_exp);
		return NULL;
	}
	switch(recent_token->type){
	case Tokenizer::END_ARRAY_BLOCK:
	case Tokenizer::END_BRACKET_BLOCK:
	case Tokenizer::END_CODE_BLOCK:
	case Tokenizer::CODE_SEPARATOR:
		return ret_exp;
	}
	Expression * exp = expectSingleExpression(scope, Params().setAllowBinaryOperator(true).setAllowParams(true));
	if(!exp){
		allocator->deleteObj(ret_exp);
		return NULL;
	}
	if(exp->type == EXP_TYPE_PARAMS){
		ret_exp->list.swap(exp->list);
		ret_exp->ret_values = exp->ret_values;
		allocator->deleteObj(exp);
	}else{
		ret_exp->list.add(exp OS_DBG_FILEPOS);
		ret_exp->ret_values = exp->ret_values;
	}
	return ret_exp;
}

OS::Core::Compiler::Expression * OS::Core::Compiler::newBinaryExpression(Scope * scope, ExpressionType exp_type, TokenData * token, Expression * left_exp, Expression * right_exp)
{
	if(left_exp->isConstValue() && right_exp->isConstValue()){
		struct Lib {
			Compiler * compiler;
			TokenData * token;

			void * malloc(int size OS_DBG_FILEPOS_DECL)
			{
				return compiler->malloc(size OS_DBG_FILEPOS_PARAM);
			}

			Expression * newExpression(const String& str, Expression * left_exp, Expression * right_exp)
			{
				token = new (malloc(sizeof(TokenData) OS_DBG_FILEPOS)) TokenData(token->text_data, str, Tokenizer::STRING, token->line, token->pos);
				Expression * exp = new (malloc(sizeof(Expression) OS_DBG_FILEPOS)) Expression(EXP_TYPE_CONST_STRING, token);
				exp->ret_values = 1;
				token->release();
				compiler->allocator->deleteObj(left_exp);
				compiler->allocator->deleteObj(right_exp);
				return exp;
			}

			Expression * newExpression(double val, Expression * left_exp, Expression * right_exp)
			{
				token = new (malloc(sizeof(TokenData) OS_DBG_FILEPOS)) TokenData(token->text_data, String(compiler->allocator, (OS_FLOAT)val, OS_AUTO_PRECISION), Tokenizer::NUMBER, token->line, token->pos);
				token->setFloat((OS_FLOAT)val);
				Expression * exp = new (malloc(sizeof(Expression) OS_DBG_FILEPOS)) Expression(EXP_TYPE_CONST_NUMBER, token);
				exp->ret_values = 1;
				token->release();
				compiler->allocator->deleteObj(left_exp);
				compiler->allocator->deleteObj(right_exp);
				return exp;
			}

			Expression * newExpression(float val, Expression * left_exp, Expression * right_exp)
			{
				return newExpression((double)val, left_exp, right_exp);
			}

			Expression * newExpression(OS_INT val, Expression * left_exp, Expression * right_exp)
			{
				return newExpression((OS_FLOAT)val, left_exp, right_exp);
			}

			Expression * switchExpression(bool ret_left, Expression * left_exp, Expression * right_exp)
			{
				if(ret_left){
					compiler->allocator->deleteObj(right_exp);
					return left_exp;
				}
				compiler->allocator->deleteObj(left_exp);
				return right_exp;
			}

		} lib = {this, token};

		if(exp_type == EXP_TYPE_CONCAT){
			return lib.newExpression(String(allocator->core->newStringValue(left_exp->toString(), right_exp->toString())), left_exp, right_exp);
		}else if(left_exp->type != EXP_TYPE_CONST_STRING && right_exp->type != EXP_TYPE_CONST_STRING)
		switch(exp_type){
		case EXP_TYPE_CONCAT:    // ..
			return lib.newExpression(String(allocator->core->newStringValue(left_exp->toString(), right_exp->toString())), left_exp, right_exp);

		case EXP_TYPE_BIT_AND: // &
			return lib.newExpression(left_exp->toInt() & right_exp->toInt(), left_exp, right_exp);

		case EXP_TYPE_BIT_OR:  // |
			return lib.newExpression(left_exp->toInt() | right_exp->toInt(), left_exp, right_exp);

		case EXP_TYPE_BIT_XOR: // ^
			return lib.newExpression(left_exp->toInt() ^ right_exp->toInt(), left_exp, right_exp);

		case EXP_TYPE_ADD: // +
			return lib.newExpression(left_exp->toNumber() + right_exp->toNumber(), left_exp, right_exp);

		case EXP_TYPE_SUB: // -
			return lib.newExpression(left_exp->toNumber() - right_exp->toNumber(), left_exp, right_exp);

		case EXP_TYPE_MUL: // *
			return lib.newExpression(left_exp->toNumber() * right_exp->toNumber(), left_exp, right_exp);

		case EXP_TYPE_DIV: // /
			return lib.newExpression(left_exp->toNumber() / right_exp->toNumber(), left_exp, right_exp);

		case EXP_TYPE_MOD: // %
			return lib.newExpression(OS_MATH_MOD_OPERATOR(left_exp->toNumber(), right_exp->toNumber()), left_exp, right_exp);

		case EXP_TYPE_LSHIFT: // <<
			return lib.newExpression(left_exp->toInt() << right_exp->toInt(), left_exp, right_exp);

		case EXP_TYPE_RSHIFT: // >>
			return lib.newExpression(left_exp->toInt() >> right_exp->toInt(), left_exp, right_exp);

		case EXP_TYPE_POW: // **
			return lib.newExpression(OS_MATH_POW_OPERATOR(left_exp->toNumber(), right_exp->toNumber()), left_exp, right_exp);
		}
	}
	switch(exp_type){
	case EXP_TYPE_QUESTION:
		return finishQuestionOperator(scope, token, left_exp, right_exp);

	case EXP_TYPE_ASSIGN:
		{
			if(left_exp->type != EXP_TYPE_PARAMS){
				right_exp = expectExpressionValues(right_exp, 1);
				return newAssingExpression(scope, left_exp, right_exp);
			}
			Expression * values_exp = expectExpressionValues(right_exp, left_exp->list.count);
			for(int i = left_exp->list.count-1; i >= 0; i--){
				OS_ASSERT(values_exp->ret_values > 0);

				Expression * var_exp = left_exp->list[i];
				left_exp->list.removeIndex(i); // left_exp is going to be deleted

				values_exp = newAssingExpression(scope, var_exp, values_exp);
				if(!values_exp){
					break;
				}
			}
			allocator->deleteObj(left_exp);
			return values_exp;
		}
	}
	if(left_exp->type == EXP_TYPE_PARAMS){
		OS_ASSERT(right_exp->type != EXP_TYPE_PARAMS);
		right_exp = expectExpressionValues(right_exp, 1);
		left_exp->list.add(right_exp OS_DBG_FILEPOS);
		left_exp->ret_values++;
		return left_exp;
	}
	if(right_exp->type == EXP_TYPE_PARAMS){
		Expression * params = right_exp;
		OS_ASSERT(params->list.count > 0);
		if(params->list.count == 1){
			right_exp = params->list[0];
			allocator->vectorClear(params->list);
			allocator->deleteObj(params);
		}else{
			left_exp = expectExpressionValues(left_exp, 1);
			allocator->vectorInsertAtIndex(params->list, 0, left_exp OS_DBG_FILEPOS);
			params->ret_values++;
			return params; 
		}
	}
	left_exp = expectExpressionValues(left_exp, 1);
	right_exp = expectExpressionValues(right_exp, 1);
	Expression * exp = new (malloc(sizeof(Expression) OS_DBG_FILEPOS)) Expression(exp_type, token, left_exp, right_exp OS_DBG_FILEPOS);
	exp->ret_values = exp_type == EXP_TYPE_PARAMS ? 2 : 1;
	return exp;
}

bool OS::Core::Compiler::findLocalVar(LocalVarDesc& desc, Scope * scope, const String& name, int active_locals, bool all_scopes)
{
	OS_ASSERT(scope);
	for(int up_count = 0, up_scope_count = 0;;){
		for(int i = scope->locals.count-1; i >= 0; i--){
			const Scope::LocalVar& local_var = scope->locals[i];
			if((up_count || local_var.index < active_locals) && local_var.name == name){
				desc.index = local_var.index;
				desc.up_count = up_count;
				desc.up_scope_count = up_scope_count;
				desc.type = i < scope->num_params ? LOCAL_PARAM : (name.toChar()[0] != OS_TEXT('#') ? LOCAL_GENERIC : LOCAL_TEMP);
				return true;
			}
		}
		if(scope->parent){
			if(!all_scopes){
				return false;
			}
			if(scope->type == EXP_TYPE_FUNCTION){
				up_count++;
			}
			up_scope_count++;
			scope = scope->parent;
			continue;
		}
		break;
	}
	return false;
}

OS::Core::Compiler::Expression * OS::Core::Compiler::newAssingExpression(Scope * scope, Expression * var_exp, Expression * value_exp)
{
	switch(var_exp->type){
	case EXP_TYPE_CALL_DIM:
		{
			// Expression * name_exp = var_exp->list[0];
			Expression * params = var_exp->list[1];
			OS_ASSERT(params->type == EXP_TYPE_PARAMS); (void)params;
			allocator->vectorInsertAtIndex(var_exp->list, 0, value_exp OS_DBG_FILEPOS);
			var_exp->type = EXP_TYPE_SET_DIM;
			var_exp->ret_values = value_exp->ret_values-1;
			return var_exp;
		}

	case EXP_TYPE_INDIRECT:
		{
			OS_ASSERT(var_exp->list.count == 2);
			Expression * var_exp_left = var_exp->list[0];
			switch(var_exp_left->type){
			case EXP_TYPE_NAME:
				{
					OS_ASSERT(var_exp_left->ret_values == 1);
					if(findLocalVar(var_exp_left->local_var, scope, var_exp_left->token->str, var_exp_left->active_locals, true)){
						var_exp_left->type = EXP_TYPE_GET_LOCAL_VAR_AUTO_CREATE;
						if(scope->function->max_up_count < var_exp_left->local_var.up_count){
							scope->function->max_up_count = var_exp_left->local_var.up_count;
						}
					}else{
						var_exp_left->type = EXP_TYPE_GET_ENV_VAR_AUTO_CREATE;
					}
					break;
				}
			}
			ExpressionType exp_type = EXP_TYPE_SET_PROPERTY;
			Expression * var_exp_right = var_exp->list[1];
			switch(var_exp_right->type){
			case EXP_TYPE_NAME:
				OS_ASSERT(var_exp_right->ret_values == 1);
				var_exp_right->type = EXP_TYPE_CONST_STRING;
				break;

			case EXP_TYPE_CALL:
			case EXP_TYPE_CALL_AUTO_PARAM:
				OS_ASSERT(false);
				return NULL;

			case EXP_TYPE_CALL_DIM:
				OS_ASSERT(false);
				return NULL;
			}
			Expression * exp = new (malloc(sizeof(Expression) OS_DBG_FILEPOS)) Expression(exp_type, var_exp->token, value_exp, var_exp_left, var_exp_right OS_DBG_FILEPOS);
			exp->ret_values = value_exp->ret_values-1;
			allocator->vectorClear(var_exp->list);
			allocator->deleteObj(var_exp);
			return exp;
		}
		break;

	case EXP_TYPE_NAME:
		if(findLocalVar(var_exp->local_var, scope, var_exp->token->str, var_exp->active_locals, true)){
			var_exp->type = EXP_TYPE_SET_LOCAL_VAR;
			if(scope->function->max_up_count < var_exp->local_var.up_count){
				scope->function->max_up_count = var_exp->local_var.up_count;
			}		
		}else{
			var_exp->type = EXP_TYPE_SET_ENV_VAR;
		}
		var_exp->list.add(value_exp OS_DBG_FILEPOS);
		var_exp->ret_values = value_exp->ret_values-1;
		return var_exp;

	default:
		// OS_ASSERT(false);
		if(!var_exp->isWriteable()){
			setError(ERROR_EXPECT_WRITEABLE, var_exp->token);
			allocator->deleteObj(var_exp);
			allocator->deleteObj(value_exp);
			return NULL;
		}
	}
	return new (malloc(sizeof(Expression) OS_DBG_FILEPOS)) Expression(EXP_TYPE_ASSIGN, var_exp->token, var_exp, value_exp OS_DBG_FILEPOS);
}

OS::Core::Compiler::Expression * OS::Core::Compiler::finishBinaryOperator(Scope * scope, OpcodeLevel prev_level, Expression * exp, 
	const Params& _p, bool& is_finished)
{
	TokenData * binary_operator = recent_token;
	OS_ASSERT(binary_operator->isTypeOf(Tokenizer::BINARY_OPERATOR));

	Params p = Params(_p)
		.setAllowAssign(false)
		.setAllowBinaryOperator(false)
		.setAllowInOperator(_p.allow_in_operator)
		// .setAllowParams(false)
		.setAllowAutoCall(false) // binary_operator->type == Tokenizer::OPERATOR_ASSIGN)
		.setAllowRootBlocks(false);

	readToken();
	Expression * exp2 = expectSingleExpression(scope, Params(p).setAllowParams(false)); // false, allow_param, false, false, false);
	if(!exp2){
		is_finished = true;
		allocator->deleteObj(exp);
		return NULL;
	}
	// exp2 = expectExpressionValues(exp2, 1);
	if(recent_token && recent_token->type == Tokenizer::NAME){
		if(recent_token->str == allocator->core->strings->syntax_in){
			if(p.allow_in_operator){
				recent_token->type = Tokenizer::OPERATOR_IN;
			}
		}else if(recent_token->str == allocator->core->strings->syntax_isprototypeof){
			recent_token->type = Tokenizer::OPERATOR_ISPROTOTYPEOF;
		}else if(recent_token->str == allocator->core->strings->syntax_is){
			recent_token->type = Tokenizer::OPERATOR_IS;
		}
	}
	if(!recent_token || !recent_token->isTypeOf(Tokenizer::BINARY_OPERATOR) || (!p.allow_params && recent_token->type == Tokenizer::PARAM_SEPARATOR)){
		is_finished = true;
		return newBinaryExpression(scope, getExpressionType(binary_operator->type), binary_operator, exp, exp2);
	}
	ExpressionType left_exp_type = getExpressionType(binary_operator->type);
	ExpressionType right_exp_type = getExpressionType(recent_token->type);
	OpcodeLevel left_level = getOpcodeLevel(left_exp_type);
	OpcodeLevel right_level = getOpcodeLevel(right_exp_type);
	if(left_level == right_level){
		exp = newBinaryExpression(scope, left_exp_type, binary_operator, exp, exp2);
		return finishBinaryOperator(scope, prev_level, exp, p, is_finished);
	}
	if(left_level > right_level){
		exp = newBinaryExpression(scope, left_exp_type, binary_operator, exp, exp2);
		if(prev_level >= right_level){
			is_finished = false;
			return exp;
		}
		return finishBinaryOperator(scope, prev_level, exp, p, is_finished);
	}
	exp2 = finishBinaryOperator(scope, left_level, exp2, p, is_finished);
	if(!exp2){
		allocator->deleteObj(exp);
		return NULL;
	}
	exp = newBinaryExpression(scope, left_exp_type, binary_operator, exp, exp2);
	if(is_finished){
		return exp;
	}
	return finishBinaryOperator(scope, prev_level, exp, p, is_finished);
}

OS::Core::Compiler::Expression * OS::Core::Compiler::finishValueExpressionNoAutoCall(Scope * scope, Expression * exp, const Params& p)
{
	return finishValueExpression(scope, exp, Params(p).setAllowAutoCall(false));
}

OS::Core::Compiler::Expression * OS::Core::Compiler::finishValueExpressionNoNextCall(Scope * scope, Expression * exp, const Params& p)
{
	if(recent_token && recent_token->type == Tokenizer::BEGIN_BRACKET_BLOCK){
		return exp;
	}
	return finishValueExpression(scope, exp, Params(p).setAllowAutoCall(false));
}

OS::Core::Compiler::Expression * OS::Core::Compiler::finishValueExpression(Scope * scope, Expression * exp, const Params& _p)
{
	bool is_finished;
	Params p = Params(_p)
		.setAllowRootBlocks(false);
	bool next_allow_auto_call = false;
	for(;; p.allow_auto_call = next_allow_auto_call, next_allow_auto_call = false){
		if(!recent_token){
			return exp;
		}
		Expression * exp2;
		TokenData * token = recent_token;
		TokenType token_type = token->type;
		switch(token_type){
		case Tokenizer::OPERATOR_INDIRECT:    // .
			token = expectToken(Tokenizer::NAME);
			if(!token){
				allocator->deleteObj(exp);
				return NULL;
			}
			exp2 = new (malloc(sizeof(Expression) OS_DBG_FILEPOS)) Expression(EXP_TYPE_NAME, token);
			exp2->ret_values = 1;
			OS_ASSERT(scope->function);
			exp2->active_locals = scope->function->num_locals;
			exp = new (malloc(sizeof(Expression) OS_DBG_FILEPOS)) Expression(EXP_TYPE_INDIRECT, exp2->token, exp, exp2 OS_DBG_FILEPOS);
			exp->ret_values = 1;
			readToken();
			next_allow_auto_call = p.allow_auto_call;
			continue;

		// post ++, post --
		case Tokenizer::OPERATOR_INC:
		case Tokenizer::OPERATOR_DEC:
			if(exp->type != EXP_TYPE_NAME){
				return exp;
			}
			OS_ASSERT(exp->ret_values == 1);
			if(!findLocalVar(exp->local_var, scope, exp->token->str, exp->active_locals, true)){
				setError(ERROR_LOCAL_VAL_NOT_DECLARED, exp->token);
				allocator->deleteObj(exp);
				return NULL;
			}
			if(scope->function->max_up_count < exp->local_var.up_count){
				scope->function->max_up_count = exp->local_var.up_count;
			}
			exp->type = EXP_TYPE_GET_LOCAL_VAR;
			exp = new (malloc(sizeof(Expression) OS_DBG_FILEPOS)) Expression(token_type == Tokenizer::OPERATOR_INC ? EXP_TYPE_POST_INC : EXP_TYPE_POST_DEC, exp->token, exp OS_DBG_FILEPOS);
			exp->ret_values = 1;
			readToken();
			return finishValueExpressionNoAutoCall(scope, exp, p);

		case Tokenizer::OPERATOR_CONCAT:    // ..

		case Tokenizer::OPERATOR_LOGIC_AND: // &&
		case Tokenizer::OPERATOR_LOGIC_OR:  // ||

		case Tokenizer::OPERATOR_LOGIC_PTR_EQ:  // ===
		case Tokenizer::OPERATOR_LOGIC_PTR_NE:  // !==
		case Tokenizer::OPERATOR_LOGIC_EQ:  // ==
		case Tokenizer::OPERATOR_LOGIC_NE:  // !=
		case Tokenizer::OPERATOR_LOGIC_GE:  // >=
		case Tokenizer::OPERATOR_LOGIC_LE:  // <=
		case Tokenizer::OPERATOR_LOGIC_GREATER: // >
		case Tokenizer::OPERATOR_LOGIC_LESS:    // <
		case Tokenizer::OPERATOR_LOGIC_NOT:     // !

		case Tokenizer::OPERATOR_QUESTION:  // ?

		case Tokenizer::OPERATOR_BIT_AND: // &
		case Tokenizer::OPERATOR_BIT_OR:  // |
		case Tokenizer::OPERATOR_BIT_XOR: // ^
		case Tokenizer::OPERATOR_BIT_NOT: // ~
		case Tokenizer::OPERATOR_COMPARE: // <=>
		case Tokenizer::OPERATOR_ADD: // +
		case Tokenizer::OPERATOR_SUB: // -
		case Tokenizer::OPERATOR_MUL: // *
		case Tokenizer::OPERATOR_DIV: // /
		case Tokenizer::OPERATOR_MOD: // %
		case Tokenizer::OPERATOR_LSHIFT: // <<
		case Tokenizer::OPERATOR_RSHIFT: // >>
		case Tokenizer::OPERATOR_POW: // **
			if(!p.allow_binary_operator){
				return exp;
			}
			exp = finishBinaryOperator(scope, OP_LEVEL_NOTHING, exp, p, is_finished);
			if(!exp){
				return NULL;
			}
			OS_ASSERT(is_finished);
			continue;

		case Tokenizer::PARAM_SEPARATOR:
			if(!p.allow_params){
				return exp;
			}
			exp = finishBinaryOperator(scope, OP_LEVEL_NOTHING, exp, p, is_finished);
			if(!exp){
				return NULL;
			}
			OS_ASSERT(is_finished);
			continue;

		case Tokenizer::OPERATOR_BIT_AND_ASSIGN: // &=
		case Tokenizer::OPERATOR_BIT_OR_ASSIGN:  // |=
		case Tokenizer::OPERATOR_BIT_XOR_ASSIGN: // ^=
		case Tokenizer::OPERATOR_BIT_NOT_ASSIGN: // ~=
		case Tokenizer::OPERATOR_ADD_ASSIGN: // +=
		case Tokenizer::OPERATOR_SUB_ASSIGN: // -=
		case Tokenizer::OPERATOR_MUL_ASSIGN: // *=
		case Tokenizer::OPERATOR_DIV_ASSIGN: // /=
		case Tokenizer::OPERATOR_MOD_ASSIGN: // %=
		case Tokenizer::OPERATOR_LSHIFT_ASSIGN: // <<=
		case Tokenizer::OPERATOR_RSHIFT_ASSIGN: // >>=
		case Tokenizer::OPERATOR_POW_ASSIGN: // **=
			setError(ERROR_SYNTAX, token);
			return NULL;

		case Tokenizer::OPERATOR_ASSIGN: // =
			if(!p.allow_assing){ // allow_binary_operator){
				return exp;
			}
			exp = finishBinaryOperator(scope, OP_LEVEL_NOTHING, exp, Params(p).setAllowAssign(false), is_finished);
			if(!exp){
				return NULL;
			}
			OS_ASSERT(is_finished);
			return exp;

		case Tokenizer::END_ARRAY_BLOCK:
		case Tokenizer::END_BRACKET_BLOCK:
		case Tokenizer::END_CODE_BLOCK:
		case Tokenizer::CODE_SEPARATOR:
			return exp;

		case Tokenizer::BEGIN_CODE_BLOCK: // {
			exp2 = expectObjectOrFunctionExpression(scope, p, false);
			if(!exp2){
				allocator->deleteObj(exp);
				return NULL;
			}
			OS_ASSERT(exp2->ret_values == 1);
			exp2 = new (malloc(sizeof(Expression) OS_DBG_FILEPOS)) Expression(EXP_TYPE_PARAMS, exp2->token, exp2 OS_DBG_FILEPOS);
			exp2->ret_values = 1;
			exp = new (malloc(sizeof(Expression) OS_DBG_FILEPOS)) Expression(EXP_TYPE_CALL_AUTO_PARAM, token, exp, exp2 OS_DBG_FILEPOS);
			exp->ret_values = 1;
			// allow_auto_call = false;
			continue;

		case Tokenizer::NAME:
			if(token->str == allocator->core->strings->syntax_in){
				if(!p.allow_in_operator || !p.allow_binary_operator){
					return exp;
				}
				token->type = Tokenizer::OPERATOR_IN;
				exp = finishBinaryOperator(scope, OP_LEVEL_NOTHING, exp, p, is_finished);
				if(!exp){
					return NULL;
				}
				OS_ASSERT(is_finished);
				continue;
			}
			if(token->str == allocator->core->strings->syntax_isprototypeof){
				if(!p.allow_binary_operator){
					return exp;
				}
				token->type = Tokenizer::OPERATOR_ISPROTOTYPEOF;
				exp = finishBinaryOperator(scope, OP_LEVEL_NOTHING, exp, p, is_finished);
				if(!exp){
					return NULL;
				}
				OS_ASSERT(is_finished);
				continue;
			}
			if(token->str == allocator->core->strings->syntax_is){
				if(!p.allow_binary_operator){
					return exp;
				}
				token->type = Tokenizer::OPERATOR_IS;
				exp = finishBinaryOperator(scope, OP_LEVEL_NOTHING, exp, p, is_finished);
				if(!exp){
					return NULL;
				}
				OS_ASSERT(is_finished);
				continue;
			}
			// no break

		default:
			if(!p.allow_auto_call){
				return exp;
			}
			if(token->type == Tokenizer::NAME){
				Core::Strings * strings = allocator->core->strings;
				if(token->str == strings->syntax_var
					// || token->str == strings->syntax_function
					|| token->str == strings->syntax_return
					|| token->str == strings->syntax_if
					|| token->str == strings->syntax_else
					|| token->str == strings->syntax_elseif
					|| token->str == strings->syntax_for
					|| token->str == strings->syntax_break
					|| token->str == strings->syntax_continue
					|| token->str == strings->syntax_in
					|| token->str == strings->syntax_class
					|| token->str == strings->syntax_enum
					|| token->str == strings->syntax_switch
					|| token->str == strings->syntax_case 
					|| token->str == strings->syntax_default
					|| token->str == strings->syntax_try
					|| token->str == strings->syntax_catch
					|| token->str == strings->syntax_finally
					|| token->str == strings->syntax_throw
					|| token->str == strings->syntax_new
					|| token->str == strings->syntax_while
					|| token->str == strings->syntax_do
					|| token->str == strings->syntax_const
					|| token->str == strings->syntax_public
					|| token->str == strings->syntax_protected
					|| token->str == strings->syntax_private
					|| token->str == strings->syntax_yield
					|| token->str == strings->syntax_static
					|| token->str == strings->syntax_debugger
					// || token->str == strings->syntax_debuglocals
					)
				{
					return exp;
				}
			}
			exp2 = expectSingleExpression(scope, Params(p)
				.setAllowAssign(false)
				.setAllowAutoCall(false)
				.setAllowParams(false)
				.setAllowRootBlocks(false)); // allow_binary_operator, false, false, false, false);
			if(!exp2){
				allocator->deleteObj(exp);
				return NULL;
			}
			OS_ASSERT(exp2->ret_values == 1);
			exp2 = expectExpressionValues(exp2, 1);
			exp2 = new (malloc(sizeof(Expression) OS_DBG_FILEPOS)) Expression(EXP_TYPE_PARAMS, exp2->token, exp2 OS_DBG_FILEPOS);
			exp2->ret_values = 1;
			exp = new (malloc(sizeof(Expression) OS_DBG_FILEPOS)) Expression(EXP_TYPE_CALL_AUTO_PARAM, token, exp, exp2 OS_DBG_FILEPOS);
			exp->ret_values = 1;
			// allow_auto_call = false;
			continue;

		case Tokenizer::BEGIN_BRACKET_BLOCK: // (
			if(!p.allow_call){
				return exp;
			}
			exp2 = expectParamsExpression(scope);
			if(!exp2){
				allocator->deleteObj(exp);
				return NULL;
			}
			exp = new (malloc(sizeof(Expression) OS_DBG_FILEPOS)) Expression(EXP_TYPE_CALL, token, exp, exp2 OS_DBG_FILEPOS);
			exp->ret_values = 1;
			continue;

		case Tokenizer::BEGIN_ARRAY_BLOCK: // [
			exp2 = expectParamsExpression(scope);
			if(!exp2){
				allocator->deleteObj(exp);
				return NULL;
			}
			exp = new (malloc(sizeof(Expression) OS_DBG_FILEPOS)) Expression(EXP_TYPE_CALL_DIM, token, exp, exp2 OS_DBG_FILEPOS);
			exp->ret_values = 1;
			if(0 && !p.allow_binary_operator){
				return exp;
			}
			continue;
		}
	}
	return NULL; // shut up compiler
}

OS::Core::Compiler::Params::Params()
{
	OS_MEMSET(this, 0, sizeof(*this));
	allow_call = true;
}

OS::Core::Compiler::Params::Params(const Params& p)
{
	OS_MEMCPY(this, &p, sizeof(p));
}

OS::Core::Compiler::Params& OS::Core::Compiler::Params::setAllowRootBlocks(bool val)
{
	allow_root_blocks = val;
	allow_var_decl = val;
	return *this;
}

OS::Core::Compiler::Params& OS::Core::Compiler::Params::setAllowVarDecl(bool val)
{
	allow_var_decl = val;
	return *this;
}

OS::Core::Compiler::Params& OS::Core::Compiler::Params::setAllowInlineNestedBlock(bool val)
{
	allow_inline_nested_block = val;
	return *this;
}

OS::Core::Compiler::Params& OS::Core::Compiler::Params::setAllowBinaryOperator(bool val)
{
	allow_binary_operator = val;
	allow_in_operator = val;
	return *this;
}

OS::Core::Compiler::Params& OS::Core::Compiler::Params::setAllowInOperator(bool val)
{
	allow_in_operator = val;
	return *this;
}

OS::Core::Compiler::Params& OS::Core::Compiler::Params::setAllowAssign(bool val)
{
	allow_assing = val;
	return *this;
}

OS::Core::Compiler::Params& OS::Core::Compiler::Params::setAllowParams(bool val)
{
	allow_params = val;
	return *this;
}

OS::Core::Compiler::Params& OS::Core::Compiler::Params::setAllowAutoCall(bool val)
{
	allow_auto_call = val;
	return *this;
}

OS::Core::Compiler::Params& OS::Core::Compiler::Params::setAllowCall(bool val)
{
	allow_call = val;
	return *this;
}

OS::Core::Compiler::Params& OS::Core::Compiler::Params::setAllowNopResult(bool val)
{
	allow_nop_result = val;
	return *this;
}

bool OS::Core::Compiler::isVarNameValid(const String& name)
{
	Core::Strings * strings = allocator->core->strings;
	return !(name == strings->syntax_super
		|| name == strings->syntax_is
		|| name == strings->syntax_isprototypeof
		|| name == strings->syntax_extends
		|| name == strings->syntax_delete
		|| name == strings->syntax_prototype
		|| name == strings->syntax_var
		|| name == strings->syntax_arguments
		|| name == strings->syntax_function
		|| name == strings->syntax_null
		|| name == strings->syntax_true
		|| name == strings->syntax_false
		|| name == strings->syntax_return
		|| name == strings->syntax_class
		|| name == strings->syntax_enum
		|| name == strings->syntax_switch
		|| name == strings->syntax_case
		|| name == strings->syntax_default
		|| name == strings->syntax_if
		|| name == strings->syntax_else
		|| name == strings->syntax_elseif
		|| name == strings->syntax_for
		|| name == strings->syntax_in
		|| name == strings->syntax_break
		|| name == strings->syntax_continue
		|| name == strings->syntax_try
		|| name == strings->syntax_catch
		|| name == strings->syntax_finally
		|| name == strings->syntax_throw
		|| name == strings->syntax_new
		|| name == strings->syntax_while
		|| name == strings->syntax_do
		|| name == strings->syntax_const
		|| name == strings->syntax_public
		|| name == strings->syntax_protected
		|| name == strings->syntax_private
		|| name == strings->syntax_yield
		|| name == strings->syntax_static
		|| name == strings->syntax_debugger
		|| name == strings->syntax_debuglocals
		|| name == strings->var_func
		|| name == strings->var_this
		|| name == strings->var_env
#ifdef OS_GLOBAL_VAR_ENABLED
		|| name == strings->var_globals
#endif
		);
}

OS::Core::Compiler::Expression * OS::Core::Compiler::expectSingleExpression(Scope * scope, bool allow_nop_result, bool allow_inline_nested_block)
{
	return expectSingleExpression(scope, Params()
		.setAllowAssign(true)
		.setAllowAutoCall(true)
		.setAllowBinaryOperator(true)
		.setAllowParams(true)
		.setAllowRootBlocks(true)
		.setAllowNopResult(allow_nop_result)
		.setAllowInlineNestedBlock(allow_inline_nested_block));
}

OS::Core::Compiler::Expression * OS::Core::Compiler::expectSingleExpression(Scope * scope, const Params& p)
{
#ifdef OS_DEBUG
	allocator->checkNativeStackUsage(OS_TEXT("OS::Core::Compiler::expectSingleExpression"));
#endif
	TokenData * token = recent_token; // readToken();
	if(!token){
		setError(ERROR_EXPECT_EXPRESSION, token);
		return NULL;
	}
	Expression * exp;
	TokenType token_type = token->type;
	switch(token_type){
		// begin unary operators
	case Tokenizer::OPERATOR_ADD:
	case Tokenizer::OPERATOR_SUB:
	case Tokenizer::OPERATOR_LENGTH:
	case Tokenizer::OPERATOR_BIT_NOT:
	case Tokenizer::OPERATOR_LOGIC_NOT:
		if(!expectToken()){
			return NULL;
		}
		exp = expectSingleExpression(scope, Params());
		if(!exp){
			return NULL;
		}
		OS_ASSERT(exp->ret_values == 1);
		exp = new (malloc(sizeof(Expression) OS_DBG_FILEPOS)) Expression(getUnaryExpressionType(token_type), exp->token, exp OS_DBG_FILEPOS);
		exp->ret_values = 1;
		return finishValueExpressionNoAutoCall(scope, exp, p);

		// pre ++, pre --
	case Tokenizer::OPERATOR_INC:
	case Tokenizer::OPERATOR_DEC:
		if(!expectToken(Tokenizer::NAME)){
			return NULL;
		}
		exp = new (malloc(sizeof(Expression) OS_DBG_FILEPOS)) Expression(EXP_TYPE_GET_LOCAL_VAR, recent_token);
		exp->ret_values = 1;
		exp->active_locals = scope->function->num_locals;
		if(!findLocalVar(exp->local_var, scope, exp->token->str, exp->active_locals, true)){
			setError(ERROR_LOCAL_VAL_NOT_DECLARED, exp->token);
			allocator->deleteObj(exp);
			return NULL;
		}
		if(scope->function->max_up_count < exp->local_var.up_count){
			scope->function->max_up_count = exp->local_var.up_count;
		}
		exp = new (malloc(sizeof(Expression) OS_DBG_FILEPOS)) Expression(token_type == Tokenizer::OPERATOR_INC ? EXP_TYPE_PRE_INC : EXP_TYPE_PRE_DEC, exp->token, exp OS_DBG_FILEPOS);
		exp->ret_values = 1;
		readToken();
		return finishValueExpressionNoAutoCall(scope, exp, p);
		// end unary operators

	case Tokenizer::OPERATOR_THIS:
		exp = new (malloc(sizeof(Expression) OS_DBG_FILEPOS)) Expression(EXP_TYPE_GET_THIS, token);
		exp->ret_values = 1;
		readToken();
		if(recent_token && recent_token->type == Tokenizer::NAME){
			Expression * exp2 = new (malloc(sizeof(Expression) OS_DBG_FILEPOS)) Expression(EXP_TYPE_NAME, recent_token);
			exp2->ret_values = 1;
			OS_ASSERT(scope->function);
			exp2->active_locals = scope->function->num_locals; // TODO: remove it?
			exp = new (malloc(sizeof(Expression) OS_DBG_FILEPOS)) Expression(EXP_TYPE_INDIRECT, exp2->token, exp, exp2 OS_DBG_FILEPOS);
			exp->ret_values = 1;
			readToken();
		}
		return finishValueExpression(scope, exp, p);

	case Tokenizer::BEGIN_CODE_BLOCK:
		if(p.allow_root_blocks && !isNextToken(Tokenizer::OPERATOR_BIT_OR) && !isNextToken(Tokenizer::OPERATOR_LOGIC_OR)){
			if(!p.allow_inline_nested_block){
				TokenData * check_token = getPrevToken();
				if(!check_token || (check_token->type != Tokenizer::CODE_SEPARATOR && check_token->type != Tokenizer::BEGIN_CODE_BLOCK)){
					setError(ERROR_EXPECT_CODE_SEP_BEFORE_NESTED_BLOCK, recent_token);
					return NULL;
				}
			}
			return expectCodeExpression(scope);
		}
		return expectObjectOrFunctionExpression(scope, p);

	case Tokenizer::BEGIN_ARRAY_BLOCK:
		return expectArrayExpression(scope, p);

	case Tokenizer::BEGIN_BRACKET_BLOCK:
		return expectBracketExpression(scope, p);

	case Tokenizer::STRING:
		exp = new (malloc(sizeof(Expression) OS_DBG_FILEPOS)) Expression(EXP_TYPE_CONST_STRING, token);
		exp->ret_values = 1;
		readToken();
		return finishValueExpressionNoNextCall(scope, exp, p);

	case Tokenizer::NUMBER:
		exp = new (malloc(sizeof(Expression) OS_DBG_FILEPOS)) Expression(EXP_TYPE_CONST_NUMBER, token);
		exp->ret_values = 1;
		readToken();
		return finishValueExpressionNoNextCall(scope, exp, p);

	case Tokenizer::REST_ARGUMENTS:
		exp = new (malloc(sizeof(Expression) OS_DBG_FILEPOS)) Expression(EXP_TYPE_GET_REST_ARGUMENTS, token);
		exp->ret_values = 1;
		readToken();
		return finishValueExpressionNoNextCall(scope, exp, p);

	case Tokenizer::CODE_SEPARATOR:
	case Tokenizer::END_ARRAY_BLOCK:
	case Tokenizer::END_BRACKET_BLOCK:
	case Tokenizer::END_CODE_BLOCK:
		if(!p.allow_nop_result){
			return NULL;
		}
		return new (malloc(sizeof(Expression) OS_DBG_FILEPOS)) Expression(EXP_TYPE_NOP, token);

	case Tokenizer::NAME:
		if(token->str == allocator->core->strings->syntax_var){
			if(!p.allow_var_decl){
				setError(ERROR_NESTED_ROOT_BLOCK, token);
				return NULL;
			}
			return expectVarExpression(scope);
		}
		if(token->str == allocator->core->strings->syntax_function){
			exp = expectFunctionExpression(scope);
			if(!exp){
				return NULL;
			}
			if(!exp->ret_values){
				return exp;
			}
			return finishValueExpression(scope, exp, p);
		}
		if(token->str == allocator->core->strings->syntax_return){
			return expectReturnExpression(scope);
		}
		if(token->str == allocator->core->strings->syntax_if){
			if(!p.allow_root_blocks){
				setError(ERROR_NESTED_ROOT_BLOCK, token);
				return NULL;
			}
			return expectIfExpression(scope);
		}
		if(token->str == allocator->core->strings->syntax_else){
			setError(ERROR_SYNTAX, token);
			return NULL;
		}
		if(token->str == allocator->core->strings->syntax_elseif){
			setError(ERROR_SYNTAX, token);
			return NULL;
		}
		if(token->str == allocator->core->strings->syntax_for){
			if(!p.allow_root_blocks){
				setError(ERROR_NESTED_ROOT_BLOCK, token);
				return NULL;
			}
			return expectForExpression(scope);
		}
		if(token->str == allocator->core->strings->syntax_in){
			setError(ERROR_SYNTAX, token);
			return NULL;
		}
		if(token->str == allocator->core->strings->var_this){
			exp = new (malloc(sizeof(Expression) OS_DBG_FILEPOS)) Expression(EXP_TYPE_GET_THIS, token);
			exp->ret_values = 1;
			readToken();
			return finishValueExpression(scope, exp, p);
		}
		if(token->str == allocator->core->strings->syntax_arguments){
			exp = new (malloc(sizeof(Expression) OS_DBG_FILEPOS)) Expression(EXP_TYPE_GET_ARGUMENTS, token);
			exp->ret_values = 1;
			readToken();
			return finishValueExpressionNoAutoCall(scope, exp, p);
		}
		if(token->str == allocator->core->strings->syntax_null){
			exp = new (malloc(sizeof(Expression) OS_DBG_FILEPOS)) Expression(EXP_TYPE_CONST_NULL, token);
			exp->ret_values = 1;
			readToken();
			return finishValueExpressionNoAutoCall(scope, exp, p);
		}
		if(token->str == allocator->core->strings->syntax_true){
			token->setFloat(1);
			exp = new (malloc(sizeof(Expression) OS_DBG_FILEPOS)) Expression(EXP_TYPE_CONST_TRUE, token);
			exp->ret_values = 1;
			readToken();
			return finishValueExpressionNoAutoCall(scope, exp, p);
		}
		if(token->str == allocator->core->strings->syntax_false){
			exp = new (malloc(sizeof(Expression) OS_DBG_FILEPOS)) Expression(EXP_TYPE_CONST_FALSE, token);
			exp->ret_values = 1;
			readToken();
			return finishValueExpressionNoAutoCall(scope, exp, p);
		}
		if(token->str == allocator->core->strings->syntax_extends){
			exp = expectExtendsExpression(scope);
			if(!exp){
				return NULL;
			}
			return finishValueExpressionNoAutoCall(scope, exp, p);
		}
		if(token->str == allocator->core->strings->syntax_delete){
			if(!p.allow_root_blocks){
				setError(ERROR_NESTED_ROOT_BLOCK, token);
				return NULL;
			}
			return expectDeleteExpression(scope);
		}
		if(token->str == allocator->core->strings->syntax_break){
			if(!p.allow_root_blocks){
				setError(ERROR_NESTED_ROOT_BLOCK, token);
				return NULL;
			}
			readToken();
			return new (malloc(sizeof(Expression) OS_DBG_FILEPOS)) Expression(EXP_TYPE_BREAK, token);
		}
		if(token->str == allocator->core->strings->syntax_continue){
			if(!p.allow_root_blocks){
				setError(ERROR_NESTED_ROOT_BLOCK, token);
				return NULL;
			}
			readToken();
			return new (malloc(sizeof(Expression) OS_DBG_FILEPOS)) Expression(EXP_TYPE_CONTINUE, token);
		}
		if(token->str == allocator->core->strings->syntax_debugger){
			if(!p.allow_root_blocks){
				setError(ERROR_NESTED_ROOT_BLOCK, token);
				return NULL;
			}
			readToken();
			return new (malloc(sizeof(Expression) OS_DBG_FILEPOS)) Expression(EXP_TYPE_DEBUGGER, token);
		}
		if(token->str == allocator->core->strings->syntax_debuglocals){
			exp = expectDebugLocalsExpression(scope);
			if(!exp){
				return NULL;
			}
			return finishValueExpressionNoAutoCall(scope, exp, p);
		}
		if(token->str == allocator->core->strings->syntax_is){
			setError(ERROR_SYNTAX, token);
			return NULL;
		}
		if(token->str == allocator->core->strings->syntax_isprototypeof){
			setError(ERROR_SYNTAX, token);
			return NULL;
		}
		if(token->str == allocator->core->strings->syntax_class){
			setError(ERROR_SYNTAX, token);
			return NULL;
		}
		if(token->str == allocator->core->strings->syntax_enum){
			setError(ERROR_SYNTAX, token);
			return NULL;
		}
		if(token->str == allocator->core->strings->syntax_switch){
			setError(ERROR_SYNTAX, token);
			return NULL;
		}
		if(token->str == allocator->core->strings->syntax_case || token->str == allocator->core->strings->syntax_default){
			setError(ERROR_SYNTAX, token);
			return NULL;
		}
		if(token->str == allocator->core->strings->syntax_try){
			setError(ERROR_SYNTAX, token);
			return NULL;
		}
		if(token->str == allocator->core->strings->syntax_catch){
			setError(ERROR_SYNTAX, token);
			return NULL;
		}
		if(token->str == allocator->core->strings->syntax_finally){
			setError(ERROR_SYNTAX, token);
			return NULL;
		}
		if(token->str == allocator->core->strings->syntax_throw){
			setError(ERROR_SYNTAX, token);
			return NULL;
		}
		if(token->str == allocator->core->strings->syntax_new){
			setError(ERROR_SYNTAX, token);
			return NULL;
		}
		if(token->str == allocator->core->strings->syntax_while){
			setError(ERROR_SYNTAX, token);
			return NULL;
		}
		if(token->str == allocator->core->strings->syntax_do){
			setError(ERROR_SYNTAX, token);
			return NULL;
		}
		if(token->str == allocator->core->strings->syntax_const){
			setError(ERROR_SYNTAX, token);
			return NULL;
		}
		if(token->str == allocator->core->strings->syntax_public){
			setError(ERROR_SYNTAX, token);
			return NULL;
		}
		if(token->str == allocator->core->strings->syntax_protected){
			setError(ERROR_SYNTAX, token);
			return NULL;
		}
		if(token->str == allocator->core->strings->syntax_private){
			setError(ERROR_SYNTAX, token);
			return NULL;
		}
		if(token->str == allocator->core->strings->syntax_yield){
			setError(ERROR_SYNTAX, token);
			return NULL;
		}
		if(token->str == allocator->core->strings->syntax_static){
			setError(ERROR_SYNTAX, token);
			return NULL;
		}
		if(token->str == allocator->core->strings->syntax_super){
			exp = new (malloc(sizeof(Expression) OS_DBG_FILEPOS)) Expression(EXP_TYPE_SUPER, token);
			exp->ret_values = 1;
			readToken();
			return finishValueExpression(scope, exp, p);
		}
		exp = new (malloc(sizeof(Expression) OS_DBG_FILEPOS)) Expression(EXP_TYPE_NAME, token);
		exp->ret_values = 1;
		OS_ASSERT(scope->function);
		exp->active_locals = scope->function->num_locals;
		readToken();
		return finishValueExpression(scope, exp, p);

	case Tokenizer::OUTPUT_STRING:
	case Tokenizer::OUTPUT_NEXT_VALUE:
		{
			TokenData * globals_name_token = new (malloc(sizeof(TokenData) OS_DBG_FILEPOS)) TokenData(tokenizer->getTextData(), 
				allocator->core->strings->var_globals, Tokenizer::NAME, token->line, token->pos);
			exp = new (malloc(sizeof(Expression) OS_DBG_FILEPOS)) Expression(EXP_TYPE_NAME, globals_name_token);
			exp->ret_values = 1;
			exp->active_locals = scope->function->num_locals;

			TokenData * name_token = new (malloc(sizeof(TokenData) OS_DBG_FILEPOS)) TokenData(tokenizer->getTextData(), 
				allocator->core->strings->func_echo, Tokenizer::NAME, token->line, token->pos);
			Expression * exp2 = new (malloc(sizeof(Expression) OS_DBG_FILEPOS)) Expression(EXP_TYPE_NAME, name_token);
			exp2->ret_values = 1;
			exp2->active_locals = scope->function->num_locals;

			exp = new (malloc(sizeof(Expression) OS_DBG_FILEPOS)) Expression(EXP_TYPE_INDIRECT, token, exp, exp2 OS_DBG_FILEPOS);
			exp->ret_values = 1;

			name_token->release();
			globals_name_token->release();

			if(recent_token->type == Tokenizer::OUTPUT_STRING){
				recent_token->type = Tokenizer::STRING;
			}else{
				recent_token->type = Tokenizer::NAME;
				readToken();
			}
			return finishValueExpression(scope, exp, p);
		}
	}
	setError(ERROR_EXPECT_EXPRESSION, token);
	return NULL;
}

void OS::Core::Compiler::debugPrintSourceLine(Buffer& out, TokenData * token)
{
	if(!token){
		return;
	}

	bool filePrinted = false;
	if(recent_printed_text_data != token->text_data){
		if(recent_printed_text_data){
			recent_printed_text_data->release();
		}
		filePrinted = true;
		recent_printed_line = -1;
		recent_printed_text_data = token->text_data->retain();
		out += String::format(allocator, OS_TEXT("\n[FILE] %s"), token->text_data->filename.toChar());
	}
	if(recent_printed_line != token->line && token->line >= 0){
		recent_printed_line = token->line;
		String line = allocator->core->newStringValue(token->text_data->lines[token->line], true, true);
		out += String::format(allocator, OS_TEXT("\n[%d] %s\n\n"), token->line+1, line.toChar());
	}
	else if(filePrinted){
		out += String::format(allocator, OS_TEXT("\n"));
	}
	return;
}

const OS_CHAR * OS::Core::Compiler::getExpName(ExpressionType type)
{
	switch(type){
	case EXP_TYPE_NOP:
		return OS_TEXT("nop");

	case EXP_TYPE_CODE_LIST:
		return OS_TEXT("code list");

	case EXP_TYPE_CONST_NUMBER:
		return OS_TEXT("const number");

	case EXP_TYPE_CONST_STRING:
		return OS_TEXT("const string");

	case EXP_TYPE_CONST_NULL:
		return OS_TEXT("const null");

	case EXP_TYPE_CONST_TRUE:
		return OS_TEXT("const true");

	case EXP_TYPE_CONST_FALSE:
		return OS_TEXT("const false");

	case EXP_TYPE_NAME:
		return OS_TEXT("name");

	case EXP_TYPE_PARAMS:
		return OS_TEXT("params");

	case EXP_TYPE_INDIRECT:
		return OS_TEXT("indirect");

	case EXP_TYPE_SET_PROPERTY:
		return OS_TEXT("set property");

	case EXP_TYPE_SET_PROPERTY_BY_LOCALS_AUTO_CREATE:
		return OS_TEXT("set property by locals auto create");

	case EXP_TYPE_GET_SET_PROPERTY_BY_LOCALS_AUTO_CREATE:
		return OS_TEXT("get & set property by locals auto create");

	case EXP_TYPE_GET_PROPERTY:
		return OS_TEXT("get property");

	case EXP_TYPE_GET_THIS_PROPERTY_BY_STRING:
		return OS_TEXT("get this property by string");

	case EXP_TYPE_GET_PROPERTY_BY_LOCALS:
		return OS_TEXT("get property by locals");

	case EXP_TYPE_GET_PROPERTY_BY_LOCAL_AND_NUMBER:
		return OS_TEXT("get property by local & number");

	case EXP_TYPE_GET_PROPERTY_AUTO_CREATE:
		return OS_TEXT("get property auto create");

	case EXP_TYPE_SET_DIM:
		return OS_TEXT("set dim");

	case EXP_TYPE_POP_VALUE:
		return OS_TEXT("pop");

	case EXP_TYPE_CALL:
	case EXP_TYPE_CALL_AUTO_PARAM:
		return OS_TEXT("call");

	case EXP_TYPE_SUPER_CALL:
		return OS_TEXT("super call");

	case EXP_TYPE_CALL_DIM:
		return OS_TEXT("dim");

	case EXP_TYPE_CALL_METHOD:
		return OS_TEXT("call method");

	case EXP_TYPE_TAIL_CALL_METHOD:
		return OS_TEXT("tail call method");

	case EXP_TYPE_TAIL_CALL:
		return OS_TEXT("tail call");

	case EXP_TYPE_VALUE:
		return OS_TEXT("single value");

	case EXP_TYPE_EXTENDS:
		return OS_TEXT("extends");

	case EXP_TYPE_DELETE:
		return OS_TEXT("delete");

	case EXP_TYPE_RETURN:
		return OS_TEXT("return");

	case EXP_TYPE_FUNCTION:
		return OS_TEXT("function");

	case EXP_TYPE_SCOPE:
		return OS_TEXT("scope");

	case EXP_TYPE_LOOP_SCOPE:
		return OS_TEXT("loop");

	case EXP_TYPE_GET_THIS:
		return OS_TEXT("push this");

	case EXP_TYPE_GET_ARGUMENTS:
		return OS_TEXT("get arguments");

	case EXP_TYPE_GET_REST_ARGUMENTS:
		return OS_TEXT("get rest arguments");

	case EXP_TYPE_GET_LOCAL_VAR:
		return OS_TEXT("get local var");

	case EXP_TYPE_GET_LOCAL_VAR_AUTO_CREATE:
		return OS_TEXT("get local var auto create");

	case EXP_TYPE_GET_ENV_VAR:
		return OS_TEXT("get env var");

	case EXP_TYPE_GET_ENV_VAR_AUTO_CREATE:
		return OS_TEXT("get env var auto create");

	case EXP_TYPE_SET_LOCAL_VAR:
		return OS_TEXT("set local var");

	case EXP_TYPE_SET_LOCAL_VAR_BY_BIN_OPERATOR_LOCALS:
		return OS_TEXT("set local var by bin operator locals");

	case EXP_TYPE_SET_LOCAL_VAR_BY_BIN_OPERATOR_LOCAL_AND_NUMBER:
		return OS_TEXT("set local var by bin operator local & number");

	case EXP_TYPE_SET_ENV_VAR:
		return OS_TEXT("set env var");

	case EXP_TYPE_BIN_OPERATOR_BY_LOCALS:
		return OS_TEXT("binary operator by locals");

	case EXP_TYPE_BIN_OPERATOR_BY_LOCAL_AND_NUMBER:
		return OS_TEXT("binary operator by local & number");

	case EXP_TYPE_ASSIGN:
		return OS_TEXT("operator =");

	case EXP_TYPE_LOGIC_AND: // &&
		return OS_TEXT("logic &&");

	case EXP_TYPE_LOGIC_OR:  // ||
		return OS_TEXT("logic ||");

	case EXP_TYPE_LOGIC_PTR_EQ:  // ===
		return OS_TEXT("logic ===");

	case EXP_TYPE_LOGIC_PTR_NE:  // !==
		return OS_TEXT("logic !==");

	case EXP_TYPE_LOGIC_EQ:  // ==
		return OS_TEXT("logic ==");

	case EXP_TYPE_LOGIC_NE:  // !=
		return OS_TEXT("logic !=");

	case EXP_TYPE_LOGIC_GE:  // >=
		return OS_TEXT("logic >=");

	case EXP_TYPE_LOGIC_LE:  // <=
		return OS_TEXT("logic <=");

	case EXP_TYPE_LOGIC_GREATER: // >
		return OS_TEXT("logic >");

	case EXP_TYPE_LOGIC_LESS:    // <
		return OS_TEXT("logic <");

	case EXP_TYPE_LOGIC_BOOL:     // !!
		return OS_TEXT("logic bool");

	case EXP_TYPE_LOGIC_NOT:     // !
		return OS_TEXT("logic not");

	case EXP_TYPE_PLUS:
		return OS_TEXT("plus");

	case EXP_TYPE_NEG:
		return OS_TEXT("neg");

	case EXP_TYPE_LENGTH:
		return OS_TEXT("length");

	case EXP_TYPE_IN:
		return OS_TEXT("in");

	case EXP_TYPE_ISPROTOTYPEOF:
		return OS_TEXT("isprototypeof");

	case EXP_TYPE_IS:
		return OS_TEXT("is");

	case EXP_TYPE_SUPER:
		return OS_TEXT("super");

	case EXP_TYPE_PRE_INC:     // ++
		return OS_TEXT("pre ++");

	case EXP_TYPE_PRE_DEC:     // --
		return OS_TEXT("pre --");

	case EXP_TYPE_POST_INC:    // ++
		return OS_TEXT("post ++");

	case EXP_TYPE_POST_DEC:    // --
		return OS_TEXT("post --");

	case EXP_TYPE_BIT_AND: // &
		return OS_TEXT("bit &");

	case EXP_TYPE_BIT_OR:  // |
		return OS_TEXT("bit |");

	case EXP_TYPE_BIT_XOR: // ^
		return OS_TEXT("bit ^");

	case EXP_TYPE_BIT_NOT: // ~
		return OS_TEXT("bit ~");

	case EXP_TYPE_BIT_AND_ASSIGN: // &=
		return OS_TEXT("bit &=");

	case EXP_TYPE_BIT_OR_ASSIGN:  // |=
		return OS_TEXT("bit |=");

	case EXP_TYPE_BIT_XOR_ASSIGN: // ^=
		return OS_TEXT("bit ^=");

	case EXP_TYPE_BIT_NOT_ASSIGN: // ~=
		return OS_TEXT("bit ~=");

	case EXP_TYPE_CONCAT: // ..
		return OS_TEXT("operator ..");

	case EXP_TYPE_COMPARE:    // <=>
		return OS_TEXT("operator <=>");

	case EXP_TYPE_ADD: // +
		return OS_TEXT("operator +");

	case EXP_TYPE_SUB: // -
		return OS_TEXT("operator -");

	case EXP_TYPE_MUL: // *
		return OS_TEXT("operator *");

	case EXP_TYPE_DIV: // /
		return OS_TEXT("operator /");

	case EXP_TYPE_MOD: // %
		return OS_TEXT("operator %");

	case EXP_TYPE_LSHIFT: // <<
		return OS_TEXT("operator <<");

	case EXP_TYPE_RSHIFT: // >>
		return OS_TEXT("operator >>");

	case EXP_TYPE_POW: // **
		return OS_TEXT("operator **");

	case EXP_TYPE_ADD_ASSIGN: // +=
		return OS_TEXT("operator +=");

	case EXP_TYPE_SUB_ASSIGN: // -=
		return OS_TEXT("operator -=");

	case EXP_TYPE_MUL_ASSIGN: // *=
		return OS_TEXT("operator *=");

	case EXP_TYPE_DIV_ASSIGN: // /=
		return OS_TEXT("operator /=");

	case EXP_TYPE_MOD_ASSIGN: // %=
		return OS_TEXT("operator %=");

	case EXP_TYPE_LSHIFT_ASSIGN: // <<=
		return OS_TEXT("operator <<=");

	case EXP_TYPE_RSHIFT_ASSIGN: // >>=
		return OS_TEXT("operator >>=");

	case EXP_TYPE_POW_ASSIGN: // **=
		return OS_TEXT("operator **=");

	case EXP_TYPE_MOVE:
		return OS_TEXT("move");

	case EXP_TYPE_GET_XCONST:
		return OS_TEXT("get xconst");
	}
	return OS_TEXT("unknown exp");
}

// =====================================================================
// =====================================================================
// =====================================================================

OS::Core::FunctionDecl::LocalVar::LocalVar(const String& p_name): name(p_name)
{
	start_code_pos = -1;
	end_code_pos = -1;
}

OS::Core::FunctionDecl::LocalVar::~LocalVar()
{
}

OS::Core::FunctionDecl::FunctionDecl()
{
#ifdef OS_DEBUG
	prog_func_index = -1;
#endif
	prog_parent_func_index = -1;
	locals = NULL;
	num_locals = 0;
	num_params = 0;
	max_up_count = 0;
	func_depth = 0;
	func_index = 0;
	num_local_funcs = 0;
	opcodes_pos = 0;
	opcodes_size = 0;
}

OS::Core::FunctionDecl::~FunctionDecl()
{
	OS_ASSERT(!locals);
}

// =====================================================================

OS::Core::Program::Program(OS * allocator): filename(allocator)
{
	this->allocator = allocator;
	ref_count = 1;
	const_values = NULL;
	num_numbers = 0;
	num_strings = 0;
}

OS::Core::Program::~Program()
{
	OS_ASSERT(ref_count == 0);
	int i;
	for(i = 0; i < num_strings; i++){
		int j = i + num_numbers + CONST_STD_VALUES;
		OS_ASSERT(OS_VALUE_TYPE(const_values[j]) == OS_VALUE_TYPE_STRING);
		OS_ASSERT(dynamic_cast<GCStringValue*>(OS_VALUE_VARIANT(const_values[j]).string));
		GCStringValue * string = OS_VALUE_VARIANT(const_values[j]).string;
		OS_ASSERT(string->external_ref_count > 0);
		string->external_ref_count--;
		if(string->gc_color == GC_WHITE){
			string->gc_color = GC_BLACK;
		}
	}

	allocator->free(const_values);
	const_values = NULL;

	for(i = 0; i < num_functions; i++){
		FunctionDecl * func = functions + i;
		for(int j = 0; j < func->num_locals; j++){
			func->locals[j].~LocalVar();
		}
		allocator->free(func->locals);
		func->locals = NULL;
		func->~FunctionDecl();
	}
	allocator->free(functions);
	functions = NULL;

	allocator->vectorClear(opcodes);
	allocator->vectorClear(debug_info);
}

bool OS::Core::Compiler::saveToStream(StreamWriter * writer, StreamWriter * debug_info_writer)
{
	writer->writeBytes(OS_COMPILED_HEADER, OS_STRLEN(OS_COMPILED_HEADER));

	int i, len = OS_STRLEN(OS_VERSION)+1;
	writer->writeByte(len);
	writer->writeBytes(OS_VERSION, len);

	MemStreamWriter int_stream(allocator);
	MemStreamWriter float_stream(allocator);
	MemStreamWriter double_stream(allocator);
	int int_count = 0, float_count = 0, double_count = 0;
	int int_index = 0, float_index = 0, double_index = 0;
	for(i = 0; i < prog_numbers.count; i++){
		double val = prog_numbers[i];
		if(val >= 0 && (double)(int)val == val){
			int_count++;
			int_stream.writeUVariable(i - int_index); int_index = i;
			int_stream.writeUVariable((int)val);
			continue;
		}
		if((double)(float)val == val){
			float_count++;
			float_stream.writeUVariable(i - float_index); float_index = i;
			float_stream.writeFloat((float)val);
			continue;
		}
		double_count++;
		double_stream.writeUVariable(i - double_index); double_index = i;
		double_stream.writeDouble(val);
	}

	writer->writeUVariable(int_count);
	writer->writeUVariable(float_count);
	writer->writeUVariable(double_count);
	writer->writeUVariable(prog_strings.count);
	writer->writeUVariable(prog_functions.count);
	writer->writeUVariable(prog_opcodes.count);

	writer->writeBytes(int_stream.buffer.buf, int_stream.buffer.count);
	writer->writeBytes(float_stream.buffer.buf, float_stream.buffer.count);
	writer->writeBytes(double_stream.buffer.buf, double_stream.buffer.count);

	for(i = 0; i < prog_strings.count; i++){
		const String& str = prog_strings[i];
		int data_size = str.getDataSize();
		writer->writeUVariable(data_size);
		writer->writeBytes(str.toChar(), data_size);
	}
	for(i = 0; i < prog_functions.count; i++){
		Compiler::Scope * func_scope = prog_functions[i];
		writer->writeUVariable(func_scope->parent ? func_scope->parent->func_index+1 : 0); // prog_functions.indexOf(func_scope->parent));
		writer->writeUVariable(func_scope->stack_size);
		writer->writeUVariable(func_scope->num_locals);
		writer->writeUVariable(func_scope->num_params);
		writer->writeUVariable(func_scope->max_up_count);
		writer->writeUVariable(func_scope->func_depth);
		writer->writeUVariable(func_scope->func_index);
		writer->writeUVariable(func_scope->num_local_funcs);
		writer->writeUVariable(func_scope->opcodes_pos);
		writer->writeUVariable(func_scope->opcodes_size);

		OS_ASSERT(func_scope->locals_compiled.count == func_scope->num_locals);
		for(int j = 0; j < func_scope->locals_compiled.count; j++){
			Compiler::Scope::LocalVarCompiled& var_scope = func_scope->locals_compiled[j];
			OS_ASSERT(var_scope.start_code_pos >= func_scope->opcodes_pos && var_scope.start_code_pos < func_scope->opcodes_pos+func_scope->opcodes_size);
			OS_ASSERT(var_scope.end_code_pos > func_scope->opcodes_pos && var_scope.end_code_pos <= func_scope->opcodes_pos+func_scope->opcodes_size);
			writer->writeUVariable(var_scope.cached_name_index);
			writer->writeUVariable(var_scope.start_code_pos - func_scope->opcodes_pos);
			writer->writeUVariable(var_scope.end_code_pos - func_scope->opcodes_pos);
		}
	}

	for(i = 0; i < prog_opcodes.count; i++){
		writer->writeInt32(prog_opcodes[i]);
	}

	if(debug_info_writer){
		debug_info_writer->writeBytes(OS_DEBUGINFO_HEADER, OS_STRLEN(OS_DEBUGINFO_HEADER));

		len = OS_STRLEN(OS_VERSION)+1;
		debug_info_writer->writeByte(len);
		debug_info_writer->writeBytes(OS_VERSION, len);

		debug_info_writer->writeUVariable(prog_debug_strings.count);
		debug_info_writer->writeUVariable(prog_num_debug_infos);

		for(i = 0; i < prog_debug_strings.count; i++){
			const String& str = prog_debug_strings[i];
			int data_size = str.getDataSize();
			debug_info_writer->writeUVariable(data_size);
			debug_info_writer->writeBytes(str.toChar(), data_size);
		}

		debug_info_writer->writeBytes(prog_debug_info->buffer.buf, prog_debug_info->buffer.count);
	}

	return true;
}

bool OS::Core::Program::loadFromStream(StreamReader * reader, StreamReader * debuginfo_reader)
{
	OS_ASSERT(!opcodes.count && !const_values && !num_numbers && !num_strings && !debug_info.count);

	int i, len = OS_STRLEN(OS_COMPILED_HEADER);
	if(!reader->checkBytes(OS_COMPILED_HEADER, len)){
		return false;
	}

	len = OS_STRLEN(OS_VERSION)+1;
	reader->movePos(1);
	if(!reader->checkBytes(OS_VERSION, len)){
		return false;
	}

	int int_count = reader->readUVariable();
	int float_count = reader->readUVariable();
	int double_count = reader->readUVariable();
	num_numbers = int_count + float_count + double_count;
	num_strings = reader->readUVariable();
	num_functions = reader->readUVariable();
	int opcodes_size = reader->readUVariable();

	const_values = (Value*)allocator->malloc(sizeof(Value) * (num_numbers + num_strings + CONST_STD_VALUES) OS_DBG_FILEPOS);

	const_values[CONST_NULL] = Value();
	const_values[CONST_TRUE] = Value(true);
	const_values[CONST_FALSE] = Value(false);

	int num_index = 0;
	for(i = 0; i < int_count; i++){
		num_index += reader->readUVariable();
		OS_ASSERT(num_index >= 0 && num_index < num_numbers);
		OS_NUMBER number = (OS_NUMBER)reader->readUVariable();
		const_values[num_index + CONST_STD_VALUES] = number;
	}
	for(num_index = 0, i = 0; i < float_count; i++){
		num_index += reader->readUVariable();
		OS_ASSERT(num_index >= 0 && num_index < num_numbers);
		OS_NUMBER number = (OS_NUMBER)reader->readFloat();
		const_values[num_index + CONST_STD_VALUES] = number;
	}
	for(num_index = 0, i = 0; i < double_count; i++){
		num_index += reader->readUVariable();
		OS_ASSERT(num_index >= 0 && num_index < num_numbers);
		OS_NUMBER number = (OS_NUMBER)reader->readDouble();
		const_values[num_index + CONST_STD_VALUES] = number;
	}
	Buffer buf(allocator);
	for(i = 0; i < num_strings; i++){
		int data_size = reader->readUVariable();
		buf.reserveCapacity(data_size);
		reader->readBytes(buf.buffer.buf, data_size);
		buf.buffer.count = data_size;
		GCStringValue * string = buf.toGCStringValue();
		string->external_ref_count++;
		const_values[i + num_numbers + CONST_STD_VALUES] = string;
		buf.freeCacheStr();
	}

	functions = (FunctionDecl*)allocator->malloc(sizeof(FunctionDecl) * num_functions OS_DBG_FILEPOS);
	for(i = 0; i < num_functions; i++){
		FunctionDecl * func = functions + i;
		new (func) FunctionDecl();
#ifdef OS_DEBUG
		func->prog_func_index = i;
#endif
		func->prog_parent_func_index = reader->readUVariable() - 1;
		func->stack_size = reader->readUVariable();
		func->num_locals = reader->readUVariable();
		func->num_params = reader->readUVariable();
		func->max_up_count = reader->readUVariable();
		func->func_depth = reader->readUVariable();
		func->func_index = reader->readUVariable();
		func->num_local_funcs = reader->readUVariable();
		func->opcodes_pos = reader->readUVariable();
		func->opcodes_size = reader->readUVariable();

		func->locals = (FunctionDecl::LocalVar*)allocator->malloc(sizeof(FunctionDecl::LocalVar) * func->num_locals OS_DBG_FILEPOS);
		for(int j = 0; j < func->num_locals; j++){
			int cached_name_index = reader->readUVariable();
			OS_ASSERT(cached_name_index >= 0 && cached_name_index < num_strings);
			FunctionDecl::LocalVar * local_var = func->locals + j;
			OS_ASSERT(dynamic_cast<GCStringValue*>(OS_VALUE_VARIANT(const_values[cached_name_index + num_numbers + CONST_STD_VALUES]).string));
			String var_name(OS_VALUE_VARIANT(const_values[cached_name_index + num_numbers + CONST_STD_VALUES]).string);
			new (local_var) FunctionDecl::LocalVar(var_name);
			local_var->start_code_pos = reader->readUVariable() + func->opcodes_pos;
			local_var->end_code_pos = reader->readUVariable() + func->opcodes_pos;
		}
	}

	allocator->vectorReserveCapacity(opcodes, opcodes_size OS_DBG_FILEPOS);
	opcodes.count = opcodes_size;
	for(i = 0; i < opcodes_size; i++){
		opcodes[i] = (OS_U32)reader->readInt32();
	}

	if(debuginfo_reader){
		len = OS_STRLEN(OS_DEBUGINFO_HEADER);
		if(!debuginfo_reader->checkBytes(OS_DEBUGINFO_HEADER, len)){
			return false;
		}

		len = OS_STRLEN(OS_VERSION)+1;
		debuginfo_reader->movePos(1);
		if(!debuginfo_reader->checkBytes(OS_VERSION, len)){
			return false;
		}
		int num_strings = debuginfo_reader->readUVariable();
		int num_debug_infos = debuginfo_reader->readUVariable();

		Vector<String> strings;
		allocator->vectorReserveCapacity(strings, num_strings OS_DBG_FILEPOS);

		Buffer buf(allocator);
		for(i = 0; i < num_strings; i++){
			int data_size = debuginfo_reader->readUVariable();
			buf.reserveCapacity(data_size);
			debuginfo_reader->readBytes(buf.buffer.buf, data_size);
			buf.buffer.count = data_size;
			allocator->vectorAddItem(strings, buf.toString() OS_DBG_FILEPOS);
			buf.freeCacheStr();
		}

		allocator->vectorReserveCapacity(debug_info, num_debug_infos OS_DBG_FILEPOS);
		for(i = 0; i < num_debug_infos; i++){
			int end_opcode_offs = debuginfo_reader->readUVariable();
			int line = debuginfo_reader->readUVariable();
			int pos = debuginfo_reader->readUVariable();
			int string_index = debuginfo_reader->readUVariable();
			allocator->vectorAddItem(debug_info, DebugInfoItem(end_opcode_offs, line, pos, strings[string_index]) OS_DBG_FILEPOS);
		}
		allocator->vectorClear(strings);
	}

	return true;
}

OS::Core::Program::DebugInfoItem * OS::Core::Program::getDebugInfo(int opcode_pos)
{
	Program::DebugInfoItem * info = NULL;
	for(int i = 0; i < debug_info.count; i++){
		Program::DebugInfoItem * cur = &debug_info[i];
		if(cur->opcode_pos < opcode_pos){
			info = cur;
		}
		if(cur->opcode_pos > opcode_pos){
			break;
		}
	}
	return info;
}

OS::Core::Program::DebugInfoItem::DebugInfoItem(int p_opcode_pos, int p_line, int p_pos, const String& p_token): token(p_token)
{
	opcode_pos = p_opcode_pos;
	line = p_line;
	pos = p_pos;
}

void OS::Core::Program::pushStartFunction()
{
	int opcode = opcodes[0];
	if(OS_GET_OPCODE_TYPE(opcode) != OP_NEW_FUNCTION){
		OS_ASSERT(false);
		allocator->pushNull();
		return;
	}

	int prog_func_index = OS_GETARG_B(opcode);
	OS_ASSERT(prog_func_index == 0 && !OS_GETARG_A(opcode));
	FunctionDecl * func_decl = functions + prog_func_index;
	OS_ASSERT(func_decl->max_up_count == 0);

	GCFunctionValue * func_value = allocator->core->newFunctionValue(NULL, this, func_decl, allocator->core->global_vars);
	allocator->core->pushValue(func_value);
	if(filename.getDataSize()){
		Buffer buf(allocator);
		buf += OS_TEXT("<<");
		buf += allocator->getFilename(filename);
		buf += OS_TEXT(">>");
		func_value->name = buf.toGCStringValue();
	}else{
		func_value->name = OS::String(allocator, OS_TEXT("<<CORE>>")).string;
	}

	allocator->core->gcMarkProgram(this);
}

OS::Core::Program * OS::Core::Program::retain()
{
	ref_count++;
	return this;
}

void OS::Core::Program::release()
{
	if(--ref_count <= 0){
		OS_ASSERT(ref_count == 0);
		OS * allocator = this->allocator;
		this->~Program();
		allocator->free(this);
	}
}

OS::Core::OpcodeType OS::Core::Program::getOpcodeType(Compiler::ExpressionType exp_type)
{
	switch(exp_type){
	case Compiler::EXP_TYPE_CALL: return OP_CALL;
	case Compiler::EXP_TYPE_CALL_AUTO_PARAM: return OP_CALL;
	case Compiler::EXP_TYPE_CALL_METHOD: return OP_CALL_METHOD;
#ifdef OS_TAIL_CALL_ENABLED
	case Compiler::EXP_TYPE_TAIL_CALL: return OP_TAIL_CALL;
	case Compiler::EXP_TYPE_TAIL_CALL_METHOD: return OP_TAIL_CALL_METHOD;
#endif
	case Compiler::EXP_TYPE_SUPER_CALL: return OP_SUPER_CALL;
	// case Compiler::EXP_TYPE_SUPER: return OP_SUPER;

	case Compiler::EXP_TYPE_GET_PROPERTY: return OP_GET_PROPERTY;
	case Compiler::EXP_TYPE_SET_PROPERTY:return OP_SET_PROPERTY;

	case Compiler::EXP_TYPE_GET_UPVALUE: return OP_GET_UPVALUE;
	case Compiler::EXP_TYPE_SET_UPVALUE: return OP_SET_UPVALUE;

	case Compiler::EXP_TYPE_MOVE: return OP_MOVE;
	case Compiler::EXP_TYPE_RETURN: return OP_RETURN;

	case Compiler::EXP_TYPE_BIT_NOT: return OP_BIT_NOT;
	case Compiler::EXP_TYPE_PLUS: return OP_PLUS;
	case Compiler::EXP_TYPE_NEG: return OP_NEG;

	case Compiler::EXP_TYPE_BIT_AND: return OP_BIT_AND;
	case Compiler::EXP_TYPE_BIT_OR: return OP_BIT_OR;
	case Compiler::EXP_TYPE_BIT_XOR: return OP_BIT_XOR;

	// case Compiler::EXP_TYPE_CONCAT: return OP_CONCAT;
	case Compiler::EXP_TYPE_COMPARE: return OP_COMPARE;
	case Compiler::EXP_TYPE_ADD: return OP_ADD;
	case Compiler::EXP_TYPE_SUB: return OP_SUB;
	case Compiler::EXP_TYPE_MUL: return OP_MUL;
	case Compiler::EXP_TYPE_DIV: return OP_DIV;
	case Compiler::EXP_TYPE_MOD: return OP_MOD;
	case Compiler::EXP_TYPE_LSHIFT: return OP_LSHIFT;
	case Compiler::EXP_TYPE_RSHIFT: return OP_RSHIFT;
	case Compiler::EXP_TYPE_POW: return OP_POW;
	}
	OS_ASSERT(false);
	return OP_NOP;
}

// =====================================================================
// =====================================================================
// =====================================================================

OS::Core::StreamWriter::StreamWriter(OS * p_allocator)
{
	allocator = p_allocator;
}

OS::Core::StreamWriter::~StreamWriter()
{
}

void OS::Core::StreamWriter::writeFromStream(StreamReader * reader)
{
	int size = reader->getSize() - reader->getPos();
	int buf_size = 1024 * 16;
	void * buf = allocator->malloc(buf_size < size ? buf_size : size OS_DBG_FILEPOS);
	OS_ASSERT(buf || !size);
	for(; size > 0; size -= buf_size){
		int chunk_size = buf_size <= size ? buf_size : size;
		reader->readBytes(buf, chunk_size);
		writeBytes(buf, chunk_size);
	}
	allocator->free(buf);
}

void OS::Core::StreamWriter::writeByte(int value)
{
	OS_ASSERT(value >= 0 && value <= 0xff);
	OS_BYTE le_value = toLittleEndianByteOrder((OS_BYTE)value);
	writeBytes(&le_value, sizeof(le_value));
}

void OS::Core::StreamWriter::writeByteAtPos(int value, int pos)
{
	OS_ASSERT(value >= 0 && value <= 0xff);
	OS_BYTE le_value = toLittleEndianByteOrder((OS_BYTE)value);
	writeBytesAtPos(&le_value, sizeof(le_value), pos);
}

void OS::Core::StreamWriter::writeUVariable(int value)
{
	OS_ASSERT(value >= 0);
	for(;;){
		if(value >= 0x7f){
			writeByte((value & 0x7f) | 0x80);
			value >>= 7;
		}else{
			writeByte(value);
			return;
		}
	}
}

void OS::Core::StreamWriter::writeU16(int value)
{
	OS_ASSERT(value >= 0 && value <= 0xffff);
	OS_U16 le_value = toLittleEndianByteOrder((OS_U16)value);
	writeBytes(&le_value, sizeof(le_value));
}

void OS::Core::StreamWriter::writeU16AtPos(int value, int pos)
{
	OS_ASSERT(value >= 0 && value <= 0xffff);
	OS_U16 le_value = toLittleEndianByteOrder((OS_U16)value);
	writeBytesAtPos(&le_value, sizeof(le_value), pos);
}

void OS::Core::StreamWriter::writeInt8(int value)
{
	OS_ASSERT((int)(OS_INT8)value == value);
	OS_INT8 le_value = toLittleEndianByteOrder((OS_INT8)value);
	writeBytes(&le_value, sizeof(le_value));
}

void OS::Core::StreamWriter::writeInt8AtPos(int value, int pos)
{
	OS_ASSERT((int)(OS_INT8)value == value);
	OS_INT8 le_value = toLittleEndianByteOrder((OS_INT8)value);
	writeBytesAtPos(&le_value, sizeof(le_value), pos);
}

void OS::Core::StreamWriter::writeInt16(int value)
{
	OS_ASSERT((int)(OS_INT16)value == value);
	OS_INT16 le_value = toLittleEndianByteOrder((OS_INT16)value);
	writeBytes(&le_value, sizeof(le_value));
}

void OS::Core::StreamWriter::writeInt16AtPos(int value, int pos)
{
	OS_ASSERT((int)(OS_INT16)value == value);
	OS_INT16 le_value = toLittleEndianByteOrder((OS_INT16)value);
	writeBytesAtPos(&le_value, sizeof(le_value), pos);
}

void OS::Core::StreamWriter::writeInt32(int value)
{
	OS_INT32 le_value = toLittleEndianByteOrder((OS_INT32)value);
	writeBytes(&le_value, sizeof(le_value));
}

void OS::Core::StreamWriter::writeInt32AtPos(int value, int pos)
{
	OS_ASSERT((int)(OS_INT32)value == value);
	OS_INT32 le_value = toLittleEndianByteOrder((OS_INT32)value);
	writeBytesAtPos(&le_value, sizeof(le_value), pos);
}

void OS::Core::StreamWriter::writeInt64(OS_INT64 value)
{
	OS_INT64 le_value = toLittleEndianByteOrder((OS_INT64)value);
	writeBytes(&le_value, sizeof(le_value));
}

void OS::Core::StreamWriter::writeInt64AtPos(OS_INT64 value, int pos)
{
	OS_INT64 le_value = toLittleEndianByteOrder((OS_INT64)value);
	writeBytesAtPos(&le_value, sizeof(le_value), pos);
}

void OS::Core::StreamWriter::writeFloat(float value)
{
	float le_value = toLittleEndianByteOrder(value);
	writeBytes(&le_value, sizeof(le_value));
}

void OS::Core::StreamWriter::writeFloatAtPos(float value, int pos)
{
	float le_value = toLittleEndianByteOrder(value);
	writeBytesAtPos(&le_value, sizeof(le_value), pos);
}

void OS::Core::StreamWriter::writeDouble(double value)
{
	double le_value = toLittleEndianByteOrder(value);
	writeBytes(&le_value, sizeof(le_value));
}

void OS::Core::StreamWriter::writeDoubleAtPos(double value, int pos)
{
	double le_value = toLittleEndianByteOrder(value);
	writeBytesAtPos(&le_value, sizeof(le_value), pos);
}

// =====================================================================

OS::Core::MemStreamWriter::MemStreamWriter(OS * allocator): StreamWriter(allocator)
{
	pos = 0;
}

OS::Core::MemStreamWriter::~MemStreamWriter()
{
	allocator->vectorClear(buffer);
}

int OS::Core::MemStreamWriter::getPos() const
{
	return pos;
}

void OS::Core::MemStreamWriter::setPos(int new_pos)
{
	OS_ASSERT(new_pos >= 0 && new_pos <= buffer.count);
	pos = new_pos;
}

int OS::Core::MemStreamWriter::getSize() const
{
	return buffer.count;
}

void OS::Core::MemStreamWriter::reserveCapacity(int new_capacity)
{
	if(buffer.capacity < new_capacity){
		buffer.capacity = buffer.capacity > 0 ? buffer.capacity*2 : 4;
		if(buffer.capacity < new_capacity){
			buffer.capacity = new_capacity; // (capacity+3) & ~3;
		}
		OS_BYTE * new_buf = (OS_BYTE*)allocator->malloc(sizeof(OS_BYTE)*buffer.capacity OS_DBG_FILEPOS);
		OS_ASSERT(new_buf);
		OS_MEMCPY(new_buf, buffer.buf, sizeof(OS_BYTE) * buffer.count);
		allocator->free(buffer.buf);
		buffer.buf = new_buf;
	}
}

void OS::Core::MemStreamWriter::writeBytes(const void * buf, int len)
{
	reserveCapacity(pos + len);
	int save_pos = pos;
	pos += len;
	if(buffer.count <= pos){
		buffer.count = pos;
	}
	writeBytesAtPos(buf, len, save_pos);
}

void OS::Core::MemStreamWriter::writeBytesAtPos(const void * buf, int len, int pos)
{
	OS_ASSERT(pos >= 0 && pos <= buffer.count-len);
	OS_MEMCPY(buffer.buf+pos, buf, len);
}

void OS::Core::MemStreamWriter::writeByte(int value)
{
	OS_ASSERT(value >= 0 && value <= 0xff);
	if(pos < buffer.count){
		OS_ASSERT(pos >= 0);
		buffer[pos++] = (OS_BYTE)value;
	}else{
		allocator->vectorAddItem(buffer, (OS_BYTE)value OS_DBG_FILEPOS);
		pos++;
	}
}

void OS::Core::MemStreamWriter::writeByteAtPos(int value, int pos)
{
	OS_ASSERT(value >= 0 && value <= 0xff);
	OS_ASSERT(pos >= 0 && pos <= buffer.count-1);
	buffer[pos] = (OS_BYTE)value;
}

// =====================================================================

OS::Core::FileStreamWriter::FileStreamWriter(OS * allocator, const OS_CHAR * filename): StreamWriter(allocator)
{
	f = allocator->openFile(filename, "wb");
}

OS::Core::FileStreamWriter::~FileStreamWriter()
{
	allocator->closeFile(f);
}

int OS::Core::FileStreamWriter::getPos() const
{
	return allocator->seekFile(f, 0, SEEK_CUR);
}

void OS::Core::FileStreamWriter::setPos(int new_pos)
{
	OS_ASSERT(new_pos >= 0 && new_pos <= getSize());
	allocator->seekFile(f, new_pos, SEEK_SET);
}

int OS::Core::FileStreamWriter::getSize() const
{
	if(f){
		int save_pos = getPos();
		allocator->seekFile(f, 0, SEEK_END);
		int size = getPos();
		allocator->seekFile(f, save_pos, SEEK_SET);
		return size;
	}
	return 0;
}

void OS::Core::FileStreamWriter::writeBytes(const void * buf, int len)
{
	allocator->writeFile(buf, len, f);
}

void OS::Core::FileStreamWriter::writeBytesAtPos(const void * buf, int len, int pos)
{
	int save_pos = getPos();
	allocator->seekFile(f, pos, SEEK_SET);
	writeBytes(buf, len);
	allocator->seekFile(f, save_pos, SEEK_SET);
}

// =====================================================================
// =====================================================================
// =====================================================================

OS::Core::StreamReader::StreamReader(OS * p_allocator)
{
	allocator = p_allocator;
}

OS::Core::StreamReader::~StreamReader()
{
}

OS_BYTE OS::Core::StreamReader::readByte()
{
	OS_BYTE le_value;
	readBytes(&le_value, sizeof(le_value));
	return fromLittleEndianByteOrder(le_value);
}

OS_BYTE OS::Core::StreamReader::readByteAtPos(int pos)
{
	OS_BYTE le_value;
	readBytesAtPos(&le_value, sizeof(le_value), pos);
	return fromLittleEndianByteOrder(le_value);
}

int OS::Core::StreamReader::readUVariable()
{
	int value = readByte();
	if(!(value & 0x80)){
		return value;
	}
	value &= 0x7f;
	for(int i = 7;; i += 7){
		int b = readByte();
		if(b & 0x80){
			value |= (b & 0x7f) << i;
		}else{
			OS_ASSERT((value | (b << i)) >= 0);
			return value | (b << i);
		}
	}
	return 0; // shut up compiler
}

OS_U16 OS::Core::StreamReader::readU16()
{
	OS_U16 le_value;
	readBytes(&le_value, sizeof(le_value));
	return fromLittleEndianByteOrder(le_value);
}

OS_U16 OS::Core::StreamReader::readU16AtPos(int pos)
{
	OS_U16 le_value;
	readBytesAtPos(&le_value, sizeof(le_value), pos);
	return fromLittleEndianByteOrder(le_value);
}

OS_INT8 OS::Core::StreamReader::readInt8()
{
	OS_INT8 le_value;
	readBytes(&le_value, sizeof(le_value));
	return fromLittleEndianByteOrder(le_value);
}

OS_INT8 OS::Core::StreamReader::readInt8AtPos(int pos)
{
	OS_INT8 le_value;
	readBytesAtPos(&le_value, sizeof(le_value), pos);
	return fromLittleEndianByteOrder(le_value);
}

OS_INT16 OS::Core::StreamReader::readInt16()
{
	OS_INT16 le_value;
	readBytes(&le_value, sizeof(le_value));
	return fromLittleEndianByteOrder(le_value);
}

OS_INT16 OS::Core::StreamReader::readInt16AtPos(int pos)
{
	OS_INT16 le_value;
	readBytesAtPos(&le_value, sizeof(le_value), pos);
	return fromLittleEndianByteOrder(le_value);
}

OS_INT32 OS::Core::StreamReader::readInt32()
{
	OS_INT32 le_value;
	readBytes(&le_value, sizeof(le_value));
	return fromLittleEndianByteOrder(le_value);
}

OS_INT32 OS::Core::StreamReader::readInt32AtPos(int pos)
{
	OS_INT32 le_value;
	readBytesAtPos(&le_value, sizeof(le_value), pos);
	return fromLittleEndianByteOrder(le_value);
}

OS_INT64 OS::Core::StreamReader::readInt64()
{
	OS_INT64 le_value;
	readBytes(&le_value, sizeof(le_value));
	return fromLittleEndianByteOrder(le_value);
}

OS_INT64 OS::Core::StreamReader::readInt64AtPos(int pos)
{
	OS_INT64 le_value;
	readBytesAtPos(&le_value, sizeof(le_value), pos);
	return fromLittleEndianByteOrder(le_value);
}

float OS::Core::StreamReader::readFloat()
{
	float le_value;
	readBytes(&le_value, sizeof(le_value));
	return fromLittleEndianByteOrder(le_value);
}

float OS::Core::StreamReader::readFloatAtPos(int pos)
{
	float le_value;
	readBytesAtPos(&le_value, sizeof(le_value), pos);
	return fromLittleEndianByteOrder(le_value);
}

double OS::Core::StreamReader::readDouble()
{
	double le_value;
	readBytes(&le_value, sizeof(le_value));
	return fromLittleEndianByteOrder(le_value);
}

double OS::Core::StreamReader::readDoubleAtPos(int pos)
{
	double le_value;
	readBytesAtPos(&le_value, sizeof(le_value), pos);
	return fromLittleEndianByteOrder(le_value);
}

// =====================================================================

OS::Core::MemStreamReader::MemStreamReader(OS * allocator, int buf_size): StreamReader(allocator)
{
	cur = buffer = (OS_BYTE*)allocator->malloc(buf_size OS_DBG_FILEPOS);
	size = buf_size;
}

OS::Core::MemStreamReader::MemStreamReader(OS * allocator, OS_BYTE * buf, int buf_size): StreamReader(allocator)
{
	cur = buffer = buf;
	size = buf_size;
}

OS::Core::MemStreamReader::~MemStreamReader()
{
	if(allocator){
		allocator->free(buffer);
	}
}

int OS::Core::MemStreamReader::getPos() const
{
	return cur - buffer;
}

void OS::Core::MemStreamReader::setPos(int new_pos)
{
	OS_ASSERT(new_pos >= 0 && new_pos <= size);
	cur = buffer + new_pos;
}

int OS::Core::MemStreamReader::getSize() const
{
	return size;
}

void OS::Core::MemStreamReader::movePos(int len)
{
	OS_ASSERT(getPos()+len >= 0 && getPos()+len <= size);
	cur += len;
}

bool OS::Core::MemStreamReader::checkBytes(const void * src, int len)
{
	OS_ASSERT(getPos() >= 0 && getPos()+len <= size);
	bool r = OS_MEMCMP(cur, src, len) == 0;
	cur += len;
	return r;
}

void * OS::Core::MemStreamReader::readBytes(void * dst, int len)
{
	OS_ASSERT(getPos() >= 0 && getPos()+len <= size);
	OS_MEMCPY(dst, cur, len);
	cur += len;
	return dst;
}

void * OS::Core::MemStreamReader::readBytesAtPos(void * dst, int len, int pos)
{
	OS_ASSERT(pos >= 0 && pos+len <= size);
	OS_MEMCPY(dst, buffer + pos, len);
	return dst;
}

OS_BYTE OS::Core::MemStreamReader::readByte()
{
	OS_ASSERT(getPos() >= 0 && getPos()+(int)sizeof(OS_BYTE) <= size);
	return *cur++;
}

OS_BYTE OS::Core::MemStreamReader::readByteAtPos(int pos)
{
	OS_ASSERT(pos >= 0 && pos+(int)sizeof(OS_BYTE) <= size);
	return buffer[pos];
}

OS_INT8 OS::Core::MemStreamReader::readInt8()
{
	OS_ASSERT(getPos() >= 0 && getPos()+1 <= size);
	return (OS_INT8)*cur++;
}

OS_INT16 OS::Core::MemStreamReader::readInt16()
{
	OS_ASSERT(getPos() >= 0 && getPos()+(int)sizeof(OS_INT16) <= size);
	OS_BYTE * buf = cur;
	cur += sizeof(OS_INT16);
	OS_INT16 value = buf[0] | (buf[1] << 8);
	return value;
}

OS_INT32 OS::Core::MemStreamReader::readInt32()
{
	OS_ASSERT(getPos() >= 0 && getPos()+(int)sizeof(OS_INT32) <= size);
	OS_BYTE * buf = cur;
	cur += sizeof(OS_INT32);
	OS_INT32 value = buf[0] | (buf[1] << 8) | (buf[2] << 16) | (buf[3] << 24);
	return value;
}

// =====================================================================

OS::Core::FileStreamReader::FileStreamReader(OS * allocator, const OS_CHAR * filename): StreamReader(allocator)
{
	f = allocator->openFile(filename, "rb");
}

OS::Core::FileStreamReader::~FileStreamReader()
{
	allocator->closeFile(f);
}

int OS::Core::FileStreamReader::getPos() const
{
	return allocator->seekFile(f, 0, SEEK_CUR);
}

void OS::Core::FileStreamReader::setPos(int new_pos)
{
	OS_ASSERT(new_pos >= 0 && new_pos <= getSize());
	allocator->seekFile(f, new_pos, SEEK_SET);
}

int OS::Core::FileStreamReader::getSize() const
{
	if(f){
		int save_pos = getPos();
		allocator->seekFile(f, 0, SEEK_END);
		int size = getPos();
		allocator->seekFile(f, save_pos, SEEK_SET);
		return size;
	}
	return 0;
}

void OS::Core::FileStreamReader::movePos(int len)
{
	allocator->seekFile(f, len, SEEK_CUR);
}

bool OS::Core::FileStreamReader::checkBytes(const void * src, int len)
{
	void * buf = alloca(len);
	readBytes(buf, len);
	return OS_MEMCMP(buf, src, len) == 0;
}

void * OS::Core::FileStreamReader::readBytes(void * buf, int len)
{
	if(!f || !allocator->readFile(buf, len, f)){
		OS_MEMSET(buf, 0, len);
	}
	return buf;
}

void * OS::Core::FileStreamReader::readBytesAtPos(void * buf, int len, int pos)
{
	int save_pos = getPos();
	allocator->seekFile(f, pos, SEEK_SET);
	readBytes(buf, len);
	allocator->seekFile(f, save_pos, SEEK_SET);
	return buf;
}

// =====================================================================
// =====================================================================
// =====================================================================

/*
static bool isDecString(const OS_CHAR * str, int len)
{
	OS_INT val;
	const OS_CHAR * end = str + len;
	return parseSimpleDec(str, val) && str == end;
}
*/

OS::Core::PropertyIndex::PropertyIndex(const PropertyIndex& p_index): index(p_index.index)
{
}

OS::Core::PropertyIndex::PropertyIndex(const Value& p_index): index(p_index)
{
	convertIndexStringToNumber();
}

OS::Core::PropertyIndex::PropertyIndex(const Value& p_index, const KeepStringIndex&): index(p_index)
{
	OS_ASSERT(OS_VALUE_TYPE(index) != OS_VALUE_TYPE_STRING || OS_VALUE_TYPE(PropertyIndex(p_index).index) == OS_VALUE_TYPE_STRING);
}

OS::Core::PropertyIndex::PropertyIndex(GCStringValue * p_index): index(p_index)
{
	convertIndexStringToNumber();
}

OS::Core::PropertyIndex::PropertyIndex(GCStringValue * p_index, const KeepStringIndex&): index(p_index)
{
	// OS_ASSERT(index.type != OS_VALUE_TYPE_STRING || PropertyIndex(p_index).index.type == OS_VALUE_TYPE_STRING);
}

OS::Core::PropertyIndex::PropertyIndex(const String& p_index): index(p_index)
{
	convertIndexStringToNumber();
}

OS::Core::PropertyIndex::PropertyIndex(const String& p_index, const KeepStringIndex&): index(p_index)
{
	// OS_ASSERT(index.type != OS_VALUE_TYPE_STRING || PropertyIndex(p_index).index.type == OS_VALUE_TYPE_STRING);
}

void OS::Core::PropertyIndex::convertIndexStringToNumber()
{
	if(OS_VALUE_TYPE(index) == OS_VALUE_TYPE_STRING){
		bool neg = false;
		const OS_CHAR * str = OS_VALUE_VARIANT(index).string->toChar();
		if((*str >= OS_TEXT('0') && *str <= OS_TEXT('9'))
			|| ((neg = *str == OS_TEXT('-')) && str[1] >= OS_TEXT('0') && str[1] <= OS_TEXT('9')))
		{
			const OS_CHAR * end = str + OS_VALUE_VARIANT(index).string->getLen();
			str += (int)neg;
			OS_FLOAT val;
			if(parseSimpleFloat(str, val)){
				if(*str == OS_TEXT('.')){
					OS_FLOAT m = (OS_FLOAT)0.1;
					for(str++; *str >= OS_TEXT('0') && *str <= OS_TEXT('9'); str++, m *= (OS_FLOAT)0.1){
						val += (OS_FLOAT)(*str - OS_TEXT('0')) * m;
					}
				}
				if(str == end){
					OS_SET_VALUE_NUMBER(index, (OS_NUMBER)(neg ? -val : val));
					// index.type = OS_VALUE_TYPE_NUMBER;
					OS_ASSERT(OS_IS_VALUE_NUMBER(index));
				}
			}
		}
	}
}

bool OS::Core::PropertyIndex::isEqual(const PropertyIndex& b) const
{
	int type = OS_VALUE_TYPE(index);
	switch(type){
	case OS_VALUE_TYPE_NULL:
		return OS_VALUE_TYPE(b.index) == OS_VALUE_TYPE_NULL;

		// case OS_VALUE_TYPE_BOOL:
		//	return b.index.type == OS_VALUE_TYPE_BOOL && index.v.boolean == b.index.v.boolean;

	case OS_VALUE_TYPE_NUMBER:
		return OS_VALUE_TYPE(b.index) == OS_VALUE_TYPE_NUMBER && OS_VALUE_NUMBER(index) == OS_VALUE_NUMBER(b.index);
	}
	return type == OS_VALUE_TYPE(b.index) && OS_VALUE_VARIANT(index).value == OS_VALUE_VARIANT(b.index).value;
}

bool OS::Core::GCStringValue::isEqual(int hash, const void * b, int size) const
{
	return this->hash == hash 
		&& data_size == size
		&& OS_MEMCMP(toMemory(), b, size) == 0;
}

bool OS::Core::GCStringValue::isEqual(int hash, const void * buf1, int size1, const void * buf2, int size2) const
{
	if(this->hash != hash || data_size != size1 + size2){
		return false;
	}
	const OS_BYTE * src = toBytes();
	return OS_MEMCMP(src, buf1, size1) == 0
		&& (!size2 || OS_MEMCMP(src + size1, buf2, size2) == 0);
}

bool OS::Core::PropertyIndex::isEqual(int hash, const void * b, int size) const
{
	if(OS_VALUE_TYPE(index) == OS_VALUE_TYPE_STRING){
		return OS_VALUE_VARIANT(index).string->hash == hash 
			&& OS_VALUE_VARIANT(index).string->data_size == size
			&& OS_MEMCMP(OS_VALUE_VARIANT(index).string->toMemory(), b, size) == 0;
	}
	return false;
}

bool OS::Core::PropertyIndex::isEqual(int hash, const void * buf1, int size1, const void * buf2, int size2) const
{
	if(OS_VALUE_TYPE(index) == OS_VALUE_TYPE_STRING){
		int src_size = OS_VALUE_VARIANT(index).string->data_size;
		if(OS_VALUE_VARIANT(index).string->hash != hash || src_size != size1 + size2){
			return false;
		}
		const OS_BYTE * src = OS_VALUE_VARIANT(index).string->toBytes();
		return Utils::cmp(src, size1, buf1, size1) == 0
			&& Utils::cmp(src + size1, size2, buf2, size2) == 0;
	}
	return false;
}

template <class T> int getNumberHash(T val)
{
	return (int)val;
}
template <> int getNumberHash<double>(double val)
{
	float t = (float)val;
	return *(int*)&t;
}
template <> int getNumberHash<float>(float t)
{
	return *(int*)&t;
}
template <> int getNumberHash<int>(int t)
{
	return t;
}

int OS::Core::PropertyIndex::getHash() const
{
	switch(OS_VALUE_TYPE(index)){
	case OS_VALUE_TYPE_NUMBER:
		{
			union { 
				double d; 
				OS_INT32 p[2];
			} u;
			u.d = (double)OS_VALUE_NUMBER(index); // + 1.0f;
			return u.p[0] + u.p[1];
		}

	case OS_VALUE_TYPE_STRING:
		return OS_VALUE_VARIANT(index).string->hash;
	}
	// all other values share same area with index.v.value so just use it as hash
	return (ptrdiff_t) OS_VALUE_VARIANT(index).value;
}

// =====================================================================

OS::Core::Property::Property(const PropertyIndex& index): PropertyIndex(index)
{
	hash_next = NULL;
	prev = NULL;
	next = NULL;
}

OS::Core::Property::Property(Value index): PropertyIndex(index)
{
	hash_next = NULL;
	prev = NULL;
	next = NULL;
}

OS::Core::Property::Property(Value index, const KeepStringIndex& keep): PropertyIndex(index, keep)
{
	hash_next = NULL;
	prev = NULL;
	next = NULL;
}

OS::Core::Property::Property(GCStringValue * index): PropertyIndex(index)
{
	hash_next = NULL;
	prev = NULL;
	next = NULL;
}

OS::Core::Property::Property(GCStringValue * index, const KeepStringIndex& keep): PropertyIndex(index, keep)
{
	hash_next = NULL;
	prev = NULL;
	next = NULL;
}

OS::Core::Property::~Property()
{
	OS_ASSERT(!hash_next);
	OS_ASSERT(!prev);
	OS_ASSERT(!next);
}

// =====================================================================

OS::Core::Table::IteratorState::IteratorState()
{
	table = NULL;
	prop = NULL;
	next = NULL;
	ascending = true;
}

OS::Core::Table::IteratorState::~IteratorState()
{
	OS_ASSERT(!table && !prop && !next);
}

OS::Core::Table::Table()
{
	head_mask = 0;
	heads = NULL;
	next_index = 0;
	count = 0;
	first = last = NULL;
	iterators = NULL;
}

OS::Core::Table::~Table()
{
	OS_ASSERT(count == 0 && !first && !last && !iterators);
	OS_ASSERT(!heads);
}

bool OS::Core::Table::containsIterator(IteratorState * iter)
{
	for(IteratorState * cur = iterators; cur; cur = cur->next){
		if(cur == iter){
			OS_ASSERT(iter->table == this);
			return true;
		}
	}
	OS_ASSERT(iter->table != this);
	return false;
}

void OS::Core::Table::addIterator(IteratorState * iter)
{
	OS_ASSERT(!containsIterator(iter));
	OS_ASSERT(!iter->prop && !iter->table);
	iter->table = this;
	iter->prop = iter->ascending ? first : last;
	iter->next = iterators;
	iterators = iter;
}

void OS::Core::Table::removeIterator(IteratorState * iter)
{
	OS_ASSERT(containsIterator(iter));
	IteratorState * prev = NULL;
	for(IteratorState * cur = iterators; cur; prev = cur, cur = cur->next){
		if(cur == iter){
			if(!prev){
				iterators = cur->next;
			}else{
				prev->next = cur->next;
			}
			cur->table = NULL;
			cur->next = NULL;
			cur->prop = NULL;
			return;
		}
	}
	OS_ASSERT(false);
}

OS::Core::Table * OS::Core::newTable(OS_DBG_FILEPOS_START_DECL)
{
	return new (malloc(sizeof(Table) OS_DBG_FILEPOS_PARAM)) Table();
}

void OS::Core::clearTable(Table * table)
{
	OS_ASSERT(table);
	Property * prop = table->last, * prev;

	table->count = 0;
	table->first = NULL;
	table->last = NULL;

	for(; prop; prop = prev){
		prev = prop->prev;
		prop->hash_next = NULL;
		prop->prev = NULL;
		prop->next = NULL;
		prop->~Property();
		free(prop);
	}

	while(table->iterators){
		table->removeIterator(table->iterators);
	}

	// OS_ASSERT(table->count == 0 && !table->first && !table->last);
	free(table->heads);
	table->heads = NULL;
	table->head_mask = 0;
	table->next_index = 0;
}

void OS::Core::deleteTable(Table * table)
{
	OS_ASSERT(table);
	clearTable(table);
	table->~Table();
	free(table);
}

void OS::Core::addTableProperty(Table * table, Property * prop)
{
	OS_ASSERT(prop->next == NULL);
	OS_ASSERT(!table->get(*prop));

	if((table->count>>HASH_GROW_SHIFT) >= table->head_mask){
		int new_size = table->heads ? (table->head_mask+1) * 2 : 4;
		int alloc_size = sizeof(Property*)*new_size;
		Property ** new_heads = (Property**)malloc(alloc_size OS_DBG_FILEPOS);
		OS_ASSERT(new_heads);
		OS_MEMSET(new_heads, 0, alloc_size);

		Property ** old_heads = table->heads;
		table->heads = new_heads;
		table->head_mask = new_size-1;

		for(Property * cur = table->first; cur; cur = cur->next){
			int slot = cur->getHash() & table->head_mask;
			cur->hash_next = table->heads[slot];
			table->heads[slot] = cur;
		}

		// delete [] old_heads;
		free(old_heads);
	}

	int slot = prop->getHash() & table->head_mask;
	prop->hash_next = table->heads[slot];
	table->heads[slot] = prop;

	if(!table->first){
		table->first = prop;    
	}else{
		OS_ASSERT(table->last);
		table->last->next = prop;
		prop->prev = table->last;
	}
	table->last = prop;

	if(OS_IS_VALUE_NUMBER(prop->index) && table->next_index <= OS_VALUE_NUMBER(prop->index)){
		table->next_index = (OS_INT) OS_VALUE_NUMBER(prop->index) + 1;
	}

	table->count++;
}

void OS::Core::changePropertyIndex(Table * table, Property * prop, const PropertyIndex& new_index)
{
	int slot = prop->getHash() & table->head_mask;
	Property * cur = table->heads[slot], * chain_prev = NULL;
	for(; cur; chain_prev = cur, cur = cur->hash_next){
		if(cur == prop){ // cur->isEqual(index)){
			if(chain_prev){
				chain_prev->hash_next = cur->hash_next;
			}else{
				table->heads[slot] = cur->hash_next;
			}
			break;
		}
	}
	OS_ASSERT(cur && cur == prop);
	if(cur){
		*prop = new_index;

		slot = prop->getHash() & table->head_mask;
		prop->hash_next = table->heads[slot];
		table->heads[slot] = prop;

		if(OS_IS_VALUE_NUMBER(prop->index) && table->next_index <= OS_VALUE_NUMBER(prop->index)){
			table->next_index = (OS_INT)OS_VALUE_NUMBER(prop->index) + 1;
		}
	}
}

OS::Core::Property * OS::Core::removeTableProperty(Table * table, const PropertyIndex& index)
{
	OS_ASSERT(table);
	int slot = index.getHash() & table->head_mask;
	Property * cur = table->heads[slot], * chain_prev = NULL;
	for(; cur; chain_prev = cur, cur = cur->hash_next){
		if(cur->isEqual(index)){
			if(table->first == cur){
				table->first = cur->next;
				if(table->first){
					table->first->prev = NULL;
				}
			}else{
				OS_ASSERT(cur->prev);
				cur->prev->next = cur->next;
			}

			if(table->last == cur){
				table->last = cur->prev;
				if(table->last){
					table->last->next = NULL;
				}
			}else{
				OS_ASSERT(cur->next);
				cur->next->prev = cur->prev;
			}

			if(chain_prev){
				chain_prev->hash_next = cur->hash_next;
			}else{
				table->heads[slot] = cur->hash_next;
			}

			for(Table::IteratorState * iter = table->iterators; iter; iter = iter->next){
				if(iter->prop == cur){
					iter->prop = iter->ascending ? cur->next : cur->prev;
				}
			}

			cur->next = NULL;
			cur->prev = NULL;
			cur->hash_next = NULL;
			// cur->value.clear();

			table->count--;

			return cur;
		}
	}  
	return NULL;
}

bool OS::Core::deleteTableProperty(Table * table, const PropertyIndex& index)
{
	Property * prop = removeTableProperty(table, index);
	if(prop){
		prop->~Property();
		free(prop);
		return true;
	}
	return false;
}

void OS::Core::deleteValueProperty(GCValue * table_value, const PropertyIndex& index, bool del_enabled, bool prototype_enabled)
{
	Table * table = table_value->table;
	if(table && deleteTableProperty(table, index)){
		return;
	}
	if(1){ // prototype_enabled){
		GCValue * cur_value = table_value;
		while(cur_value->prototype){
			cur_value = cur_value->prototype;
			Table * cur_table = cur_value->table;
			if(!cur_table){
				continue;
			}
			if(prototype_enabled){
				if(cur_table && deleteTableProperty(cur_table, index)){
					return;
				}
			}else{
				if(cur_table && cur_table->get(index)){
					return;
				}
			}
		}
	}
	if(OS_VALUE_TYPE(index.index) == OS_VALUE_TYPE_STRING && strings->syntax_prototype == OS_VALUE_VARIANT(index.index).string){
		return;
	}
	if(table_value->type == OS_VALUE_TYPE_ARRAY){
		OS_ASSERT(dynamic_cast<GCArrayValue*>(table_value));
		GCArrayValue * arr = (GCArrayValue*)table_value;
		int i = (int)valueToInt(index.index);
		if(i >= 0 && i < arr->values.count){
			allocator->vectorRemoveAtIndex(arr->values, i);
		}
		return;
	}
	if(del_enabled && !hasSpecialPrefix(index.index)){
		Value value;
		if(OS_VALUE_TYPE(index.index) == OS_VALUE_TYPE_STRING){
			const void * buf1 = strings->__delAt.toChar();
			int size1 = strings->__delAt.getDataSize();
			const void * buf2 = OS_VALUE_VARIANT(index.index).string->toChar();
			int size2 = OS_VALUE_VARIANT(index.index).string->getDataSize();
			GCStringValue * del_name = newStringValue(buf1, size1, buf2, size2);
			if(getPropertyValue(value, table_value, PropertyIndex(del_name, PropertyIndex::KeepStringIndex()), prototype_enabled)
				&& value.isFunction())
			{
				pushValue(value);
				pushValue(table_value);
				pushValue(index.index);
				call(1, 0);
				return;
			}
		}
		if(getPropertyValue(value, table_value, PropertyIndex(strings->__del, PropertyIndex::KeepStringIndex()), prototype_enabled)
			&& value.isFunction())
		{
			pushValue(value);
			pushValue(table_value);
			pushValue(index.index);
			call(1, 0);
		}
	}
}

void OS::Core::deleteValueProperty(const Value& table_value, const PropertyIndex& index, bool del_enabled, bool prototype_enabled)
{
	switch(OS_VALUE_TYPE(table_value)){
	case OS_VALUE_TYPE_NULL:
		return;

	case OS_VALUE_TYPE_BOOL:
		if(prototype_enabled){
			return deleteValueProperty(prototypes[PROTOTYPE_BOOL], index, del_enabled, prototype_enabled);
		}
		return;

	case OS_VALUE_TYPE_NUMBER:
		if(prototype_enabled){
			return deleteValueProperty(prototypes[PROTOTYPE_NUMBER], index, del_enabled, prototype_enabled);
		}
		return;

	case OS_VALUE_TYPE_STRING:
	case OS_VALUE_TYPE_ARRAY:
	case OS_VALUE_TYPE_OBJECT:
	case OS_VALUE_TYPE_USERDATA:
	case OS_VALUE_TYPE_USERPTR:
	case OS_VALUE_TYPE_FUNCTION:
	case OS_VALUE_TYPE_CFUNCTION:
		return deleteValueProperty(OS_VALUE_VARIANT(table_value).value, index, del_enabled, prototype_enabled);
	}
}

void OS::Core::copyTableProperties(Table * dst, Table * src)
{
	for(Property * prop = src->first; prop; prop = prop->next){
		setTableValue(dst, PropertyIndex(*prop), prop->value);
	}
}

void OS::Core::copyTableProperties(GCValue * dst_value, GCValue * src_value, bool setter_enabled)
{
	if(src_value->table){
		for(Property * prop = src_value->table->first; prop; prop = prop->next){
			setPropertyValue(dst_value, *prop, prop->value, setter_enabled);
		}
	}
}

void OS::Core::sortTable(Table * table, int(*comp)(OS*, const void*, const void*, void*), void * user_param, bool reorder_keys)
{
	if(table->count > 1){
		Property ** props = (Property**)malloc(sizeof(Property*) * table->count OS_DBG_FILEPOS);
		int i = 0;
		Property * cur = table->first;
		for(; cur && i < table->count; cur = cur->next, i++){
			props[i] = cur;
		}
		OS_ASSERT(!cur && i == table->count);
		allocator->qsort(props, table->count, sizeof(Core::Property*), comp, user_param);
		table->first = props[0];
		props[0]->prev = NULL;
		for(i = 1; i < table->count; i++){
			props[i-1]->next = props[i];
			props[i]->prev = props[i-1];
		}
		props[i-1]->next = NULL;
		table->last = props[i-1];

		if(reorder_keys){
#if 1 // speed optimization
			OS_MEMSET(table->heads, 0, sizeof(Property*)*(table->head_mask+1));
			for(i = 0; i < table->count; i++){
				Property * cur = props[i];
				cur->index = Value(i);
				int slot = cur->getHash() & table->head_mask;
				cur->hash_next = table->heads[slot];
				table->heads[slot] = cur;
			}
#else
			for(i = 0; i < table->count; i++){
				changePropertyIndex(table, props[i], Value(i));
			}
#endif
			table->next_index = table->count;
		}

		free(props);
	}
}

void OS::Core::sortArray(GCArrayValue * arr, int(*comp)(OS*, const void*, const void*, void*), void * user_param)
{
	allocator->qsort(arr->values.buf, arr->values.count, sizeof(Value), comp, user_param);
}

static int compareResult(OS_NUMBER num)
{
	if(num < 0) return -1;
	if(num > 0) return 1;
	return 0;
}

int OS::Core::comparePropValues(OS * os, const void * a, const void * b, void*)
{
	Property * props[] = {*(Property**)a, *(Property**)b};
	os->core->pushOpResultValue(OP_COMPARE, props[0]->value, props[1]->value);
	return compareResult(os->popNumber());
}

int OS::Core::comparePropValuesReverse(OS * os, const void * a, const void * b, void * user_param)
{
	return comparePropValues(os, b, a, user_param);
}

int OS::Core::compareObjectProperties(OS * os, const void * a, const void * b, void * user_param)
{
	Property * props[] = {*(Property**)a, *(Property**)b};
	const String& name = *(String*)user_param;

	os->core->pushValue(props[0]->value);
	os->core->pushStringValue(name);
	os->getProperty();

	os->core->pushValue(props[1]->value);
	os->core->pushStringValue(name);
	os->getProperty();

	os->runOp(ObjectScript::OP_COMPARE);
	return compareResult(os->popNumber());
}

int OS::Core::compareObjectPropertiesReverse(OS * os, const void * a, const void * b, void * user_param)
{
	return compareObjectProperties(os, b, a, user_param);
}

int OS::Core::compareUserPropValues(OS * os, const void * a, const void * b, void*)
{
	Property * props[] = {*(Property**)a, *(Property**)b};
	os->pushStackValue(-1);
	os->pushNull();
	os->core->pushValue(props[0]->value);
	os->core->pushValue(props[1]->value);
	os->core->pushValue(props[0]->index);
	os->core->pushValue(props[1]->index);
	os->call(4, 1);
	return compareResult(os->popNumber());
}

int OS::Core::compareUserPropValuesReverse(OS * os, const void * a, const void * b, void * user_param)
{
	return compareUserPropValues(os, b, a, user_param);
}

int OS::Core::comparePropKeys(OS * os, const void * a, const void * b, void*)
{
	Property * props[] = {*(Property**)a, *(Property**)b};
	os->core->pushOpResultValue(OP_COMPARE, props[0]->index, props[1]->index);
	return compareResult(os->popNumber());
}

int OS::Core::comparePropKeysReverse(OS * os, const void * a, const void * b, void * user_param)
{
	return comparePropKeys(os, b, a, user_param);
}

int OS::Core::compareUserPropKeys(OS * os, const void * a, const void * b, void*)
{
	Property * props[] = {*(Property**)a, *(Property**)b};
	os->pushStackValue(-1);
	os->pushNull();
	os->core->pushValue(props[0]->index);
	os->core->pushValue(props[1]->index);
	os->core->pushValue(props[0]->value);
	os->core->pushValue(props[1]->value);
	os->call(4, 1);
	return compareResult(os->popNumber());
}

int OS::Core::compareUserPropKeysReverse(OS * os, const void * a, const void * b, void * user_param)
{
	return compareUserPropKeys(os, b, a, user_param);
}

int OS::Core::compareArrayValues(OS * os, const void * a, const void * b, void*)
{
	Value * values[] = {(Value*)a, (Value*)b};
	os->core->pushOpResultValue(OP_COMPARE, *values[0], *values[1]);
	return compareResult(os->popNumber());
}

int OS::Core::compareArrayValuesReverse(OS * os, const void * a, const void * b, void * user_param)
{
	return compareArrayValues(os, b, a, user_param);
}

int OS::Core::compareUserArrayValues(OS * os, const void * a, const void * b, void*)
{
	Value * values[] = {(Value*)a, (Value*)b};
	os->pushStackValue(-1);
	os->pushNull();
	os->core->pushValue(*values[0]);
	os->core->pushValue(*values[1]);
	os->call(2, 1);
	return compareResult(os->popNumber());
}

int OS::Core::compareUserArrayValuesReverse(OS * os, const void * a, const void * b, void * user_param)
{
	return compareUserArrayValues(os, b, a, user_param);
}

int OS::Core::compareUserReverse(OS * os, const void * a, const void * b, void * user_param)
{
	int (*comp)(OS*, const void*, const void*, void*) = (int(*)(OS*, const void*, const void*, void*))user_param;
	return comp(os, b, a, NULL);
}

OS::Core::Property * OS::Core::Table::get(const PropertyIndex& index)
{
	if(heads){
		Property * cur = heads[index.getHash() & head_mask];
		for(; cur; cur = cur->hash_next){
			if(cur->isEqual(index)){
				return cur;
			}
		}
	}
	return NULL;
}

// =====================================================================

OS::Core::GCFunctionValue::GCFunctionValue()
{
	/*
	prog = NULL;
	func_decl = NULL;
	env = NULL;
	locals = NULL;
	*/
}

OS::Core::GCFunctionValue::~GCFunctionValue()
{
	OS_ASSERT(!locals && !name);
	OS_ASSERT(!prog && !func_decl);
}

OS::Core::GCFunctionValue * OS::Core::newFunctionValue(StackFunction * stack_func, Program * prog, FunctionDecl * func_decl, Value env)
{
	GCFunctionValue * func_value = new (allocator->malloc(sizeof(GCFunctionValue) OS_DBG_FILEPOS)) GCFunctionValue();
	func_value->type = OS_VALUE_TYPE_FUNCTION;
	func_value->prototype = prototypes[PROTOTYPE_FUNCTION];
	func_value->prog = prog->retain();
	func_value->func_decl = func_decl;
	func_value->env = env; // global_vars;
	func_value->locals = stack_func ? stack_func->locals->retain() : NULL;
	func_value->name = NULL;
	registerValue(func_value);
	return func_value;
}

void OS::Core::clearFunctionValue(GCFunctionValue * func_value)
{
	OS_ASSERT(func_value->prog && func_value->func_decl); // && func_data->env); //  && func_data->self

	// value could be already destroyed by gc or will be destroyed soon
	// releaseValue(func_data->env);
	func_value->env = (GCValue*)NULL;

	// releaseValue(func_data->self);
	// func_data->self = NULL;
	OS_ASSERT(func_value->func_decl);

	if(func_value->locals){
		releaseLocals(func_value->locals);
		func_value->locals = NULL;
	}
	func_value->name = NULL;

	func_value->func_decl = NULL;

	func_value->prog->release();
	func_value->prog = NULL;

	// func_value->~GCFunctionValue();
	// free(func_value);
}

// =====================================================================

OS::Core::Locals ** OS::Core::Locals::getParents()
{
	return (Locals**)(this + 1);
}

OS::Core::Locals * OS::Core::Locals::getParent(int i)
{
	OS_ASSERT(i >= 0 && i < num_parents);
	return ((Locals**)(this+1))[i];
}

OS::Core::Locals * OS::Core::Locals::retain()
{
	ref_count++;
	return this;
}

// =====================================================================

OS::Core::Value::Value()
{
	OS_SET_VALUE_NULL(*this);
}

OS::Core::Value::Value(bool val)
{
	OS_VALUE_VARIANT(*this).boolean = val;
	OS_SET_VALUE_TYPE(*this, OS_VALUE_TYPE_BOOL);
}

OS::Core::Value::Value(OS_INT32 val)
{
	OS_SET_VALUE_NUMBER(*this, val);
	// type = OS_VALUE_TYPE_NUMBER;
}

OS::Core::Value::Value(OS_INT64 val)
{
	OS_SET_VALUE_NUMBER(*this, val);
	// type = OS_VALUE_TYPE_NUMBER;
}

OS::Core::Value::Value(float val)
{
	OS_SET_VALUE_NUMBER(*this, val);
	// type = OS_VALUE_TYPE_NUMBER;
}

OS::Core::Value::Value(double val)
{
	OS_SET_VALUE_NUMBER(*this, val);
	// type = OS_VALUE_TYPE_NUMBER;
}

OS::Core::Value::Value(const String& str)
{
	OS_ASSERT(str.string);
	OS_VALUE_VARIANT(*this).value = str.string;
	OS_SET_VALUE_TYPE(*this, OS_VALUE_TYPE_STRING);
}

OS::Core::Value::Value(int val, const WeakRef&)
{
	OS_VALUE_VARIANT(*this).value_id = val;
	OS_SET_VALUE_TYPE(*this, OS_VALUE_TYPE_WEAKREF);
}

OS::Core::Value::Value(GCValue * val)
{
	if(val){
		OS_VALUE_VARIANT(*this).value = val;
		OS_SET_VALUE_TYPE(*this, val->type);
	}else{
		OS_SET_VALUE_NULL(*this);
	}
}

OS::Core::Value& OS::Core::Value::operator=(GCValue * val)
{
	if(val){
		OS_VALUE_VARIANT(*this).value = val;
		OS_SET_VALUE_TYPE(*this, val->type);
	}else{
		OS_SET_VALUE_NULL(*this);
	}
	return *this;
}

OS::Core::Value& OS::Core::Value::operator=(bool val)
{
	OS_VALUE_VARIANT(*this).boolean = val;
	OS_SET_VALUE_TYPE(*this, OS_VALUE_TYPE_BOOL);
	return *this;
}

OS::Core::Value& OS::Core::Value::operator=(OS_INT32 val)
{
	OS_SET_VALUE_NUMBER(*this, val);
	return *this;
}

OS::Core::Value& OS::Core::Value::operator=(OS_INT64 val)
{
	OS_SET_VALUE_NUMBER(*this, val);
	return *this;
}

OS::Core::Value& OS::Core::Value::operator=(float val)
{
	OS_SET_VALUE_NUMBER(*this, val);
	return *this;
}

OS::Core::Value& OS::Core::Value::operator=(double val)
{
	OS_SET_VALUE_NUMBER(*this, val);
	return *this;
}

void OS::Core::Value::clear()
{
	OS_SET_VALUE_NULL(*this);
}

OS::Core::GCValue * OS::Core::Value::getGCValue() const
{
	switch(OS_VALUE_TYPE(*this)){
	case OS_VALUE_TYPE_STRING:
	case OS_VALUE_TYPE_ARRAY:
	case OS_VALUE_TYPE_OBJECT:
	case OS_VALUE_TYPE_FUNCTION:
	case OS_VALUE_TYPE_CFUNCTION:
	case OS_VALUE_TYPE_USERDATA:
	case OS_VALUE_TYPE_USERPTR:
		OS_ASSERT(OS_VALUE_VARIANT(*this).value);
		return OS_VALUE_VARIANT(*this).value;
	}
	return NULL;
}

bool OS::Core::Value::isNull() const
{
	return OS_VALUE_TYPE(*this) == OS_VALUE_TYPE_NULL;
}

bool OS::Core::Value::isFunction() const
{
	switch(OS_VALUE_TYPE(*this)){
	case OS_VALUE_TYPE_FUNCTION:
	case OS_VALUE_TYPE_CFUNCTION:
		return true;
	}
	return false;
}

bool OS::Core::Value::isUserdata() const
{
	switch(OS_VALUE_TYPE(*this)){
	case OS_VALUE_TYPE_USERDATA:
	case OS_VALUE_TYPE_USERPTR:
		return true;
	}
	return false;
}

// =====================================================================

OS::Core::ValueRetained::ValueRetained(): super()
{
}

OS::Core::ValueRetained::ValueRetained(bool val): super(val)
{
	// retain();
}

OS::Core::ValueRetained::ValueRetained(OS_FLOAT val): super(val)
{
	// retain();
}

OS::Core::ValueRetained::ValueRetained(int val): super(val)
{
	// retain();
}

OS::Core::ValueRetained::ValueRetained(int val, const WeakRef& wr): super(val, wr)
{
	// retain();
}

OS::Core::ValueRetained::ValueRetained(GCValue * val): super(val)
{
	// retain();
	if(val){
		val->external_ref_count++;
	}
}

OS::Core::ValueRetained::ValueRetained(Value b): super(b)
{
	retain();
}

OS::Core::ValueRetained::~ValueRetained()
{
	release();
}

OS::Core::ValueRetained& OS::Core::ValueRetained::operator=(Value b)
{
	release();
	super::operator=(b);
	retain();
	return *this;
}


void OS::Core::ValueRetained::clear()
{
	release();
	super::clear();
}

void OS::Core::ValueRetained::retain()
{
	switch(OS_VALUE_TYPE(*this)){
	case OS_VALUE_TYPE_STRING:
	case OS_VALUE_TYPE_ARRAY:
	case OS_VALUE_TYPE_OBJECT:
	case OS_VALUE_TYPE_FUNCTION:
	case OS_VALUE_TYPE_CFUNCTION:
	case OS_VALUE_TYPE_USERDATA:
	case OS_VALUE_TYPE_USERPTR:
		OS_ASSERT(OS_VALUE_VARIANT(*this).value);
		OS_VALUE_VARIANT(*this).value->external_ref_count++;
		break;
	}
}

void OS::Core::ValueRetained::release()
{
	switch(OS_VALUE_TYPE(*this)){
	case OS_VALUE_TYPE_STRING:
	case OS_VALUE_TYPE_ARRAY:
	case OS_VALUE_TYPE_OBJECT:
	case OS_VALUE_TYPE_FUNCTION:
	case OS_VALUE_TYPE_CFUNCTION:
	case OS_VALUE_TYPE_USERDATA:
	case OS_VALUE_TYPE_USERPTR:
		OS_ASSERT(OS_VALUE_VARIANT(*this).value && OS_VALUE_VARIANT(*this).value->external_ref_count > 0);
		OS_VALUE_VARIANT(*this).value->external_ref_count--;
		if(OS_VALUE_VARIANT(*this).value->gc_color == GC_WHITE){
			OS_VALUE_VARIANT(*this).value->gc_color = GC_BLACK;
		}
		break;
	}
}

// =====================================================================

OS::Core::GCValue::GCValue()
{
	value_id = 0;
	external_ref_count = 0;
	hash_next = NULL;
	prototype = NULL;
	table = NULL;
	gc_grey_next = NULL;
#ifdef OS_DEBUG
	gc_time = -1;
#endif
	gc_color = GC_WHITE;
	type = OS_VALUE_TYPE_NULL;
	is_object_instance = false;
	is_destructor_called = false;
}

OS::Core::GCValue::~GCValue()
{
	value_id = 0;

	OS_ASSERT(type == OS_VALUE_TYPE_UNKNOWN);
	OS_ASSERT(!table);
	OS_ASSERT(!hash_next);
	OS_ASSERT(!prototype);
	OS_ASSERT(gc_color != GC_GREY);
}

// =====================================================================

OS::Core::GCStringValue::GCStringValue(int p_data_size)
{
	data_size = p_data_size;
}

OS::Core::GCStringValue * OS::Core::GCStringValue::alloc(OS * allocator, const void * buf, int data_size OS_DBG_FILEPOS_DECL)
{
	OS_ASSERT(data_size >= 0);
	int alloc_size = data_size + sizeof(GCStringValue) + sizeof(wchar_t) + sizeof(wchar_t)/2;
	GCStringValue * string = new (allocator->malloc(alloc_size OS_DBG_FILEPOS_PARAM)) GCStringValue(data_size);
	string->type = OS_VALUE_TYPE_STRING;
	string->prototype = allocator->core->prototypes[PROTOTYPE_STRING];
	OS_BYTE * data_buf = string->toBytes();
	OS_MEMCPY(data_buf, buf, data_size);
	OS_MEMSET(data_buf + data_size, 0, sizeof(wchar_t) + sizeof(wchar_t)/2);
	string->calcHash();
	allocator->core->registerValue(string);
#ifdef OS_DEBUG
	string->str = string->toChar();
#endif
	return string;
}

OS::Core::GCStringValue * OS::Core::GCStringValue::alloc(OS * allocator, const void * buf1, int len1, const void * buf2, int len2 OS_DBG_FILEPOS_DECL)
{
	OS_ASSERT(len1 >= 0 && len2 >= 0);
	int alloc_size = len1 + len2 + sizeof(GCStringValue) + sizeof(wchar_t) + sizeof(wchar_t)/2;
	GCStringValue * string = new (allocator->malloc(alloc_size OS_DBG_FILEPOS_PARAM)) GCStringValue(len1 + len2);
	string->type = OS_VALUE_TYPE_STRING;
	string->prototype = allocator->core->prototypes[PROTOTYPE_STRING];
	OS_BYTE * data_buf = string->toBytes();
	OS_MEMCPY(data_buf, buf1, len1); data_buf += len1;
	if(len2){ OS_MEMCPY(data_buf, buf2, len2); data_buf += len2; }
	OS_MEMSET(data_buf, 0, sizeof(wchar_t) + sizeof(wchar_t)/2);
	string->calcHash();
	allocator->core->registerValue(string);
#ifdef OS_DEBUG
	string->str = string->toChar();
#endif
	return string;
}

OS::Core::GCStringValue * OS::Core::GCStringValue::alloc(OS * allocator, GCStringValue * a, GCStringValue * b OS_DBG_FILEPOS_DECL)
{
	return alloc(allocator, a->toMemory(), a->data_size, b->toMemory(), b->data_size OS_DBG_FILEPOS_PARAM);
}

bool OS::Core::GCStringValue::isNumber(OS_NUMBER* p_val) const
{
	const OS_CHAR * str = toChar();
	const OS_CHAR * end = str + getLen();
	OS_FLOAT val;
	if(Utils::parseFloat(str, val) && (str == end || (*str==OS_TEXT('f') && str+1 == end))){
		if(p_val) *p_val = (OS_NUMBER)val;
		return true;
	}
	if(p_val) *p_val = 0;
	return false;
}

OS_NUMBER OS::Core::GCStringValue::toNumber() const
{
	const OS_CHAR * str = toChar();
	const OS_CHAR * end = str + getLen();
	OS_FLOAT val;
	if(Utils::parseFloat(str, val) && (str == end || (*str==OS_TEXT('f') && str+1 == end))){
		return (OS_NUMBER)val;
	}
	return 0;
}

int OS::Core::GCStringValue::cmp(GCStringValue * string) const
{
	if(this == string){
		return 0;
	}
	return Utils::cmp(toBytes(), data_size, string->toBytes(), string->data_size);
}

int OS::Core::GCStringValue::cmp(const OS_CHAR * str) const
{
	return cmp(str, OS_STRLEN(str));
}

int OS::Core::GCStringValue::cmp(const OS_CHAR * str, int len) const
{
	return Utils::cmp(toBytes(), data_size, str, len * sizeof(OS_CHAR));
}

void OS::Core::GCStringValue::calcHash()
{
	hash = Utils::keyToHash(toBytes(), data_size); 
}

// =====================================================================

bool OS::Core::valueToBool(const Value& val)
{
	switch(OS_VALUE_TYPE(val)){
	case OS_VALUE_TYPE_NULL:
		return false;

	case OS_VALUE_TYPE_BOOL:
		return OS_VALUE_VARIANT(val).boolean ? true : false;

#ifndef OS_NUMBER_NAN_TRICK
	case OS_VALUE_TYPE_NUMBER:
		return !OS_ISNAN((OS_FLOAT)OS_VALUE_NUMBER(val));
#endif
	}
	// OS_EValueType type = (OS_EValueType)OS_VALUE_TYPE(val);
	return true;
}

OS_INT OS::Core::valueToInt(const Value& val, bool valueof_enabled)
{
	return (OS_INT)valueToNumber(val, valueof_enabled);
}

OS_INT OS::Core::Compiler::Expression::toInt()
{
	return (OS_INT)toNumber();
}

OS_NUMBER OS::Core::Compiler::Expression::toNumber()
{
	switch(type){
	case EXP_TYPE_CONST_NULL:
		return 0;

	case EXP_TYPE_CONST_STRING:
		return token->str.toNumber();

	case EXP_TYPE_CONST_NUMBER:
		return (OS_NUMBER)token->getFloat();

	case EXP_TYPE_CONST_TRUE:
		return 1;

	case EXP_TYPE_CONST_FALSE:
		return 0;
	}
	OS_ASSERT(false);
	return 0;
}

OS_NUMBER OS::Core::valueToNumber(const Value& val, bool valueof_enabled)
{
	switch(OS_VALUE_TYPE(val)){
	case OS_VALUE_TYPE_NULL:
		return 0; // nan_float;

	case OS_VALUE_TYPE_BOOL:
		return (OS_NUMBER)OS_VALUE_VARIANT(val).boolean;

	case OS_VALUE_TYPE_NUMBER:
		return OS_VALUE_NUMBER(val);

	case OS_VALUE_TYPE_STRING:
		return OS_VALUE_VARIANT(val).string->toNumber();
	}
	if(valueof_enabled){
		pushValueOf(val);
		struct Pop { Core * core; ~Pop(){ core->pop(); } } pop = {this}; (void)pop;
		return valueToNumber(stack_values.lastElement(), false);
	}
	return 0;
}

bool OS::Core::isValueNumber(const Value& val, OS_NUMBER * out)
{
	switch(OS_VALUE_TYPE(val)){
	case OS_VALUE_TYPE_BOOL:
		if(out){
			*out = (OS_NUMBER)OS_VALUE_VARIANT(val).boolean;
		}
		return true;

	case OS_VALUE_TYPE_NUMBER:
		if(out){
			*out = (OS_NUMBER)OS_VALUE_NUMBER(val);
		}
		return true;

	case OS_VALUE_TYPE_STRING:
		OS_ASSERT(dynamic_cast<GCStringValue*>(OS_VALUE_VARIANT(val).string));
		return OS_VALUE_VARIANT(val).string->isNumber(out);
	}
	if(out){
		*out = 0;
	}
	return false;
}

OS::Core::String OS::Core::Compiler::Expression::toString()
{
	switch(type){
	case EXP_TYPE_CONST_NULL:
		// return String(getAllocator());
		return String(getAllocator(), OS_TEXT("null"));

	case EXP_TYPE_CONST_STRING:
		return token->str;

	case EXP_TYPE_CONST_NUMBER:
		// OS_ASSERT(token->str.toFloat() == token->getFloat());
		// return token->str;
		return String(getAllocator(), token->getFloat());

	case EXP_TYPE_CONST_TRUE:
		// return String(getAllocator(), OS_TEXT("1"));
		return String(getAllocator(), OS_TEXT("true"));

	case EXP_TYPE_CONST_FALSE:
		// return String(getAllocator());
		return String(getAllocator(), OS_TEXT("false"));
	}
	OS_ASSERT(false);
	return String(getAllocator());
}

OS::Core::String OS::Core::valueToString(const Value& val, bool valueof_enabled)
{
	switch(OS_VALUE_TYPE(val)){
	case OS_VALUE_TYPE_NULL:
		// return String(allocator);
		return strings->syntax_null;

	case OS_VALUE_TYPE_BOOL:
		// return val->value.boolean ? String(allocator, OS_TEXT("1")) : String(allocator);
		return OS_VALUE_VARIANT(val).boolean ? strings->syntax_true : strings->syntax_false;

	case OS_VALUE_TYPE_NUMBER:
		return String(allocator, (OS_FLOAT)OS_VALUE_NUMBER(val), OS_AUTO_PRECISION);

	case OS_VALUE_TYPE_STRING:
		return String(OS_VALUE_VARIANT(val).string);
	}
	if(valueof_enabled){
		pushValueOf(val);
		struct Pop { Core * core; ~Pop(){ core->pop(); } } pop = {this}; (void)pop;
		return valueToString(stack_values.lastElement(), false);
	}
	return String(allocator);
}

OS::String OS::Core::valueToStringOS(const Value& val, bool valueof_enabled)
{
	switch(OS_VALUE_TYPE(val)){
	case OS_VALUE_TYPE_NULL:
		return OS::String(allocator, strings->syntax_null);

	case OS_VALUE_TYPE_BOOL:
		return OS::String(allocator, OS_VALUE_VARIANT(val).boolean ? strings->syntax_true : strings->syntax_false);

	case OS_VALUE_TYPE_NUMBER:
		return OS::String(allocator, (OS_FLOAT)OS_VALUE_NUMBER(val), OS_AUTO_PRECISION);

	case OS_VALUE_TYPE_STRING:
		return OS::String(allocator, OS_VALUE_VARIANT(val).string);
	}
	if(valueof_enabled){
		pushValueOf(val);
		struct Pop { Core * core; ~Pop(){ core->pop(); } } pop = {this}; (void)pop;
		return valueToStringOS(stack_values.lastElement(), false);
	}
	return OS::String(allocator);
}

bool OS::Core::isValueString(const Value& val, String * out)
{
	switch(OS_VALUE_TYPE(val)){
	case OS_VALUE_TYPE_NULL:
		if(out){
			// *out = String(allocator);
			*out = strings->syntax_null;
		}
		return false;

	case OS_VALUE_TYPE_BOOL:
		if(out){
			// *out = String(allocator, val->value.boolean ? OS_TEXT("1") : OS_TEXT(""));
			*out = OS_VALUE_VARIANT(val).boolean ? strings->syntax_true : strings->syntax_false;
		}
		return true;

	case OS_VALUE_TYPE_NUMBER:
		if(out){
			*out = String(allocator, (OS_FLOAT)OS_VALUE_NUMBER(val), OS_AUTO_PRECISION);
		}
		return true;

	case OS_VALUE_TYPE_STRING:
		if(out){
			OS_ASSERT(dynamic_cast<GCStringValue*>(OS_VALUE_VARIANT(val).string));
			*out = String(OS_VALUE_VARIANT(val).string);
		}
		return true;
	}
	if(out){
		*out = String(allocator);
	}
	return false;
}

bool OS::Core::isValueStringOS(const Value& val, OS::String * out)
{
	switch(OS_VALUE_TYPE(val)){
	case OS_VALUE_TYPE_NULL:
		if(out){
			// *out = String(allocator);
			*out = OS::String(allocator, strings->syntax_null);
		}
		return false;

	case OS_VALUE_TYPE_BOOL:
		if(out){
			// *out = String(allocator, val->value.boolean ? OS_TEXT("1") : OS_TEXT(""));
			*out = OS::String(allocator, OS_VALUE_VARIANT(val).boolean ? strings->syntax_true : strings->syntax_false);
		}
		return true;

	case OS_VALUE_TYPE_NUMBER:
		if(out){
			*out = OS::String(allocator, (OS_FLOAT)OS_VALUE_NUMBER(val), OS_AUTO_PRECISION);
		}
		return true;

	case OS_VALUE_TYPE_STRING:
		if(out){
			OS_ASSERT(dynamic_cast<GCStringValue*>(OS_VALUE_VARIANT(val).string));
			*out = OS::String(allocator, OS_VALUE_VARIANT(val).string);
		}
		return true;
	}
	if(out){
		*out = String(allocator);
	}
	return false;
}

// =====================================================================
// =====================================================================
// =====================================================================

OS::Core::StringRefs::StringRefs()
{
	head_mask = 0;
	heads = NULL;
	count = 0;
}
OS::Core::StringRefs::~StringRefs()
{
	OS_ASSERT(count == 0);
	OS_ASSERT(!heads);
}

void OS::Core::registerStringRef(StringRef * str_ref)
{
	if((string_refs.count>>HASH_GROW_SHIFT) >= string_refs.head_mask){
		int new_size = string_refs.heads ? (string_refs.head_mask+1) * 2 : 32;
		int alloc_size = sizeof(StringRef*) * new_size;
		StringRef ** new_heads = (StringRef**)malloc(alloc_size OS_DBG_FILEPOS);
		OS_ASSERT(new_heads);
		OS_MEMSET(new_heads, 0, alloc_size);

		StringRef ** old_heads = string_refs.heads;
		int old_mask = string_refs.head_mask;

		string_refs.heads = new_heads;
		string_refs.head_mask = new_size-1;

		if(old_heads){
			for(int i = 0; i <= old_mask; i++){
				for(StringRef * str_ref = old_heads[i], * next; str_ref; str_ref = next){
					next = str_ref->hash_next;
					int slot = str_ref->string_hash & string_refs.head_mask;
					str_ref->hash_next = string_refs.heads[slot];
					string_refs.heads[slot] = str_ref;
				}
			}
			free(old_heads);
		}
	}

	int slot = str_ref->string_hash & string_refs.head_mask;
	str_ref->hash_next = string_refs.heads[slot];
	string_refs.heads[slot] = str_ref;
	string_refs.count++;
}

void OS::Core::unregisterStringRef(StringRef * str_ref)
{
	int slot = str_ref->string_hash & string_refs.head_mask;
	StringRef * cur = string_refs.heads[slot], * prev = NULL;
	for(; cur; prev = cur, cur = cur->hash_next){
		if(cur == str_ref){
			if(prev){
				prev->hash_next = cur->hash_next;
			}else{
				string_refs.heads[slot] = cur->hash_next;
			}
			OS_ASSERT(string_refs.count > 0);
			string_refs.count--;
			cur->hash_next = NULL;
			return;
		}
	}
	OS_ASSERT(false);
}

void OS::Core::deleteStringRefs()
{
	if(!string_refs.heads){
		return;
	}
	for(int i = 0; i <= string_refs.head_mask; i++){
		while(string_refs.heads[i]){
			StringRef * cur = string_refs.heads[i];
			string_refs.heads[i] = cur->hash_next;
			free(cur);
		}
	}
	free(string_refs.heads);
	string_refs.heads = NULL;
	string_refs.head_mask = 0;
	string_refs.count = 0;
}

// =====================================================================
// =====================================================================
// =====================================================================

OS::Core::UserptrRefs::UserptrRefs()
{
	head_mask = 0;
	heads = NULL;
	count = 0;
}
OS::Core::UserptrRefs::~UserptrRefs()
{
	OS_ASSERT(count == 0);
	OS_ASSERT(!heads);
}

void OS::Core::registerUserptrRef(UserptrRef * user_pointer_ref)
{
	OS_ASSERT(!user_pointer_ref->hash_next);
	if((userptr_refs.count>>HASH_GROW_SHIFT) >= userptr_refs.head_mask){
		int new_size = userptr_refs.heads ? (userptr_refs.head_mask+1) * 2 : 32;
		int alloc_size = sizeof(UserptrRef*) * new_size;
		UserptrRef ** new_heads = (UserptrRef**)malloc(alloc_size OS_DBG_FILEPOS);
		OS_ASSERT(new_heads);
		OS_MEMSET(new_heads, 0, alloc_size);

		UserptrRef ** old_heads = userptr_refs.heads;
		int old_mask = userptr_refs.head_mask;

		userptr_refs.heads = new_heads;
		userptr_refs.head_mask = new_size-1;

		if(old_heads){
			for(int i = 0; i <= old_mask; i++){
				for(UserptrRef * user_pointer_ref = old_heads[i], * next; user_pointer_ref; user_pointer_ref = next){
					next = user_pointer_ref->hash_next;
					int slot = user_pointer_ref->userptr_hash & userptr_refs.head_mask;
					user_pointer_ref->hash_next = userptr_refs.heads[slot];
					userptr_refs.heads[slot] = user_pointer_ref;
				}
			}
			free(old_heads);
		}
	}

	int slot = user_pointer_ref->userptr_hash & userptr_refs.head_mask;
	user_pointer_ref->hash_next = userptr_refs.heads[slot];
	userptr_refs.heads[slot] = user_pointer_ref;
	userptr_refs.count++;
}

void OS::Core::unregisterUserptrRef(UserptrRef * userptr_ref)
{
	int slot = userptr_ref->userptr_hash & userptr_refs.head_mask;
	UserptrRef * cur = userptr_refs.heads[slot], * prev = NULL;
	for(; cur; prev = cur, cur = cur->hash_next){
		if(cur == userptr_ref){
			if(prev){
				prev->hash_next = cur->hash_next;
			}else{
				userptr_refs.heads[slot] = cur->hash_next;
			}
			OS_ASSERT(userptr_refs.count > 0);
			userptr_refs.count--;
			cur->hash_next = NULL;
			return;
		}
	}
	OS_ASSERT(false);
}

void OS::Core::unregisterUserptrRef(void * ptr, int value_id)
{
	if(userptr_refs.count > 0){
		OS_ASSERT(userptr_refs.heads && userptr_refs.head_mask);
		int hash = OS_PTR_HASH(ptr);
		int slot = hash & userptr_refs.head_mask;
		UserptrRef * userptr_ref = userptr_refs.heads[slot];
		for(UserptrRef * prev = NULL; userptr_ref; prev = userptr_ref, userptr_ref = userptr_ref->hash_next){
			if(userptr_ref->userptr_value_id == value_id){
				if(!prev){
					userptr_refs.heads[slot] = userptr_ref->hash_next;
				}else{
					prev->hash_next = userptr_ref->hash_next;
				}
				free(userptr_ref);
				userptr_refs.count--;
				return;
			}
		}
	}
}

void OS::Core::deleteUserptrRefs()
{
	if(!userptr_refs.heads){
		return;
	}
	for(int i = 0; i <= userptr_refs.head_mask; i++){
		while(userptr_refs.heads[i]){
			UserptrRef * cur = userptr_refs.heads[i];
			userptr_refs.heads[i] = cur->hash_next;
			free(cur);
		}
	}
	free(userptr_refs.heads);
	userptr_refs.heads = NULL;
	userptr_refs.head_mask = 0;
	userptr_refs.count = 0;
}

// =====================================================================
// =====================================================================
// =====================================================================

OS::Core::Values::Values()
{
	head_mask = 0;
	heads = NULL;
	next_id = 1;
	count = 0;
}
OS::Core::Values::~Values()
{
	OS_ASSERT(count == 0);
	OS_ASSERT(!heads);
}

void OS::Core::registerValue(GCValue * value)
{
	value->value_id = values.next_id++;

	if((values.count>>HASH_GROW_SHIFT) >= values.head_mask){
		int new_size = values.heads ? (values.head_mask+1) * 2 : 32;
		int alloc_size = sizeof(GCValue*) * new_size;
		GCValue ** new_heads = (GCValue**)malloc(alloc_size OS_DBG_FILEPOS); // new Value*[new_size];
		OS_ASSERT(new_heads);
		OS_MEMSET(new_heads, 0, alloc_size);

		GCValue ** old_heads = values.heads;
		int old_mask = values.head_mask;

		values.heads = new_heads;
		values.head_mask = new_size-1;

		if(old_heads){
			for(int i = 0; i <= old_mask; i++){
				for(GCValue * value = old_heads[i], * next; value; value = next){
					gcAddToGreyList(value);
					next = value->hash_next;
					int slot = value->value_id & values.head_mask;
					value->hash_next = values.heads[slot];
					values.heads[slot] = value;
				}
			}
			free(old_heads);
		}
		if(gc_values_head_index >= 0){
			// restart gc ASAP
			gc_values_head_index = -1;
			gc_start_next_values = 0;
			gc_continuous = false;
			gc_step_size_auto_mult *= 4.0f;
		}
	}

	int slot = value->value_id & values.head_mask;
	value->hash_next = values.heads[slot];
	values.heads[slot] = value;
	values.count++;

	num_created_values++;

	gcAddToGreyList(value);

	gcStepIfNeeded();
}

OS::Core::GCValue * OS::Core::unregisterValue(int value_id)
{
	int slot = value_id & values.head_mask;
	GCValue * value = values.heads[slot], * prev = NULL;
	for(; value; prev = value, value = value->hash_next){
		if(value->value_id == value_id){
			if(prev){
				prev->hash_next = value->hash_next;
			}else{
				values.heads[slot] = value->hash_next;
			}
			OS_ASSERT(values.count > 0);
			values.count--;
			value->hash_next = NULL;
			value->value_id = 0;
			return value;
		}
	}
	return NULL;
}

void OS::Core::deleteValues(bool del_ref_counted_also)
{
	if(values.heads && values.count > 0){
		while(true){
			for(int i = 0; i <= values.head_mask; i++){
#if 0
				for(GCValue * value; value = values.heads[i]; ){
					deleteValue(value);
				}
#else
				for(GCValue * value = values.heads[i], * next; value; value = next){
					next = value->hash_next;
					if(del_ref_counted_also || !value->external_ref_count){
						unregisterValue(value->value_id);
						clearValue(value);
						deleteValue(value);
					}
				}
#endif
			}
			if(!values.count || !del_ref_counted_also){
				break;
			}
		}
	}
	if(values.heads && values.count == 0){
		free(values.heads);
		values.heads = NULL;
		values.head_mask = 0;
		values.next_id = 1;
	}
}

OS::Core::GCValue * OS::Core::Values::get(int value_id)
{
	int slot = value_id & head_mask;
	for(GCValue * value = heads[slot]; value; value = value->hash_next){
		if(value->value_id == value_id){
			return value;
		}
	}
	return NULL;
}

// =====================================================================
// =====================================================================
// =====================================================================

OS::Core::Strings::Strings(OS * allocator)
	:
	__construct(allocator, OS_TEXT("__construct")),
	__destruct(allocator, OS_TEXT("__destruct")),
	__object(allocator, OS_TEXT("__object")),
	__get(allocator, OS_TEXT("__get")),
	__set(allocator, OS_TEXT("__set")),
	__isset(allocator, OS_TEXT("__isset")),
	__getAt(allocator, OS_TEXT("__get@")),
	__setAt(allocator, OS_TEXT("__set@")),
	__issetAt(allocator, OS_TEXT("__isset@")),
	__del(allocator, OS_TEXT("__del")),
	__delAt(allocator, OS_TEXT("__del@")),
	__getempty(allocator, OS_TEXT("__getempty")),
	__setempty(allocator, OS_TEXT("__setempty")),
	__delempty(allocator, OS_TEXT("__delempty")),
	__getdim(allocator, OS_TEXT("__getdim")),
	__setdim(allocator, OS_TEXT("__setdim")),
	__deldim(allocator, OS_TEXT("__deldim")),
	__cmp(allocator, OS_TEXT("__cmp")),
	__iter(allocator, OS_TEXT("__iter")),
	__bitand(allocator, OS_TEXT("__bitand")),
	__bitor(allocator, OS_TEXT("__bitor")),
	__bitxor(allocator, OS_TEXT("__bitxor")),
	__bitnot(allocator, OS_TEXT("__bitnot")),
	__plus(allocator, OS_TEXT("__plus")),
	__neg(allocator, OS_TEXT("__neg")),
	__len(allocator, OS_TEXT("__len")),
	__add(allocator, OS_TEXT("__add")),
	__sub(allocator, OS_TEXT("__sub")),
	__mul(allocator, OS_TEXT("__mul")),
	__div(allocator, OS_TEXT("__div")),
	__mod(allocator, OS_TEXT("__mod")),
	__lshift(allocator, OS_TEXT("__lshift")),
	__rshift(allocator, OS_TEXT("__rshift")),
	__pow(allocator, OS_TEXT("__pow")),
	func_extends(allocator, OS_TEXT("__extends")),
	func_delete(allocator, OS_TEXT("__delete")),
	func_in(allocator, OS_TEXT("__in")),
	func_is(allocator, OS_TEXT("__is")),
	func_isprototypeof(allocator, OS_TEXT("__isprototypeof")),
	func_push(allocator, OS_TEXT("push")),
	func_valueOf(allocator, OS_TEXT("valueOf")),
	func_clone(allocator, OS_TEXT("clone")),
	func_concat(allocator, OS_TEXT("concat")),
	func_echo(allocator, OS_TEXT("echo")),
	func_require(allocator, OS_TEXT("require")),

	typeof_null(allocator, OS_TEXT("null")),
	typeof_boolean(allocator, OS_TEXT("boolean")),
	typeof_number(allocator, OS_TEXT("number")),
	typeof_string(allocator, OS_TEXT("string")),
	typeof_object(allocator, OS_TEXT("object")),
	typeof_array(allocator, OS_TEXT("array")),
	typeof_userdata(allocator, OS_TEXT("userdata")),
	typeof_function(allocator, OS_TEXT("function")),

	syntax_get(allocator, OS_TEXT("get")),
	syntax_set(allocator, OS_TEXT("set")),
	syntax_super(allocator, OS_TEXT("super")),
	syntax_is(allocator, OS_TEXT("is")),
	syntax_isprototypeof(allocator, OS_TEXT("isprototypeof")),

	syntax_extends(allocator, OS_TEXT("extends")),
	syntax_delete(allocator, OS_TEXT("delete")),
	syntax_prototype(allocator, OS_TEXT("prototype")),
	syntax_var(allocator, OS_TEXT("var")),
	syntax_arguments(allocator, OS_TEXT("arguments")),
	syntax_function(allocator, OS_TEXT("function")),
	syntax_null(allocator, OS_TEXT("null")),
	syntax_true(allocator, OS_TEXT("true")),
	syntax_false(allocator, OS_TEXT("false")),
	syntax_return(allocator, OS_TEXT("return")),
	syntax_class(allocator, OS_TEXT("class")),
	syntax_enum(allocator, OS_TEXT("enum")),
	syntax_switch(allocator, OS_TEXT("switch")),
	syntax_case(allocator, OS_TEXT("case")),
	syntax_default(allocator, OS_TEXT("default")),
	syntax_if(allocator, OS_TEXT("if")),
	syntax_else(allocator, OS_TEXT("else")),
	syntax_elseif(allocator, OS_TEXT("elseif")),
	syntax_for(allocator, OS_TEXT("for")),
	syntax_in(allocator, OS_TEXT("in")),
	syntax_break(allocator, OS_TEXT("break")),
	syntax_continue(allocator, OS_TEXT("continue")),
	syntax_try(allocator, OS_TEXT("try")),
	syntax_catch(allocator, OS_TEXT("catch")),
	syntax_finally(allocator, OS_TEXT("finally")),
	syntax_throw(allocator, OS_TEXT("throw")),
	syntax_new(allocator, OS_TEXT("new")),
	syntax_while(allocator, OS_TEXT("while")),
	syntax_do(allocator, OS_TEXT("do")),
	syntax_const(allocator, OS_TEXT("const")),
	syntax_public(allocator, OS_TEXT("public")),
	syntax_protected(allocator, OS_TEXT("protected")),
	syntax_private(allocator, OS_TEXT("private")),
	syntax_yield(allocator, OS_TEXT("yield")),
	syntax_static(allocator, OS_TEXT("static")),
	syntax_debugger(allocator, OS_TEXT("debugger")),
	syntax_debuglocals(allocator, OS_TEXT("debuglocals")),
#ifdef OS_GLOBAL_VAR_ENABLED
	var_globals(allocator, OS_GLOBALS_VAR_NAME),
#endif
	var_func(allocator, OS_FUNC_VAR_NAME),
	var_this(allocator, OS_THIS_VAR_NAME),
	var_env(allocator, OS_ENV_VAR_NAME),
	var_temp_prefix(allocator, OS_TEXT("#")),

	__dummy__(0)
{
}

// =====================================================================
// =====================================================================
// =====================================================================

OS::MemoryManager::MemoryManager()
{
	ref_count = 1;
}

OS::MemoryManager::~MemoryManager()
{
}

OS::MemoryManager * OS::MemoryManager::retain()
{
	ref_count++;
	return this;
}

void OS::MemoryManager::release()
{
	if(--ref_count <= 0){
		OS_ASSERT(ref_count == 0);
		delete this;
	}
}

bool OS::isFileExist(const OS_CHAR * filename)
{
	void * f = openFile(filename, "rb");
	if(f){
		closeFile(f);
		return true;
	}
	return false;
}

int OS::getFileSize(const OS_CHAR * filename)
{
	void * f = openFile(filename, "rb");
	if(f){
		int size = getFileSize(f);
		closeFile(f);
		return size;
	}
	return 0;
}

int OS::getFileSize(void * f)
{
	if(f){
		int pos = seekFile(f, 0, SEEK_CUR);
		int size = seekFile(f, 0, SEEK_END);
		seekFile(f, pos, SEEK_SET);
		return size;
	}
	return 0;
}

void * OS::openFile(const OS_CHAR * filename, const OS_CHAR * mode)\
{
	return fopen(filename, mode);
}

int OS::readFile(void * buf, int size, void * f)
{
	if(f){
		return fread(buf, size, 1, (FILE*)f) * size;
	}
	return 0;
}

int OS::writeFile(const void * buf, int size, void * f)
{
	if(f){
		return fwrite(buf, size, 1, (FILE*)f) * size;
	}
	return 0;
}

int OS::seekFile(void * f, int offset, int whence)
{
	if(f){
		fseek((FILE*)f, offset, whence);
		return ftell((FILE*)f);
	}
	return 0;
}

void OS::closeFile(void * f)
{
	if(f){
		fclose((FILE*)f);
	}
}

void OS::echo(const OS_CHAR * str)
{
	// fputs(os->toString(-params + i).toChar(), stdout);
	OS_PRINTF(OS_TEXT("%s"), str);
}

void OS::printf(const OS_CHAR * format, ...)
{
	va_list va;
	va_start(va, format);
	
	getGlobal(core->strings->func_echo);
	pushGlobals();
	pushString(Core::String::formatVa(this, format, va));
	call(1);

	// OS_VPRINTF(format, va);
	va_end(va);
}

OS::SmartMemoryManager::SmartMemoryManager()
{
	allocated_bytes = 0;
	max_allocated_bytes = 0;
	cached_bytes = 0;
	OS_MEMSET(page_desc, 0, sizeof(page_desc));
	num_page_desc = 0;
	OS_MEMSET(pages, 0, sizeof(pages));
	OS_MEMSET(cached_blocks, 0, sizeof(cached_blocks));

#ifdef OS_DEBUG
	dbg_mem_list = NULL;
	dbg_std_mem_list = NULL;
	dbg_breakpoint_id = -1;
#endif

	stat_malloc_count = 0;
	stat_free_count = 0;

	registerPageDesc(sizeof(Core::GCObjectValue), OS_MEMORY_MANAGER_PAGE_BLOCKS);
	registerPageDesc(sizeof(Core::GCStringValue), OS_MEMORY_MANAGER_PAGE_BLOCKS);
	registerPageDesc(sizeof(Core::GCUserdataValue), OS_MEMORY_MANAGER_PAGE_BLOCKS);
	registerPageDesc(sizeof(Core::GCFunctionValue), OS_MEMORY_MANAGER_PAGE_BLOCKS);
	registerPageDesc(sizeof(Core::GCCFunctionValue), OS_MEMORY_MANAGER_PAGE_BLOCKS);
	registerPageDesc(sizeof(Core::GCCFunctionValue) + sizeof(Core::Value)*4, OS_MEMORY_MANAGER_PAGE_BLOCKS);
	registerPageDesc(sizeof(Core::Property), OS_MEMORY_MANAGER_PAGE_BLOCKS);
	// registerPageDesc(sizeof(Core::StackFunction), OS_MEMORY_MANAGER_PAGE_BLOCKS);
	registerPageDesc(sizeof(Core::Locals), OS_MEMORY_MANAGER_PAGE_BLOCKS);
	registerPageDesc(sizeof(Core::Locals) + sizeof(void*)*4, OS_MEMORY_MANAGER_PAGE_BLOCKS);
	registerPageDesc(sizeof(Core::Locals) + sizeof(void*)*8, OS_MEMORY_MANAGER_PAGE_BLOCKS);
	registerPageDesc(sizeof(Core::Table), OS_MEMORY_MANAGER_PAGE_BLOCKS);
	registerPageDesc(sizeof(Core::Compiler::EXPRESSION_SIZE), OS_MEMORY_MANAGER_PAGE_BLOCKS);
	registerPageDesc(sizeof(Core::TokenData), OS_MEMORY_MANAGER_PAGE_BLOCKS);
	registerPageDesc(8, OS_MEMORY_MANAGER_PAGE_BLOCKS);
	registerPageDesc(16, OS_MEMORY_MANAGER_PAGE_BLOCKS);
	registerPageDesc(32, OS_MEMORY_MANAGER_PAGE_BLOCKS);
	registerPageDesc(64, OS_MEMORY_MANAGER_PAGE_BLOCKS);
	registerPageDesc(128, OS_MEMORY_MANAGER_PAGE_BLOCKS/2);
	registerPageDesc(256, OS_MEMORY_MANAGER_PAGE_BLOCKS/4);
	sortPageDesc();
}

OS::SmartMemoryManager::~SmartMemoryManager()
{
	freeCachedMemory(0);
#ifdef OS_DEBUG
	{
		for(MemBlock * mem = dbg_mem_list; mem; mem = mem->dbg_mem_next){
			OS_PRINTF("[LEAK] %d bytes, id: %d, line %d, %s\n", mem->block_size, mem->dbg_id, mem->dbg_line, mem->dbg_filename);
		}
	}
	{
		for(StdMemBlock * mem = dbg_std_mem_list; mem; mem = mem->dbg_mem_next){
			OS_ASSERT(mem->block_size & 0x80000000);
			OS_PRINTF("[LEAK] %d bytes, id: %d, line %d, %s\n", (mem->block_size & ~0x80000000), mem->dbg_id, mem->dbg_line, mem->dbg_filename);
		}
	}
#endif
	// OS_ASSERT(!allocated_bytes && !cached_bytes);
}

#ifdef OS_DEBUG
static const int MEM_MARK_BEGIN = 0xabcdef98;
static const int MEM_MARK_END = 0x3579faec;
static const int FREE_MARK_BEGIN = 0xdabcef98;
static const int FREE_MARK_END = 0x3faec579;
static const int STD_MEM_MARK_BEGIN = 0xaefbcd98;
static const int STD_MEM_MARK_END = 0x35ae79fc;
#define MEM_MARK_END_SIZE sizeof(int)
#else
#define MEM_MARK_END_SIZE 0
#endif

int OS::SmartMemoryManager::comparePageDesc(const void * pa, const void * pb)
{
	PageDesc * a = (PageDesc*)pa;
	PageDesc * b = (PageDesc*)pb;
	return a->block_size - b->block_size;
}

void OS::SmartMemoryManager::sortPageDesc()
{
	::qsort(page_desc, num_page_desc, sizeof(page_desc[0]), comparePageDesc);
}

void OS::SmartMemoryManager::registerPageDesc(int block_size, int num_blocks)
{
	if(num_page_desc == MAX_PAGE_TYPE_COUNT){
		return;
	}
	if(block_size > 128){
		block_size = (block_size + 31) & ~31;
	}else if(block_size > 64){
		block_size = (block_size + 15) & ~15;
	}else if(block_size > 32){
		block_size = (block_size + 7) & ~7;
	}else{
		block_size = (block_size + 3) & ~3;
	}
	int i;
	for(i = 0; i < num_page_desc; i++){
		if(page_desc[i].block_size == block_size){
			if(page_desc[i].num_blocks < num_blocks){
				page_desc[i].num_blocks = num_blocks;
				page_desc[i].allocated_bytes = sizeof(Page) + (sizeof(MemBlock) + block_size + MEM_MARK_END_SIZE) * num_blocks;
			}
			return;
		}
	}
	page_desc[i].block_size = block_size;
	page_desc[i].num_blocks = num_blocks;
	page_desc[i].allocated_bytes = sizeof(Page) + (sizeof(MemBlock) + block_size + MEM_MARK_END_SIZE) * num_blocks;
	num_page_desc++;
}

void * OS::SmartMemoryManager::allocFromCachedBlock(int i OS_DBG_FILEPOS_DECL)
{
#ifdef OS_DEBUG
	if(stat_malloc_count == dbg_breakpoint_id){
		DEBUG_BREAK;
	}
#endif
	stat_malloc_count++;
	OS_ASSERT(i >= 0 && i < num_page_desc);
	CachedBlock * cached_block = cached_blocks[i];
	OS_ASSERT(cached_block);
#ifdef OS_DEBUG
	OS_ASSERT(cached_block->mark == FREE_MARK_BEGIN);
	OS_ASSERT(*(int*)(((OS_BYTE*)((MemBlock*)cached_block+1)) + page_desc[i].block_size) == FREE_MARK_END);
#endif
	cached_blocks[i] = cached_block->next;
	Page * page = cached_block->page;
	OS_ASSERT(page->num_cached_blocks > 0);
	page->num_cached_blocks--;
	MemBlock * mem_block = (MemBlock*)cached_block;
	mem_block->page = page;
	mem_block->block_size = page_desc[i].block_size;
#ifdef OS_DEBUG
	mem_block->mark = MEM_MARK_BEGIN;
	*(int*)(((OS_BYTE*)(mem_block+1)) + mem_block->block_size) = MEM_MARK_END;

	mem_block->dbg_filename = dbg_filename;
	mem_block->dbg_line = dbg_line;
	mem_block->dbg_id = stat_malloc_count-1;

	mem_block->dbg_mem_prev = NULL;
	mem_block->dbg_mem_next = dbg_mem_list;
	if(dbg_mem_list){
		dbg_mem_list->dbg_mem_prev = mem_block;
	}
	dbg_mem_list = mem_block;
#endif
	cached_bytes -= mem_block->block_size + sizeof(MemBlock);
	void * p = mem_block + 1;
	OS_MEMSET(p, 0, mem_block->block_size);
	// OS_ASSERT(mem_block->mark == MEM_MARK_BEGIN);
	// OS_ASSERT(*(int*)(((OS_BYTE*)(mem_block+1)) + mem_block->block_size) == MEM_MARK_END);
	return p;
}

void * OS::SmartMemoryManager::allocFromPageType(int i OS_DBG_FILEPOS_DECL)
{
	OS_ASSERT(i >= 0 && i < num_page_desc);
	if(cached_blocks[i]){
		return allocFromCachedBlock(i OS_DBG_FILEPOS_PARAM);
	}

	int allocated_bytes = page_desc[i].allocated_bytes;
	Page * page = (Page*)stdAlloc(allocated_bytes OS_DBG_FILEPOS);
	page->index = i;
	page->next_page = pages[i];
	pages[i] = page;
	page->num_cached_blocks = page_desc[i].num_blocks;
	cached_bytes += allocated_bytes;

	OS_BYTE * next_page_block = (OS_BYTE*)(page + 1);
	for(int j = 0; j < page_desc[i].num_blocks; j++){
		CachedBlock * cached_block = (CachedBlock*)next_page_block;
		cached_block->page = page;
		cached_block->next = cached_blocks[i];
#ifdef OS_DEBUG
		cached_block->mark = FREE_MARK_BEGIN;
		*(int*)(((OS_BYTE*)((MemBlock*)cached_block+1)) + page_desc[page->index].block_size) = FREE_MARK_END;
		OS_MEMSET(cached_block+1, 0xde, page_desc[i].block_size + (sizeof(MemBlock) - sizeof(CachedBlock)));
#endif
		cached_blocks[i] = cached_block;
		next_page_block += sizeof(MemBlock) + page_desc[i].block_size + MEM_MARK_END_SIZE;
	}

	return allocFromCachedBlock(i OS_DBG_FILEPOS_PARAM);
}

void OS::SmartMemoryManager::freeMemBlock(MemBlock * mem_block)
{
	stat_free_count++;
#ifdef OS_DEBUG
	OS_ASSERT(mem_block->mark == MEM_MARK_BEGIN);
	OS_ASSERT(*(int*)(((OS_BYTE*)(mem_block+1)) + mem_block->block_size) == MEM_MARK_END);
	if(mem_block->dbg_id == dbg_breakpoint_id){
		DEBUG_BREAK;
	}
	if(mem_block == dbg_mem_list){
		OS_ASSERT(!mem_block->dbg_mem_prev);
		dbg_mem_list = mem_block->dbg_mem_next;
	}else{ // if(mem_block->dbg_mem_prev){
		OS_ASSERT(mem_block->dbg_mem_prev);
		mem_block->dbg_mem_prev->dbg_mem_next = mem_block->dbg_mem_next;
	}
	if(mem_block->dbg_mem_next){
		mem_block->dbg_mem_next->dbg_mem_prev = mem_block->dbg_mem_prev;
	}
#endif
	Page * page = mem_block->page;
	int size = mem_block->block_size;
	cached_bytes += size + sizeof(MemBlock);
	CachedBlock * cached_block = (CachedBlock*)mem_block;
	cached_block->page = page;
	cached_block->next = cached_blocks[page->index];
#ifdef OS_DEBUG
	cached_block->mark = FREE_MARK_BEGIN;
	*(int*)(((OS_BYTE*)((MemBlock*)cached_block+1)) + page_desc[page->index].block_size) = FREE_MARK_END;
	OS_MEMSET(cached_block+1, 0xde, size + (sizeof(MemBlock) - sizeof(CachedBlock)));
#endif
	cached_blocks[page->index] = cached_block;
	page->num_cached_blocks++;
}

void OS::SmartMemoryManager::freeCachedMemory(int new_cached_bytes)
{
	if(cached_bytes > new_cached_bytes){
		for(int i = num_page_desc-1; i >= 0; i--){
			bool found_free_page = false;
			int num_blocks = page_desc[i].num_blocks;
			CachedBlock * prev_cached_block = NULL, * next_cached_block = NULL;
			for(CachedBlock * cached_block = cached_blocks[i]; cached_block; cached_block = next_cached_block){
				OS_ASSERT(cached_block->page->index == i);
				next_cached_block = cached_block->next;
				if(cached_block->page->num_cached_blocks == num_blocks){
					found_free_page = true;
					if(!prev_cached_block){
						cached_blocks[i] = next_cached_block;
					}else{
						prev_cached_block->next = next_cached_block;
					}
					// keep prev_cached_block
					continue;
				}
				prev_cached_block = cached_block;
			}
			if(found_free_page){
				Page * prev = NULL, * next;
				for(Page * page = pages[i]; page; page = next){
					next = page->next_page;
					if(page->num_cached_blocks == num_blocks){
						if(!prev){
							pages[i] = page->next_page;
						}else{
							prev->next_page = page->next_page;
						}
						cached_bytes -= page_desc[i].allocated_bytes;
						stdFree(page);
						// stat_free_count++;
					}else{
						prev = page;
					}
				}
				if(cached_bytes <= new_cached_bytes){
					break;
				}
			}
		}
	}
}

void * OS::SmartMemoryManager::stdAlloc(int size OS_DBG_FILEPOS_DECL)
{
#ifdef OS_DEBUG
	if(stat_malloc_count == dbg_breakpoint_id){
		DEBUG_BREAK;
	}
#endif
	stat_malloc_count++;
	size = (size + 7) & ~7;
	StdMemBlock * mem_block = (StdMemBlock*)::malloc(size + sizeof(StdMemBlock) + MEM_MARK_END_SIZE);
	if(!mem_block && cached_bytes > 0){
		freeCachedMemory(0);
		mem_block = (StdMemBlock*)::malloc(size + sizeof(StdMemBlock) + MEM_MARK_END_SIZE);
		if(!mem_block){
			return NULL;
		}
	}
#ifdef OS_DEBUG
	mem_block->mark = STD_MEM_MARK_BEGIN;
	*(int*)(((OS_BYTE*)(mem_block+1)) + size) = STD_MEM_MARK_END;

	mem_block->dbg_filename = dbg_filename;
	mem_block->dbg_line = dbg_line;
	mem_block->dbg_id = stat_malloc_count-1;

	mem_block->dbg_mem_prev = NULL;
	mem_block->dbg_mem_next = dbg_std_mem_list;
	if(dbg_std_mem_list){
		dbg_std_mem_list->dbg_mem_prev = mem_block;
	}
	dbg_std_mem_list = mem_block;
#endif
	mem_block->block_size = size | 0x80000000;
	allocated_bytes += size + sizeof(StdMemBlock) + MEM_MARK_END_SIZE;
	if(max_allocated_bytes < allocated_bytes){
		max_allocated_bytes = allocated_bytes;
	}
	OS_MEMSET(mem_block+1, 0, size);
	return mem_block+1;
}

void OS::SmartMemoryManager::stdFree(void * ptr)
{
	stat_free_count++;
	StdMemBlock * mem_block = (StdMemBlock*)ptr - 1;
	OS_ASSERT(mem_block->block_size & 0x80000000);
#ifdef OS_DEBUG
	OS_ASSERT(mem_block->mark == STD_MEM_MARK_BEGIN);
	OS_ASSERT(*(int*)(((OS_BYTE*)(mem_block+1)) + (mem_block->block_size & ~0x80000000)) == STD_MEM_MARK_END);

	if(mem_block->dbg_id == dbg_breakpoint_id){
		DEBUG_BREAK;
	}
	if(mem_block == dbg_std_mem_list){
		OS_ASSERT(!mem_block->dbg_mem_prev);
		dbg_std_mem_list = mem_block->dbg_mem_next;
	}else{ // if(mem_block->dbg_mem_prev){
		OS_ASSERT(mem_block->dbg_mem_prev);
		mem_block->dbg_mem_prev->dbg_mem_next = mem_block->dbg_mem_next;
	}
	if(mem_block->dbg_mem_next){
		mem_block->dbg_mem_next->dbg_mem_prev = mem_block->dbg_mem_prev;
	}
#endif
	int size = mem_block->block_size & ~0x80000000;
	allocated_bytes -= size + sizeof(StdMemBlock) + MEM_MARK_END_SIZE;
#ifdef OS_DEBUG
	OS_MEMSET(ptr, 0xde, size);
#endif
	::free(mem_block);
}

void * OS::SmartMemoryManager::malloc(int size OS_DBG_FILEPOS_DECL)
{
	if(size <= 0){
		return NULL;
	}
	// stat_malloc_count++;
#if 0
	int start = 0, end = num_page_desc-1;
	if(size <= page_desc[end].block_size){
		for(;;){
			if(start >= end){
				int block_size = page_desc[start].block_size;
				if(size > block_size){
					start++;
				}
				return allocFromPageType(start);
			}
			int mid = (start + end) / 2;
			int block_size = page_desc[mid].block_size;
			if(size == block_size){
				return allocFromPageType(mid);
			}
			if(size < block_size){
				end = mid - 1;
				continue;
			}
			start = mid + 1;
		}
	}
#else
	if(size <= page_desc[num_page_desc-1].block_size){
		for(int i = 0; i < num_page_desc; i++){
			if(size <= page_desc[i].block_size){
				return allocFromPageType(i OS_DBG_FILEPOS_PARAM);
			}
		}
	}
#endif
	return stdAlloc(size OS_DBG_FILEPOS_PARAM);
}

void OS::SmartMemoryManager::free(void * ptr)
{
	if(!ptr){
		return;
	}
	// stat_free_count++;
#ifdef OS_DEBUG
	int * p = (int*)ptr - 2;
#else
	int * p = (int*)ptr - 1;
#endif
	int size = p[0];
	if(size & 0x80000000){
		stdFree(ptr); // p, size & ~0x80000000);
		return;
	}
	MemBlock * mem_block = (MemBlock*)ptr - 1;
	OS_ASSERT(mem_block->block_size == size);
	freeMemBlock(mem_block);
	if(!(stat_free_count % 1024) && cached_bytes > allocated_bytes / 2){
		freeCachedMemory(cached_bytes / 2);
	}
}

void OS::SmartMemoryManager::setBreakpointId(int id)
{
#ifdef OS_DEBUG
	dbg_breakpoint_id = id;
#endif
}

int OS::SmartMemoryManager::getAllocatedBytes()
{
	return allocated_bytes;
}

int OS::SmartMemoryManager::getMaxAllocatedBytes()
{
	return max_allocated_bytes;
}

int OS::SmartMemoryManager::getCachedBytes()
{
	return cached_bytes;
}

// =====================================================================
// =====================================================================
// =====================================================================

OS::OS()
{
	ref_count = 1;
	memory_manager = NULL;
	core = NULL;
#ifdef OS_DEBUG
	int mark = 0;
	native_stack_start_mark = (int)&mark;
	native_stack_max_usage = 0;
#endif
}

OS::~OS()
{
	OS_ASSERT(ref_count == 0);
	OS_ASSERT(!core && !memory_manager);
}

#ifdef OS_DEBUG
void OS::checkNativeStackUsage(const OS_CHAR * func_name)
{
	int mark = 0;
	int cur_native_stack_usage = (int)&mark - native_stack_start_mark;
	if(cur_native_stack_usage < 0){
		cur_native_stack_usage = -cur_native_stack_usage;
	}
	if(native_stack_max_usage < cur_native_stack_usage){
		if(cur_native_stack_usage > 1024*10 && cur_native_stack_usage > native_stack_max_usage * 5 / 4){
			printf(OS_TEXT("native stack usage: %.1f Kb (%s)\n"), (float)cur_native_stack_usage/1024.0f, func_name);
		}
		native_stack_max_usage = cur_native_stack_usage;
	}
}
#endif

void * OS::malloc(int size OS_DBG_FILEPOS_DECL)
{
	return memory_manager->malloc(size OS_DBG_FILEPOS_PARAM);
}

void OS::free(void * p)
{
	memory_manager->free(p);
}

void * OS::Core::malloc(int size OS_DBG_FILEPOS_DECL)
{
	return allocator->malloc(size OS_DBG_FILEPOS_PARAM);
}

void OS::Core::free(void * p)
{
	allocator->free(p);
}

int OS::getAllocatedBytes()
{
	return memory_manager->getAllocatedBytes();
}

int OS::getMaxAllocatedBytes()
{
	return memory_manager->getMaxAllocatedBytes();
}

int OS::getCachedBytes()
{
	return memory_manager->getCachedBytes();
}

void OS::setMemBreakpointId(int id)
{
	memory_manager->setBreakpointId(id);
}

bool OS::isTerminated()
{
	return core->terminated;
}

int OS::getTerminatedCode()
{
	return core->terminated_code;
}

void OS::setTerminated(bool terminated, int code)
{
	core->terminated = terminated;
	core->terminated_code = code;
}

void OS::resetTerminated()
{
	core->terminated = false;
	core->terminated_code = 0;
}

OS::Core::Core(OS * p_allocator)
{
	allocator = p_allocator;
	strings = NULL;
	OS_MEMSET(prototypes, 0, sizeof(prototypes));

	check_recursion = NULL;

	num_created_values = 0;
	num_destroyed_values = 0;

	stack_func = NULL;
	stack_func_locals = NULL;
	stack_func_env_index = 0;
	stack_func_prog_values = NULL;

	settings.create_compiled_file = true;
	settings.create_debug_info = true;
	settings.create_debug_opcodes = true;
	settings.create_debug_eval_opcodes = false;
	settings.primary_compiled_file = false;

	gcInitGreyList();

	OS_MEMSET(rand_state, 0, sizeof(rand_state));
	rand_next = NULL;
	rand_seed = 0;
	rand_left = 0;

	terminated = false;
	terminated_code = 0;
}

OS::Core::~Core()
{
	OS_ASSERT(!strings && global_vars.isNull() && user_pool.isNull() && !check_recursion);
	for(int i = 0; i < PROTOTYPE_COUNT; i++){
		OS_ASSERT(!prototypes[i]);
	}
}

OS * OS::create(MemoryManager * manager)
{
	return create(new OS(), manager);
}

OS * OS::start(MemoryManager * manager)
{
	if(init(manager)){
		return this;
	}
	delete this;
	return NULL;
}

bool OS::init(MemoryManager * p_manager)
{
	memory_manager = p_manager ? p_manager : new SmartMemoryManager();
	core = new (malloc(sizeof(Core) OS_DBG_FILEPOS)) Core(this);

	if(core->init()){
#if 1
		initPreScript();
		initGlobalFunctions();
		initObjectClass();
		initArrayClass();
		initStringClass();
		initBufferClass();
		initFunctionClass();
		initFileClass();
		initMathModule();
		initGCModule();
		initLangTokenizerModule();
		initPostScript();
#endif
		return true;
	}
	return false;
}

void OS::shutdown()
{
	core->shutdown();
	core->~Core();
	free(core);
	core = NULL;

	memory_manager->release();
	memory_manager = NULL;
}

OS * OS::retain()
{
	ref_count++;
	return this;
}

void OS::release()
{
	if(--ref_count <= 0){
		OS_ASSERT(ref_count == 0);
		shutdown();
		delete this;
	}
}

bool OS::Core::init()
{
	int i;
	for(i = 0; i < PROTOTYPE_COUNT; i++){
		prototypes[i] = newObjectValue(NULL);
		prototypes[i]->type = OS_VALUE_TYPE_OBJECT;
		prototypes[i]->external_ref_count++;
	}
	check_recursion = newObjectValue();
	global_vars = newObjectValue();
	user_pool = newObjectValue();
	// error_handlers

	prototypes[PROTOTYPE_BOOL]->prototype = prototypes[PROTOTYPE_OBJECT];
	prototypes[PROTOTYPE_NUMBER]->prototype = prototypes[PROTOTYPE_OBJECT];
	prototypes[PROTOTYPE_STRING]->prototype = prototypes[PROTOTYPE_OBJECT];
	prototypes[PROTOTYPE_ARRAY]->prototype = prototypes[PROTOTYPE_OBJECT];
	prototypes[PROTOTYPE_FUNCTION]->prototype = prototypes[PROTOTYPE_OBJECT];
	prototypes[PROTOTYPE_USERDATA]->prototype = prototypes[PROTOTYPE_OBJECT];

	strings = new (malloc(sizeof(Strings) OS_DBG_FILEPOS)) Strings(allocator);

	setGlobalValue(OS_TEXT("Object"), Value(prototypes[PROTOTYPE_OBJECT]), false);
	setGlobalValue(OS_TEXT("Boolean"), Value(prototypes[PROTOTYPE_BOOL]), false);
	setGlobalValue(OS_TEXT("Number"), Value(prototypes[PROTOTYPE_NUMBER]), false);
	setGlobalValue(OS_TEXT("String"), Value(prototypes[PROTOTYPE_STRING]), false);
	setGlobalValue(OS_TEXT("Array"), Value(prototypes[PROTOTYPE_ARRAY]), false);
	setGlobalValue(OS_TEXT("Function"), Value(prototypes[PROTOTYPE_FUNCTION]), false);
	setGlobalValue(OS_TEXT("Userdata"), Value(prototypes[PROTOTYPE_USERDATA]), false);

	/*
		SAFE usage of user function arguments 
		so user can use os->toNumber(-params+3) and so on
		if function call has no enough arguments, for example params == 0
		then (-params+3) will be not relative offset but absolute offset 3
		lets make top OS_TOP_STACK_NULL_VALUES value as null values
	*/
	for(i = 0; i < OS_TOP_STACK_NULL_VALUES; i++){
		pushValue(Value());
	}

	return true;
}

int OS::Core::compareGCValues(const void * a, const void * b)
{
	GCValue * v1 = *(GCValue**)a;
	GCValue * v2 = *(GCValue**)b;
	if(v1->external_ref_count != v2->external_ref_count){
		return v2->external_ref_count - v1->external_ref_count;
	}
	return v1->value_id - v2->value_id;
}

void OS::Core::shutdown()
{
	int i;
	OS_ASSERT(stack_values.count >= OS_TOP_STACK_NULL_VALUES);
	for(i = 0; i < OS_TOP_STACK_NULL_VALUES; i++){
		OS_ASSERT(OS_VALUE_TYPE(stack_values[i]) == OS_VALUE_TYPE_NULL);
	}
	// stack_values.count = 0;
	while(call_stack_funcs.count > 0){
		StackFunction * stack_func = &call_stack_funcs.lastElement();
		--call_stack_funcs.count;
		clearStackFunction(stack_func);
	}
	allocator->vectorClear(call_stack_funcs);
	// vectorClear(cache_values);

	// gcFull();
	gcResetGreyList();

	// try to finalize the values accurately
	GCValue * destroy_list = NULL;
	int destroy_list_size = 0;
	const int MAX_FIX_TIMES = 10;
	for(int j = 0; j < MAX_FIX_TIMES && values.count; j++){
		for(i = 0; i <= values.head_mask; i++){
			while(values.heads[i]){
				GCValue * value = values.heads[i];
				if(i+1 < MAX_FIX_TIMES){
					triggerValueDestructor(value);
				}
				values.heads[i] = value->hash_next;
				value->hash_next = destroy_list;
				// value->value_id = 0; needed to sort properly
				destroy_list = value;
				destroy_list_size++;
				values.count--;
			}
		}
	}
	OS_ASSERT(!values.count);

	Vector<GCValue*> collectedValues;
	allocator->vectorReserveCapacity(collectedValues, destroy_list_size OS_DBG_FILEPOS);
	while(destroy_list){
		GCValue * value = destroy_list;
		destroy_list = value->hash_next;
		value->hash_next = NULL;
		allocator->vectorAddItem(collectedValues, value OS_DBG_FILEPOS);
	}
	
	allocator->deleteObj(strings);

	::qsort(collectedValues.buf, collectedValues.count, sizeof(GCValue*), compareGCValues);
	for(i = collectedValues.count-1; i >= 0; i--){
		GCValue * value = collectedValues[i];
		clearValue(value);
		deleteValue(value);
	}
	allocator->vectorClear(collectedValues);
	deleteValues(true); // just clear values.heads

	check_recursion = NULL;
	global_vars = (GCValue*)NULL;
	user_pool = (GCValue*)NULL;

	for(i = 0; i < OS_ERROR_LEVELS; i++){
		error_handlers[i] = NULL;
	}
	for(i = 0; i < PROTOTYPE_COUNT; i++){
		prototypes[i] = NULL;
	}
	deleteStringRefs();
	deleteUserptrRefs();
	if(stack_values.buf){ // it makes sense because of someone could use stack while finalizing
		free(stack_values.buf);
		stack_values.buf = NULL;
		stack_values.capacity = 0;
		stack_values.count = 0;
	}
	OS_ASSERT(!call_stack_funcs.count);
}

OS::String OS::changeFilenameExt(const String& filename, const String& ext)
{
	int len = filename.getLen();
	for(int i = len-1; i >= 0; i--){
		if(filename[i] == OS_TEXT('.')){
			return String(this, filename, i, ext, ext.getLen());
		}
		if(OS_IS_SLASH(filename[i])){
			break;
		}
	}
	return String(this, filename, len, ext, ext.getLen());
}

OS::String OS::changeFilenameExt(const String& filename, const OS_CHAR * ext)
{
	int len = filename.getLen();
	for(int i = len-1; i >= 0; i--){
		if(filename[i] == OS_TEXT('.')){
			if(OS_STRCMP(filename.toChar()+i, ext) == 0){
				return filename;
			}
			return String(this, filename, i, ext, OS_STRLEN(ext));
		}
		if(OS_IS_SLASH(filename[i])){
			break;
		}
	}
	return String(this, filename, len, ext, OS_STRLEN(ext));
}

OS::String OS::getFilenameExt(const String& filename)
{
	return getFilenameExt(filename, filename.getLen());
}

OS::String OS::getFilenameExt(const OS_CHAR * filename)
{
	return getFilenameExt(filename, OS_STRLEN(filename));
}

OS::String OS::getFilenameExt(const OS_CHAR * filename, int len)
{
	for(int i = len-1; i >= 0; i--){
		if(filename[i] == OS_TEXT('.')){
			return String(this, filename+i, len-i);
		}
		if(OS_IS_SLASH(filename[i])){
			break;
		}
	}
	return String(this);
}

OS::String OS::getFilename(const String& filename)
{
	return getFilename(filename, filename.getLen());
}

OS::String OS::getFilename(const OS_CHAR * filename)
{
	return getFilename(filename, OS_STRLEN(filename));
}

OS::String OS::getFilename(const OS_CHAR * filename, int len)
{
	for(int i = len-1; i >= 0; i--){
		if(OS_IS_SLASH(filename[i])){
			return String(this, filename+i+1, len-i-1);
		}
	}
	return String(this, filename, len);
}

OS::String OS::getFilenamePath(const String& filename)
{
	return getFilenamePath(filename, filename.getLen());
}

OS::String OS::getFilenamePath(const OS_CHAR * filename)
{
	return getFilenamePath(filename, OS_STRLEN(filename));
}

OS::String OS::getFilenamePath(const OS_CHAR * filename, int len)
{
	for(int i = len-1; i >= 0; i--){
		if(OS_IS_SLASH(filename[i])){
			return String(this, filename, i);
		}
	}
	return String(this);
}

bool OS::isAbsolutePath(const String& p_filename)
{
	int len = p_filename.getLen();
	const OS_CHAR * filename = p_filename;
	if(OS_IS_ALPHA(filename[0])){
		for(int i = 1; i < len-2; i++){
			if(!OS_IS_ALPHA(filename[i])){
				return filename[i] == OS_TEXT(':') && OS_IS_SLASH(filename[i+1]);
			}
		}
	}
	return len >= 2 && OS_IS_SLASH(filename[0]) && OS_IS_SLASH(filename[1]);
}

OS::String OS::resolvePath(const String& filename, const String& cur_path)
{
	String resolved_path = filename;
	if(!isAbsolutePath(filename) && cur_path.getLen()){
		if(filename.getLen() < cur_path.getLen() || String(this, filename.toChar(), cur_path.getLen()) != cur_path){
			resolved_path = cur_path + OS_PATH_SEPARATOR + filename;
		}
	}
	if(isFileExist(resolved_path)){
		return resolved_path;
	}
	resolved_path = changeFilenameExt(resolved_path, OS_EXT_SOURCECODE);
	if(isFileExist(resolved_path)){
		return resolved_path;
	}
	resolved_path = changeFilenameExt(resolved_path, OS_EXT_COMPILED);
	if(isFileExist(resolved_path)){
		return resolved_path;
	}
	core->error(OS_E_WARNING, String::format(this, OS_TEXT("filename %s is not resolved"), filename.toChar()));
	return String(this);
}

OS::String OS::getCompiledFilename(const OS::String& resolved_filename)
{
	return changeFilenameExt(resolved_filename, OS_EXT_COMPILED);
}

OS::String OS::getDebugInfoFilename(const String& resolved_filename)
{
	return changeFilenameExt(resolved_filename, OS_EXT_DEBUG_INFO);
}

OS::String OS::getDebugOpcodesFilename(const String& resolved_filename)
{
	if(resolved_filename.getDataSize()){
		return changeFilenameExt(resolved_filename, OS_EXT_DEBUG_OPCODES);
	}
	static int num_evals = 0;
	return String(this, Core::String::format(this, OS_TEXT("eval-%d%s"), ++num_evals, OS_EXT_DEBUG_OPCODES));
}

OS::String OS::resolvePath(const String& filename)
{
	String cur_path(this);
	if(core->call_stack_funcs.count > 0){
		for(int i = core->call_stack_funcs.count-1; i >= 0; i--){
			Core::StackFunction * stack_func = core->call_stack_funcs.buf + i;
			if(stack_func->func->prog->filename.getLen() > 0){
				cur_path = getFilenamePath(stack_func->func->prog->filename);
				break;
			}
		}
	}
	return resolvePath(filename, cur_path);
}

OS::EFileUseType OS::checkFileUsage(const String& sourcecode_filename, const String& compiled_filename)
{
	return COMPILE_SOURCECODE_FILE;
}

void OS::Core::errorDivisionByZero()
{
	error(OS_E_WARNING, OS_TEXT("division by zero"));
}

void OS::Core::error(int code, const OS_CHAR * message)
{
	error(code, String(allocator, message));
}

void OS::Core::error(int code, const String& message)
{
	Program * prog = NULL;
	Program::DebugInfoItem * debug_info = NULL;
	for(int i = call_stack_funcs.count-1; i >= 0 && !debug_info; i--){
		Core::StackFunction * stack_func = call_stack_funcs.buf + i;
		prog = stack_func->func->prog;
		if(prog->filename.getLen() > 0){
			int opcode_pos = stack_func->opcodes - prog->opcodes.buf;
			debug_info = prog->getDebugInfo(opcode_pos);
		}
	}
	int error_level = 0;
	for(int i = 0; i < OS_ERROR_LEVELS; i++){
		if(code & (1<<i)){
			error_level = i;
			break;
		}
	}
	if(error_handlers[error_level].isFunction()){
		pushValue(error_handlers[error_level]);
		pushNull();
		pushNumber(code);
		pushStringValue(message);
		if(debug_info){
			pushStringValue(prog->filename);
			pushNumber(debug_info->line);
			call(4, 0);
		}else{
			call(2, 0);
		}
		return;
	}
	const OS_CHAR * error_type = NULL;
	switch(code){
	case OS_E_WARNING:
		error_type = OS_TEXT("WARNING");
		break;

	default:
	case OS_E_ERROR:
		error_type = OS_TEXT("ERROR");
		code = OS_E_ERROR;
		break;
	}
	if(debug_info){
		allocator->printf("[%s] %s (line: %d, pos: %d, token: %s, filename: %s)\n", error_type, message.toChar(), debug_info->line, debug_info->pos, 
			debug_info->token.toChar(), prog->filename.toChar());
	}else{
		allocator->printf("[%s] %s\n", error_type, message.toChar());
	}
}

void OS::Core::gcInitGreyList()
{
	gc_grey_list_first = NULL;
	gc_grey_root_initialized = false;
	gc_start_allocated_bytes = 0;
	gc_max_allocated_bytes = 0;
	gc_keep_heap_count = 0;
	gc_continuous_count = 0;
	gc_continuous = false;
	gc_values_head_index = -1;
	gc_time = 0;
	gc_in_process = false;
	gc_grey_added_count = 0;
	gc_start_values_mult = 1.5f;
	gc_step_size_mult = 0.005f;
	gc_step_size_auto_mult = 1.0f;
	gc_start_next_values = 16;
	gc_step_size = 0;
}

void OS::Core::gcResetGreyList()
{
	while(gc_grey_list_first){
		gcRemoveFromGreyList(gc_grey_list_first);
	}
	gc_grey_root_initialized = false;
}

void OS::Core::gcMarkList(int step_size)
{
	if(step_size < 16){
		step_size = 16;
	}
	for(; step_size > 0 && gc_grey_list_first; step_size--){
		gcMarkValue(gc_grey_list_first);
	}
}

void OS::Core::gcMarkTable(Table * table)
{
	Property * prop = table->first, * prop_next;
	for(; prop; prop = prop_next){
		prop_next = prop->next;
		if(OS_VALUE_TYPE(prop->index) == OS_VALUE_TYPE_WEAKREF){
			OS_ASSERT(false);
			if(!values.get(OS_VALUE_VARIANT(prop->index).value_id)){
				PropertyIndex index = *prop;
				deleteTableProperty(table, index);
				continue;
			}
		}
		if(OS_VALUE_TYPE(prop->value) == OS_VALUE_TYPE_WEAKREF){
			if(!values.get(OS_VALUE_VARIANT(prop->value).value_id)){
				PropertyIndex index = *prop;
				deleteTableProperty(table, index);
				continue;
			}
		}
		gcAddToGreyList(prop->index);
		gcAddToGreyList(prop->value);
	}
}

void OS::Core::gcMarkProgram(Program * prog)
{
}

void OS::Core::gcMarkLocals(Locals * locals)
{
	if(locals->gc_time == gc_time){
		return;
	}
	locals->gc_time = gc_time;

	int i;
	for(i = 0; i < locals->func_decl->num_locals; i++){
		gcAddToGreyList(locals->values[i]);
	}
	for(i = 0; i < locals->num_parents; i++){
		gcMarkLocals(locals->getParent(i));
	}
}

void OS::Core::gcMarkStackFunction(StackFunction * stack_func)
{
	OS_ASSERT(stack_func->func && stack_func->func->type == OS_VALUE_TYPE_FUNCTION);

	gcAddToGreyList(stack_func->func);
	if(stack_func->self_for_proto){
		gcAddToGreyList(stack_func->self_for_proto);
	}

	gcMarkLocals(stack_func->locals);

	if(stack_func->arguments){
		gcAddToGreyList(stack_func->arguments);
	}
	if(stack_func->rest_arguments){
		gcAddToGreyList(stack_func->rest_arguments);
	}
}

void OS::Core::gcAddToGreyList(const Value& val)
{
	switch(OS_VALUE_TYPE(val)){
	case OS_VALUE_TYPE_STRING:
	case OS_VALUE_TYPE_ARRAY:
	case OS_VALUE_TYPE_OBJECT:
	case OS_VALUE_TYPE_USERDATA:
	case OS_VALUE_TYPE_USERPTR:
	case OS_VALUE_TYPE_FUNCTION:
	case OS_VALUE_TYPE_CFUNCTION:
		gcAddToGreyList(OS_VALUE_VARIANT(val).value);
		break;
	}
}

void OS::Core::gcAddToGreyList(GCValue * value)
{
	if(value->gc_color != GC_WHITE){
		return;
	}
	OS_ASSERT(!value->gc_grey_next);
	value->gc_grey_next = gc_grey_list_first;
	gc_grey_list_first = value;
	value->gc_color = GC_GREY;
	gc_grey_added_count++;
}

void OS::Core::gcRemoveFromGreyList(GCValue * value)
{
	OS_ASSERT(value->gc_color == GC_GREY);
	OS_ASSERT(gc_grey_list_first == value);
	gc_grey_list_first = value->gc_grey_next;
	value->gc_grey_next = NULL;
	value->gc_color = GC_BLACK;
}

void OS::Core::gcMarkValue(GCValue * value)
{
	gcRemoveFromGreyList(value);
	if(value->prototype){
		gcAddToGreyList(value->prototype);
	}
	if(value->table){
		gcMarkTable(value->table);
	}
	switch(value->type){
	case OS_VALUE_TYPE_NULL:
	case OS_VALUE_TYPE_BOOL:
	case OS_VALUE_TYPE_NUMBER:
	default:
		OS_ASSERT(false);
		break;

	case OS_VALUE_TYPE_STRING:
		OS_ASSERT(dynamic_cast<GCStringValue*>(value));
		break;

	case OS_VALUE_TYPE_OBJECT:
		OS_ASSERT(dynamic_cast<GCObjectValue*>(value));
		break;

	case OS_VALUE_TYPE_ARRAY:
		{
			OS_ASSERT(dynamic_cast<GCArrayValue*>(value));
			GCArrayValue * arr = (GCArrayValue*)value;
			for(int i = 0; i < arr->values.count; i++){
				gcAddToGreyList(arr->values[i]);
			}
			break;
		}

	case OS_VALUE_TYPE_FUNCTION:
		{
			OS_ASSERT(dynamic_cast<GCFunctionValue*>(value));
			GCFunctionValue * func_value = (GCFunctionValue*)value;
			gcMarkProgram(func_value->prog);
			gcAddToGreyList(func_value->env);
			if(func_value->locals){
				gcMarkLocals(func_value->locals);
			}
			if(func_value->name){
				gcAddToGreyList(func_value->name);
			}
			break;
		}

	case OS_VALUE_TYPE_CFUNCTION:
		{
			OS_ASSERT(dynamic_cast<GCCFunctionValue*>(value));
			GCCFunctionValue * func_value = (GCCFunctionValue*)value;
			Value * closure_values = (Value*)(func_value + 1);
			for(int i = 0; i < func_value->num_closure_values; i++){
				gcAddToGreyList(closure_values[i]);
			}
			if(func_value->name){
				gcAddToGreyList(func_value->name);
			}
			break;
		}

	case OS_VALUE_TYPE_USERDATA:
	case OS_VALUE_TYPE_USERPTR:
		OS_ASSERT(dynamic_cast<GCUserdataValue*>(value));
		break;
	}
}

int OS::Core::gcStep()
{
#if 0
	gc_in_process = true;
	return OS_GC_PHASE_MARK;
#endif
	if(gc_in_process){
		return OS_GC_PHASE_MARK;
	}
	struct GCTouch {
		Core * core;
		GCTouch(Core * p_core)
		{
			core = p_core; core->gc_in_process = true;
			OS_PROFILE_BEGIN_GC;
		}
		~GCTouch()
		{
			core->gc_in_process = false;
			OS_PROFILE_END_GC;
		}
	} gc_touch(this);

	if(values.count == 0){
		gc_values_head_index = -1;
		gc_grey_root_initialized = false;
		gc_continuous = false;
		return OS_GC_PHASE_MARK;
	}
	int step_size = gc_step_size;
	if(gc_values_head_index >= 0){
		OS_ASSERT(gc_values_head_index <= values.head_mask);
		int i = gc_values_head_index;
		step_size += 2; // step_size/16;
		Value func;
		GCValue * destroy_list = NULL;
		for(; i <= values.head_mask && step_size > 0; i++){
			GCValue * value = values.heads[i], * prev = NULL, * next;
			for(; value; value = next, step_size--){
				next = value->hash_next;
				if(value->gc_color == GC_WHITE && !value->external_ref_count){
					OS_ASSERT(!isValueUsed(value));
					triggerValueDestructor(value);
					if(prev){
						prev->hash_next = next;
					}else{
						values.heads[i] = next;
					}
					value->hash_next = destroy_list;
					// value->value_id = 0; need by clearValue
					destroy_list = value;
					values.count--;
					/* deleteValue(value);
					if(gc_values_head_index < 0){
						return OS_GC_PHASE_MARK;
					} */
					continue;
				}
				if(value->gc_color == GC_BLACK){
					value->gc_color = GC_WHITE;
				}
				prev = value;
			}
		}
		while(destroy_list){
			GCValue * value = destroy_list;
			destroy_list = value->hash_next;
			value->hash_next = NULL;
			clearValue(value);
			deleteValue(value);
		}
		if(i <= values.head_mask){
			gc_values_head_index = i;
			gc_step_size_auto_mult *= 1.01f;
			gc_step_size = (int)((float)values.count * gc_step_size_mult * gc_step_size_auto_mult * 2);
			return OS_GC_PHASE_SWEEP;
		}
		gc_values_head_index = -1;
		gc_start_next_values = (int)((float)values.count * gc_start_values_mult);

		int end_allocated_bytes = allocator->getAllocatedBytes();
		gc_continuous_count++;
		if(gc_start_allocated_bytes == end_allocated_bytes){
			gc_step_size_auto_mult *= 0.5f;
			if(gc_step_size_auto_mult < 1){
				gc_step_size_auto_mult = 1.0f;
			}
			if(++gc_keep_heap_count >= 2){
				gc_continuous = false;
			}
		}else{
			gc_start_allocated_bytes = end_allocated_bytes;
			gc_keep_heap_count = 0;
		}

		if((!gc_continuous || !(gc_continuous_count%16)) && gc_max_allocated_bytes < end_allocated_bytes){
			gc_max_allocated_bytes = end_allocated_bytes;
			// allocator->printf("[GC] max allocated bytes %d, values %d\n", gc_max_allocated_bytes, values.count);
		}

		return OS_GC_PHASE_MARK;
	}
	if(!gc_grey_root_initialized){
		gc_grey_root_initialized = true;
		gc_step_size = (int)((float)values.count * gc_step_size_mult * gc_step_size_auto_mult * 2);
		gc_time++;

		if(!gc_continuous){
			gc_continuous = true;
			gc_continuous_count = 0;
			gc_keep_heap_count = 0;
			gc_step_size_auto_mult = 1.0f;
		}

		gcAddToGreyList(check_recursion);
		gcAddToGreyList(global_vars);
		gcAddToGreyList(user_pool);
		int i;
		for(i = 0; i < OS_ERROR_LEVELS; i++){
			gcAddToGreyList(error_handlers[i]);
		}
		for(i = 0; i < PROTOTYPE_COUNT; i++){
			gcAddToGreyList(prototypes[i]);
		}
	}
	int i;
	for(i = 0; i < stack_values.count; i++){
		gcAddToGreyList(stack_values[i]);
	}
	for(i = 0; i < call_stack_funcs.count; i++){
		gcMarkStackFunction(&call_stack_funcs[i]);
	}
	gcMarkList(step_size);
	gc_step_size = (int)((float)values.count * gc_step_size_mult * gc_step_size_auto_mult * 2);
	if(!gc_grey_list_first){
		gc_grey_root_initialized = false;
		gc_values_head_index = 0;
		gc_step_size_auto_mult *= 0.25f;
		if(gc_step_size_auto_mult < 1.0f){
			gc_step_size_auto_mult = 1.0f;
		}
		return OS_GC_PHASE_SWEEP;
	}
	gc_step_size_auto_mult *= 1.01f;
	return OS_GC_PHASE_MARK;
}

void OS::Core::gcFinishSweepPhase()
{
	if(gc_in_process || values.count == 0){
		return;
	}
	if(gc_values_head_index >= 0){
		gc_step_size = values.count * 2;
		gcStep();
		OS_ASSERT(gc_values_head_index < 0);
	}
}

void OS::Core::gcFinishMarkPhase()
{
	if(gc_in_process || values.count == 0){
		return;
	}
	while(gc_values_head_index < 0){
		gc_step_size = values.count * 2;
		gcStep();
	}
}

void OS::Core::gcStepIfNeeded()
{
	if(gc_in_process){
		return;
	}
	if(gc_values_head_index >= 0 || gc_grey_root_initialized || gc_continuous){
		gcStep();
	}else if(gc_start_next_values <= values.count){
		gcFinishSweepPhase();
		gcStep();
	}
}

void OS::Core::gcFull()
{
	if(gc_in_process){
		return;
	}
	gcFinishSweepPhase();
	int start_allocated_bytes = allocator->getAllocatedBytes();
	for(int i = 1;; i++){
		gcFinishMarkPhase();
		gcFinishSweepPhase();
		int end_allocated_bytes = allocator->getAllocatedBytes();
		if(start_allocated_bytes == end_allocated_bytes && i > 1){
			return;
		}
		start_allocated_bytes = end_allocated_bytes;
	}
}

void OS::Core::triggerValueDestructor(GCValue * val)
{
	if(val->is_destructor_called){
		OS_ASSERT(false);
		return;
	}
	val->is_destructor_called = true;

	Value self = val;
	PropertyIndex func(strings->__destruct, PropertyIndex::KeepStringIndex());
	for(Property * prop;;){
		Table * table = val->table;
		if(table && (prop = table->get(func)) && prop->value.isFunction()){
			pushValue(prop->value);
			pushValue(self);
			call(0, 0);
		}
		if(!val->prototype){
			return;
		}
		val = val->prototype;
	}
}

void OS::Core::clearValue(GCValue * val)
{
	OS_ASSERT(val->value_id);
	switch(val->type){
	case OS_VALUE_TYPE_NULL:
	case OS_VALUE_TYPE_BOOL:
	case OS_VALUE_TYPE_NUMBER:
	default:
		OS_ASSERT(false);
		break;

	case OS_VALUE_TYPE_STRING:
		{
			OS_ASSERT(dynamic_cast<GCStringValue*>(val));
			break;
		}

	case OS_VALUE_TYPE_USERDATA:
	case OS_VALUE_TYPE_USERPTR:
		{
			OS_ASSERT(dynamic_cast<GCUserdataValue*>(val));
			GCUserdataValue * userdata = (GCUserdataValue*)val;

			void * ptr = userdata->ptr;
			OS_UserdataDtor dtor  = userdata->dtor;

			// prevent recursion
			userdata->ptr = NULL;
			userdata->crc = 0;
			userdata->dtor = NULL;

			if(val->type == OS_VALUE_TYPE_USERPTR){
				unregisterUserptrRef(ptr, userdata->value_id);
			}
			if(dtor){
				dtor(allocator, ptr, userdata->user_param);
			}
			break;
		}

	case OS_VALUE_TYPE_FUNCTION:
		{
			OS_ASSERT(dynamic_cast<GCFunctionValue*>(val));
			GCFunctionValue * func_value = (GCFunctionValue*)val;
			clearFunctionValue(func_value);
			break;
		}

	case OS_VALUE_TYPE_CFUNCTION:
		{
			OS_ASSERT(dynamic_cast<GCCFunctionValue*>(val));
			GCCFunctionValue * func_value = (GCCFunctionValue*)val;
			func_value->func = NULL;
			func_value->user_param = NULL;
			func_value->num_closure_values = 0;
			break;
		}

	case OS_VALUE_TYPE_ARRAY:
		{
			OS_ASSERT(dynamic_cast<GCArrayValue*>(val));
			GCArrayValue * arr = (GCArrayValue*)val;
			allocator->vectorClear(arr->values);
			break;
		}

	case OS_VALUE_TYPE_OBJECT:
		OS_ASSERT(dynamic_cast<GCObjectValue*>(val));
		break;
	}
	if(val->table){
		Table * table = val->table;
		val->table = NULL;
		deleteTable(table);
	}
	// prototype could be already destroyed by gc or will be destroyed soon
	val->prototype = NULL;
	val->type = OS_VALUE_TYPE_UNKNOWN;
}

#ifdef OS_DEBUG
bool OS::Core::isValueUsed(GCValue * val)
{
	struct Lib {
		Core * core;
		GCValue * val;

		bool findAt(Value cur)
		{
			GCValue * value = cur.getGCValue();
			return value && findAt(value);
		}

		bool findAt(Locals * locals)
		{
			int i;
			for(i = 0; i < locals->func_decl->num_locals; i++){
				if(findAt(locals->values[i])){
					return true;
				}
			}
			for(i = 0; i < locals->num_parents; i++){
				if(findAt(locals->getParent(i))){
					return true;
				}
			}
			return false;
		}

		bool findAt(StackFunction * stack_func)
		{
			OS_ASSERT(stack_func->func);
			if(findAt(stack_func->func)){
				return true;
			}
			if(stack_func->self_for_proto && findAt(stack_func->self_for_proto)){
				return true;
			}
			if(stack_func->arguments && findAt(stack_func->arguments)){
				return true;
			}
			if(stack_func->rest_arguments && findAt(stack_func->rest_arguments)){
				return true;
			}
			return findAt(stack_func->locals);
		}

		bool findAt(Table * table)
		{
			OS_ASSERT(table);
			Property * prop = table->first;
			for(; prop; prop = prop->next){
				if(findAt(prop->index)){
					return true;
				}
				if(findAt(prop->value)){
					return true;
				}
			}
			return false;
		}

		bool findAt(GCValue * cur)
		{
			OS_ASSERT(cur != (GCValue*)0xdededede);
			if(cur->gc_time == core->gc_time){
				return false;
			}
			cur->gc_time = core->gc_time;

			if(cur == val){
				return true;
			}
			if(cur->prototype && findAt(cur->prototype)){
				return true;
			}
			if(cur->table && findAt(cur->table)){
				return true;
			}
			switch(cur->type){
			case OS_VALUE_TYPE_STRING:
				{
					OS_ASSERT(dynamic_cast<GCStringValue*>(cur));
					GCStringValue * string = (GCStringValue*)cur;
					OS_ASSERT(!string->table);
					break;
				}

			case OS_VALUE_TYPE_ARRAY:
				{
					OS_ASSERT(dynamic_cast<GCArrayValue*>(cur));
					GCArrayValue * arr = (GCArrayValue*)cur;
					for(int i = 0; i < arr->values.count; i++){
						if(findAt(arr->values[i])){
							return true;
						}
					}
					break;
				}

			case OS_VALUE_TYPE_OBJECT:
				OS_ASSERT(dynamic_cast<GCObjectValue*>(cur));
				break;

			case OS_VALUE_TYPE_USERDATA:
			case OS_VALUE_TYPE_USERPTR:
				OS_ASSERT(dynamic_cast<GCUserdataValue*>(cur));
				break;

			case OS_VALUE_TYPE_FUNCTION:
				{
					OS_ASSERT(dynamic_cast<GCFunctionValue*>(cur));
					GCFunctionValue * func_value = (GCFunctionValue*)cur;
					if(findAt(func_value->env)){
						return true;
					}
					if(func_value->locals && findAt(func_value->locals)){
						return true;
					}
					if(func_value->name && findAt(func_value->name)){
						return true;
					}
					for(int i = 0; i < func_value->prog->num_strings + func_value->prog->num_numbers + CONST_STD_VALUES; i++){
						if(findAt(func_value->prog->const_values[i])){
							return true;
						}
					}
					break;
				}

			case OS_VALUE_TYPE_CFUNCTION:
				{
					OS_ASSERT(dynamic_cast<GCCFunctionValue*>(cur));
					GCCFunctionValue * func_value = (GCCFunctionValue*)cur;
					Value * closure_values = (Value*)(func_value + 1);
					for(int i = 0; i < func_value->num_closure_values; i++){
						if(findAt(closure_values[i])){
							return true;
						}
					}
					if(func_value->name && findAt(func_value->name)){
						return true;
					}
					break;
				}

			case OS_VALUE_TYPE_WEAKREF:
				break;

			default:
				OS_ASSERT(false);
			}
			return false;
		}

	} lib = {this, val};

	if(lib.findAt(check_recursion)){
		return true;
	}
	if(lib.findAt(global_vars)){
		return true;
	}
	if(lib.findAt(user_pool)){
		return true;
	}
	int i;
	for(i = 0; i < PROTOTYPE_COUNT; i++){
		if(lib.findAt(prototypes[i])){
			return true;
		}
	}
	for(i = 0; i < stack_values.count; i++){
		if(lib.findAt(stack_values[i])){
			return true;
		}
	}
	for(i = 0; i < call_stack_funcs.count; i++){
		if(lib.findAt(&call_stack_funcs[i])){
			return true;
		}
	}
	return false;
}
#endif

void OS::Core::deleteValue(GCValue * val)
{
	OS_ASSERT(val);
	OS_ASSERT(val->value_id);
	OS_ASSERT(!val->hash_next);
	// OS_ASSERT(!isValueUsed(val));
	OS_ASSERT(val->gc_color != GC_GREY);
	/* if(val->value_id){
		triggerValueDestructor(val);
		unregisterValue(val->value_id);
		clearValue(val);
	}else{
		OS_ASSERT(val->is_destructor_called);
	} */
	val->~GCValue();
	free(val);
	num_destroyed_values++;
}

OS::Core::Property * OS::Core::setTableValue(Table * table, const PropertyIndex& index, Value value)
{
	OS_ASSERT(table);

	// TODO: correct ???
	gcAddToGreyList(value);

	Property * prop = table->get(index);
	if(prop){
		prop->value = value;
		return prop;
	}
	prop = new (malloc(sizeof(Property) OS_DBG_FILEPOS)) Property(index);
	prop->value = value;
	addTableProperty(table, prop);
	return prop;
}

bool OS::Core::hasSpecialPrefix(const Value& value)
{
	if(OS_VALUE_TYPE(value) != OS_VALUE_TYPE_STRING){
		return false;
	}
	OS_ASSERT(dynamic_cast<GCStringValue*>(OS_VALUE_VARIANT(value).string));
	GCStringValue * string = OS_VALUE_VARIANT(value).string;
#if 0
	// the first char is always exists because of any string data is null terminated
	return string->toChar()[0] == OS_TEXT('_');
#else
	if(string->getLen() >= 2){
		const OS_CHAR * s = string->toChar();
		return s[0] == OS_TEXT('_') && s[1] == OS_TEXT('_');
	}
	return false;
#endif
}

void OS::Core::setPropertyValue(GCValue * table_value, const PropertyIndex& index, Value value, bool setter_enabled)
{
#if defined OS_DEBUG && defined OS_WARN_NULL_INDEX
	if(table_value != check_recursion && index.index.type == OS_VALUE_TYPE_NULL){
		error(OS_E_WARNING, OS_TEXT("object set null index"));
	}
#endif
	// TODO: correct ???
	gcAddToGreyList(value);
	
	if(OS_VALUE_TYPE(index.index) == OS_VALUE_TYPE_STRING){
		OS_ASSERT(dynamic_cast<GCStringValue*>(OS_VALUE_VARIANT(index.index).string));
		switch(OS_VALUE_TYPE(value)){
		case OS_VALUE_TYPE_FUNCTION:
			OS_ASSERT(dynamic_cast<GCFunctionValue*>(OS_VALUE_VARIANT(value).func));
			if(!OS_VALUE_VARIANT(value).func->name){
				OS_VALUE_VARIANT(value).func->name = OS_VALUE_VARIANT(index.index).string;
			}
			break;

		case OS_VALUE_TYPE_CFUNCTION:
			OS_ASSERT(dynamic_cast<GCCFunctionValue*>(OS_VALUE_VARIANT(value).cfunc));
			if(!OS_VALUE_VARIANT(value).cfunc->name){
				OS_VALUE_VARIANT(value).cfunc->name = OS_VALUE_VARIANT(index.index).string;
			}
			break;
		}
	}

	Property * prop = NULL;
	Table * table = table_value->table;
	if(table && (prop = table->get(index))){
		prop->value = value;
		return;
	}

	// prototype should not be used in set
	/* if(prototype_enabled){
	} */

	if(OS_VALUE_TYPE(index.index) == OS_VALUE_TYPE_STRING && strings->syntax_prototype == OS_VALUE_VARIANT(index.index).string){
		switch(table_value->type){
		case OS_VALUE_TYPE_STRING:
		case OS_VALUE_TYPE_ARRAY:
		case OS_VALUE_TYPE_OBJECT:
		case OS_VALUE_TYPE_FUNCTION:
			table_value->prototype = OS_VALUE_VARIANT(value).value;
			break;

		case OS_VALUE_TYPE_USERDATA:
		case OS_VALUE_TYPE_USERPTR:
		case OS_VALUE_TYPE_CFUNCTION:
			// TODO: warning???
			break;
		}
		return;
	}

	if(table_value->type == OS_VALUE_TYPE_ARRAY){
		OS_ASSERT(dynamic_cast<GCArrayValue*>(table_value));
		GCArrayValue * arr = (GCArrayValue*)table_value;
		int i = (int)valueToInt(index.index);
		if(i >= 0 || (i += arr->values.count) >= 0){
			while(i >= arr->values.count){
				allocator->vectorAddItem(arr->values, Value() OS_DBG_FILEPOS);
			}
			OS_ASSERT(i < arr->values.count);
			arr->values[i] = value;
		}
		return;
	}

	if(setter_enabled && !hasSpecialPrefix(index.index)){
		Value func;
		if(OS_VALUE_TYPE(index.index) == OS_VALUE_TYPE_STRING){
			const void * buf1 = strings->__setAt.toChar();
			int size1 = strings->__setAt.getDataSize();
			const void * buf2 = OS_VALUE_VARIANT(index.index).string->toChar();
			int size2 = OS_VALUE_VARIANT(index.index).string->getDataSize();
			GCStringValue * setter_name = newStringValue(buf1, size1, buf2, size2);
			if(getPropertyValue(func, table_value, PropertyIndex(setter_name, PropertyIndex::KeepStringIndex()), true)){
				pushValue(func);
				pushValue(table_value);
				pushValue(value);
				call(1, 0);
				return;
			}
		}
		if(getPropertyValue(func, table_value, PropertyIndex(strings->__set, PropertyIndex::KeepStringIndex()), true)){
			pushValue(func);
			pushValue(table_value);
			pushValue(index.index);
			pushValue(value);
			call(2, 0);
			return;
		}
	}
	if(table_value->type == OS_VALUE_TYPE_STRING){
		// TODO: trigger error???
		return;
	}
	if(!table){
		table_value->table = table = newTable(OS_DBG_FILEPOS_START);
	}
	prop = new (malloc(sizeof(Property) OS_DBG_FILEPOS)) Property(index);
	prop->value = value;
	addTableProperty(table, prop);
}

void OS::Core::setPropertyValue(const Value& table_value, const PropertyIndex& index, const Value& value, bool setter_enabled)
{
	switch(OS_VALUE_TYPE(table_value)){
	case OS_VALUE_TYPE_NULL:
		return;

	case OS_VALUE_TYPE_BOOL:
		// return setPropertyValue(prototypes[PROTOTYPE_BOOL], index, value, setter_enabled);
		return;

	case OS_VALUE_TYPE_NUMBER:
		// return setPropertyValue(prototypes[PROTOTYPE_NUMBER], index, value, setter_enabled);
		return;

	case OS_VALUE_TYPE_STRING:
		// return setPropertyValue(prototypes[PROTOTYPE_STRING], index, value, setter_enabled);
		// return;
		// no break

	case OS_VALUE_TYPE_ARRAY:
	case OS_VALUE_TYPE_OBJECT:
	case OS_VALUE_TYPE_USERDATA:
	case OS_VALUE_TYPE_USERPTR:
	case OS_VALUE_TYPE_FUNCTION:
	case OS_VALUE_TYPE_CFUNCTION:
		return setPropertyValue(OS_VALUE_VARIANT(table_value).value, index, value, setter_enabled);
	}
}

void OS::Core::pushPrototype(const Value& val)
{
	switch(OS_VALUE_TYPE(val)){
	case OS_VALUE_TYPE_NULL:
		pushNull();
		return;

	case OS_VALUE_TYPE_BOOL:
		pushValue(prototypes[PROTOTYPE_BOOL]);
		return;

	case OS_VALUE_TYPE_NUMBER:
		pushValue(prototypes[PROTOTYPE_NUMBER]);
		return;

	case OS_VALUE_TYPE_STRING:
	case OS_VALUE_TYPE_ARRAY:
	case OS_VALUE_TYPE_OBJECT:
	case OS_VALUE_TYPE_USERDATA:
	case OS_VALUE_TYPE_USERPTR:
	case OS_VALUE_TYPE_FUNCTION:
	case OS_VALUE_TYPE_CFUNCTION:
		pushValue(OS_VALUE_VARIANT(val).value->prototype);
		return;
	}
}

void OS::Core::setPrototype(const Value& val, const Value& proto, int userdata_crc)
{
	switch(OS_VALUE_TYPE(val)){
	case OS_VALUE_TYPE_NULL:
	case OS_VALUE_TYPE_BOOL:
	case OS_VALUE_TYPE_NUMBER:
		return;

	case OS_VALUE_TYPE_USERDATA:
	case OS_VALUE_TYPE_USERPTR:
		if(OS_VALUE_VARIANT(val).userdata->crc != userdata_crc){
			return;
		}
		// no break

	case OS_VALUE_TYPE_STRING:
	case OS_VALUE_TYPE_ARRAY:
	case OS_VALUE_TYPE_OBJECT:
	case OS_VALUE_TYPE_FUNCTION:
	case OS_VALUE_TYPE_CFUNCTION:
		OS_VALUE_VARIANT(val).value->prototype = proto.getGCValue();
		return;
	}
}

OS::Core::GCStringValue * OS::Core::newStringValue(const OS_CHAR * str)
{
	return newStringValue(str, OS_STRLEN(str));
}

OS::Core::GCStringValue * OS::Core::newStringValue(const OS_CHAR * str, int len)
{
	return newStringValue((void*)str, len * sizeof(OS_CHAR));
}

OS::Core::GCStringValue * OS::Core::newStringValue(const OS_CHAR * str, int len, const OS_CHAR * str2, int len2)
{
	return newStringValue((void*)str, len * sizeof(OS_CHAR), str2, len2 * sizeof(OS_CHAR));
}

OS::Core::GCStringValue * OS::Core::newStringValue(const OS_CHAR * str, int len, bool trim_left, bool trim_right)
{
	if(trim_left){
		while(len > 0 && OS_IS_SPACE(*str)){
			str++;
			len--;
		}
	}
	if(trim_right){
		while(len > 0 && OS_IS_SPACE(str[len-1])){
			len--;
		}
	}
	return newStringValue((void*)str, len * sizeof(OS_CHAR));
}

OS::Core::GCStringValue * OS::Core::newStringValue(const String& p_str, bool trim_left, bool trim_right)
{
	const OS_CHAR * str = p_str.toChar();
	int len = p_str.getLen();
	bool changed = false;
	if(trim_left){
		while(len > 0 && OS_IS_SPACE(*str)){
			str++;
			len--;
			changed = true;
		}
	}
	if(trim_right){
		while(len > 0 && OS_IS_SPACE(str[len-1])){
			len--;
			changed = true;
		}
	}
	if(changed){
		return newStringValue(str, len);
	}
	return p_str.string;
}

OS::Core::GCStringValue * OS::Core::newStringValue(const String& str)
{
	return str.string;
}

OS::Core::GCStringValue * OS::Core::newStringValue(const void * buf, int size)
{
	return newStringValue(buf, size, NULL, 0);
}

OS::Core::GCStringValue * OS::Core::newStringValue(const void * buf1, int size1, const void * buf2, int size2)
{
	if(string_refs.count > 0){
		OS_ASSERT(string_refs.heads && string_refs.head_mask);
		int hash = Utils::keyToHash(buf1, size1, buf2, size2);
		int slot = hash & string_refs.head_mask;
		StringRef * str_ref = string_refs.heads[slot];
		for(StringRef * prev = NULL, * next; str_ref; str_ref = next){
			next = str_ref->hash_next;
			GCStringValue * string_value = (GCStringValue*)values.get(str_ref->string_value_id);
			if(!string_value){
				if(!prev){
					string_refs.heads[slot] = next;
				}else{
					prev->hash_next = next;					
				}
				free(str_ref);
				string_refs.count--;
				continue;
			}
			OS_ASSERT(string_value->type == OS_VALUE_TYPE_STRING);
			OS_ASSERT(dynamic_cast<GCStringValue*>(string_value));
			if(string_value->isEqual(hash, buf1, size1, buf2, size2)){
				return string_value;
			}
			prev = str_ref;
		}
	}
	GCStringValue * string_value = GCStringValue::alloc(allocator, buf1, size1, buf2, size2 OS_DBG_FILEPOS);
	StringRef * str_ref = (StringRef*)malloc(sizeof(StringRef) OS_DBG_FILEPOS);
	str_ref->string_hash = string_value->hash;
	str_ref->string_value_id = string_value->value_id;
	str_ref->hash_next = NULL;
	registerStringRef(str_ref);
	return string_value;
}

OS::Core::GCStringValue * OS::Core::newStringValue(const void * buf1, int size1, const void * buf2, int size2, const void * buf3, int size3)
{
	if(size1 <= 0){
		return newStringValue(buf2, size2, buf3, size3);
	}
	if(size2 <= 0){
		return newStringValue(buf1, size1, buf3, size3);
	}
	if(size3 <= 0){
		return newStringValue(buf1, size1, buf2, size2);
	}
	if(size1 + size2 + size3 <= 1024){
		OS_BYTE * buf = (OS_BYTE*)alloca(size1 + size2 + size3 + sizeof(OS_CHAR));
		OS_MEMCPY(buf, buf1, size1);
		OS_MEMCPY(buf+size1, buf2, size2);
		OS_MEMCPY(buf+size1+size2, buf3, size3);
		buf[size1+size2+size3] = (OS_CHAR)0;
		return newStringValue(buf, (size1 + size2 + size3) / sizeof(OS_CHAR));
	}
	GCStringValue * str = newStringValue(buf1, size1, buf2, size2);
	return newStringValue(str->toBytes(), str->data_size, buf3, size3);
}

OS::Core::GCStringValue * OS::Core::newStringValue(GCStringValue * a, GCStringValue * b)
{
	if(a->data_size <= 0){
		return b;
	}
	if(b->data_size <= 0){
		return a;
	}
	return newStringValue((void*)a->toBytes(), a->data_size, (void*)b->toBytes(), b->data_size);
}

OS::Core::GCStringValue * OS::Core::newStringValue(const String& a, const String& b)
{
	return newStringValue(a.string, b.string);
}

OS::Core::GCStringValue * OS::Core::newStringValue(OS_INT val)
{
	OS_CHAR str[128];
	Utils::numToStr(str, val);
	return newStringValue(str);
}

OS::Core::GCStringValue * OS::Core::newStringValue(OS_FLOAT val, int precision)
{
	OS_CHAR str[128];
	Utils::numToStr(str, val, precision);
	return newStringValue(str);
}

OS::Core::GCStringValue * OS::Core::newStringValue(int temp_buf_len, const OS_CHAR * fmt, ...)
{
	va_list va;
	va_start(va, fmt);
	OS_VaListDtor va_dtor(&va);
	return newStringValueVa(temp_buf_len, fmt, va);
}

OS::Core::GCStringValue * OS::Core::newStringValueVa(int temp_buf_len, const OS_CHAR * fmt, va_list va)
{
	OS_ASSERT(temp_buf_len <= OS_DEF_FMT_BUF_LEN);
	OS_CHAR * buf = (OS_CHAR*)malloc(temp_buf_len * sizeof(OS_CHAR) OS_DBG_FILEPOS);
	OS_VSNPRINTF(buf, sizeof(OS_CHAR) * (temp_buf_len-1), fmt, va);
	GCStringValue * result = newStringValue(buf);
	free(buf);
	return result;
}

OS::Core::GCCFunctionValue * OS::Core::newCFunctionValue(OS_CFunction func, void * user_param)
{
	return newCFunctionValue(func, 0, user_param);
}

OS::Core::GCCFunctionValue * OS::Core::newCFunctionValue(OS_CFunction func, int num_closure_values, void * user_param)
{
	OS_ASSERT(stack_values.count >= num_closure_values);
	if(!func){
		return NULL;
	}
	GCCFunctionValue * res = new (malloc(sizeof(GCCFunctionValue) + sizeof(Value) * num_closure_values OS_DBG_FILEPOS)) GCCFunctionValue();
	res->prototype = prototypes[PROTOTYPE_FUNCTION];
	res->name = NULL;
	res->func = func;
	res->user_param = user_param;
	res->num_closure_values = num_closure_values;
	Value * closure_values = (Value*)(res + 1);
	if(num_closure_values > 0){
		OS_MEMCPY(closure_values, stack_values.buf + (stack_values.count - num_closure_values), sizeof(Value)*num_closure_values);
	}
	res->type = OS_VALUE_TYPE_CFUNCTION;
	pop(num_closure_values);
	registerValue(res);
	return res;
}

OS::Core::GCUserdataValue * OS::Core::newUserdataValue(int crc, int data_size, OS_UserdataDtor dtor, void * user_param)
{
	GCUserdataValue * res = new (malloc(sizeof(GCUserdataValue) + data_size OS_DBG_FILEPOS)) GCUserdataValue();
	res->prototype = prototypes[PROTOTYPE_USERDATA];
	res->crc = crc;
	res->dtor = dtor;
	res->user_param = user_param;
	res->ptr = data_size ? res + 1 : NULL;
	res->type = OS_VALUE_TYPE_USERDATA;
	registerValue(res);
	return res;
}

OS::Core::GCUserdataValue * OS::Core::newUserPointerValue(int crc, void * ptr, OS_UserdataDtor dtor, void * user_param)
{
	int hash = OS_PTR_HASH(ptr);
	if(userptr_refs.count > 0){
		OS_ASSERT(userptr_refs.heads && userptr_refs.head_mask > 0);
		int slot = hash & userptr_refs.head_mask;
		UserptrRef * userptr_ref = userptr_refs.heads[slot];
		for(UserptrRef * prev = NULL, * next; userptr_ref; userptr_ref = next){
			next = userptr_ref->hash_next;
			GCUserdataValue * userptr_value = (GCUserdataValue*)values.get(userptr_ref->userptr_value_id);
			if(!userptr_value){
				if(!prev){
					userptr_refs.heads[slot] = next;
				}else{
					prev->hash_next = next;					
				}
				free(userptr_ref);
				userptr_refs.count--;
				continue;
			}
			OS_ASSERT(userptr_value->type == OS_VALUE_TYPE_USERPTR);
			OS_ASSERT(dynamic_cast<GCUserdataValue*>(userptr_value));
			if(userptr_value->ptr == ptr){ // && userptr_value->crc == crc){
				OS_ASSERT(userptr_value->crc == crc);
				OS_ASSERT(userptr_value->dtor == dtor);
				if(userptr_value->crc != crc){
					if(!prev){
						userptr_refs.heads[slot] = next;
					}else{
						prev->hash_next = next;					
					}
					free(userptr_ref);
					userptr_refs.count--;
					continue;
				}
				return userptr_value;
			}
			prev = userptr_ref;
		}
	}
	GCUserdataValue * res = new (malloc(sizeof(GCUserdataValue) OS_DBG_FILEPOS)) GCUserdataValue();
	res->prototype = prototypes[PROTOTYPE_USERDATA];
	res->crc = crc;
	res->dtor = dtor;
	res->user_param = user_param;
	res->ptr = ptr;
	res->type = OS_VALUE_TYPE_USERPTR;
	registerValue(res);

	UserptrRef * userptr_ref = (UserptrRef*)malloc(sizeof(UserptrRef) OS_DBG_FILEPOS);
	userptr_ref->userptr_hash = hash;
	userptr_ref->userptr_value_id = res->value_id;
	userptr_ref->hash_next = NULL;
	registerUserptrRef(userptr_ref);

	return res;
}

OS::Core::GCObjectValue * OS::Core::newObjectValue()
{
	return newObjectValue(prototypes[PROTOTYPE_OBJECT]);
}

OS::Core::GCObjectValue * OS::Core::newObjectValue(GCValue * prototype)
{
	GCObjectValue * res = new (malloc(sizeof(GCObjectValue) OS_DBG_FILEPOS)) GCObjectValue();
	res->prototype = prototype;
	res->type = OS_VALUE_TYPE_OBJECT;
	registerValue(res);
	return res;
}

OS::Core::GCArrayValue * OS::Core::newArrayValue(int initial_capacity)
{
	GCArrayValue * res = new (malloc(sizeof(GCArrayValue) OS_DBG_FILEPOS)) GCArrayValue();
	res->prototype = prototypes[PROTOTYPE_ARRAY];
	res->type = OS_VALUE_TYPE_ARRAY;
	if(initial_capacity > 0){
		allocator->vectorReserveCapacity(res->values, initial_capacity OS_DBG_FILEPOS);
	}
	registerValue(res);
	return res;
}

void OS::Core::pushValue(const Value& p_val)
{
	StackValues& stack_values = this->stack_values;
	if(stack_values.capacity < stack_values.count+1){
		Value val = p_val;
		reserveStackValues(stack_values.count+1);
		stack_values.buf[stack_values.count++] = val;
	}else{
		stack_values.buf[stack_values.count++] = p_val;
	}
}

void OS::Core::pushNull()
{
	pushValue(Value());
}

void OS::Core::pushStackValue(int offs)
{
	pushValue(getStackValue(offs));
}

void OS::Core::copyValue(int raw_from, int raw_to)
{
	OS_ASSERT(raw_from < stack_values.count);
	reserveStackValues(raw_to+1);
	stack_values.buf[raw_to] = stack_values.buf[raw_from];
}

void OS::Core::pushBool(bool val)
{
#if 1 // speed optimization
	StackValues& stack_values = this->stack_values;
	if(stack_values.capacity < stack_values.count+1){
		reserveStackValues(stack_values.count+1);
	}
	stack_values.buf[stack_values.count++] = val;
#else
	pushValue(val);
#endif
}

void OS::Core::pushNumber(OS_INT32 val)
{
#if 1 // speed optimization
	StackValues& stack_values = this->stack_values;
	if(stack_values.capacity < stack_values.count+1){
		reserveStackValues(stack_values.count+1);
	}
	stack_values.buf[stack_values.count++] = val;
#else
	pushValue(val);
#endif
}

void OS::Core::pushNumber(OS_INT64 val)
{
#if 1 // speed optimization
	StackValues& stack_values = this->stack_values;
	if(stack_values.capacity < stack_values.count+1){
		reserveStackValues(stack_values.count+1);
	}
	stack_values.buf[stack_values.count++] = val;
#else
	pushValue(val);
#endif
}

void OS::Core::pushNumber(float val)
{
#if 1 // speed optimization
	StackValues& stack_values = this->stack_values;
	if(stack_values.capacity < stack_values.count+1){
		reserveStackValues(stack_values.count+1);
	}
	stack_values.buf[stack_values.count++] = val;
#else
	pushValue(val);
#endif
}

void OS::Core::pushNumber(double val)
{
#if 1 // speed optimization
	StackValues& stack_values = this->stack_values;
	if(stack_values.capacity < stack_values.count+1){
		reserveStackValues(stack_values.count+1);
	}
	stack_values.buf[stack_values.count++] = val;
#else
	pushValue(val);
#endif
}

OS::Core::GCStringValue * OS::Core::pushStringValue(const String& val)
{
	pushValue(Value(val)); // newStringValue(val));
	return val.string;
}

OS::Core::GCStringValue * OS::Core::pushStringValue(const OS_CHAR * val)
{
	return pushValue(newStringValue(val));
}

OS::Core::GCStringValue * OS::Core::pushStringValue(const OS_CHAR * val, int len)
{
	return pushValue(newStringValue(val, len));
}

OS::Core::GCCFunctionValue * OS::Core::pushCFunctionValue(OS_CFunction func, void * user_param)
{
	return pushValue(newCFunctionValue(func, user_param));
}

OS::Core::GCCFunctionValue * OS::Core::pushCFunctionValue(OS_CFunction func, int closure_values, void * user_param)
{
	return pushValue(newCFunctionValue(func, closure_values, user_param));
}

OS::Core::GCUserdataValue * OS::Core::pushUserdataValue(int crc, int data_size, OS_UserdataDtor dtor, void * user_param)
{
	return pushValue(newUserdataValue(crc, data_size, dtor, user_param));
}

OS::Core::GCUserdataValue * OS::Core::pushUserPointerValue(int crc, void * data, OS_UserdataDtor dtor, void * user_param)
{
	return pushValue(newUserPointerValue(crc, data, dtor, user_param));
}

OS::Core::GCObjectValue * OS::Core::pushObjectValue()
{
	return pushValue(newObjectValue());
}

OS::Core::GCObjectValue * OS::Core::pushObjectValue(GCValue * prototype)
{
	return pushValue(newObjectValue(prototype));
}

OS::Core::GCArrayValue * OS::Core::pushArrayValue(int initial_capacity)
{
	return pushValue(newArrayValue(initial_capacity));
}

void OS::Core::pushTypeOf(const Value& val)
{
	switch(OS_VALUE_TYPE(val)){
		// case OS_VALUE_TYPE_NULL:
	case OS_VALUE_TYPE_BOOL:
		pushStringValue(strings->typeof_boolean);
		return;

	case OS_VALUE_TYPE_NUMBER:
		pushStringValue(strings->typeof_number);
		return;

	case OS_VALUE_TYPE_STRING:
		pushStringValue(strings->typeof_string);
		return;

	case OS_VALUE_TYPE_ARRAY:
		pushStringValue(strings->typeof_array);
		return;

	case OS_VALUE_TYPE_OBJECT:
		pushStringValue(strings->typeof_object);
		return;

	case OS_VALUE_TYPE_USERDATA:
	case OS_VALUE_TYPE_USERPTR:
		pushStringValue(strings->typeof_userdata);
		return;

	case OS_VALUE_TYPE_FUNCTION:
	case OS_VALUE_TYPE_CFUNCTION:
		pushStringValue(strings->typeof_function);
		return;
	}
	pushStringValue(strings->typeof_null);
}

bool OS::Core::pushNumberOf(const Value& val)
{
	if(OS_IS_VALUE_NUMBER(val)){
		pushValue(val);
		return true;
	}
	OS_NUMBER number;
	if(isValueNumber(val, &number)){
		pushNumber(number);
		return true;
	}
	pushNull();
	return false;
}

bool OS::Core::pushStringOf(const Value& val)
{
	if(OS_VALUE_TYPE(val) == OS_VALUE_TYPE_STRING){
		pushValue(val);
		return true;
	}
	String str(allocator);
	if(isValueString(val, &str)){
		pushStringValue(str);
		return true;
	}
	pushNull();
	return false;
}

bool OS::Core::pushValueOf(Value val)
{
	switch(OS_VALUE_TYPE(val)){
	case OS_VALUE_TYPE_NULL:
	case OS_VALUE_TYPE_NUMBER:
	case OS_VALUE_TYPE_BOOL:
	case OS_VALUE_TYPE_STRING:
		pushValue(val);
		return true;
	}

	OS_ASSERT(check_recursion && check_recursion->type == OS_VALUE_TYPE_OBJECT);
	if(++check_recursion->external_ref_count == 1 && check_recursion->table){
		clearTable(check_recursion->table);
	}
	setPropertyValue(check_recursion, val, Value(true), false);
	struct Finalizer { 
		Core * core; 
		~Finalizer()
		{ 
			if(--core->check_recursion->external_ref_count == 0 && core->check_recursion->table){
				core->clearTable(core->check_recursion->table);
			}
			if(core->check_recursion->gc_color == GC_WHITE){
				core->check_recursion->gc_color = GC_BLACK;
			}
		}
	} finalizer = {this}; (void)finalizer;

	bool prototype_enabled = true;
	Value func;
	if(getPropertyValue(func, OS_VALUE_VARIANT(val).value, PropertyIndex(strings->func_valueOf, PropertyIndex::KeepStringIndex()), prototype_enabled)
		&& func.isFunction())
	{
		pushValue(func);
		pushValue(val);
		call(0, 1);
		switch(OS_VALUE_TYPE(stack_values.lastElement())){
		case OS_VALUE_TYPE_NULL:
		case OS_VALUE_TYPE_NUMBER:
		case OS_VALUE_TYPE_BOOL:
		case OS_VALUE_TYPE_STRING:
			return true;
		}
		// TODO: warning
		pop();
	}
	pushNull();
	return false;
}

OS::Core::GCArrayValue * OS::Core::pushArrayOf(const Value& val)
{
	switch(OS_VALUE_TYPE(val)){
	case OS_VALUE_TYPE_ARRAY:
		pushValue(val);
		return OS_VALUE_VARIANT(val).arr;
	}
	pushNull();
	return NULL;
}

OS::Core::GCObjectValue * OS::Core::pushObjectOf(const Value& val)
{
	switch(OS_VALUE_TYPE(val)){
	case OS_VALUE_TYPE_OBJECT:
		pushValue(val);
		return OS_VALUE_VARIANT(val).object;
	
	/*
	case OS_VALUE_TYPE_USERDATA:
	case OS_VALUE_TYPE_USERPTR:
		pushValue(val);
		return OS_VALUE_VARIANT(val).userdata;
	*/
	}
	pushNull();
	return NULL;
}

OS::Core::GCUserdataValue * OS::Core::pushUserdataOf(const Value& val)
{
	switch(OS_VALUE_TYPE(val)){
	case OS_VALUE_TYPE_USERDATA:
	case OS_VALUE_TYPE_USERPTR:
		pushValue(val);
		return OS_VALUE_VARIANT(val).userdata;
	}
	pushNull();
	return NULL;
}

bool OS::Core::pushFunctionOf(const Value& val)
{
	switch(OS_VALUE_TYPE(val)){
	case OS_VALUE_TYPE_FUNCTION:
	case OS_VALUE_TYPE_CFUNCTION:
		pushValue(val);
		return true;
	}
	pushNull();
	return false;
}

void OS::Core::pushCloneValue(Value value)
{
	bool prototype_enabled = true;
	Value func;
	if(getPropertyValue(func, value, 
		PropertyIndex(strings->func_clone, PropertyIndex::KeepStringIndex()), prototype_enabled)
		&& func.isFunction())
	{
		pushValue(func);
		pushValue(value);
		call(0, 1);
		return;
	}
	pushNull();
}

void OS::Core::pushOpResultValue(OpcodeType opcode, const Value& value)
{
	struct Lib
	{
		static void pushObjectMethodOpcodeValue(Core * core, const String& method_name, Value value)
		{
			Value func;
			bool prototype_enabled = true;
			PropertyIndex index(method_name, PropertyIndex::KeepStringIndex());
			if(core->getPropertyValue(func, value, index, prototype_enabled) && func.isFunction()){
				core->pushValue(func);
				core->pushValue(value);
				core->call(0, 1);
				return;
			}
			core->error(OS_E_ERROR, String::format(core->allocator, OS_TEXT("Method %s is not found"), method_name.toChar()));
			core->pushNull();
		}
	};

	switch(OS_VALUE_TYPE(value)){
	// case OS_VALUE_TYPE_STRING:
	case OS_VALUE_TYPE_NULL:
	case OS_VALUE_TYPE_NUMBER:
	case OS_VALUE_TYPE_BOOL:
		switch(opcode){
		case OP_BIT_NOT:
			return pushNumber(~valueToInt(value));

		case OP_PLUS:
			return pushNumber(valueToNumber(value));

		case OP_NEG:
			return pushNumber(-valueToNumber(value));
		}
		OS_ASSERT(false);
		return pushNull();

	default:
		switch(opcode){
		case OP_BIT_NOT:
			return Lib::pushObjectMethodOpcodeValue(this, strings->__bitnot, value);

		case OP_PLUS:
			return Lib::pushObjectMethodOpcodeValue(this, strings->__plus, value);

		case OP_NEG:
			return Lib::pushObjectMethodOpcodeValue(this, strings->__neg, value);
		}
	}
	OS_ASSERT(false);
	pushNull();
}

bool OS::Core::isEqualExactly(const Value& left_value, const Value& right_value)
{
	if(OS_VALUE_TYPE(left_value) == OS_VALUE_TYPE(right_value)){ // && left_value->prototype == right_value->prototype){
		switch(OS_VALUE_TYPE(left_value)){
		case OS_VALUE_TYPE_NULL:
			return true;

		case OS_VALUE_TYPE_NUMBER:
			return OS_VALUE_NUMBER(left_value) == OS_VALUE_NUMBER(right_value);

		case OS_VALUE_TYPE_BOOL:
			return OS_VALUE_VARIANT(left_value).boolean == OS_VALUE_VARIANT(right_value).boolean;

		default:
			return OS_VALUE_VARIANT(left_value).value == OS_VALUE_VARIANT(right_value).value;

		case OS_VALUE_TYPE_WEAKREF:
			return OS_VALUE_VARIANT(left_value).value_id == OS_VALUE_VARIANT(right_value).value_id;
		}
	}
	return false;
}

void OS::Core::pushOpResultValue(OpcodeType opcode, const Value& left_value, const Value& right_value)
{
	struct Lib
	{
		static void pushObjectMethodOpcodeValue(Core * core, const String& method_name, const Value& left_value, const Value& right_value)
		{
			Value func;
			bool prototype_enabled = true;
			PropertyIndex index(method_name, PropertyIndex::KeepStringIndex());
			if(core->getPropertyValue(func, left_value, index, prototype_enabled) && func.isFunction()){
				core->pushValue(func);
				core->pushValue(left_value);
				core->pushValue(right_value);
				core->call(1, 1);
				return;
			}
			core->error(OS_E_ERROR, String::format(core->allocator, OS_TEXT("method %s is not found!"), method_name.toChar()));
			core->pushNull();
		}
	};

	OS_NUMBER right;
	int is_gc_left_value = 0;
	switch(OS_VALUE_TYPE(left_value)){
	case OS_VALUE_TYPE_NULL:
	case OS_VALUE_TYPE_NUMBER:
	case OS_VALUE_TYPE_BOOL:
		switch(OS_VALUE_TYPE(right_value)){
		case OS_VALUE_TYPE_NULL:
		case OS_VALUE_TYPE_NUMBER:
		case OS_VALUE_TYPE_BOOL:
			switch(opcode){
			case OP_COMPARE:
				return pushNumber(valueToNumber(left_value) - valueToNumber(right_value));

			case OP_LOGIC_PTR_EQ:
				return pushBool(isEqualExactly(left_value, right_value));

			case OP_LOGIC_EQ:
				return pushBool(valueToNumber(left_value) == valueToNumber(right_value));

			case OP_LOGIC_GE:
				return pushBool(valueToNumber(left_value) >= valueToNumber(right_value));

			case OP_LOGIC_GREATER:
				return pushBool(valueToNumber(left_value) > valueToNumber(right_value));

			case OP_BIT_AND:
				return pushNumber(valueToInt(left_value) & valueToInt(right_value));

			case OP_BIT_OR:
				return pushNumber(valueToInt(left_value) | valueToInt(right_value));

			case OP_BIT_XOR:
				return pushNumber(valueToInt(left_value) ^ valueToInt(right_value));

			case OP_ADD: // +
				return pushNumber(valueToNumber(left_value) + valueToNumber(right_value));

			case OP_SUB: // -
				return pushNumber(valueToNumber(left_value) - valueToNumber(right_value));

			case OP_MUL: // *
				return pushNumber(valueToNumber(left_value) * valueToNumber(right_value));

			case OP_DIV: // /
				right = valueToNumber(right_value);
				if(!right){
					errorDivisionByZero();
					return pushNull();
				}
				return pushNumber(valueToNumber(left_value) / right);

			case OP_MOD: // %
				right = valueToNumber(right_value);
				if(!right){
					errorDivisionByZero();
					return pushNull();
				}
				return pushNumber(OS_MATH_MOD_OPERATOR(valueToNumber(left_value), right));

			case OP_LSHIFT: // <<
				return pushNumber(valueToInt(left_value) << valueToInt(right_value));

			case OP_RSHIFT: // >>
				return pushNumber(valueToInt(left_value) >> valueToInt(right_value));

			case OP_POW: // **
				return pushNumber(OS_MATH_POW_OPERATOR(valueToNumber(left_value), valueToNumber(right_value)));
			}
			OS_ASSERT(false);
			return pushNull();
		}
		is_gc_left_value ^= 1;
		// no break

	default:
		is_gc_left_value ^= 1;
		switch(opcode){
		case OP_COMPARE:
			if(is_gc_left_value){
				switch(OS_VALUE_TYPE(right_value)){
				case OS_VALUE_TYPE_STRING:
				case OS_VALUE_TYPE_ARRAY:
				case OS_VALUE_TYPE_OBJECT:
				case OS_VALUE_TYPE_USERDATA:
				case OS_VALUE_TYPE_USERPTR:
				case OS_VALUE_TYPE_FUNCTION:
				case OS_VALUE_TYPE_CFUNCTION:
					if(OS_VALUE_VARIANT(left_value).value == OS_VALUE_VARIANT(right_value).value){
						return pushNumber((OS_NUMBER)0.0);
					}
				}
			}
			return Lib::pushObjectMethodOpcodeValue(this, strings->__cmp, left_value, right_value);

		case OP_LOGIC_PTR_EQ:
			return pushBool(isEqualExactly(left_value, right_value));

		case OP_LOGIC_EQ:
			if(is_gc_left_value){
				switch(OS_VALUE_TYPE(right_value)){
				case OS_VALUE_TYPE_STRING:
				case OS_VALUE_TYPE_ARRAY:
				case OS_VALUE_TYPE_OBJECT:
				case OS_VALUE_TYPE_USERDATA:
				case OS_VALUE_TYPE_USERPTR:
				case OS_VALUE_TYPE_FUNCTION:
				case OS_VALUE_TYPE_CFUNCTION:
					if(OS_VALUE_VARIANT(left_value).value == OS_VALUE_VARIANT(right_value).value){
						return pushBool(true);
					}
				}
			}
			Lib::pushObjectMethodOpcodeValue(this, strings->__cmp, left_value, right_value);
			stack_values.lastElement() = valueToNumber(stack_values.lastElement()) == (OS_NUMBER)0.0;
			return;

		case OP_LOGIC_GE:
			if(is_gc_left_value){
				switch(OS_VALUE_TYPE(right_value)){
				case OS_VALUE_TYPE_STRING:
				case OS_VALUE_TYPE_ARRAY:
				case OS_VALUE_TYPE_OBJECT:
				case OS_VALUE_TYPE_USERDATA:
				case OS_VALUE_TYPE_USERPTR:
				case OS_VALUE_TYPE_FUNCTION:
				case OS_VALUE_TYPE_CFUNCTION:
					if(OS_VALUE_VARIANT(left_value).value == OS_VALUE_VARIANT(right_value).value){
						return pushBool(true);
					}
				}
			}
			Lib::pushObjectMethodOpcodeValue(this, strings->__cmp, left_value, right_value);
			stack_values.lastElement() = valueToNumber(stack_values.lastElement()) >= (OS_NUMBER)0.0;
			return;

		case OP_LOGIC_GREATER:
			if(is_gc_left_value){
				switch(OS_VALUE_TYPE(right_value)){
				case OS_VALUE_TYPE_STRING:
				case OS_VALUE_TYPE_ARRAY:
				case OS_VALUE_TYPE_OBJECT:
				case OS_VALUE_TYPE_USERDATA:
				case OS_VALUE_TYPE_USERPTR:
				case OS_VALUE_TYPE_FUNCTION:
				case OS_VALUE_TYPE_CFUNCTION:
					if(OS_VALUE_VARIANT(left_value).value == OS_VALUE_VARIANT(right_value).value){
						return pushBool(false);
					}
				}
			}
			Lib::pushObjectMethodOpcodeValue(this, strings->__cmp, left_value, right_value);
			stack_values.lastElement() = valueToNumber(stack_values.lastElement()) > (OS_NUMBER)0.0;
			return;

		case OP_BIT_AND:
			return Lib::pushObjectMethodOpcodeValue(this, strings->__bitand, left_value, right_value);

		case OP_BIT_OR:
			return Lib::pushObjectMethodOpcodeValue(this, strings->__bitor, left_value, right_value);

		case OP_BIT_XOR:
			return Lib::pushObjectMethodOpcodeValue(this, strings->__bitxor, left_value, right_value);

		case OP_ADD: // +
			return Lib::pushObjectMethodOpcodeValue(this, strings->__add, left_value, right_value);

		case OP_SUB: // -
			return Lib::pushObjectMethodOpcodeValue(this, strings->__sub, left_value, right_value);

		case OP_MUL: // *
			return Lib::pushObjectMethodOpcodeValue(this, strings->__mul, left_value, right_value);

		case OP_DIV: // /
			return Lib::pushObjectMethodOpcodeValue(this, strings->__div, left_value, right_value);

		case OP_MOD: // %
			return Lib::pushObjectMethodOpcodeValue(this, strings->__mod, left_value, right_value);

		case OP_LSHIFT: // <<
			return Lib::pushObjectMethodOpcodeValue(this, strings->__lshift, left_value, right_value);

		case OP_RSHIFT: // >>
			return Lib::pushObjectMethodOpcodeValue(this, strings->__rshift, left_value, right_value);

		case OP_POW: // **
			return Lib::pushObjectMethodOpcodeValue(this, strings->__pow, left_value, right_value);
		}
	}
	OS_ASSERT(false);
	pushNull();
}

void OS::Core::setGlobalValue(const String& name, Value value, bool setter_enabled)
{
	setPropertyValue(global_vars, Core::PropertyIndex(name), value, setter_enabled);
}

void OS::Core::setGlobalValue(const OS_CHAR * name, Value value, bool setter_enabled)
{
	setGlobalValue(String(allocator, name), value, setter_enabled);
}

int OS::Core::getStackOffs(int offs)
{
	return offs <= 0 ? stack_values.count + offs + 1 : offs;
}

OS::Core::Value OS::Core::getStackValue(int offs)
{
	offs = offs <= 0 ? stack_values.count + offs : offs - 1;
	if(offs >= 0 && offs < stack_values.count){
		return stack_values.buf[offs];
	}
	if(offs == OS_REGISTER_GLOBALS - 1){
		return global_vars;
	}
	if(offs == OS_REGISTER_USERPOOL - 1){
		return user_pool;
	}
	// OS_ASSERT(false);
	return Value();
}

OS::Core::StackValues::StackValues()
{
	buf = NULL;
	capacity = 0;
	count = 0;
}

OS::Core::StackValues::~StackValues()
{
	OS_ASSERT(!buf && !capacity && !count);
}

void OS::Core::reserveStackValues(int new_capacity)
{
	if(stack_values.capacity < new_capacity){
		stack_values.capacity = (stack_values.capacity*2 + 16) & ~15;
		if(stack_values.capacity < new_capacity){
			stack_values.capacity = (new_capacity + 16) & ~15;
		}
		Value * new_buf = (Value*)malloc(sizeof(Value)*stack_values.capacity OS_DBG_FILEPOS);
		OS_MEMCPY(new_buf, stack_values.buf, sizeof(Value) * stack_values.count);
		free(stack_values.buf);
		stack_values.buf = new_buf;

		for(int i = 0; i < call_stack_funcs.count; i++){
			StackFunction * stack_func = &call_stack_funcs[i];
			OS_ASSERT(stack_func->locals_stack_pos >= 0);
			OS_ASSERT(stack_func->locals && stack_func->locals->is_stack_locals);
			stack_func->locals->values = stack_values.buf + stack_func->locals_stack_pos;
		}

		if(stack_func){
			stack_func_locals = stack_func->locals->values;
		}
	}
}

void OS::Core::removeStackValues(int offs, int count)
{
	if(count <= 0){
		OS_ASSERT(count == 0);
		return;
	}
	int start = offs <= 0 ? stack_values.count + offs : offs - 1;
	if(start < 0 || start >= stack_values.count){
		OS_ASSERT(false);
		return;
	}
	int end = start + count;
	if(end >= stack_values.count){
		OS_ASSERT(end == stack_values.count);
		stack_values.count = start;
	}else{
		count = stack_values.count - end;
		if(count == 1){
			stack_values.buf[start] = stack_values.buf[end];
		}else{
			OS_MEMMOVE(stack_values.buf + start, stack_values.buf + end, sizeof(Value) * count);
		}
		stack_values.count -= end - start;
	}
}

void OS::Core::removeStackValue(int offs)
{
	removeStackValues(offs, 1);
}

void OS::Core::removeAllStackValues()
{
	stack_values.count = 0;
}

void OS::Core::pop(int count)
{
	if(count >= stack_values.count){
		OS_ASSERT(count == stack_values.count);
		stack_values.count = 0;
	}else{
		stack_values.count -= count;
	}
}

void OS::Core::moveStackValues(int offs, int count, int new_offs)
{
	if(count <= 0){
		OS_ASSERT(count == 0);
		return;
	}
	offs = offs <= 0 ? stack_values.count + offs : offs - 1;
	if(offs < 0 || offs >= stack_values.count){
		OS_ASSERT(false);
		return;
	}
	int end = offs + count;
	if(end > stack_values.count){
		OS_ASSERT(false);
		return;
	}
	new_offs = new_offs <= 0 ? stack_values.count + new_offs : new_offs - 1;
	if(new_offs < 0 || new_offs >= stack_values.count){
		OS_ASSERT(false);
		return;
	}
	int new_end = new_offs + count;
	if(new_end > stack_values.count){
		OS_ASSERT(false);
		return;
	}
	Value * temp_values = (Value*)alloca(sizeof(Value) * count);
	OS_MEMCPY(temp_values, stack_values.buf + offs, sizeof(Value) * count);
	if(new_offs > offs){
		OS_MEMMOVE(stack_values.buf + offs, stack_values.buf + offs+count, sizeof(Value) * (new_offs - offs));
	}else{
		OS_MEMMOVE(stack_values.buf + new_offs+count, stack_values.buf + new_offs, sizeof(Value) * (offs - new_offs));
	}
	OS_MEMCPY(stack_values.buf + new_offs, temp_values, sizeof(Value) * count);
}

void OS::Core::moveStackValue(int offs, int new_offs)
{
	offs = offs <= 0 ? stack_values.count + offs : offs - 1;
	if(offs < 0 || offs >= stack_values.count){
		OS_ASSERT(false);
		return;
	}

	new_offs = new_offs <= 0 ? stack_values.count + new_offs : new_offs - 1;
	if(new_offs < 0 || new_offs >= stack_values.count){
		OS_ASSERT(false);
		return;
	}

	Value value = stack_values[offs];
	if(new_offs > offs){
		OS_MEMMOVE(stack_values.buf + offs, stack_values.buf + offs+1, sizeof(Value) * (new_offs - offs));
	}else{
		OS_MEMMOVE(stack_values.buf + new_offs+1, stack_values.buf + new_offs, sizeof(Value) * (offs - new_offs));
	}
	stack_values[new_offs] = value;
}

void OS::Core::insertValue(Value val, int offs)
{
	offs = offs <= 0 ? stack_values.count + offs : offs - 1;

	reserveStackValues(stack_values.count+1);
	stack_values.count++;

	if(offs < 0 || offs >= stack_values.count){
		OS_ASSERT(false);
		return;
	}
	int count = stack_values.count - offs - 1;
	if(count > 0){
		OS_MEMMOVE(stack_values.buf + offs+1, stack_values.buf + offs, sizeof(Value) * count);
	}
	stack_values[offs] = val;
}

void OS::pushNull()
{
	core->pushNull();
}

void OS::pushNumber(OS_INT32 val)
{
	core->pushNumber(val);
}

void OS::pushNumber(OS_INT64 val)
{
	core->pushNumber(val);
}

void OS::pushNumber(float val)
{
	core->pushNumber(val);
}

void OS::pushNumber(double val)
{
	core->pushNumber(val);
}

void OS::pushBool(bool val)
{
	core->pushBool(val);
}

void OS::pushString(const OS_CHAR * val)
{
	core->pushStringValue(val);
}

void OS::pushString(const OS_CHAR * val, int len)
{
	core->pushStringValue(val, len);
}

void OS::pushString(const Core::String& val)
{
	core->pushStringValue(val);
}

void OS::pushCFunction(OS_CFunction func, void * user_param)
{
	core->pushCFunctionValue(func, user_param);
}

void OS::pushCFunction(OS_CFunction func, int closure_values, void * user_param)
{
	core->pushCFunctionValue(func, closure_values, user_param);
}

void * OS::pushUserdata(int crc, int data_size, OS_UserdataDtor dtor, void * user_param)
{
	Core::GCUserdataValue * userdata = core->pushUserdataValue(crc, data_size, dtor, user_param);
	return userdata ? userdata->ptr : NULL;
}

void * OS::pushUserdata(int data_size, OS_UserdataDtor dtor, void * user_param)
{
	return pushUserdata(0, data_size, dtor, user_param);
}

void * OS::pushUserPointer(int crc, void * data, OS_UserdataDtor dtor, void * user_param)
{
	Core::GCUserdataValue * userdata = core->pushUserPointerValue(crc, data, dtor, user_param);
	return userdata ? userdata->ptr : NULL;
}

void * OS::pushUserPointer(void * data, OS_UserdataDtor dtor, void * user_param)
{
	return pushUserPointer(0, data, dtor, user_param);
}

void OS::newObject()
{
	core->pushObjectValue();
}

void OS::newArray(int initial_capacity)
{
	core->pushArrayValue(initial_capacity);
}

void OS::pushStackValue(int offs)
{
	core->pushStackValue(offs);
}

void OS::pushGlobals()
{
	pushStackValue(OS_REGISTER_GLOBALS);
}

void OS::pushUserPool()
{
	pushStackValue(OS_REGISTER_USERPOOL);
}

void OS::pushValueById(int id)
{
	core->pushValue(core->values.get(id));
}

void OS::retainValueById(int id)
{
	Core::GCValue * value = core->values.get(id);
	if(value){
		value->external_ref_count++;
	}
}

void OS::releaseValueById(int id)
{
	Core::GCValue * value = core->values.get(id);
	if(value){
		OS_ASSERT(value->external_ref_count > 0);
		if(!--value->external_ref_count && value->gc_color == Core::GC_WHITE){
			value->gc_color = Core::GC_BLACK;
		}
	}
}

void OS::clone(int offs)
{
	core->pushCloneValue(core->getStackValue(offs));
}

int OS::getStackSize()
{
	return core->stack_values.count;
}

int OS::getAbsoluteOffs(int offs)
{
	return core->getStackOffs(offs);
}

void OS::remove(int start_offs, int count)
{
	core->removeStackValues(start_offs, count);
}

void OS::pop(int count)
{
	core->pop(count);
}

void OS::removeAll()
{
	core->removeAllStackValues();
}

void OS::move(int start_offs, int count, int new_offs)
{
	core->moveStackValues(start_offs, count, new_offs);
}

void OS::move(int offs, int new_offs)
{
	core->moveStackValue(offs, new_offs);
}

bool OS::toBool(int offs)
{
	return core->valueToBool(core->getStackValue(offs));
}

bool OS::toBool(int offs, bool def)
{
	Core::Value value = core->getStackValue(offs);
	return value.isNull() ? def : core->valueToBool(value);
}

OS_NUMBER OS::toNumber(int offs, bool valueof_enabled)
{
	return core->valueToNumber(core->getStackValue(offs), valueof_enabled);
}

OS_NUMBER OS::toNumber(int offs, OS_NUMBER def, bool valueof_enabled)
{
	Core::Value value = core->getStackValue(offs);
	return value.isNull() ? def : core->valueToNumber(value, valueof_enabled);
}

float OS::toFloat(int offs, bool valueof_enabled)
{
	return (float)toNumber(offs, valueof_enabled);
}

float OS::toFloat(int offs, float def, bool valueof_enabled)
{
	return (float)toNumber(offs, (OS_NUMBER)def, valueof_enabled);
}

double OS::toDouble(int offs, bool valueof_enabled)
{
	return (double)toNumber(offs, valueof_enabled);
}

double OS::toDouble(int offs, double def, bool valueof_enabled)
{
	return (double)toNumber(offs, (OS_NUMBER)def, valueof_enabled);
}

int OS::toInt(int offs, bool valueof_enabled)
{
	return (int)toNumber(offs, valueof_enabled);
}

int OS::toInt(int offs, int def, bool valueof_enabled)
{
	return (int)toNumber(offs, (OS_NUMBER)def, valueof_enabled);
}

bool OS::isNumber(int offs, OS_NUMBER * out)
{
	return core->isValueNumber(core->getStackValue(offs), out);
}

OS::String OS::toString(int offs, bool valueof_enabled)
{
	return core->valueToStringOS(core->getStackValue(offs), valueof_enabled);
}

OS::String OS::toString(int offs, const String& def, bool valueof_enabled)
{
	Core::Value value = core->getStackValue(offs);
	return value.isNull() ? def : core->valueToStringOS(value, valueof_enabled);
}

bool OS::isString(int offs, String * out)
{
	return core->isValueStringOS(core->getStackValue(offs), out);
}

bool OS::popBool()
{
	struct Pop { OS * os; ~Pop(){ os->pop(); } } pop = {this}; (void)pop;
	return toBool(-1);
}

bool OS::popBool(bool def)
{
	struct Pop { OS * os; ~Pop(){ os->pop(); } } pop = {this}; (void)pop;
	return toBool(-1, def);
}

OS_NUMBER OS::popNumber(bool valueof_enabled)
{
	struct Pop { OS * os; ~Pop(){ os->pop(); } } pop = {this}; (void)pop;
	return toNumber(-1, valueof_enabled);
}

OS_NUMBER OS::popNumber(OS_NUMBER def, bool valueof_enabled)
{
	struct Pop { OS * os; ~Pop(){ os->pop(); } } pop = {this}; (void)pop;
	return toNumber(-1, def, valueof_enabled);
}

float OS::popFloat(bool valueof_enabled)
{
	struct Pop { OS * os; ~Pop(){ os->pop(); } } pop = {this}; (void)pop;
	return toFloat(-1, valueof_enabled);
}

float OS::popFloat(float def, bool valueof_enabled)
{
	struct Pop { OS * os; ~Pop(){ os->pop(); } } pop = {this}; (void)pop;
	return toFloat(-1, def, valueof_enabled);
}

double OS::popDouble(bool valueof_enabled)
{
	struct Pop { OS * os; ~Pop(){ os->pop(); } } pop = {this}; (void)pop;
	return toDouble(-1, valueof_enabled);
}

double OS::popDouble(double def, bool valueof_enabled)
{
	struct Pop { OS * os; ~Pop(){ os->pop(); } } pop = {this}; (void)pop;
	return toDouble(-1, def, valueof_enabled);
}

int OS::popInt(bool valueof_enabled)
{
	struct Pop { OS * os; ~Pop(){ os->pop(); } } pop = {this}; (void)pop;
	return toInt(-1, valueof_enabled);
}

int OS::popInt(int def, bool valueof_enabled)
{
	struct Pop { OS * os; ~Pop(){ os->pop(); } } pop = {this}; (void)pop;
	return toInt(-1, def, valueof_enabled);
}

OS::String OS::popString(bool valueof_enabled)
{
	struct Pop { OS * os; ~Pop(){ os->pop(); } } pop = {this}; (void)pop;
	return toString(-1, valueof_enabled);
}

OS::String OS::popString(const String& def, bool valueof_enabled)
{
	struct Pop { OS * os; ~Pop(){ os->pop(); } } pop = {this}; (void)pop;
	return toString(-1, def, valueof_enabled);
}

OS_EValueType OS::getType(int offs)
{
	Core::Value value = core->getStackValue(offs);
	return (OS_EValueType)OS_VALUE_TYPE(value);
}

OS_EValueType OS::getTypeById(int id)
{
	Core::GCValue * val = core->values.get(id);
	return val ? val->type : OS_VALUE_TYPE_NULL;
}

OS::String OS::getTypeStr(int offs)
{
	core->pushTypeOf(core->getStackValue(offs));
	return popString();
}

OS::String OS::getTypeStrById(int id)
{
	core->pushTypeOf(core->values.get(id));
	return popString();
}

bool OS::isType(OS_EValueType type, int offs)
{
	return OS_VALUE_TYPE(core->getStackValue(offs)) == type;
}

bool OS::isNull(int offs)
{
	return isType(OS_VALUE_TYPE_NULL, offs);
}

bool OS::isObject(int offs)
{
	switch(OS_VALUE_TYPE(core->getStackValue(offs))){
	case OS_VALUE_TYPE_OBJECT:
		// case OS_VALUE_TYPE_ARRAY:
		return true;
	}
	return false;
}

bool OS::isUserdata(int crc, int offs, int prototype_crc)
{
	Core::Value val = core->getStackValue(offs);
	switch(OS_VALUE_TYPE(val)){
	case OS_VALUE_TYPE_USERDATA:
	case OS_VALUE_TYPE_USERPTR:
		if(OS_VALUE_VARIANT(val).userdata->crc == crc){
			return true;
		}
		if(prototype_crc && OS_VALUE_VARIANT(val).userdata->prototype 
			&& core->isValuePrototypeOfUserdata(OS_VALUE_VARIANT(val).userdata->prototype, prototype_crc))
		{
			return true;
		}
	}
	return false;
}

void * OS::toUserdata(int crc, int offs, int prototype_crc)
{
	Core::Value val = core->getStackValue(offs);
	switch(OS_VALUE_TYPE(val)){
	case OS_VALUE_TYPE_USERDATA:
	case OS_VALUE_TYPE_USERPTR:
		if(OS_VALUE_VARIANT(val).userdata->crc == crc){
			return OS_VALUE_VARIANT(val).userdata->ptr;
		}
		if(prototype_crc && OS_VALUE_VARIANT(val).userdata->prototype 
			&& core->isValuePrototypeOfUserdata(OS_VALUE_VARIANT(val).userdata->prototype, prototype_crc))
		{
			return OS_VALUE_VARIANT(val).userdata->ptr;
		}
	}
	return NULL;
}

void OS::clearUserdata(int crc, int offs, int prototype_crc)
{
	Core::Value val = core->getStackValue(offs);
	switch(OS_VALUE_TYPE(val)){
	case OS_VALUE_TYPE_USERDATA:
	case OS_VALUE_TYPE_USERPTR:
		if(OS_VALUE_VARIANT(val).userdata->crc == crc){ // && val.v.userdata->ptr){
			core->triggerValueDestructor(OS_VALUE_VARIANT(val).value);
			core->clearValue(OS_VALUE_VARIANT(val).value);
			// val.v.userdata->ptr = NULL;
			return;
		}
		if(prototype_crc && OS_VALUE_VARIANT(val).userdata->prototype 
			&& core->isValuePrototypeOfUserdata(OS_VALUE_VARIANT(val).userdata->prototype, prototype_crc))
		{
			core->triggerValueDestructor(OS_VALUE_VARIANT(val).value);
			core->clearValue(OS_VALUE_VARIANT(val).value);
			return;
		}
	}
}

bool OS::isArray(int offs)
{
	return isType(OS_VALUE_TYPE_ARRAY, offs);
}

bool OS::isFunction(int offs)
{
	return core->getStackValue(offs).isFunction();
}

bool OS::Core::isValuePrototypeOf(GCValue * val, GCValue * prototype_val)
{
	while(val != prototype_val){
		val = val->prototype;
		if(!val){
			return false;
		}
	}
	return true;
}

bool OS::Core::isValuePrototypeOfUserdata(GCValue * val, int prototype_crc)
{
	for(int value_crc;;){
		switch(val->type){
		case OS_VALUE_TYPE_USERDATA:
		case OS_VALUE_TYPE_USERPTR:
			OS_ASSERT(dynamic_cast<GCUserdataValue*>(val));
			value_crc = ((GCUserdataValue*)val)->crc; // make crc visible in debugger
			if(value_crc == prototype_crc){
				return true;
			}
		}
		val = val->prototype;
		if(!val){
			return false;
		}
	}
	return false; // shut up compiler
}

bool OS::Core::isValueInstanceOf(GCValue * val, GCValue * prototype_val)
{
	return val->prototype ? isValuePrototypeOf(val->prototype, prototype_val) : false;
}

bool OS::Core::isValuePrototypeOf(const Value& val, const Value& prototype_val)
{
	GCValue * object = val.getGCValue();
	GCValue * proto = prototype_val.getGCValue();
	return object && proto && isValuePrototypeOf(object, proto);
}

bool OS::Core::isValueInstanceOf(const Value& val, const Value& prototype_val)
{
	GCValue * object = val.getGCValue();
	GCValue * proto = prototype_val.getGCValue();
	return object && proto && isValueInstanceOf(object, proto);
}

bool OS::isPrototypeOf(int value_offs, int prototype_offs)
{
	return core->isValuePrototypeOf(core->getStackValue(value_offs), core->getStackValue(prototype_offs));
}

bool OS::is(int value_offs, int prototype_offs)
{
	return core->isValueInstanceOf(core->getStackValue(value_offs), core->getStackValue(prototype_offs));
}

void OS::setProperty(bool setter_enabled)
{
	if(core->stack_values.count >= 3){
		Core::Value object = core->stack_values[core->stack_values.count - 3];
		Core::Value index = core->stack_values[core->stack_values.count - 2];
		Core::Value value = core->stack_values[core->stack_values.count - 1];
		core->setPropertyValue(object, Core::PropertyIndex(index), value, setter_enabled);
		pop(3);
	}else{
		// error
		pop(3);
	}
}

void OS::setProperty(const OS_CHAR * name, bool setter_enabled)
{
	setProperty(Core::String(this, name), setter_enabled);
}

void OS::setProperty(const Core::String& name, bool setter_enabled)
{
	if(core->stack_values.count >= 2){
		Core::Value object = core->stack_values[core->stack_values.count - 2];
		Core::Value value = core->stack_values[core->stack_values.count - 1];
		core->setPropertyValue(object, Core::PropertyIndex(name), value, setter_enabled);
		pop(2);
	}else{
		// error
		pop(2);
	}
}

void OS::setProperty(int offs, const OS_CHAR * name, bool setter_enabled)
{
	setProperty(offs, Core::String(this, name), setter_enabled);
}

void OS::setProperty(int offs, const Core::String& name, bool setter_enabled)
{
	if(core->stack_values.count >= 1){
		Core::Value object = core->getStackValue(offs);
		Core::Value value = core->stack_values[core->stack_values.count - 1];
		core->setPropertyValue(object, Core::PropertyIndex(name), value, setter_enabled);
		pop();
	}else{
		// error
		pop();
	}
}

void OS::addProperty(bool setter_enabled)
{
	Core::Value value = core->getStackValue(-2);
	switch(OS_VALUE_TYPE(value)){
	case OS_VALUE_TYPE_ARRAY:
		core->insertValue(OS_VALUE_VARIANT(value).arr->values.count, -1);
		break;

	case OS_VALUE_TYPE_OBJECT:
	case OS_VALUE_TYPE_USERDATA:
	case OS_VALUE_TYPE_USERPTR:
	case OS_VALUE_TYPE_FUNCTION:
	case OS_VALUE_TYPE_CFUNCTION:
		core->insertValue(OS_VALUE_VARIANT(value).object->table ? OS_VALUE_VARIANT(value).object->table->next_index : 0, -1);
		break;
	}
	setProperty(setter_enabled);
}

void OS::deleteProperty(bool del_enabled)
{
	core->deleteValueProperty(core->getStackValue(-2), core->getStackValue(-1), del_enabled, false);
	pop(2);
}

void OS::deleteProperty(const OS_CHAR * name, bool del_enabled)
{
	deleteProperty(Core::String(this, name), del_enabled);
}

void OS::deleteProperty(const Core::String& name, bool del_enabled)
{
	pushString(name);
	deleteProperty(del_enabled);
}

void OS::getPrototype()
{
	if(core->stack_values.count >= 1){
		core->pushPrototype(core->stack_values.lastElement());
		remove(-2);
	}else{
		pop();
		pushNull();
	}
}

void OS::setPrototype()
{
	setPrototype(0);
}

void OS::setPrototype(int userdata_crc)
{
	if(core->stack_values.count >= 2){
		Core::Value value = core->stack_values[core->stack_values.count - 2];
		Core::Value proto = core->stack_values[core->stack_values.count - 1];
		core->setPrototype(value, proto, userdata_crc);
	}
	pop(2);
}

int OS::getValueId(int offs)
{
	Core::Value val = core->getStackValue(offs);
	switch(OS_VALUE_TYPE(val)){
	case OS_VALUE_TYPE_STRING:
	case OS_VALUE_TYPE_ARRAY:
	case OS_VALUE_TYPE_OBJECT:
	case OS_VALUE_TYPE_USERDATA:
	case OS_VALUE_TYPE_USERPTR:
	case OS_VALUE_TYPE_FUNCTION:
	case OS_VALUE_TYPE_CFUNCTION:
		return OS_VALUE_VARIANT(val).value->value_id;

	case OS_VALUE_TYPE_WEAKREF:
		return OS_VALUE_VARIANT(val).value_id;
	}
	return 0;
}

bool OS::Core::getPropertyValue(Value& result, Table * table, const PropertyIndex& index)
{
#if defined OS_DEBUG && defined OS_WARN_NULL_INDEX
	if(table != check_recursion->table && index.index.type == OS_VALUE_TYPE_NULL){
		error(OS_E_WARNING, OS_TEXT("object get null index"));
	}
#endif
	if(table){
		Property * prop = table->get(index);
		if(prop){
			result = prop->value;
			return true;
		}
	}
	return false;
}

bool OS::Core::getPropertyValue(Value& result, GCValue * table_value, const PropertyIndex& index, bool prototype_enabled)
{
#if defined OS_DEBUG && defined OS_WARN_NULL_INDEX
	if(table_value != check_recursion && index.index.type == OS_VALUE_TYPE_NULL){
		error(OS_E_WARNING, OS_TEXT("object get null index"));
	}
#endif

	if(table_value->type == OS_VALUE_TYPE_ARRAY && OS_IS_VALUE_NUMBER(index.index)){
		OS_ASSERT(dynamic_cast<GCArrayValue*>(table_value));
		int i = (int)OS_VALUE_NUMBER(index.index);
		if((i >= 0 || (i += ((GCArrayValue*)table_value)->values.count) >= 0) && i < ((GCArrayValue*)table_value)->values.count){
			result = ((GCArrayValue*)table_value)->values[i];
		}else{
			result = Value();
		}
		return true;
	}
	Property * prop = NULL;
	Table * table = table_value->table;
	if(table && (prop = table->get(index))){
		result = prop->value;
		return true;
	}
	if(prototype_enabled){
		GCValue * cur_value = table_value;
		while(cur_value->prototype){
			cur_value = cur_value->prototype;
			Table * cur_table = cur_value->table;
			if(cur_table && (prop = cur_table->get(index))){
				result = prop->value;
				return true;
			}
		}
	}
	if(OS_VALUE_TYPE(index.index) == OS_VALUE_TYPE_STRING && strings->syntax_prototype == OS_VALUE_VARIANT(index.index).string){
		result = table_value->prototype;
		return true;
	}
	if(table_value->type == OS_VALUE_TYPE_ARRAY){
		OS_ASSERT(dynamic_cast<GCArrayValue*>(table_value));
		OS_NUMBER number;
		if(isValueNumber(index.index, &number)){
			int i = (int)number;
			if((i >= 0 || (i += ((GCArrayValue*)table_value)->values.count) >= 0) && i < ((GCArrayValue*)table_value)->values.count){
				result = ((GCArrayValue*)table_value)->values[i];
			}else{
				result = Value();
			}
			return true;
		}
	}
	return false;
}

bool OS::Core::getPropertyValue(Value& result, const Value& table_value, const PropertyIndex& index, bool prototype_enabled)
{
	switch(OS_VALUE_TYPE(table_value)){
	case OS_VALUE_TYPE_NULL:
		return false;

	case OS_VALUE_TYPE_BOOL:
		return prototype_enabled && getPropertyValue(result, prototypes[PROTOTYPE_BOOL], index, prototype_enabled);

	case OS_VALUE_TYPE_NUMBER:
		return prototype_enabled && getPropertyValue(result, prototypes[PROTOTYPE_NUMBER], index, prototype_enabled);

	case OS_VALUE_TYPE_STRING:
		// return prototype_enabled && getPropertyValue(result, prototypes[PROTOTYPE_STRING], index, prototype_enabled);

	case OS_VALUE_TYPE_ARRAY:
	case OS_VALUE_TYPE_OBJECT:
	case OS_VALUE_TYPE_USERDATA:
	case OS_VALUE_TYPE_USERPTR:
	case OS_VALUE_TYPE_FUNCTION:
	case OS_VALUE_TYPE_CFUNCTION:
		return getPropertyValue(result, OS_VALUE_VARIANT(table_value).value, index, prototype_enabled);
	}
	return false;
}

bool OS::Core::hasProperty(GCValue * table_value, const PropertyIndex& index, bool getter_enabled, bool prototype_enabled)
{
	Value value;
	if(getPropertyValue(value, table_value, index, prototype_enabled)){
		return true; // OS_VALUE_TYPE(value) != OS_VALUE_TYPE_NULL;
	}
	if(!getter_enabled || hasSpecialPrefix(index.index)){
		return false;
	}
	if(OS_VALUE_TYPE(index.index) == OS_VALUE_TYPE_STRING){
		const void * buf2 = OS_VALUE_VARIANT(index.index).string->toChar();
		int size2 = OS_VALUE_VARIANT(index.index).string->getDataSize();
		{
			const void * buf1 = strings->__issetAt.toChar();
			int size1 = strings->__issetAt.getDataSize();
			GCStringValue * isset_name = newStringValue(buf1, size1, buf2, size2);
			if(getPropertyValue(value, table_value, PropertyIndex(isset_name, PropertyIndex::KeepStringIndex()), prototype_enabled)
				&& value.isFunction())
			{
				pushValue(value);
				pushValue(table_value);
				call(0, 1);
				return allocator->popBool();
			}
		}
		{
			const void * buf1 = strings->__getAt.toChar();
			int size1 = strings->__getAt.getDataSize();
			GCStringValue * getter_name = newStringValue(buf1, size1, buf2, size2);
			if(getPropertyValue(value, table_value, PropertyIndex(getter_name, PropertyIndex::KeepStringIndex()), prototype_enabled)){
				return true;
			}
		}
	}
	if(getPropertyValue(value, table_value, PropertyIndex(strings->__isset, PropertyIndex::KeepStringIndex()), prototype_enabled)
		&& value.isFunction())
	{
		pushValue(value);
		pushValue(table_value);
		pushValue(index.index);
		call(1, 1);
		return allocator->popBool();
	}
	return false;
}

void OS::Core::pushPropertyValue(GCValue * table_value, const PropertyIndex& index, bool getter_enabled, bool prototype_enabled)
{
	GCValue * self = table_value;
	for(;;){
		Value value;
		if(getPropertyValue(value, table_value, index, prototype_enabled)){
			return pushValue(value);
		}
		if(getter_enabled && !hasSpecialPrefix(index.index)){
			if(OS_VALUE_TYPE(index.index) == OS_VALUE_TYPE_STRING){
				const void * buf1 = strings->__getAt.toChar();
				int size1 = strings->__getAt.getDataSize();
				const void * buf2 = OS_VALUE_VARIANT(index.index).string->toChar();
				int size2 = OS_VALUE_VARIANT(index.index).string->getDataSize();
				GCStringValue * getter_name = newStringValue(buf1, size1, buf2, size2);
				if(getPropertyValue(value, table_value, PropertyIndex(getter_name, PropertyIndex::KeepStringIndex()), prototype_enabled)){
					pushValue(value);
					pushValue(self);
					call(0, 1);
					return;
				}
			}
			if(getPropertyValue(value, table_value, PropertyIndex(strings->__get, PropertyIndex::KeepStringIndex()), prototype_enabled)){
				if(OS_VALUE_TYPE(value) == OS_VALUE_TYPE_OBJECT){
					table_value = OS_VALUE_VARIANT(value).value;
					continue;
				}
				pushValue(value);
				pushValue(self);
				pushValue(index.index);
				call(1, 1);
				return;
			}
		}
		break;
	}
	return pushNull();
}

void OS::Core::pushPropertyValueForPrimitive(Value self, const PropertyIndex& index, bool getter_enabled, bool prototype_enabled)
{
	GCValue * proto;
	switch(OS_VALUE_TYPE(self)){
	case OS_VALUE_TYPE_NUMBER:
		proto = prototypes[PROTOTYPE_NUMBER];
		break;

	case OS_VALUE_TYPE_BOOL:
		proto = prototypes[PROTOTYPE_BOOL];
		break;

	default:
		pushNull();
		return;
	}
	for(;;){
		OS_ASSERT(proto);
		Value value;
		if(prototype_enabled && getPropertyValue(value, proto, index, prototype_enabled)){
			return pushValue(value);
		}
		if(getter_enabled && !hasSpecialPrefix(index.index)){
			if(OS_VALUE_TYPE(index.index) == OS_VALUE_TYPE_STRING){
				const void * buf1 = strings->__getAt.toChar();
				int size1 = strings->__getAt.getDataSize();
				const void * buf2 = OS_VALUE_VARIANT(index.index).string->toChar();
				int size2 = OS_VALUE_VARIANT(index.index).string->getDataSize();
				GCStringValue * getter_name = newStringValue(buf1, size1, buf2, size2);
				if(getPropertyValue(value, proto, PropertyIndex(getter_name, PropertyIndex::KeepStringIndex()), prototype_enabled)){
					pushValue(value);
					pushValue(self);
					call(0, 1);
					return;
				}
			}
			if(getPropertyValue(value, proto, PropertyIndex(strings->__get, PropertyIndex::KeepStringIndex()), prototype_enabled)){
				if(OS_VALUE_TYPE(value) == OS_VALUE_TYPE_OBJECT){
					proto = OS_VALUE_VARIANT(value).value;
					continue;
				}
				pushValue(value);
				pushValue(self);
				pushValue(index.index);
				call(1, 1);
				return;
			}
		}
		break;
	}
	return pushNull();
}

void OS::Core::pushPropertyValue(Value table_value, const PropertyIndex& index, bool getter_enabled, bool prototype_enabled)
{
	switch(OS_VALUE_TYPE(table_value)){
	case OS_VALUE_TYPE_NULL:
		break;

	case OS_VALUE_TYPE_BOOL:
		/* if(prototype_enabled){
			return pushPropertyValue(prototypes[PROTOTYPE_BOOL], index, getter_enabled, prototype_enabled);
		}
		break; */

	case OS_VALUE_TYPE_NUMBER:
		/* if(prototype_enabled){
			return pushPropertyValue(prototypes[PROTOTYPE_NUMBER], index, getter_enabled, prototype_enabled);
		}
		break; */
		return pushPropertyValueForPrimitive(table_value, index, getter_enabled, prototype_enabled);

	case OS_VALUE_TYPE_STRING:
		/* if(prototype_enabled){
			return pushPropertyValue(prototypes[PROTOTYPE_STRING], index, getter_enabled, prototype_enabled);
		}
		break; */

	case OS_VALUE_TYPE_ARRAY:
	case OS_VALUE_TYPE_OBJECT:
	case OS_VALUE_TYPE_USERDATA:
	case OS_VALUE_TYPE_USERPTR:
	case OS_VALUE_TYPE_FUNCTION:
	case OS_VALUE_TYPE_CFUNCTION:
		return pushPropertyValue(OS_VALUE_VARIANT(table_value).value, index, getter_enabled, prototype_enabled);
	}
	pushNull();
}

void OS::getProperty(bool getter_enabled, bool prototype_enabled)
{
	if(core->stack_values.count >= 2){
		Core::Value object = core->stack_values[core->stack_values.count - 2];
		Core::Value index = core->stack_values[core->stack_values.count - 1];
		core->pushPropertyValue(object, Core::PropertyIndex(index), getter_enabled, prototype_enabled);
		core->removeStackValues(-3, 2);
	}else{
		// error
		pop(2);
		pushNull();
	}
}

void OS::getProperty(const OS_CHAR * name, bool getter_enabled, bool prototype_enabled)
{
	getProperty(Core::String(this, name), getter_enabled, prototype_enabled);
}

void OS::getProperty(const Core::String& name, bool getter_enabled, bool prototype_enabled)
{
	pushString(name);
	getProperty(getter_enabled, prototype_enabled);
}

void OS::getProperty(int offs, const OS_CHAR * name, bool getter_enabled, bool prototype_enabled)
{
	getProperty(offs, Core::String(this, name), getter_enabled, prototype_enabled);
}

void OS::getProperty(int offs, const Core::String& name, bool getter_enabled, bool prototype_enabled)
{
	pushStackValue(offs);
	getProperty(name, getter_enabled, prototype_enabled);
}

void OS::Core::releaseLocals(Locals * locals)
{
	if(--locals->ref_count > 0){
		return;
	}
	deleteLocals(locals);
}

void OS::Core::deleteLocals(Locals * locals)
{
	locals->prog->release();
	if(locals->num_parents > 0){
		releaseLocals(locals->getParent(0));
	}
	if(!locals->is_stack_locals){
		free(locals->values);
	}
	free(locals);
}

void OS::Core::clearStackFunction(StackFunction * stack_func)
{
	if(--stack_func->locals->ref_count > 0){
		int count = stack_func->locals->func_decl->num_locals; // >opcode_stack_pos - stack_func->locals_stack_pos;
		if(count > 0){
			Value * locals = (Value*)malloc(sizeof(Value) * count OS_DBG_FILEPOS);
			OS_MEMCPY(locals, stack_func->locals->values, sizeof(Value) * count);
			stack_func->locals->values = locals;
		}else{
			stack_func->locals->values = NULL;
		}
		stack_func->locals->is_stack_locals = false;
	}else{
		deleteLocals(stack_func->locals);
	}
}

void OS::Core::reloadStackFunctionCache()
{
	if(call_stack_funcs.count > 0){
		stack_func = &call_stack_funcs.lastElement();
		FunctionDecl * func_decl = stack_func->func->func_decl;
		stack_func_locals = stack_func->locals->values;
		stack_func_env_index = func_decl->num_params + POST_VAR_ENV;
		stack_func_prog_values = stack_func->func->prog->const_values;
	}else{
		stack_func = NULL;
		stack_func_locals = NULL;
		stack_func_env_index = 0;
		stack_func_prog_values = NULL;
	}
}

int OS::Core::execute()
{
#ifdef OS_DEBUG
	allocator->checkNativeStackUsage(OS_TEXT("OS::Core::execute"));
#endif
	StackFunction * stack_func;
	int a, b, c, up_count, res, ret_stack_funcs = call_stack_funcs.count-1;
	Program * prog;
	Value * left_value, * right_value, value;
#ifdef OS_INFINITE_LOOP_OPCODES
	for(int opcodes_executed = 0;; opcodes_executed++){
#else
	for(;;){
#endif
		OS_ASSERT(this->stack_values.count >= this->stack_func->locals_stack_pos + this->stack_func->func->func_decl->stack_size);
		if(terminated
#ifdef OS_INFINITE_LOOP_OPCODES
			|| opcodes_executed >= OS_INFINITE_LOOP_OPCODES
#endif
			)
		{
			break;
		}
		OS_ASSERT(this->stack_func->opcodes >= this->stack_func->func->prog->opcodes.buf + this->stack_func->func->func_decl->opcodes_pos);
		OS_ASSERT(this->stack_func->opcodes < this->stack_func->func->prog->opcodes.buf + this->stack_func->func->func_decl->opcodes_pos + this->stack_func->func->func_decl->opcodes_size);
		Instruction instruction = *(stack_func = this->stack_func)->opcodes++;
#ifdef OS_DEBUG
		OpcodeType opcode = (OpcodeType)OS_GET_OPCODE_WITH_CC(instruction);
#else
		unsigned int opcode = OS_GET_OPCODE_WITH_CC(instruction);
#endif
#if 0
		{
			prog = stack_func->func->prog;
			int opcode_pos = stack_func->opcodes - prog->opcodes.buf;
			Program::DebugInfoItem * debug_info = prog->getDebugInfo(opcode_pos);
			int i = 0;
		}
#endif
		OS_PROFILE_BEGIN_OPCODE(opcode);
		switch(opcode){
		// case 0: case 1: case 2: case 3:
		default:
			error(OS_E_ERROR, "Unknown opcode, program is corrupted!!!");
			allocator->setTerminated();
			break;

		OS_CASE_OPCODE_ALL(OP_LOGIC_BOOL):
			{
				a = OS_GETARG_A(instruction);
				OS_ASSERT(a >= 0 && a < stack_func->func->func_decl->stack_size);
				// b = GETARG_B(instruction); // inverse
				// c = GETARG_C(instruction); // if opcode
				res = (int)valueToBool(stack_func_locals[a]) ^ OS_GETARG_B(instruction);
				if(!(OS_GETARG_C(instruction))){
					stack_func_locals[a] = res != 0;
					break;
				}
#ifdef OS_DEBUG
				instruction = stack_func->opcodes[0];
				opcode = (OpcodeType)OS_GET_OPCODE_TYPE(instruction);
				OS_ASSERT(opcode == OP_JUMP);
#endif
				if(res){
					stack_func->opcodes++;
				}else{
					instruction = stack_func->opcodes[0];
					int b = OS_GETARG_sBx(instruction);
					stack_func->opcodes += b + 1;
				}
				break;
			}

		OS_CASE_OPCODE_ALL(OP_LOGIC_PTR_EQ):
			{
				a = OS_GETARG_A(instruction);
				OS_ASSERT(a >= 0 && a+1 < stack_func->func->func_decl->stack_size);
				
				// left_value = & ARG_B_VALUE(a);
				// b = a + 1;
				// right_value = & ARG_C_VALUE(b);
				left_value = &stack_func_locals[a];
				right_value = left_value + 1;

				// b = GETARG_B(instruction); // inverse
				// c = GETARG_C(instruction); // if opcode
				res = (int)isEqualExactly(*left_value, *right_value) ^ OS_GETARG_B(instruction);
				if(!(OS_GETARG_C(instruction))){
					stack_func_locals[a] = res != 0;
					break;
				}
#ifdef OS_DEBUG
				instruction = stack_func->opcodes[0];
				opcode = (OpcodeType)OS_GET_OPCODE_TYPE(instruction);
				OS_ASSERT(opcode == OP_JUMP);
#endif
				if(res){
					stack_func->opcodes++;
				}else{
					instruction = stack_func->opcodes[0];
					int b = OS_GETARG_sBx(instruction);
					stack_func->opcodes += b + 1;
				}
				break;
			}

		OS_CASE_OPCODE_ALL(OP_LOGIC_EQ):
			{
				a = OS_GETARG_A(instruction);
				OS_ASSERT(a >= 0 && a+1 < stack_func->func->func_decl->stack_size);
				
				// left_value = & ARG_B_VALUE(a);
				// b = a + 1;
				// right_value = & ARG_C_VALUE(b);
				left_value = &stack_func_locals[a];
				right_value = left_value + 1;

				int b = OS_GETARG_B(instruction); // inverse
				// c = GETARG_C(instruction); // if opcode

				if(OS_IS_VALUE_NUMBER(*left_value) && OS_IS_VALUE_NUMBER(*right_value)){
					res = (int)(OS_VALUE_NUMBER(*left_value) == OS_VALUE_NUMBER(*right_value)) ^ b;
				}else{
					pushOpResultValue((OpcodeType)OS_TO_OPCODE_TYPE(opcode), *left_value, *right_value);
					OS_ASSERT(OS_VALUE_TYPE(stack_values.lastElement()) == OS_VALUE_TYPE_BOOL);
					res = OS_VALUE_VARIANT(stack_values.buf[--stack_values.count]).boolean ^ b;
				}
				if(!(OS_GETARG_C(instruction))){
					stack_func_locals[a] = res != 0;
					break;
				}
#ifdef OS_DEBUG
				instruction = stack_func->opcodes[0];
				opcode = (OpcodeType)OS_GET_OPCODE_TYPE(instruction);
				OS_ASSERT(opcode == OP_JUMP);
#endif
				if(res){
					stack_func->opcodes++;
				}else{
					instruction = stack_func->opcodes[0];
					b = OS_GETARG_sBx(instruction);
					stack_func->opcodes += b + 1;
				}
				break;
			}

		OS_CASE_OPCODE_ALL(OP_LOGIC_GREATER):
			{
				a = OS_GETARG_A(instruction);
				OS_ASSERT(a >= 0 && a+1 < stack_func->func->func_decl->stack_size);
				
				// left_value = & ARG_B_VALUE(a);
				// b = a + 1;
				// right_value = & ARG_C_VALUE(b);
				left_value = &stack_func_locals[a];
				right_value = left_value + 1;

				int b = OS_GETARG_B(instruction); // inverse
				// c = GETARG_C(instruction); // if opcode

				if(OS_IS_VALUE_NUMBER(*left_value) && OS_IS_VALUE_NUMBER(*right_value)){
					res = (int)(OS_VALUE_NUMBER(*left_value) > OS_VALUE_NUMBER(*right_value)) ^ b;
				}else{
					pushOpResultValue((OpcodeType)OS_TO_OPCODE_TYPE(opcode), *left_value, *right_value);
					OS_ASSERT(OS_VALUE_TYPE(stack_values.lastElement()) == OS_VALUE_TYPE_BOOL);
					res = OS_VALUE_VARIANT(stack_values.buf[--stack_values.count]).boolean ^ b;
				}
				if(!(OS_GETARG_C(instruction))){
					stack_func_locals[a] = res != 0;
					break;
				}
#ifdef OS_DEBUG
				instruction = stack_func->opcodes[0];
				opcode = (OpcodeType)OS_GET_OPCODE_TYPE(instruction);
				OS_ASSERT(opcode == OP_JUMP);
#endif
				if(res){
					stack_func->opcodes++;
				}else{
					instruction = stack_func->opcodes[0];
					b = OS_GETARG_sBx(instruction);
					stack_func->opcodes += b + 1;
				}
				break;
			}

		OS_CASE_OPCODE_ALL(OP_LOGIC_GE):
			{
				a = OS_GETARG_A(instruction);
				OS_ASSERT(a >= 0 && a+1 < stack_func->func->func_decl->stack_size);
				
				// left_value = & ARG_B_VALUE(a);
				// b = a + 1;
				// right_value = & ARG_C_VALUE(b);
				left_value = &stack_func_locals[a];
				right_value = left_value + 1;

				int b = OS_GETARG_B(instruction); // inverse
				// c = GETARG_C(instruction); // if opcode

				if(OS_IS_VALUE_NUMBER(*left_value) && OS_IS_VALUE_NUMBER(*right_value)){
					res = (int)(OS_VALUE_NUMBER(*left_value) >= OS_VALUE_NUMBER(*right_value)) ^ b;
				}else{
					pushOpResultValue((OpcodeType)OS_TO_OPCODE_TYPE(opcode), *left_value, *right_value);
					OS_ASSERT(OS_VALUE_TYPE(stack_values.lastElement()) == OS_VALUE_TYPE_BOOL);
					res = OS_VALUE_VARIANT(stack_values.buf[--stack_values.count]).boolean ^ b;
				}
				if(!(OS_GETARG_C(instruction))){
					stack_func_locals[a] = res != 0;
					break;
				}
#ifdef OS_DEBUG
				instruction = stack_func->opcodes[0];
				opcode = (OpcodeType)OS_GET_OPCODE_TYPE(instruction);
				OS_ASSERT(opcode == OP_JUMP);
#endif
				if(res){
					stack_func->opcodes++;
				}else{
					instruction = stack_func->opcodes[0];
					b = OS_GETARG_sBx(instruction);
					stack_func->opcodes += b + 1;
				}
				break;
			}

		OS_CASE_OPCODE(OP_JUMP):
			{
				stack_func->opcodes += OS_GETARG_sBx(instruction);
				break;
			}

		OS_CASE_OPCODE_ALL(OP_BIT_NOT):
			{
				a = OS_GETARG_A(instruction);
				OS_ASSERT(a >= 0 && a < stack_func->func->func_decl->stack_size);
				b = OS_GETARG_B(instruction);
				left_value = & OS_GETARG_B_VALUE();
				if(OS_IS_VALUE_NUMBER(*left_value)){
					OS_SET_VALUE_NUMBER(stack_func_locals[a],  ~(OS_INT)OS_VALUE_NUMBER(*left_value));
				}else{
					pushOpResultValue((OpcodeType)OS_TO_OPCODE_TYPE(opcode), *left_value);
					stack_func_locals[a] = stack_values.buf[--stack_values.count];
				}
				break;
			}

		OS_CASE_OPCODE_ALL(OP_PLUS):
			{
				a = OS_GETARG_A(instruction);
				OS_ASSERT(a >= 0 && a < stack_func->func->func_decl->stack_size);
				b = OS_GETARG_B(instruction);
				left_value = & OS_GETARG_B_VALUE();
				if(OS_IS_VALUE_NUMBER(*left_value)){
					stack_func_locals[a] =  *left_value; // OS_VALUE_NUMBER(*left_value);
				}else{
					pushOpResultValue((OpcodeType)OS_TO_OPCODE_TYPE(opcode), *left_value);
					stack_func_locals[a] = stack_values.buf[--stack_values.count];
				}
				break;
			}

		OS_CASE_OPCODE_ALL(OP_NEG):
			{
				a = OS_GETARG_A(instruction);
				OS_ASSERT(a >= 0 && a < stack_func->func->func_decl->stack_size);
				b = OS_GETARG_B(instruction);
				left_value = & OS_GETARG_B_VALUE();
				if(OS_IS_VALUE_NUMBER(*left_value)){
					stack_func_locals[a] = -OS_VALUE_NUMBER(*left_value);
				}else{
					pushOpResultValue((OpcodeType)OS_TO_OPCODE_TYPE(opcode), *left_value);
					stack_func_locals[a] = stack_values.buf[--stack_values.count];
				}
				break;
			}

		OS_CASE_OPCODE_ALL(OP_BIT_AND):
			{
				a = OS_GETARG_A(instruction);
				OS_ASSERT(a >= 0 && a < stack_func->func->func_decl->stack_size);
				b = OS_GETARG_B(instruction);
				c = OS_GETARG_C(instruction);
				left_value = & OS_GETARG_B_VALUE();
				right_value = & OS_GETARG_C_VALUE();
				if(OS_IS_VALUE_NUMBER(*left_value) && OS_IS_VALUE_NUMBER(*right_value)){
					stack_func_locals[a] = (OS_INT)OS_VALUE_NUMBER(*left_value) & (OS_INT)OS_VALUE_NUMBER(*right_value);
				}else{
					pushOpResultValue((OpcodeType)OS_TO_OPCODE_TYPE(opcode), *left_value, *right_value);
					stack_func_locals[a] = stack_values.buf[--stack_values.count];
				}
				break;
			}

		OS_CASE_OPCODE_ALL(OP_BIT_OR):
			{
				a = OS_GETARG_A(instruction);
				OS_ASSERT(a >= 0 && a < stack_func->func->func_decl->stack_size);
				b = OS_GETARG_B(instruction);
				c = OS_GETARG_C(instruction);
				left_value = & OS_GETARG_B_VALUE();
				right_value = & OS_GETARG_C_VALUE();
				if(OS_IS_VALUE_NUMBER(*left_value) && OS_IS_VALUE_NUMBER(*right_value)){
					stack_func_locals[a] = (OS_INT)OS_VALUE_NUMBER(*left_value) | (OS_INT)OS_VALUE_NUMBER(*right_value);
				}else{
					pushOpResultValue((OpcodeType)OS_TO_OPCODE_TYPE(opcode), *left_value, *right_value);
					stack_func_locals[a] = stack_values.buf[--stack_values.count];
				}
				break;
			}

		OS_CASE_OPCODE_ALL(OP_BIT_XOR):
			{
				a = OS_GETARG_A(instruction);
				OS_ASSERT(a >= 0 && a < stack_func->func->func_decl->stack_size);
				b = OS_GETARG_B(instruction);
				c = OS_GETARG_C(instruction);
				left_value = & OS_GETARG_B_VALUE();
				right_value = & OS_GETARG_C_VALUE();
				if(OS_IS_VALUE_NUMBER(*left_value) && OS_IS_VALUE_NUMBER(*right_value)){
					stack_func_locals[a] = (OS_INT)OS_VALUE_NUMBER(*left_value) ^ (OS_INT)OS_VALUE_NUMBER(*right_value);
				}else{
					pushOpResultValue((OpcodeType)OS_TO_OPCODE_TYPE(opcode), *left_value, *right_value);
					stack_func_locals[a] = stack_values.buf[--stack_values.count];
				}
				break;
			}

		OS_CASE_OPCODE_ALL(OP_ADD): // +
			{
				a = OS_GETARG_A(instruction);
				OS_ASSERT(a >= 0 && a < stack_func->func->func_decl->stack_size);
				b = OS_GETARG_B(instruction);
				c = OS_GETARG_C(instruction);
				left_value = & OS_GETARG_B_VALUE();
				right_value = & OS_GETARG_C_VALUE();
				if(OS_IS_VALUE_NUMBER(*left_value) && OS_IS_VALUE_NUMBER(*right_value)){
					OS_SET_VALUE_NUMBER(stack_func_locals[a], OS_VALUE_NUMBER(*left_value) + OS_VALUE_NUMBER(*right_value));
				}else{
					pushOpResultValue((OpcodeType)OS_TO_OPCODE_TYPE(opcode), *left_value, *right_value);
					stack_func_locals[a] = stack_values.buf[--stack_values.count];
				}
				break;
			}

		OS_CASE_OPCODE_ALL(OP_COMPARE): // <=>
		OS_CASE_OPCODE_ALL(OP_SUB): // -
			{
				a = OS_GETARG_A(instruction);
				OS_ASSERT(a >= 0 && a < stack_func->func->func_decl->stack_size);
				b = OS_GETARG_B(instruction);
				c = OS_GETARG_C(instruction);
				left_value = & OS_GETARG_B_VALUE();
				right_value = & OS_GETARG_C_VALUE();
				if(OS_IS_VALUE_NUMBER(*left_value) && OS_IS_VALUE_NUMBER(*right_value)){
					OS_SET_VALUE_NUMBER(stack_func_locals[a], OS_VALUE_NUMBER(*left_value) - OS_VALUE_NUMBER(*right_value));
				}else{
					pushOpResultValue((OpcodeType)OS_TO_OPCODE_TYPE(opcode), *left_value, *right_value);
					stack_func_locals[a] = stack_values.buf[--stack_values.count];
				}
				break;
			}

		OS_CASE_OPCODE_ALL(OP_MUL): // *
			{
				a = OS_GETARG_A(instruction);
				OS_ASSERT(a >= 0 && a < stack_func->func->func_decl->stack_size);
				b = OS_GETARG_B(instruction);
				c = OS_GETARG_C(instruction);
				left_value = & OS_GETARG_B_VALUE();
				right_value = & OS_GETARG_C_VALUE();
				if(OS_IS_VALUE_NUMBER(*left_value) && OS_IS_VALUE_NUMBER(*right_value)){
					OS_SET_VALUE_NUMBER(stack_func_locals[a], OS_VALUE_NUMBER(*left_value) * OS_VALUE_NUMBER(*right_value));
				}else{
					pushOpResultValue((OpcodeType)OS_TO_OPCODE_TYPE(opcode), *left_value, *right_value);
					stack_func_locals[a] = stack_values.buf[--stack_values.count];
				}
				break;
			}

		OS_CASE_OPCODE_ALL(OP_DIV): // /
			{
				a = OS_GETARG_A(instruction);
				OS_ASSERT(a >= 0 && a < stack_func->func->func_decl->stack_size);
				b = OS_GETARG_B(instruction);
				c = OS_GETARG_C(instruction);
				left_value = & OS_GETARG_B_VALUE();
				right_value = & OS_GETARG_C_VALUE();
				if(OS_IS_VALUE_NUMBER(*left_value) && OS_IS_VALUE_NUMBER(*right_value)){
					if(!OS_VALUE_NUMBER(*right_value)){
						errorDivisionByZero();
						// OS_SET_VALUE_NUMBER(stack_func_locals[a], 0.0); // TODO: NaN or null ???
						OS_SET_VALUE_NULL(stack_func_locals[a]);
					}else{
						OS_SET_VALUE_NUMBER(stack_func_locals[a], OS_VALUE_NUMBER(*left_value) / OS_VALUE_NUMBER(*right_value));
					}
				}else{
					pushOpResultValue((OpcodeType)OS_TO_OPCODE_TYPE(opcode), *left_value, *right_value);
					stack_func_locals[a] = stack_values.buf[--stack_values.count];
				}
				break;
			}

		OS_CASE_OPCODE_ALL(OP_MOD): // %
			{
				a = OS_GETARG_A(instruction);
				OS_ASSERT(a >= 0 && a < stack_func->func->func_decl->stack_size);
				b = OS_GETARG_B(instruction);
				c = OS_GETARG_C(instruction);
				left_value = & OS_GETARG_B_VALUE();
				right_value = & OS_GETARG_C_VALUE();
				if(OS_IS_VALUE_NUMBER(*left_value) && OS_IS_VALUE_NUMBER(*right_value)){
					if(!OS_VALUE_NUMBER(*right_value)){
						errorDivisionByZero();
						// stack_func_locals[a] = (OS_NUMBER)0.0; // TODO: NaN ???
						OS_SET_VALUE_NULL(stack_func_locals[a]);
					}else{
						stack_func_locals[a] = OS_MATH_MOD_OPERATOR(OS_VALUE_NUMBER(*left_value), OS_VALUE_NUMBER(*right_value));
					}
				}else{
					pushOpResultValue((OpcodeType)OS_TO_OPCODE_TYPE(opcode), *left_value, *right_value);
					stack_func_locals[a] = stack_values.buf[--stack_values.count];
				}
				break;
			}

		OS_CASE_OPCODE_ALL(OP_LSHIFT): // <<
			{
				a = OS_GETARG_A(instruction);
				OS_ASSERT(a >= 0 && a < stack_func->func->func_decl->stack_size);
				b = OS_GETARG_B(instruction);
				c = OS_GETARG_C(instruction);
				left_value = & OS_GETARG_B_VALUE();
				right_value = & OS_GETARG_C_VALUE();
				if(OS_IS_VALUE_NUMBER(*left_value) && OS_IS_VALUE_NUMBER(*right_value)){
					stack_func_locals[a] = (OS_INT)OS_VALUE_NUMBER(*left_value) << (OS_INT)OS_VALUE_NUMBER(*right_value);
				}else{
					pushOpResultValue((OpcodeType)OS_TO_OPCODE_TYPE(opcode), *left_value, *right_value);
					stack_func_locals[a] = stack_values.buf[--stack_values.count];
				}
				break;
			}

		OS_CASE_OPCODE_ALL(OP_RSHIFT): // >>
			{
				a = OS_GETARG_A(instruction);
				OS_ASSERT(a >= 0 && a < stack_func->func->func_decl->stack_size);
				b = OS_GETARG_B(instruction);
				c = OS_GETARG_C(instruction);
				left_value = & OS_GETARG_B_VALUE();
				right_value = & OS_GETARG_C_VALUE();
				if(OS_IS_VALUE_NUMBER(*left_value) && OS_IS_VALUE_NUMBER(*right_value)){
					stack_func_locals[a] = (OS_INT)OS_VALUE_NUMBER(*left_value) >> (OS_INT)OS_VALUE_NUMBER(*right_value);
				}else{
					pushOpResultValue((OpcodeType)OS_TO_OPCODE_TYPE(opcode), *left_value, *right_value);
					stack_func_locals[a] = stack_values.buf[--stack_values.count];
				}
				break;
			}

		OS_CASE_OPCODE_ALL(OP_POW): // **
			{
				a = OS_GETARG_A(instruction);
				OS_ASSERT(a >= 0 && a < stack_func->func->func_decl->stack_size);
				b = OS_GETARG_B(instruction);
				c = OS_GETARG_C(instruction);
				left_value = & OS_GETARG_B_VALUE();
				right_value = & OS_GETARG_C_VALUE();
				if(OS_IS_VALUE_NUMBER(*left_value) && OS_IS_VALUE_NUMBER(*right_value)){
					stack_func_locals[a] = OS_MATH_POW_OPERATOR(OS_VALUE_NUMBER(*left_value), OS_VALUE_NUMBER(*right_value));
				}else{
					pushOpResultValue((OpcodeType)OS_TO_OPCODE_TYPE(opcode), *left_value, *right_value);
					stack_func_locals[a] = stack_values.buf[--stack_values.count];
				}
				break;
			}

		OS_CASE_OPCODE(OP_NEW_FUNCTION):
			{
				a = OS_GETARG_A(instruction);
				OS_ASSERT(a >= 0 && a < stack_func->func->func_decl->stack_size);
				b = OS_GETARG_B(instruction);
				prog = stack_func->func->prog;
				OS_ASSERT(b > 0 && b < prog->num_functions);
				FunctionDecl * func_decl = prog->functions + b;
				// int env_index = stack_func->func->func_decl->num_params + VAR_ENV;
				GCFunctionValue * func_value = newFunctionValue(stack_func, prog, func_decl, stack_func_locals[stack_func_env_index]);
				stack_func_locals[a] = func_value;
				stack_func->opcodes += func_decl->opcodes_size;
				break;
			}

		OS_CASE_OPCODE(OP_CALL):
			{
				OS_PROFILE_END_OPCODE(opcode); // we shouldn't profile call here
				a = OS_GETARG_A(instruction);
				OS_ASSERT(a >= 0 && a < stack_func->func->func_decl->stack_size);
				b = OS_GETARG_B(instruction);
				OS_ASSERT(b >= 2 && a+b <= stack_func->func->func_decl->stack_size);
				c = OS_GETARG_C(instruction);
				OS_ASSERT(c >= 0 && a+c <= stack_func->func->func_decl->stack_size);
				call(stack_func->locals_stack_pos + a, b, c, NULL, true);
				continue;
			}

#ifdef OS_TAIL_CALL_ENABLED
		case OP_TAIL_CALL:
			{
				OS_PROFILE_END_OPCODE(opcode); // we shouldn't profile call here
				a = GETARG_A(instruction);
				OS_ASSERT(a >= 0 && a < stack_func->func->func_decl->stack_size);
				b = GETARG_B(instruction);
				OS_ASSERT(b >= 2 && a+b <= stack_func->func->func_decl->stack_size);
				c = stack_func->need_ret_values; // GETARG_C(instruction);
				OS_ASSERT(c >= 0 && a+c <= stack_func->func->func_decl->stack_size);

				OS_MEMMOVE(stack_values.buf + stack_func->locals_stack_pos, stack_func_locals + a, sizeof(Value) * b);
				a = stack_func->locals_stack_pos;
				OS_ASSERT(call_stack_funcs.count > 0 && &call_stack_funcs[call_stack_funcs.count-1] == stack_func);
				int caller_stack_size = stack_func->caller_stack_size;
				OS_ASSERT(caller_stack_size <= stack_values.capacity);
				stack_values.count = stack_func->caller_stack_size;
				if(a + b > stack_values.count){
					OS_ASSERT(a + b <= stack_values.capacity);
					// OS_MEMSET(stack_values.buf + stack_values.count, 0, sizeof(Value) * (a + b - stack_values.count));
					stack_values.count = a + b;
				}
				call_stack_funcs.count--;
				clearStackFunction(stack_func);
				reloadStackFunctionCache();

				int call_stack_funcs_size = call_stack_funcs.count;
				call(a, b, c, NULL, true);
				if(ret_stack_funcs >= call_stack_funcs.count){
					OS_ASSERT(ret_stack_funcs == call_stack_funcs.count);
					return c;
				}				
				if(call_stack_funcs.count > call_stack_funcs_size){
					stack_func->caller_stack_size = caller_stack_size;
					// error(OS_E_WARNING, "check point");
				}
				continue;
			}
#endif

		OS_CASE_OPCODE(OP_CALL_METHOD):
			{
				OS_PROFILE_END_OPCODE(opcode); // we shouldn't profile call here
				a = OS_GETARG_A(instruction);
				OS_ASSERT(a >= 0 && a < stack_func->func->func_decl->stack_size);
				b = OS_GETARG_B(instruction);
				OS_ASSERT(b >= 2 && a+b <= stack_func->func->func_decl->stack_size);
				c = OS_GETARG_C(instruction);
				OS_ASSERT(c >= 0 && a+c <= stack_func->func->func_decl->stack_size);
				pushPropertyValue(stack_func_locals[a], PropertyIndex(stack_func_locals[a + 1]), true, true);
				stack_func_locals[a + 1] = stack_func_locals[a]; // this
				stack_func_locals[a] = stack_values.buf[--stack_values.count]; // func
				call(stack_func->locals_stack_pos + a, b, c, NULL, true);
				continue;
			}

#ifdef OS_TAIL_CALL_ENABLED
		case OP_TAIL_CALL_METHOD:
			{
				OS_PROFILE_END_OPCODE(opcode); // we shouldn't profile call here
				a = GETARG_A(instruction);
				OS_ASSERT(a >= 0 && a < stack_func->func->func_decl->stack_size);
				b = GETARG_B(instruction);
				OS_ASSERT(b >= 2 && a+b <= stack_func->func->func_decl->stack_size);
				c = stack_func->need_ret_values; // GETARG_C(instruction);
				OS_ASSERT(c >= 0 && a+c <= stack_func->func->func_decl->stack_size);
				pushPropertyValue(stack_func_locals[a], PropertyIndex(stack_func_locals[a + 1]), true, true, true, false);
				stack_func_locals[a + 1] = stack_func_locals[a]; // this
				stack_func_locals[a] = stack_values.buf[--stack_values.count]; // func
				
				OS_MEMMOVE(stack_values.buf + stack_func->locals_stack_pos, stack_func_locals + a, sizeof(Value) * b);
				a = stack_func->locals_stack_pos;
				OS_ASSERT(call_stack_funcs.count > 0 && &call_stack_funcs[call_stack_funcs.count-1] == stack_func);
				int caller_stack_size = stack_func->caller_stack_size;
				OS_ASSERT(caller_stack_size <= stack_values.capacity);
				stack_values.count = stack_func->caller_stack_size;
				if(a + b > stack_values.count){
					OS_ASSERT(a + b <= stack_values.capacity);
					// OS_MEMSET(stack_values.buf + stack_values.count, 0, sizeof(Value) * (a + b - stack_values.count));
					stack_values.count = a + b;
				}
				call_stack_funcs.count--;
				clearStackFunction(stack_func);
				reloadStackFunctionCache();

				int call_stack_funcs_size = call_stack_funcs.count;
				call(a, b, c, NULL, true);
				if(ret_stack_funcs >= call_stack_funcs.count){
					OS_ASSERT(ret_stack_funcs == call_stack_funcs.count);
					return c;
				}
				if(call_stack_funcs.count > call_stack_funcs_size){
					stack_func->caller_stack_size = caller_stack_size;				
				}
				continue;
			}
#endif

		OS_CASE_OPCODE(OP_SUPER_CALL):
			{
				OS_PROFILE_END_OPCODE(opcode); // we shouldn't profile call here
				a = OS_GETARG_A(instruction);
				OS_ASSERT(a >= 0 && a < stack_func->func->func_decl->stack_size);
				b = OS_GETARG_B(instruction);
				OS_ASSERT(b >= 2 && a+b <= stack_func->func->func_decl->stack_size);
				c = OS_GETARG_C(instruction);
				OS_ASSERT(c >= 0 && a+c <= stack_func->func->func_decl->stack_size);
				OS_ASSERT(OS_VALUE_TYPE(stack_func_locals[a]) == OS_VALUE_TYPE_NULL);
				OS_ASSERT(OS_VALUE_TYPE(stack_func_locals[a + 1]) == OS_VALUE_TYPE_NULL);

				GCValue * proto = NULL;
				GCFunctionValue * func_value = stack_func->func;
				if(stack_func->self_for_proto && func_value->name){
					proto = stack_func->self_for_proto->prototype;
					if(stack_func->self_for_proto->is_object_instance){
						proto = proto ? proto->prototype : NULL;
					}
					if(proto){
						bool prototype_enabled = true;
						OS_ASSERT(strings->__destruct != func_value->name);
						if(getPropertyValue(value, proto, PropertyIndex(func_value->name, PropertyIndex::KeepStringIndex()), prototype_enabled)
							&& value.isFunction())
						{
							stack_func_locals[a] = value;
							stack_func_locals[a + 1] = stack_func_locals[PRE_VAR_THIS];
						}else{
							stack_func_locals[a] = stack_func_locals[a + 1] = Value();
						}
					}
				}

				call(stack_func->locals_stack_pos + a, b, c, proto, true);
				continue;
			}

		OS_CASE_OPCODE_ALL(OP_MOVE):
			{
				a = OS_GETARG_A(instruction);
				OS_ASSERT(a >= 0 && a < stack_func->func->func_decl->stack_size);
				b = OS_GETARG_B(instruction);
#if OS_MAX_GENERIC_CONST_INDEX == 0
				stack_func_locals[a] = stack_func_locals[b];
#else
				stack_func_locals[a] = OS_GETARG_B_VALUE();
#endif
				break;
			}

		OS_CASE_OPCODE_ALL(OP_MOVE2):
			{
				a = OS_GETARG_A(instruction);
				OS_ASSERT(a >= 0 && a+1 < stack_func->func->func_decl->stack_size);
				b = OS_GETARG_B(instruction);
				c = OS_GETARG_C(instruction);
#if OS_MAX_GENERIC_CONST_INDEX == 0
				stack_func_locals[a] = stack_func_locals[b];
				stack_func_locals[a + 1] = stack_func_locals[c];
#else
				stack_func_locals[a] = OS_GETARG_B_VALUE();
				stack_func_locals[a + 1] = OS_GETARG_C_VALUE();
#endif
				break;
			}

		OS_CASE_OPCODE(OP_GET_XCONST):
			{
				a = OS_GETARG_A(instruction);
				OS_ASSERT(a >= 0 && a < stack_func->func->func_decl->stack_size);
				b = OS_GETARG_Bx(instruction);
				OS_ASSERT(b >= 0 && b < stack_func->func->prog->num_numbers + stack_func->func->prog->num_strings + CONST_STD_VALUES);
				stack_func_locals[a] = stack_func_prog_values[b];
				break;
			}

		OS_CASE_OPCODE(OP_GET_UPVALUE):
			{
				a = OS_GETARG_A(instruction);
				OS_ASSERT(a >= 0 && a < stack_func->func->func_decl->stack_size);
				b = OS_GETARG_B(instruction); // local
				up_count = OS_GETARG_C(instruction);
				OS_ASSERT(up_count <= stack_func->func->func_decl->max_up_count);
				Locals * func_locals = stack_func->locals;
				OS_ASSERT(up_count <= func_locals->num_parents);
				Locals * scope = func_locals->getParent(up_count-1);
				OS_ASSERT(b >= 0 && b < scope->func_decl->num_locals);
				stack_func_locals[a] = scope->values[b];
				break;
			}

		OS_CASE_OPCODE(OP_SET_UPVALUE):
			{
				a = OS_GETARG_A(instruction); // local
				b = OS_GETARG_B(instruction);
				OS_ASSERT(b >= 0 && b < stack_func->func->func_decl->stack_size);
				up_count = OS_GETARG_C(instruction);
				OS_ASSERT(up_count <= stack_func->func->func_decl->max_up_count);
				Locals * func_locals = stack_func->locals;
				OS_ASSERT(up_count <= func_locals->num_parents);
				Locals * scope = func_locals->getParent(up_count-1);
				OS_ASSERT(a >= 0 && a < scope->func_decl->num_locals);
				scope->values[a] = stack_func_locals[b];
				break;
			}

		OS_CASE_OPCODE_ALL(OP_GET_PROPERTY):
			{
				a = OS_GETARG_A(instruction);
				OS_ASSERT(a >= 0 && a < stack_func->func->func_decl->stack_size);
				b = OS_GETARG_B(instruction);
				OS_ASSERT(b >= 0 && b < stack_func->func->func_decl->stack_size);
				c = OS_GETARG_C(instruction);
				// pushPropertyValue(stack_func_locals[b], PropertyIndex(OS_GETARG_C_VALUE(instruction)), true, true, true, false);
				Value& obj = stack_func_locals[b];
				const PropertyIndex index(OS_GETARG_C_VALUE());
				int obj_type = OS_VALUE_TYPE(obj);
				if(OS_IS_VALUE_NUMBER(index.index) && obj_type == OS_VALUE_TYPE_ARRAY){
					OS_ASSERT(dynamic_cast<GCArrayValue*>(OS_VALUE_VARIANT(obj).value));
					GCArrayValue * arr = (GCArrayValue*)OS_VALUE_VARIANT(obj).value;
					int i = (int)OS_VALUE_NUMBER(index.index);
					if((i >= 0 || (i += arr->values.count) >= 0) && i < arr->values.count){
						stack_func_locals[a] = arr->values[i];
					}else{
						stack_func_locals[a] = Value();
					}
					break;
				}
				const bool getter_enabled = true, prototype_enabled = true;
				switch(obj_type){
				case OS_VALUE_TYPE_NULL:
				default:
					stack_func_locals[a] = Value();
					break;

				case OS_VALUE_TYPE_BOOL:
				case OS_VALUE_TYPE_NUMBER:
					pushPropertyValueForPrimitive(obj, index, getter_enabled, prototype_enabled);
					stack_func_locals[a] = stack_values.buf[--stack_values.count];
					break;

				case OS_VALUE_TYPE_ARRAY:
					OS_ASSERT(!OS_IS_VALUE_NUMBER(index.index));
					// no break

				case OS_VALUE_TYPE_STRING:
				case OS_VALUE_TYPE_OBJECT:
				case OS_VALUE_TYPE_USERDATA:
				case OS_VALUE_TYPE_USERPTR:
				case OS_VALUE_TYPE_FUNCTION:
				case OS_VALUE_TYPE_CFUNCTION:
					{
						GCValue * self = OS_VALUE_VARIANT(obj).value;
						GCValue * table_value = self;
						for(;;){
							if(getPropertyValue(value, table_value, index, prototype_enabled)){
								stack_func_locals[a] = value;
								break;
							}
							if(getter_enabled && !hasSpecialPrefix(index.index)){
								if(OS_VALUE_TYPE(index.index) == OS_VALUE_TYPE_STRING){
									const void * buf1 = strings->__getAt.toChar();
									int size1 = strings->__getAt.getDataSize();
									const void * buf2 = OS_VALUE_VARIANT(index.index).string->toChar();
									int size2 = OS_VALUE_VARIANT(index.index).string->getDataSize();
									GCStringValue * getter_name = newStringValue(buf1, size1, buf2, size2);
									if(getPropertyValue(value, table_value, PropertyIndex(getter_name, PropertyIndex::KeepStringIndex()), prototype_enabled)){
										pushValue(value);
										pushValue(self);
										call(0, 1);
										stack_func_locals[a] = stack_values.buf[--stack_values.count];
										break;
									}
								}
								if(getPropertyValue(value, table_value, PropertyIndex(strings->__get, PropertyIndex::KeepStringIndex()), prototype_enabled)){
									if(OS_VALUE_TYPE(value) == OS_VALUE_TYPE_OBJECT){
										table_value = OS_VALUE_VARIANT(value).value;
										continue;
									}
									pushValue(value);
									pushValue(self);
									pushValue(index.index);
									call(1, 1);
									stack_func_locals[a] = stack_values.buf[--stack_values.count];
									break;
								}
							}
							stack_func_locals[a] = Value();
							break;
						}
						break;
					}
				}
				break;
			}

		OS_CASE_OPCODE_ALL(OP_SET_PROPERTY):
			{
				a = OS_GETARG_A(instruction);
				OS_ASSERT(a >= 0 && a < stack_func->func->func_decl->stack_size);
				b = OS_GETARG_B(instruction);
				c = OS_GETARG_C(instruction);
				Value& obj = stack_func_locals[a];
				Value& index = OS_GETARG_B_VALUE();
				if(OS_IS_VALUE_NUMBER(index) && OS_VALUE_TYPE(obj) == OS_VALUE_TYPE_ARRAY){
					OS_ASSERT(dynamic_cast<GCArrayValue*>(OS_VALUE_VARIANT(obj).value));
					GCArrayValue * arr = (GCArrayValue*)OS_VALUE_VARIANT(obj).value;
					int i = (int)OS_VALUE_NUMBER(index);
					if(i >= 0 || (i += arr->values.count) >= 0){
						while(i >= arr->values.count){
							allocator->vectorAddItem(arr->values, Value() OS_DBG_FILEPOS);
						}
						OS_ASSERT(i < arr->values.count);
						arr->values[i] = OS_GETARG_C_VALUE();
					}
					OS_PROFILE_END_OPCODE(opcode);
					continue;
				}
#if 1
				setPropertyValue(obj, PropertyIndex(index), OS_GETARG_C_VALUE(), true);
#else			// inline setPropertyValue
				Value& obj = stack_func_locals[b];
				const PropertyIndex index(OS_GETARG_B_VALUE());
				const bool setter_enabled = true;

				switch(OS_VALUE_TYPE(obj)){
				default:
				case OS_VALUE_TYPE_NULL:
				case OS_VALUE_TYPE_BOOL:
				case OS_VALUE_TYPE_NUMBER:
					OS_PROFILE_END_OPCODE(opcode);
					continue;

				case OS_VALUE_TYPE_STRING:
					// return setPropertyValue(prototypes[PROTOTYPE_STRING], index, value, setter_enabled);
					// return;

				case OS_VALUE_TYPE_ARRAY:
				case OS_VALUE_TYPE_OBJECT:
				case OS_VALUE_TYPE_USERDATA:
				case OS_VALUE_TYPE_USERPTR:
				case OS_VALUE_TYPE_FUNCTION:
				case OS_VALUE_TYPE_CFUNCTION:
					break;
				}

				GCValue * table_value = OS_VALUE_VARIANT(obj).value;

				// TODO: correct ???
				Value value = OS_GETARG_C_VALUE();
				gcAddToGreyList(value);

				if(OS_VALUE_TYPE(index.index) == OS_VALUE_TYPE_STRING){
					OS_ASSERT(dynamic_cast<GCStringValue*>(OS_VALUE_VARIANT(index.index).string));
					switch(OS_VALUE_TYPE(value)){
					case OS_VALUE_TYPE_FUNCTION:
						OS_ASSERT(dynamic_cast<GCFunctionValue*>(OS_VALUE_VARIANT(value).func));
						if(!OS_VALUE_VARIANT(value).func->name){
							OS_VALUE_VARIANT(value).func->name = OS_VALUE_VARIANT(index.index).string;
						}
						break;

					case OS_VALUE_TYPE_CFUNCTION:
						OS_ASSERT(dynamic_cast<GCCFunctionValue*>(OS_VALUE_VARIANT(value).cfunc));
						if(!OS_VALUE_VARIANT(value).cfunc->name){
							OS_VALUE_VARIANT(value).cfunc->name = OS_VALUE_VARIANT(index.index).string;
						}
						break;
					}
				}

				Property * prop = NULL;
				Table * table = table_value->table;
				if(table && (prop = table->get(index))){
					prop->value = value;
					return;
				}

				if(OS_VALUE_TYPE(index.index) == OS_VALUE_TYPE_STRING && strings->syntax_prototype == OS_VALUE_VARIANT(index.index).string){
					switch(table_value->type){
					case OS_VALUE_TYPE_STRING:
					case OS_VALUE_TYPE_ARRAY:
					case OS_VALUE_TYPE_OBJECT:
					case OS_VALUE_TYPE_FUNCTION:
						table_value->prototype = OS_VALUE_VARIANT(value).value;
						break;

					case OS_VALUE_TYPE_USERDATA:
					case OS_VALUE_TYPE_USERPTR:
					case OS_VALUE_TYPE_CFUNCTION:
						// TODO: warning???
						break;
					}
					return;
				}

				if(table_value->type == OS_VALUE_TYPE_ARRAY){
					OS_ASSERT(dynamic_cast<GCArrayValue*>(table_value));
					GCArrayValue * arr = (GCArrayValue*)table_value;
					int i = (int)valueToInt(index.index);
					if(i < 0) i += arr->values.count;
					if(i >= 0){
						while(i >= arr->values.count){
							allocator->vectorAddItem(arr->values, Value() OS_DBG_FILEPOS);
						}
						OS_ASSERT(i < arr->values.count);
						arr->values[i] = value;
					}
					return;
				}

				if(setter_enabled && !hasSpecialPrefix(index.index)){
					Value func;
					if(OS_VALUE_TYPE(index.index) == OS_VALUE_TYPE_STRING){
						const void * buf1 = strings->__setAt.toChar();
						int size1 = strings->__setAt.getDataSize();
						const void * buf2 = OS_VALUE_VARIANT(index.index).string->toChar();
						int size2 = OS_VALUE_VARIANT(index.index).string->getDataSize();
						GCStringValue * setter_name = newStringValue(buf1, size1, buf2, size2);
						if(getPropertyValue(func, table_value, PropertyIndex(setter_name, PropertyIndex::KeepStringIndex()), true)){
							pushValue(func);
							pushValue(table_value);
							pushValue(value);
							call(1, 0);
							return;
						}
					}
					if(getPropertyValue(func, table_value, PropertyIndex(strings->__set, PropertyIndex::KeepStringIndex()), true)){
						pushValue(func);
						pushValue(table_value);
						pushValue(index.index);
						pushValue(value);
						call(2, 0);
						return;
					}
				}
				if(table_value->type == OS_VALUE_TYPE_STRING){
					// TODO: trigger error???
					return;
				}
				if(!table){
					table_value->table = table = newTable(OS_DBG_FILEPOS_START);
				}
				prop = new (malloc(sizeof(Property) OS_DBG_FILEPOS)) Property(index);
				prop->value = value;
				addTableProperty(table, prop);
				// setTableValue(table, index, value);
#endif
				break;
			}

		OS_CASE_OPCODE(OP_NEW_OBJECT):
			{
				a = OS_GETARG_A(instruction);
				OS_ASSERT(a >= 0 && a < stack_func->func->func_decl->stack_size);
				GCValue * gc_value = newObjectValue();
				stack_func_locals[a] = gc_value;
				break;
			}

		OS_CASE_OPCODE(OP_NEW_ARRAY):
			{
				a = OS_GETARG_A(instruction);
				OS_ASSERT(a >= 0 && a < stack_func->func->func_decl->stack_size);
				b = OS_GETARG_B(instruction);
				GCValue * gc_value = newArrayValue(b);
				stack_func_locals[a] = gc_value;
				break;
			}

		OS_CASE_OPCODE(OP_MULTI):
			{
				a = OS_GETARG_A(instruction);
				OS_ASSERT(a >= 0 && a < stack_func->func->func_decl->stack_size);
				c = OS_GETARG_C(instruction);
				switch(c){
				case OP_MULTI_GET_ARGUMENTS:
					if(stack_func->arguments){
						stack_func_locals[a] = stack_func->arguments;
					}else{
						pushArguments(stack_func);
						stack_func_locals[a] = stack_values.buf[--stack_values.count];
					}
					break;

				case OP_MULTI_GET_REST_ARGUMENTS:
					if(stack_func->rest_arguments){
						stack_func_locals[a] = stack_func->rest_arguments;
					}else{
						pushRestArguments(stack_func);
						stack_func_locals[a] = stack_values.buf[--stack_values.count];
					}
					break;

				case OP_MULTI_SUPER:
					if(stack_func->self_for_proto){
						GCValue * proto = stack_func->self_for_proto->prototype;
						if(stack_func->self_for_proto->is_object_instance){
							proto = proto ? proto->prototype : NULL;
						}					
						stack_func_locals[a] = proto;
					}else{
						stack_func_locals[a] = Value();
					}
					break;

				case OP_MULTI_DEBUGGER:
					DEBUG_BREAK;
					break;
				}
				break;
			}

		OS_CASE_OPCODE(OP_RETURN):
			{
				a = OS_GETARG_A(instruction);
				OS_ASSERT(a >= 0 && a < stack_func->func->func_decl->stack_size);
				b = OS_GETARG_B(instruction);
				OS_ASSERT(b >= 0 && a+b <= stack_func->func->func_decl->stack_size);
				// c = GETARG_C(instruction);
				// OS_ASSERT(c == 0 || c == 1);

				int cur_ret_values = b;
				int need_ret_values = stack_func->need_ret_values;
				if(need_ret_values == 1){
					if(cur_ret_values > 0){
						stack_values.buf[stack_func->locals_stack_pos] = stack_func_locals[a];
					}else{
						OS_SET_VALUE_NULL(stack_values.buf[stack_func->locals_stack_pos]);
					}
				}else if(need_ret_values > 0){
					if(need_ret_values <= cur_ret_values){
						OS_MEMMOVE(stack_values.buf + stack_func->locals_stack_pos, stack_func_locals + a, sizeof(Value) * need_ret_values);
					}else{
						if(cur_ret_values > 0){
							OS_MEMMOVE(stack_values.buf + stack_func->locals_stack_pos, stack_func_locals + a, sizeof(Value) * cur_ret_values);
						}
						OS_SET_NULL_VALUES(stack_values.buf + stack_func->locals_stack_pos + cur_ret_values, need_ret_values - cur_ret_values);
					}
				}
				OS_ASSERT(call_stack_funcs.count > 0 && &call_stack_funcs[call_stack_funcs.count-1] == stack_func);
				if(stack_func->caller_stack_size > stack_values.count){
					OS_ASSERT(stack_func->caller_stack_size <= stack_values.capacity);
					OS_SET_NULL_VALUES(stack_values.buf + stack_values.count, stack_func->caller_stack_size - stack_values.count);
				}
				stack_values.count = stack_func->caller_stack_size;
				call_stack_funcs.count--;
				clearStackFunction(stack_func);
				reloadStackFunctionCache();
				if(ret_stack_funcs >= call_stack_funcs.count){
					OS_ASSERT(ret_stack_funcs == call_stack_funcs.count);
					OS_PROFILE_END_OPCODE(opcode);
					return need_ret_values;
				}
				break;
			}
		}
		OS_PROFILE_END_OPCODE(opcode);
	}
	return 0;
}

void OS::runOp(OS_EOpcode opcode)
{
	struct Lib
	{
		Core * core;

		void runBinaryOpcode(Core::OpcodeType opcode)
		{
			int count = core->stack_values.count;
			if(count < 2){
				core->pushNull();
				return;
			}
			Core::Value left_value = core->stack_values[count-2];
			Core::Value right_value = core->stack_values[count-1];
			core->pushOpResultValue(opcode, left_value, right_value);
			core->removeStackValues(-3, 2);
		}

		void runUnaryOpcode(Core::OpcodeType opcode)
		{
			int count = core->stack_values.count;
			if(count < 1){
				core->pushNull();
				return;
			}
			Core::Value value = core->stack_values[count-1];
			core->pushOpResultValue(opcode, value);
			core->removeStackValue(-2);
		}

	} lib = {core};
	switch(opcode){
	case OP_COMPARE:
		return lib.runBinaryOpcode(Core::OP_COMPARE);

	case OP_LOGIC_PTR_EQ:	// ===
		return lib.runBinaryOpcode(Core::OP_LOGIC_PTR_EQ);

	case OP_LOGIC_PTR_NE:	// !==
		lib.runBinaryOpcode(Core::OP_LOGIC_PTR_EQ);
		core->stack_values.lastElement() = !core->valueToBool(core->stack_values.lastElement());
		return;

	case OP_LOGIC_EQ:		// ==
		return lib.runBinaryOpcode(Core::OP_LOGIC_EQ);

	case OP_LOGIC_NE:		// !=
		lib.runBinaryOpcode(Core::OP_LOGIC_EQ);
		core->stack_values.lastElement() = !core->valueToBool(core->stack_values.lastElement());
		return;

	case OP_LOGIC_GE:		// >=
		return lib.runBinaryOpcode(Core::OP_LOGIC_GE);

	case OP_LOGIC_LE:		// <=
		lib.runBinaryOpcode(Core::OP_LOGIC_GREATER);
		core->stack_values.lastElement() = !core->valueToBool(core->stack_values.lastElement());
		return;

	case OP_LOGIC_GREATER:	// >
		return lib.runBinaryOpcode(Core::OP_LOGIC_GREATER);

	case OP_LOGIC_LESS:		// <
		lib.runBinaryOpcode(Core::OP_LOGIC_GE);
		core->stack_values.lastElement() = !core->valueToBool(core->stack_values.lastElement());
		return;

	case OP_BIT_AND:	// &
		return lib.runBinaryOpcode(Core::OP_BIT_AND);

	case OP_BIT_OR:	// |
		return lib.runBinaryOpcode(Core::OP_BIT_OR);

	case OP_BIT_XOR:	// ^
		return lib.runBinaryOpcode(Core::OP_BIT_XOR);

	case OP_ADD: // +
		return lib.runBinaryOpcode(Core::OP_ADD);

	case OP_SUB: // -
		return lib.runBinaryOpcode(Core::OP_SUB);

	case OP_MUL: // *
		return lib.runBinaryOpcode(Core::OP_MUL);

	case OP_DIV: // /
		return lib.runBinaryOpcode(Core::OP_DIV);

	case OP_MOD: // %
		return lib.runBinaryOpcode(Core::OP_MOD);

	case OP_LSHIFT: // <<
		return lib.runBinaryOpcode(Core::OP_LSHIFT);

	case OP_RSHIFT: // >>
		return lib.runBinaryOpcode(Core::OP_RSHIFT);

	case OP_POW: // **
		return lib.runBinaryOpcode(Core::OP_POW);

	case OP_CONCAT: // ..
		// return lib.runBinaryOpcode(Core::OP_CONCAT);
		pushString(toString(-2) + toString(-1));
		return;

	case OP_BIT_NOT: // ~
		return lib.runUnaryOpcode(Core::OP_BIT_NOT);

	case OP_PLUS: // +
		return lib.runUnaryOpcode(Core::OP_PLUS);

	case OP_NEG: // -
		return lib.runUnaryOpcode(Core::OP_NEG);

	case OP_LENGTH: // #
		getProperty(-1, core->strings->__len);
		pushStackValue(-2);
		call(0, 1);
		remove(-2);
		return;
	}
	pushNull();
}

int OS::getLen(int offs)
{
	offs = getAbsoluteOffs(offs);
	getProperty(offs, core->strings->__len);
	pushStackValue(offs);
	call(0, 1);
	return popInt();
}

void OS::getErrorHandler(int code)
{
	for(int i = 0; i < OS_ERROR_LEVELS; i++){
		if(code & (1<<i)){
			core->pushValue(core->error_handlers[i]);
			return;
		}
	}
	pushNull();
}

void OS::setErrorHandler(int code)
{
	bool returned = false;
	Core::Value func = core->getStackValue(-1);
	if(func.isFunction()){
		for(int i = 0; i < OS_ERROR_LEVELS; i++){
			if(code & (1<<i)){
				if(!returned){
					core->pushValue(core->error_handlers[i]);
					returned = true;
				}
				core->error_handlers[i] = func;
			}
		}
	}
	if(!returned){
		pushNull();
	}
	remove(-2);
}

void OS::setFunc(const FuncDef& def, bool setter_enabled, int closure_values, void * user_param)
{
	const FuncDef list[] = {def, {}};
	setFuncs(list, setter_enabled, closure_values, user_param);
}

void OS::setFuncs(const FuncDef * list, bool setter_enabled, int closure_values, void * user_param)
{
	for(; list->func; list++){
		pushStackValue(-1);
		pushString(list->name);
		// push closure_values for cfunction
		for(int i = 0; i < closure_values; i++){
			pushStackValue(-2-closure_values);
		}
		pushCFunction(list->func, closure_values, list->user_param ? list->user_param : user_param);
		setProperty(setter_enabled);
	}
}

void OS::setNumber(const NumberDef& def, bool setter_enabled)
{
	const NumberDef list[] = {def, {}};
	setNumbers(list, setter_enabled);
}

void OS::setNumbers(const NumberDef * list, bool setter_enabled)
{
	for(; list->name; list++){
		pushStackValue(-1);
		pushString(list->name);
		pushNumber(list->value);
		setProperty(setter_enabled);
	}
}

void OS::setString(const StringDef& def, bool setter_enabled)
{
	const StringDef list[] = {def, {}};
	setStrings(list, setter_enabled);
}

void OS::setStrings(const StringDef * list, bool setter_enabled)
{
	for(; list->name; list++){
		pushStackValue(-1);
		pushString(list->name);
		pushString(list->value);
		setProperty(setter_enabled);
	}
}

void OS::setNull(const NullDef& def, bool setter_enabled)
{
	const NullDef list[] = {def, {}};
	setNulls(list, setter_enabled);
}

void OS::setNulls(const NullDef * list, bool setter_enabled)
{
	for(; list->name; list++){
		pushStackValue(-1);
		pushString(list->name);
		pushNull();
		setProperty(setter_enabled);
	}
}

void OS::getObject(const OS_CHAR * name, bool getter_enabled, bool prototype_enabled)
{
	pushStackValue(-1); // 2: copy parent object
	pushString(name);	// 3: index
	getProperty(getter_enabled, prototype_enabled); // 2: value
	if(isObject()){
		remove(-2);		// 1: remove parent object
		return;
	}
	pop();				// 1: parent object
	newObject();		// 2: result object
	pushStackValue(-2);	// 3: copy parent object
	pushString(name);	// 4: index
	pushStackValue(-3);	// 5: copy result object
	setProperty(getter_enabled); // 2: parent + result
	remove(-2);			// 1: remove parent object
}

void OS::getGlobalObject(const OS_CHAR * name, bool getter_enabled, bool prototype_enabled)
{
	pushGlobals();
	getObject(name, getter_enabled, prototype_enabled);
}

void OS::getModule(const OS_CHAR * name, bool getter_enabled, bool prototype_enabled)
{
	getGlobalObject(name, getter_enabled, prototype_enabled);
	pushStackValue(-1);
	pushGlobals();
	setPrototype();
}

void OS::getGlobal(const OS_CHAR * name, bool getter_enabled, bool prototype_enabled)
{
	getGlobal(Core::String(this, name), getter_enabled, prototype_enabled);
}

void OS::getGlobal(const Core::String& name, bool getter_enabled, bool prototype_enabled)
{
	pushGlobals();
	pushString(name);
	getProperty(getter_enabled, prototype_enabled);
}

void OS::setGlobal(const OS_CHAR * name, bool setter_enabled)
{
	setGlobal(Core::String(this, name), setter_enabled);
}

void OS::setGlobal(const Core::String& name, bool setter_enabled)
{
	if(core->stack_values.count >= 1){
		Core::Value object = core->global_vars;
		Core::Value value = core->stack_values[core->stack_values.count - 1];
		Core::Value index = core->pushStringValue(name);
		core->setPropertyValue(object, Core::PropertyIndex(index), value, setter_enabled);
		pop(2);
	}
}

void OS::setGlobal(const FuncDef& func, bool setter_enabled)
{
	pushCFunction(func.func, func.user_param);
	setGlobal(func.name, setter_enabled);
}

static const OS_CHAR DIGITS[] = "0123456789abcdefghijklmnopqrstuvwxyz";
static const OS_CHAR UPPER_DIGITS[] = "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZ";

void OS::initGlobalFunctions()
{
	struct Format
	{
		enum {
			ZEROPAD = 1,               // Pad with zero
			SIGN    = 2,               // Unsigned/signed long
			PLUS    = 4,               // Show plus
			SPACE   = 8,               // Space if plus
			LEFT    = 16,              // Left justified
			SPECIAL = 32,              // 0x
			LARGE   = 64,              // Use 'ABCDEF' instead of 'abcdef'
			PRECISION = 128,
			CVTBUFSIZE = 352
		};

		static OS_CHAR toSingleChar(const String& s)
		{
			return s.getLen() > 0 ? s.toChar()[0] : OS_TEXT(' ');
		}

		static void number(OS::Core::Buffer& buf, long num, int base, int size, int precision, int type)
		{
			OS_CHAR c, sign, tmp[128];
			const OS_CHAR *dig = DIGITS;
			int i;

			if(precision < 0){
				double p = 10.0;
				for(int i = -precision-1; i > 0; i--){
					p *= 10.0;
				}
				num = (long)(::floor(num / p + 0.5) * p);
				// precision = 0;
				// type &= ~PRECISION;
			}

			if (type & LARGE)  dig = UPPER_DIGITS;
			if (type & LEFT) type &= ~ZEROPAD;
			if (base < 2 || base > 36) return;

			c = (type & ZEROPAD) ? '0' : ' ';
			sign = 0;
			if(1){ // type & SIGN) {
				if (num < 0) {
					sign = '-';
					num = -num;
					size--;
				} else if (type & PLUS) {
					sign = '+';
					size--;
				} else if (type & SPACE) {
					sign = ' ';
					size--;
				}
			}

			if (type & SPECIAL) {
				if (base == 16) {
					size -= 2;
				} else if (base == 8) {
					size--;
				}
			}

			i = 0;

			if (num == 0) {
				tmp[i++] = '0';
			} else {
				while (num != 0) {
					tmp[i++] = dig[((unsigned long) num) % (unsigned) base];
					num = ((unsigned long) num) / (unsigned) base;
				}
			}

			if (i > precision) precision = i;
			size -= precision;
			if (!(type & (ZEROPAD | LEFT))) while (size-- > 0) buf.append(OS_TEXT(' '));
			if (sign) buf.append(sign);

			if (type & SPECIAL) {
				if (base == 8) {
					buf.append(OS_TEXT('0'));
				} else if (base == 16) {
					buf.append(OS_TEXT('0'));
					buf.append(DIGITS[33]);
				}
			}

			if (!(type & LEFT)) while (size-- > 0) buf.append(c);
			while (i < precision--) buf.append(OS_TEXT('0'));
			while (i-- > 0) buf.append(tmp[i]);
			while (size-- > 0) buf.append(OS_TEXT(' '));
		}

#if 0
		static OS_CHAR *cvt(double arg, int ndigits, int *decpt, int *sign, OS_CHAR *buf, int eflag)
		{
			int r2;
			double fi, fj;
			OS_CHAR *p, *p1;

			if (ndigits < 0) ndigits = 0;
			if (ndigits >= CVTBUFSIZE - 1) ndigits = CVTBUFSIZE - 2;
			r2 = 0;
			*sign = 0;
			p = &buf[0];
			if (arg < 0) {
				*sign = 1;
				arg = -arg;
			}
			arg = modf(arg, &fi);
			p1 = &buf[CVTBUFSIZE];

			if (fi != 0) {
				p1 = &buf[CVTBUFSIZE];
				while (fi != 0) {
					fj = ::modf(fi / 10, &fi);
					*--p1 = (int)((fj + .03) * 10) + OS_TEXT('0');
					r2++;
				}
				while (p1 < &buf[CVTBUFSIZE]) *p++ = *p1++;
			} else if (arg > 0) {
				while ((fj = arg * 10) < 1) {
					arg = fj;
					r2--;
				}
			}
			p1 = &buf[ndigits];
			if (eflag == 0) p1 += r2;
			*decpt = r2;
			if (p1 < &buf[0]) {
				buf[0] = OS_TEXT('\0');
				return buf;
			}
			while (p <= p1 && p < &buf[CVTBUFSIZE]) {
				arg *= 10;
				arg = modf(arg, &fj);
				*p++ = (int) fj + OS_TEXT('0');
			}
			if (p1 >= &buf[CVTBUFSIZE]) {
				buf[CVTBUFSIZE - 1] = OS_TEXT('\0');
				return buf;
			}
			p = p1;
			*p1 += 5;
			while (*p1 > OS_TEXT('9')) {
				*p1 = OS_TEXT('0');
				if (p1 > buf) {
					++*--p1;
				} else {
					*p1 = OS_TEXT('1');
					(*decpt)++;
					if (eflag == 0) {
						if (p > buf) *p = OS_TEXT('0');
						p++;
					}
				}
			}
			*p = OS_TEXT('\0');
			return buf;
		}

		static OS_CHAR *ecvtbuf(double arg, int ndigits, int *decpt, int *sign, OS_CHAR *buf)
		{
			return cvt(arg, ndigits, decpt, sign, buf, 1);
		}

		static OS_CHAR *fcvtbuf(double arg, int ndigits, int *decpt, int *sign, OS_CHAR *buf)
		{
			return cvt(arg, ndigits, decpt, sign, buf, 0);
		}

		static void cfltcvt(double value, OS_CHAR *buffer, OS_CHAR fmt, int precision)
		{
			int decpt, sign, exp, pos;
			OS_CHAR *digits = NULL;
			OS_CHAR cvtbuf[CVTBUFSIZE];
			int capexp = 0;
			int magnitude;

			if (fmt == OS_TEXT('G') || fmt == OS_TEXT('E')) {
				capexp = 1;
				fmt += OS_TEXT('a') - OS_TEXT('A');
			}

			if (fmt == OS_TEXT('g')) {
				digits = ecvtbuf(value, precision, &decpt, &sign, cvtbuf);
				magnitude = decpt - 1;
				if (magnitude < -4  ||  magnitude > precision - 1) {
					fmt = OS_TEXT('e');
					precision -= 1;
				} else {
					fmt = OS_TEXT('f');
					precision -= decpt;
				}
			}

			if (fmt == OS_TEXT('e')) {
				digits = ecvtbuf(value, precision + 1, &decpt, &sign, cvtbuf);

				if (sign) *buffer++ = '-';
				*buffer++ = *digits;
				if (precision > 0) *buffer++ = '.';
				OS_MEMCPY(buffer, digits + 1, precision);
				buffer += precision;
				*buffer++ = capexp ? OS_TEXT('E') : OS_TEXT('e');

				if (decpt == 0) {
					if (value == 0.0) {
						exp = 0;
					} else {
						exp = -1;
					}
				} else {
					exp = decpt - 1;
				}

				if (exp < 0) {
					*buffer++ = OS_TEXT('-');
					exp = -exp;
				} else {
					*buffer++ = OS_TEXT('+');
				}

				buffer[2] = (exp % 10) + OS_TEXT('0');
				exp = exp / 10;
				buffer[1] = (exp % 10) + OS_TEXT('0');
				exp = exp / 10;
				buffer[0] = (exp % 10) + OS_TEXT('0');
				buffer += 3;
			} else if (fmt == 'f') {
				digits = fcvtbuf(value, precision, &decpt, &sign, cvtbuf);
				if (sign) *buffer++ = '-';
				if (*digits) {
					if (decpt <= 0) {
						*buffer++ = OS_TEXT('0');
						*buffer++ = OS_TEXT('.');
						for (pos = 0; pos < -decpt; pos++) *buffer++ = OS_TEXT('0');
						while (*digits) *buffer++ = *digits++;
					} else {
						pos = 0;
						while (*digits) {
							if (pos++ == decpt) *buffer++ = OS_TEXT('.');
							*buffer++ = *digits++;
						}
					}
				} else {
					*buffer++ = OS_TEXT('0');
					if (precision > 0) {
						*buffer++ = OS_TEXT('.');
						for (pos = 0; pos < precision; pos++) *buffer++ = OS_TEXT('0');
					}
				}
			}

			*buffer = OS_TEXT('\0');
		}

		static void forcdecpt(OS_CHAR *buffer)
		{
			while (*buffer) {
				if (*buffer == OS_TEXT('.')) return;
				if (*buffer == OS_TEXT('e') || *buffer == OS_TEXT('E')) break;
				buffer++;
			}

			if (*buffer) {
				int n = strlen(buffer);
				while (n > 0) {
					buffer[n + 1] = buffer[n];
					n--;
				}

				*buffer = OS_TEXT('.');
			} else {
				*buffer++ = OS_TEXT('.');
				*buffer = OS_TEXT('\0');
			}
		}

		static void cropzeros(OS_CHAR *buffer)
		{
			OS_CHAR *stop;
			while (*buffer && *buffer != OS_TEXT('.')) buffer++;
			if (*buffer++) {
				while (*buffer && *buffer != OS_TEXT('e') && *buffer != OS_TEXT('E')) buffer++;
				stop = buffer--;
				while (*buffer == OS_TEXT('0')) buffer--;
				if (*buffer == '.') buffer--;
				while (*++buffer = *stop++);
			}
		}


		static void flt(OS::Core::Buffer& buf, double num, int size, int precision, OS_CHAR fmt, int flags)
		{
			OS_CHAR tmp[128];
			OS_CHAR c, sign;
			int n, i;

			switch(fmt){
			case OS_TEXT('d'):
			case OS_TEXT('i'):
			case OS_TEXT('n'):
				fmt = OS_TEXT('G');
				break;
			}

			if(precision < 0){
				double p = 10.0;
				for(int i = -precision-1; i > 0; i--){
					p *= 10.0;
				}
				num = ::floor(num / p + 0.5) * p;
				precision = 0;
				// fmt = OS_TEXT('G');
				// flags &= ~PRECISION;
			}

			// Left align means no zero padding
			if (flags & LEFT) flags &= ~ZEROPAD;

			// Determine padding and sign char
			c = (flags & ZEROPAD) ? '0' : ' ';
			sign = 0;
			if(1){ // flags & SIGN) {
				if (num < 0.0) {
					sign = '-';
					num = -num;
					size--;
				} else if (flags & PLUS) {
					sign = '+';
					size--;
				} else if (flags & SPACE) {
					sign = ' ';
					size--;
				}
			}

			// Compute the precision value
			if (precision == 0 && (fmt == 'g' || fmt == 'G')) {
				precision = (flags & PRECISION) ? 1 : 17; // ANSI specified
			}else if(precision == 0 && !(flags & PRECISION)) {
				precision = 6; // Default precision: 6
			}

			// Convert floating point number to text
			cfltcvt(num, tmp, fmt, precision);

			// '#' and precision == 0 means force a decimal point
			if ((flags & SPECIAL) && precision == 0) forcdecpt(tmp);

			// 'g' format means crop zero unless '#' given
			if ((fmt == 'g' || fmt == 'G') && !(flags & SPECIAL)) cropzeros(tmp);

			n = OS_STRLEN(tmp);

			// Output number with alignment and padding
			size -= n;
			if (!(flags & (ZEROPAD | LEFT))) while (size-- > 0) buf.append(OS_TEXT(' '));
			if (sign) buf.append(sign);
			if (!(flags & LEFT)) while (size-- > 0) buf.append(c);
			for (i = 0; i < n; i++) buf.append(tmp[i]);
			while (size-- > 0) buf.append(OS_TEXT(' '));
		}
#else
		static void flt(OS::Core::Buffer& buf, double num, int size, int precision, OS_CHAR fmt, int flags)
		{
			OS_CHAR format[128], tmp[128];
			int i = 0;
			format[i++] = OS_TEXT('%');
			if(flags & LEFT) format[i++] = OS_TEXT('-');
			if(flags & PLUS) format[i++] = OS_TEXT('+');
			if(flags & SPACE) format[i++] = OS_TEXT(' ');
			if(flags & SPECIAL) format[i++] = OS_TEXT('#');
			if(flags & ZEROPAD) format[i++] = OS_TEXT('0');
			if(size >= 0){
				i += OS_SNPRINTF(format + i, sizeof(format) - i*sizeof(OS_CHAR), OS_TEXT("%d"), size);
			}
			if(precision < 0){
				double p = 10.0;
				for(int j = -precision-1; j > 0; j--){
					p *= 10.0;
				}
				num = ::floor(num / p + 0.5) * p;
				format[i++] = OS_TEXT('.');
				format[i++] = OS_TEXT('0');
				precision = 0;
			}else if(flags & PRECISION){
				format[i++] = OS_TEXT('.');
				i += OS_SNPRINTF(format + i, sizeof(format) - i*sizeof(OS_CHAR), OS_TEXT("%d"), precision);
			}
			switch(fmt){
			case OS_TEXT('d'):
			case OS_TEXT('i'):
			case OS_TEXT('n'):
				fmt = OS_TEXT('G');
				break;
			}
			format[i++] = fmt;
			format[i++] = OS_TEXT('\0');
			i = OS_SNPRINTF(tmp, sizeof(tmp), format, num);
			buf.append(tmp, i);
		}
#endif

		static void flt(OS::Core::Buffer& buf, double num)
		{
			OS_CHAR tmp[128];
			int i;
			if(::fabs(num) >= 1.0){
				i = OS_SNPRINTF(tmp, sizeof(tmp), OS_TEXT("%-18f"), num);
				while(i > 1 && tmp[i-1] == OS_TEXT(' ')) tmp[--i] = OS_TEXT('\0');
				for(int j = 1; j < i; j++){
					if(tmp[j] == OS_TEXT('.')){
						while(i > j && tmp[i-1] == OS_TEXT('0')) tmp[--i] = OS_TEXT('\0');
						if(i == j) tmp[--i] = OS_TEXT('\0');
						break;
					}
				}
			}else{
				i = OS_SNPRINTF(tmp, sizeof(tmp), OS_TEXT("%G"), num);
			}
			buf.append(tmp, i);
		}

		static int sprintf(OS * os, int params, int, int, void*)
		{
			if(params < 1) return 0;

			OS::Core::Buffer buf(os);
			String fmt_string = os->toString(-params);
			const OS_CHAR * fmt = fmt_string.toChar();
			const OS_CHAR * fmt_end = fmt + fmt_string.getLen();
			int arg_num = 1;
			for(; fmt < fmt_end; fmt++){
				if(*fmt != OS_TEXT('%')){
					buf.append(*fmt);
					continue;
				}

				// const OS_CHAR * fmt_start = fmt; 
				// Process flags
				int flags = 0;
				for(;;){
					fmt++; // This also skips first '%'
					switch (*fmt) {
					case '-': flags |= LEFT; continue;
					case '+': flags |= PLUS; continue;
					case ' ': flags |= SPACE; continue;
					case '#': flags |= SPECIAL; continue;
					case '0': flags |= ZEROPAD; continue;
					}
					break;
				}

				// Get field width
				int field_width = -1;
				if(OS_IS_ALNUM(*fmt)){
					parseSimpleDec(fmt, field_width);
				}else if(*fmt == OS_TEXT('*')){
					fmt++;
					field_width = arg_num < params ? os->toInt(-params + arg_num++) : 0;
					if(field_width < 0){
						field_width = -field_width;
						flags |= LEFT;
					}
				}

				// Get the precision
				int precision = 0;
				if(*fmt == OS_TEXT('.')){
					fmt++;    
					if(OS_IS_ALNUM(*fmt)){
						parseSimpleDec(fmt, precision);
						flags |= PRECISION;
					}else if(*fmt == OS_TEXT('-') && OS_IS_ALNUM(fmt[1])){
						fmt++;
						parseSimpleDec(fmt, precision);
						precision = -precision;
						flags |= PRECISION;
					}else if(*fmt == OS_TEXT('*')){
						fmt++;
						precision = arg_num < params ? os->toInt(-params + arg_num++) : 0;
						flags |= PRECISION;
					}
					// if(precision < 0) precision = 0;
				}

				// Default base
				int base = 10;
				OS_CHAR fmt_type = *fmt;
				if(fmt_type == OS_TEXT('v')){
					OS_EValueType type = arg_num < params ? os->getType(-params + arg_num) : OS_VALUE_TYPE_NULL;
					switch(type){
					case OS_VALUE_TYPE_NUMBER:
						fmt_type = OS_TEXT('n');
						break;

					default:
						fmt_type = OS_TEXT('s');
						break;
					}
				}
				switch(fmt_type){
				case OS_TEXT('c'):
					if (!(flags & LEFT)) while (--field_width > 0) buf.append(OS_TEXT(' '));
					buf.append(arg_num < params ? toSingleChar(os->toString(-params + arg_num++)) : OS_TEXT(' '));
					while (--field_width > 0) buf.append(OS_TEXT(' '));
					break;

				case OS_TEXT('s'):
					{
						String s = arg_num < params ? os->toString(-params + arg_num++) : String(os);
						int len = s.getLen();
						if((flags & PRECISION) && precision >= 0 && len > precision) len = precision;
						if (!(flags & LEFT)) while (len < field_width--) buf.append(OS_TEXT(' '));
						buf.append(s.toChar(), len);
						while (len < field_width--) buf.append(OS_TEXT(' '));
						break;
					}

				// Integer number formats - set up the flags and "break"
				case OS_TEXT('o'):
					base = 8;
					number(buf, arg_num < params ? os->toInt(-params + arg_num++) : 0, base, field_width, precision, flags);
					break;

				case OS_TEXT('b'):
					base = 2;
					number(buf, arg_num < params ? os->toInt(-params + arg_num++) : 0, base, field_width, precision, flags);
					break;

				case OS_TEXT('X'):
					flags |= LARGE;

				case OS_TEXT('x'):
					base = 16;
					number(buf, arg_num < params ? os->toInt(-params + arg_num++) : 0, base, field_width, precision, flags);
					break;

				case OS_TEXT('d'):
				case OS_TEXT('i'):
					// flags |= SIGN;

				// case 'u':
					number(buf, arg_num < params ? os->toInt(-params + arg_num++) : 0, base, field_width, precision, flags);
					break;

				case OS_TEXT('n'):
					flt(buf, arg_num < params ? os->toDouble(-params + arg_num++) : 0);
					break;

				case OS_TEXT('E'):
				case OS_TEXT('G'):
				case OS_TEXT('e'):
				case OS_TEXT('f'):
				case OS_TEXT('g'):
					flt(buf, arg_num < params ? os->toDouble(-params + arg_num++) : 0, field_width, precision, fmt_type, flags | SIGN);
					break;

				default:
					if(*fmt != '%') buf.append(OS_TEXT('%'));
					if(*fmt){
						buf.append(*fmt);
					}else{
						fmt--;
					}
					break;
				}
			}
			os->pushString(buf);
			return 1;
		}
	};

	struct Lib
	{
		static void echo(OS * os, const OS_CHAR * str)
		{
			os->getGlobal(os->core->strings->func_echo);
			os->pushGlobals();
			os->pushString(str);
			os->call(1);
		}

		static void echo(OS * os, const String& str)
		{
			os->getGlobal(os->core->strings->func_echo);
			os->pushGlobals();
			os->pushString(str);
			os->call(1);
		}

		static int print(OS * os, int params, int, int, void*)
		{
			for(int i = 0; i < params; i++){
				if(i > 0){
					echo(os, OS_TEXT("\t"));
				}
				echo(os, os->toString(-params + i));
			}
			if(params > 0){
				echo(os, OS_TEXT("\n"));
			}
			return 0;
		}

		static int echo(OS * os, int params, int, int, void*)
		{
			for(int i = 0; i < params; i++){
				// fputs(os->toString(-params + i).toChar(), stdout);
				os->echo(os->toString(-params + i).toChar());
			}
			return 0;
		}

		static int printf(OS * os, int params, int, int, void*)
		{
			if(params > 0){
				Format::sprintf(os, params, 0, 0, NULL);
				echo(os, os->toString());
			}
			return 0;
		}

		static int concat(OS * os, int params, int, int, void*)
		{
			if(params == 2){
				os->pushString(os->toString(-params) + os->toString(-params + 1));
				return 1;
			}
			if(params < 1){
				return 0;
			}
			OS::Core::Buffer buf(os);
			for(int i = 0; i < params; i++){
				buf += os->toString(-params + i);
			}
			os->pushString(buf);
			return 1;
		}

		static int compileText(OS * os, int params, int, int need_ret_values, void*)
		{
			if(params < 1){
				return 0;
			}
			String text = os->toString(-params);
			OS_ESourceCodeType source_code_type = OS_SOURCECODE_AUTO;
			if(params >= 2){
				source_code_type = (OS_ESourceCodeType)os->toInt(-params+1);
			}
			bool check_utf8_bom = params >= 3 ? os->toBool(-params+2) : true;
			os->compile(text, source_code_type, check_utf8_bom);
			return 1;
		}

		static int compileFile(OS * os, int params, int, int need_ret_values, void*)
		{
			if(params < 1){
				return 0;
			}
			String filename = os->toString(-params);
			bool required = params >= 2 ? os->toBool(-params+1) : false;
			OS_ESourceCodeType source_code_type = OS_SOURCECODE_AUTO;
			if(params >= 3){
				source_code_type = (OS_ESourceCodeType)os->toInt(-params+2);
			}
			bool check_utf8_bom = params >= 4 ? os->toBool(-params+3) : true;
			os->compileFile(filename, required, source_code_type, check_utf8_bom);
			return 1;
		}

		static int resolvePath(OS * os, int params, int, int, void*)
		{
			if(params < 1){
				return 0;
			}
			String filename = os->resolvePath(os->toString(-params));
			if(filename.getDataSize()){
				os->pushString(filename);
				return 1;
			}
			return 0;
		}

		static int debugBackTrace(OS * os, int params, int, int, void*)
		{
			switch(params){
			case 0:
				os->core->pushBackTrace(0, 10);
				break;

			case 1:
				os->core->pushBackTrace(os->toInt(-params), 10);
				break;

			default:
				os->core->pushBackTrace(os->toInt(-params), os->toInt(-params+1));
			}
			return 1;
		}

		static int terminate(OS * os, int params, int, int, void*)
		{
			os->setTerminated(true, os->toInt(-params));
			return 0;
		}

		static int setErrorHandler(OS * os, int params, int, int, void*)
		{
			switch(params){
			default:
				return 0;

			case 1:
				os->setErrorHandler();
				break;

			case 2:
				os->setErrorHandler(os->popInt());
				break;
			}
			return 1;
		}

		static int triggerError(OS * os, int params, int, int, void*)
		{
			int code = os->toInt(-params, OS_E_ERROR);
			String message = os->toString(-params+1, OS_TEXT("unknown error"));
			os->triggerError(code, message);
			return 0;
		}

		static int deleteOp(OS * os, int params, int, int, void*)
		{
			OS_ASSERT(params == 2);
			os->core->deleteValueProperty(os->core->getStackValue(-params), os->core->getStackValue(-params+1), true, false);
			return 0;
		}

		static int extends(OS * os, int params, int, int, void*)
		{
			OS_ASSERT(params == 2);
			Core::Value right_value = os->core->getStackValue(-params+1);
			switch(OS_VALUE_TYPE(right_value)){
			case OS_VALUE_TYPE_NULL:
				// null value has no prototype
				break;

			case OS_VALUE_TYPE_BOOL:
			case OS_VALUE_TYPE_NUMBER:
				break;

			case OS_VALUE_TYPE_STRING:
			case OS_VALUE_TYPE_ARRAY:
			case OS_VALUE_TYPE_OBJECT:
			case OS_VALUE_TYPE_FUNCTION:
				OS_VALUE_VARIANT(right_value).value->prototype = os->core->getStackValue(-params).getGCValue();
				break;

			case OS_VALUE_TYPE_USERDATA:
			case OS_VALUE_TYPE_USERPTR:
			case OS_VALUE_TYPE_CFUNCTION:
				// TODO: warning???
				break;
			}
			os->core->pushValue(right_value);
			return 1;
		}

		static int in(OS * os, int params, int, int, void*)
		{
			if(params != 2) return 0;
			Core::GCValue * self = os->core->getStackValue(-params+1).getGCValue();
			bool has_property = self && os->core->hasProperty(self, os->core->getStackValue(-params), true, true);
			os->pushBool(has_property);
			return 1;
		}

		static int is(OS * os, int params, int, int, void*)
		{
			if(params != 2) return 0;
			os->pushBool(os->is());
			return 1;
		}

		static int isPrototypeOf(OS * os, int params, int, int, void*)
		{
			if(params != 2) return 0;
			os->pushBool(os->isPrototypeOf());
			return 1;
		}

		static int typeOf(OS * os, int params, int, int, void*)
		{
			if(params < 1) return 0;
			os->core->pushTypeOf(os->core->getStackValue(-params));
			return 1;
		}

		static int numberOf(OS * os, int params, int, int, void*)
		{
			if(params < 1) return 0;
			os->core->pushNumberOf(os->core->getStackValue(-params));
			return 1;
		}

		static int stringOf(OS * os, int params, int, int, void*)
		{
			if(params < 1) return 0;
			os->core->pushStringOf(os->core->getStackValue(-params));
			return 1;
		}

		static int arrayOf(OS * os, int params, int, int, void*)
		{
			if(params < 1) return 0;
			os->core->pushArrayOf(os->core->getStackValue(-params));
			return 1;
		}

		static int objectOf(OS * os, int params, int, int, void*)
		{
			if(params < 1) return 0;
			os->core->pushObjectOf(os->core->getStackValue(-params));
			return 1;
		}

		static int userdataOf(OS * os, int params, int, int, void*)
		{
			if(params < 1) return 0;
			os->core->pushUserdataOf(os->core->getStackValue(-params));
			return 1;
		}

		static int functionOf(OS * os, int params, int, int, void*)
		{
			if(params < 1) return 0;
			os->core->pushFunctionOf(os->core->getStackValue(-params));
			return 1;
		}

		static int toBool(OS * os, int params, int, int, void*)
		{
			if(params < 1) return 0;
			os->toBool(-params);
			return 1;
		}

		static int toNumber(OS * os, int params, int, int, void*)
		{
			if(params < 1) return 0;
			os->pushNumber(os->toNumber(-params, params < 2 || os->toBool(-params+1)));
			return 1;
		}

		static int toString(OS * os, int params, int, int, void*)
		{
			if(params < 1) return 0;
			os->pushString(os->toString(-params, params < 2 || os->toBool(-params+1)));
			return 1;
		}
	};
	FuncDef list[] = {
		{core->strings->func_extends, Lib::extends},
		{core->strings->func_delete, Lib::deleteOp},
		{core->strings->func_in, Lib::in},
		{core->strings->func_is, Lib::is},
		{core->strings->func_isprototypeof, Lib::isPrototypeOf},
		{OS_TEXT("typeOf"), Lib::typeOf},
		// {OS_TEXT("valueOf"), Lib::valueOf},
		{OS_TEXT("numberOf"), Lib::numberOf},
		{OS_TEXT("stringOf"), Lib::stringOf},
		{OS_TEXT("arrayOf"), Lib::arrayOf},
		{OS_TEXT("objectOf"), Lib::objectOf},
		{OS_TEXT("userdataOf"), Lib::userdataOf},
		{OS_TEXT("functionOf"), Lib::functionOf},
		{OS_TEXT("toBool"), Lib::toBool},
		{OS_TEXT("toNumber"), Lib::toNumber},
		{OS_TEXT("toString"), Lib::toString},
		{OS_TEXT("print"), Lib::print},
		{OS_TEXT("echo"), Lib::echo},
		{OS_TEXT("sprintf"), Format::sprintf},
		{OS_TEXT("printf"), Lib::printf},
		{core->strings->func_concat, Lib::concat},
		{OS_TEXT("compileText"), Lib::compileText},
		{OS_TEXT("compileFile"), Lib::compileFile},
		{OS_TEXT("resolvePath"), Lib::resolvePath},
		{OS_TEXT("debugBackTrace"), Lib::debugBackTrace},
		{OS_TEXT("terminate"), Lib::terminate},
		{OS_TEXT("setErrorHandler"), Lib::setErrorHandler},
		{OS_TEXT("triggerError"), Lib::triggerError},
		{}
	};
	NumberDef numbers[] = {
		{OS_TEXT("E_ERROR"), OS_E_ERROR},
		{OS_TEXT("E_WARNING"), OS_E_WARNING},
		{OS_TEXT("E_NOTICE"), OS_E_NOTICE},
		{OS_TEXT("SOURCECODE_AUTO"), OS_SOURCECODE_AUTO},
		{OS_TEXT("SOURCECODE_PLAIN"), OS_SOURCECODE_PLAIN},
		{OS_TEXT("SOURCECODE_TEMPLATE"), OS_SOURCECODE_TEMPLATE},
		{}
	};
	pushGlobals();
	setFuncs(list);
	setNumbers(numbers);
	pop();
}

void OS::initObjectClass()
{
	static int iterator_crc = OS_PTR_HASH(&iterator_crc);
	static int array_iterator_crc = OS_PTR_HASH(&array_iterator_crc);

	struct Object
	{
		static int getProperty(OS * os, int params, int, int, void*)
		{
			bool getter_enabled = false, prototype_enabled = false;
			switch(params){
			case 0:
				break;

			default:
				os->pop(params-3);
				// no break

			case 3:
				prototype_enabled = os->popBool(false);
				// no break

			case 2:
				getter_enabled = os->popBool(false);
				// no break

			case 1:
				os->getProperty(getter_enabled, prototype_enabled);
				return 1;
			}
			return 0;
		}

		static int setProperty(OS * os, int params, int, int, void*)
		{
			bool setter_enabled = false;
			switch(params){
			case 0:
				break;

			default:
				os->pop(params-3);
				// no break

			case 3:
				setter_enabled = os->popBool(false);
				// no break

			case 2:
				os->setProperty(setter_enabled);
			}
			return 0;
		}

		static int getValueId(OS * os, int params, int, int, void*)
		{
			os->pushNumber(os->getValueId(-params-1));
			return 1;
		}

		static int iteratorStep(OS * os, int params, int closure_values, int, void*)
		{
			OS_ASSERT(closure_values == 2);
			Core::Value self_var = os->core->getStackValue(-closure_values + 0);
			void * p = os->toUserdata(iterator_crc, -closure_values + 1);
			Core::Table::IteratorState * iter = (Core::Table::IteratorState*)p;
			if(iter->table){
				Core::GCValue * self = self_var.getGCValue();
				OS_ASSERT(self && iter->table == self->table); (void)self;
				if(iter->prop){
					os->pushBool(true);
					os->core->pushValue(iter->prop->index);
					os->core->pushValue(iter->prop->value);
					iter->prop = iter->ascending ? iter->prop->next : iter->prop->prev;
					return 3;
				}
				iter->table->removeIterator(iter);
			}
			return 0;
		}

		static void iteratorStateDestructor(OS * os, void * data, void * user_param)
		{
			Core::Table::IteratorState * iter = (Core::Table::IteratorState*)data;
			if(iter->table){
				iter->table->removeIterator(iter);
			}
		}

		static int arrayIteratorStep(OS * os, int params, int closure_values, int, void*)
		{
			OS_ASSERT(closure_values == 2);
			Core::Value self_var = os->core->getStackValue(-closure_values + 0);
			int * pi = (int*)os->toUserdata(array_iterator_crc, -closure_values + 1);
			OS_ASSERT(OS_VALUE_TYPE(self_var) == OS_VALUE_TYPE_ARRAY && pi && pi[1]);
			if(pi[0] >= 0 && pi[0] < OS_VALUE_VARIANT(self_var).arr->values.count){
				os->pushBool(true);
				os->pushNumber(pi[0]);
				os->core->pushValue(OS_VALUE_VARIANT(self_var).arr->values[pi[0]]);
				pi[0] += pi[1];
				return 3;
			}
			return 0;
		}

		static int iterator(OS * os, int params, bool ascending)
		{
			Core::Value self_var = os->core->getStackValue(-params-1);
			if(OS_VALUE_TYPE(self_var) == OS_VALUE_TYPE_ARRAY){
				OS_ASSERT(dynamic_cast<Core::GCArrayValue*>(OS_VALUE_VARIANT(self_var).arr));
				os->core->pushValue(self_var);

				int * pi = (int*)os->pushUserdata(array_iterator_crc, sizeof(int)*2);
				OS_ASSERT(pi);
				pi[0] = ascending ? 0 : OS_VALUE_VARIANT(self_var).arr->values.count-1;
				pi[1] = ascending ? 1 : -1;

				os->pushCFunction(arrayIteratorStep, 2);
				return 1;
			}
			Core::GCValue * self = self_var.getGCValue();
			if(self && self->table && self->table->count > 0){
				typedef Core::Table::IteratorState IteratorState;

				os->core->pushValue(self);

				IteratorState * iter = (IteratorState*)os->pushUserdata(iterator_crc, sizeof(IteratorState), iteratorStateDestructor);
				iter->table = NULL;
				iter->next = NULL;
				iter->prop = NULL;
				iter->ascending = ascending;
				self->table->addIterator(iter);

				os->pushCFunction(iteratorStep, 2);
				return 1;
			}
			return 0;
		}

		static int iterator(OS * os, int params, int closure_values, int, void*)
		{
			return iterator(os, params + closure_values, true);
		}

		static int reverseIterator(OS * os, int params, int closure_values, int, void*)
		{
			return iterator(os, params + closure_values, false);
		}

		static int smartSort(OS * os, int params, 
			int(*arrcomp)(OS*, const void*, const void*, void*), 
			int(*objcomp)(OS*, const void*, const void*, void*), void * user_param = NULL)
		{
			Core::Value self_var = os->core->getStackValue(-params-1);
			if(OS_VALUE_TYPE(self_var) == OS_VALUE_TYPE_ARRAY){
				OS_ASSERT(dynamic_cast<Core::GCArrayValue*>(OS_VALUE_VARIANT(self_var).arr));
				if(arrcomp){
					os->core->sortArray(OS_VALUE_VARIANT(self_var).arr, arrcomp, user_param);
				}
				os->core->pushValue(self_var);
				return 1;
			}
			Core::GCValue * self = self_var.getGCValue();
			if(self){
				if(self->table && objcomp){
					os->core->sortTable(self->table, objcomp, user_param);
				}
				os->core->pushValue(self_var);
				return 1;
			}
			return 0;
		}

		static int sort(OS * os, int params, int, int, void*)
		{
			if(params < 1){
				return smartSort(os, params, Core::compareArrayValues, Core::comparePropValues);
			}
			String prop_name(os);
			if(os->core->isValueString(os->core->getStackValue(-params), &prop_name)){
				return smartSort(os, params, NULL, Core::compareObjectProperties, &prop_name);
			}
			return smartSort(os, params, Core::compareUserArrayValues, Core::compareUserPropValues);
		}

		static int length(OS * os, int params, int closure_values, int, void*)
		{
			Core::Value self_var = os->core->getStackValue(-params-closure_values-1);
			if(OS_VALUE_TYPE(self_var) == OS_VALUE_TYPE_ARRAY){
				OS_ASSERT(dynamic_cast<Core::GCArrayValue*>(OS_VALUE_VARIANT(self_var).arr));
				os->pushNumber(OS_VALUE_VARIANT(self_var).arr->values.count);
				return 1;
			}
			Core::GCValue * self = self_var.getGCValue();
			if(self){
				os->pushNumber(self->table ? self->table->count : 0);
				return 1;
			}
			return 0;
		}

		static void appendQuotedString(Core::Buffer& buf, const Core::String& string)
		{
			buf += OS_TEXT("\"");
			int len = string.getLen();
			const OS_CHAR * str = string.toChar();
			for(int i = 0; i < len; i++, str++){
				switch(*str){
				case OS_TEXT('\"'): buf += OS_TEXT("\\\""); continue;
				case OS_TEXT('\r'): buf += OS_TEXT("\\r"); continue;
				case OS_TEXT('\n'): buf += OS_TEXT("\\n"); continue;
				case OS_TEXT('\t'): buf += OS_TEXT("\\t"); continue;
				case OS_TEXT('\\'): buf += OS_TEXT("\\\\"); continue;
				}
				if(*str < OS_TEXT(' ')){
					buf += OS_TEXT("0x");
					buf.append((OS_CHAR)'A' + (((int)*str >> 4) & 0xF));
					buf.append((OS_CHAR)'A' + (((int)*str >> 0) & 0xF));
					continue;
				}
				buf.append(*str);
			}
			buf += OS_TEXT("\"");
		}

		static int valueOf(OS * os, int params, int closure_values, int, void*)
		{
			// allow usage with parameter valueOf(v)
 			Core::Value self_var = os->core->getStackValue(-params-closure_values-1 + (params > 0));
			switch(OS_VALUE_TYPE(self_var)){
			case OS_VALUE_TYPE_NULL:
				os->pushString(os->core->strings->typeof_null);
				return 1;

			case OS_VALUE_TYPE_BOOL:
				os->pushString(OS_VALUE_VARIANT(self_var).boolean ? os->core->strings->syntax_true : os->core->strings->syntax_false);
				return 1;

			case OS_VALUE_TYPE_NUMBER:
			case OS_VALUE_TYPE_STRING:
				os->core->pushValue(self_var);
				return 1;
			}
			Core::GCValue * self = self_var.getGCValue();
			if(!self){
				return 0;
			}
			Core::Buffer buf(os);
			switch(self->type){
			case OS_VALUE_TYPE_ARRAY:
				{
					OS_ASSERT(dynamic_cast<Core::GCArrayValue*>(self));
					Core::GCArrayValue * arr = (Core::GCArrayValue*)self;
					// Core::Buffer buf(os);
					buf += OS_TEXT("[");
					Core::Value temp;
					for(int i = 0; i < arr->values.count; i++){
						if(i > 0){
							buf += OS_TEXT(",");
						}
						Core::Value value = arr->values[i];
						if(os->core->getPropertyValue(temp, os->core->check_recursion, value, false)){
							buf += OS_TEXT("<<RECURSION>>");
							continue;
						}
						Core::GCValue * gcvalue = value.getGCValue();
						if(gcvalue && gcvalue->table && gcvalue->table->count){
							os->core->setPropertyValue(os->core->check_recursion, value, Core::Value(true), false);
						}
						Core::String value_str = os->core->valueToString(value, true);
						if(OS_VALUE_TYPE(value) == OS_VALUE_TYPE_STRING){
							appendQuotedString(buf, value_str);
						}else{
							buf += value_str;
						}
					}
					buf += OS_TEXT("]");
					os->pushString(buf);
					return 1;
				}

			case OS_VALUE_TYPE_USERDATA:
			case OS_VALUE_TYPE_USERPTR:
				buf += OS_TEXT("<");
				buf += os->core->strings->typeof_userdata;
				buf += OS_TEXT(":");
				buf += Core::String(os, (OS_INT)self->value_id);
				buf += OS_TEXT(">");
				if(!self->table || !self->table->count){
					os->pushString(buf);
					return 1;
				}
				// buf += OS_TEXT(" ");
				goto dump_object;

			case OS_VALUE_TYPE_FUNCTION:
			case OS_VALUE_TYPE_CFUNCTION:
				buf += OS_TEXT("<");
				buf += os->core->strings->typeof_function;
				buf += OS_TEXT(":");
				buf += Core::String(os, (OS_INT)self->value_id);
				buf += OS_TEXT(">");
				if(!self->table || !self->table->count){
					os->pushString(buf);
					return 1;
				}
				// buf += OS_TEXT(" ");
				goto dump_object;

			case OS_VALUE_TYPE_OBJECT:
				if(!self->table || !self->table->count){
					os->pushString(OS_TEXT("{}"));
					return 1;
				}
				{
dump_object:
					buf += OS_TEXT("{");
					int need_index = 0;
					Core::Property * prop = self->table->first;
					Core::Value temp;
					for(int i = 0; prop; prop = prop->next, i++){
						if(i > 0){
							buf += OS_TEXT(",");
						}
						if(OS_VALUE_TYPE(prop->index) == OS_VALUE_TYPE_NUMBER){
							if(OS_VALUE_NUMBER(prop->index) != (OS_FLOAT)need_index){
								buf += String(os, (OS_FLOAT)OS_VALUE_NUMBER(prop->index), OS_AUTO_PRECISION);
								buf += OS_TEXT(":");
							}
							need_index = (int)(OS_VALUE_NUMBER(prop->index) + 1);
						}else if(OS_VALUE_TYPE(prop->index) == OS_VALUE_TYPE_STRING){
							OS_ASSERT(!OS_VALUE_VARIANT(prop->index).string->table);
							appendQuotedString(buf, os->core->valueToString(prop->index));
							buf += OS_TEXT(":");
						}else{
							Core::GCValue * gcvalue = prop->index.getGCValue();
							if(os->core->getPropertyValue(temp, os->core->check_recursion, prop->index, false)){
								buf += OS_TEXT("<<RECURSION>>");
							}else{
								if(gcvalue && gcvalue->table && gcvalue->table->count){
									os->core->setPropertyValue(os->core->check_recursion, prop->index, Core::Value(true), false);
								}
								buf += os->core->valueToString(prop->index, true);
							}
							buf += OS_TEXT(":");
						}

						if(os->core->getPropertyValue(temp, os->core->check_recursion, prop->value, false)){
							buf += OS_TEXT("<<RECURSION>>");
							continue;
						}
						Core::GCValue * gcvalue = prop->value.getGCValue();
						if(gcvalue && gcvalue->table && gcvalue->table->count){
							os->core->setPropertyValue(os->core->check_recursion, prop->value, Core::Value(true), false);
						}

						Core::String value_str = os->core->valueToString(prop->value, true);
						if(OS_VALUE_TYPE(prop->value) == OS_VALUE_TYPE_STRING){
							appendQuotedString(buf, value_str);
						}else{
							buf += value_str;
						}
					}
					os->pushString(buf += OS_TEXT("}"));
					return 1;
				}
			}
			return 0;
		}

		static int push(OS * os, int params, int, int, void*)
		{
			Core::Value self_var = os->core->getStackValue(-params-1);
			Core::Value value = os->core->getStackValue(-params);
			OS_INT num_index = 0;
			switch(OS_VALUE_TYPE(self_var)){
			case OS_VALUE_TYPE_ARRAY:
				OS_ASSERT(dynamic_cast<Core::GCArrayValue*>(OS_VALUE_VARIANT(self_var).arr));
				os->vectorAddItem(OS_VALUE_VARIANT(self_var).arr->values, value OS_DBG_FILEPOS);
				os->core->pushValue(value);
				return 1;

			case OS_VALUE_TYPE_OBJECT:
			case OS_VALUE_TYPE_USERDATA:
			case OS_VALUE_TYPE_USERPTR:
			case OS_VALUE_TYPE_FUNCTION:
			case OS_VALUE_TYPE_CFUNCTION:
				num_index = OS_VALUE_VARIANT(self_var).object->table ? OS_VALUE_VARIANT(self_var).object->table->next_index : 0;
				break;

			default:
				return 0;
			}
			os->core->setPropertyValue(self_var, Core::PropertyIndex(num_index), value, false);
			// os->pushNumber(self_var.v.object->table->count);
			os->core->pushValue(value);
			return 1;
		}

		static int pop(OS * os, int params, int, int, void*)
		{
			Core::Value self_var = os->core->getStackValue(-params-1);
			switch(OS_VALUE_TYPE(self_var)){
			case OS_VALUE_TYPE_ARRAY:
				OS_ASSERT(dynamic_cast<Core::GCArrayValue*>(OS_VALUE_VARIANT(self_var).arr));
				if(OS_VALUE_VARIANT(self_var).arr->values.count > 0){
					os->core->pushValue(OS_VALUE_VARIANT(self_var).arr->values.lastElement());
					os->vectorRemoveAtIndex(OS_VALUE_VARIANT(self_var).arr->values, OS_VALUE_VARIANT(self_var).arr->values.count-1);
					return 1;
				}
				return 0;

			case OS_VALUE_TYPE_OBJECT:
			case OS_VALUE_TYPE_USERDATA:
			case OS_VALUE_TYPE_USERPTR:
			case OS_VALUE_TYPE_FUNCTION:
			case OS_VALUE_TYPE_CFUNCTION:
				if(OS_VALUE_VARIANT(self_var).object->table && OS_VALUE_VARIANT(self_var).object->table->count > 0){
					os->core->pushValue(OS_VALUE_VARIANT(self_var).object->table->last->value);
					Core::PropertyIndex index = *OS_VALUE_VARIANT(self_var).object->table->last;
					os->core->deleteValueProperty(OS_VALUE_VARIANT(self_var).object, index, false, false);
					return 1;
				}
				break;
			}
			return 0;
		}

		static int hasOwnProperty(OS * os, int params, int, int, void*)
		{
			Core::Value self_var = os->core->getStackValue(-params-1);
			Core::Value index = os->core->getStackValue(-params);
			Core::GCValue * self = self_var.getGCValue();
			if(self){
				os->pushBool( os->core->hasProperty(self, index, true, false) );
				return 1;
			}
			return 0;
		}

		static int hasProperty(OS * os, int params, int, int, void*)
		{
			Core::Value self_var = os->core->getStackValue(-params-1);
			Core::Value index = os->core->getStackValue(-params);
			Core::GCValue * self = self_var.getGCValue();
			if(self){
				os->pushBool( os->core->hasProperty(self, index, true, true) );
				return 1;
			}
			return 0;
		}

		static int hasProperties(OS * os, int params, int, int, void*)
		{
			Core::Value self_var = os->core->getStackValue(-params-1);
			Core::GCValue * self = self_var.getGCValue();
			if(self){
				os->pushBool(self->table && self->table->count > 0);
				return 1;
			}
			return 0;
		}

		static int sub(OS * os, int params, int, int, void*)
		{
			int start, len, size;
			Core::Value self_var = os->core->getStackValue(-params-1);
			switch(OS_VALUE_TYPE(self_var)){
			case OS_VALUE_TYPE_OBJECT:
				OS_ASSERT(dynamic_cast<Core::GCObjectValue*>(OS_VALUE_VARIANT(self_var).object));
				size = OS_VALUE_VARIANT(self_var).object->table ? OS_VALUE_VARIANT(self_var).object->table->count : 0;
				break;

			default:
				return 0;
			}
			switch(params){
			case 0:
				os->core->pushValue(self_var);
				return 1;

			case 1:
				start = os->toInt(-params);
				len = size;
				break;

			default:
				start = os->toInt(-params);
				len = os->toInt(-params+1);
			}
			if(start < 0){
				start = size + start;
				if(start < 0){
					start = 0;
				}
			}
			if(start >= size){
				os->newObject();
				return 1;
			}
			if(len < 0){
				len = size - start + len;
			}
			if(len <= 0){
				os->newObject();
				return 1;
			}
			if(start + len > size){
				len = size - start;
			}
			if(!start && len == size){
				os->core->pushValue(self_var);
				return 1;
			}
			OS_ASSERT(OS_VALUE_VARIANT(self_var).object->table && OS_VALUE_VARIANT(self_var).object->table->first);
			Core::GCObjectValue * object = os->core->pushObjectValue(OS_VALUE_VARIANT(self_var).object->prototype);
			Core::Property * prop = OS_VALUE_VARIANT(self_var).object->table->first;
			int i = 0;
			for(; i < start; i++){
				prop = prop->next;
				OS_ASSERT(prop);
			}
			Vector<Core::Value> captured_items;
			os->vectorReserveCapacity(captured_items, len*2 OS_DBG_FILEPOS);
			for(i = 0; i < len; i++, prop = prop->next){
				OS_ASSERT(prop);
				os->vectorAddItem(captured_items, prop->index OS_DBG_FILEPOS);
				os->vectorAddItem(captured_items, prop->value OS_DBG_FILEPOS);
			}
			for(i = 0; i < len; i++){
				os->core->setPropertyValue(object, captured_items[i*2], captured_items[i*2+1], false);
			}
			os->vectorClear(captured_items);
			return 1;
		}

		static int merge(OS * os, int params, int, int, void*)
		{
			if(params < 1) return 0;
			int offs = os->getAbsoluteOffs(-params);
			bool is_array = os->isArray(offs-1);
			if(is_array || os->isObject(offs-1)){
				for(int i = 0; i < params; i++){
					Core::Value value = os->core->getStackValue(offs+i);
					switch(OS_VALUE_TYPE(value)){
					case OS_VALUE_TYPE_ARRAY:
						{
							OS_ASSERT(dynamic_cast<Core::GCArrayValue*>(OS_VALUE_VARIANT(value).arr));
							for(int j = 0; j < OS_VALUE_VARIANT(value).arr->values.count; j++){
								os->pushStackValue(offs-1);
								os->core->pushValue(OS_VALUE_VARIANT(value).arr->values[j]);
								os->addProperty();
							}
							break;
						}

					case OS_VALUE_TYPE_OBJECT:
						{
							OS_ASSERT(dynamic_cast<Core::GCObjectValue*>(OS_VALUE_VARIANT(value).object));
							if(OS_VALUE_VARIANT(value).object->table){
								Core::Property * prop = OS_VALUE_VARIANT(value).object->table->first;
								for(; prop; prop = prop->next){
									os->pushStackValue(offs-1);
									if(is_array){
										os->core->pushValue(prop->value);
										os->addProperty();
									}else{
										os->core->pushValue(prop->index);
										os->core->pushValue(prop->value);
										os->setProperty();
									}
								}
							}
							break;
						}
					}
				}
				os->pushStackValue(offs-1);
				return 1;
			}
			return 0;
		}

		static int getKeys(OS * os, int params, int, int, void*)
		{
			Core::Value value = os->core->getStackValue(-params-1);
			switch(OS_VALUE_TYPE(value)){
			case OS_VALUE_TYPE_ARRAY:
				{
					Core::GCArrayValue * arr = os->core->pushArrayValue(OS_VALUE_VARIANT(value).arr->values.count);
					for(int i = 0; i < OS_VALUE_VARIANT(value).arr->values.count; i++){
						os->vectorAddItem(arr->values, Core::Value(i) OS_DBG_FILEPOS);
					}
					return 1;
				}

			case OS_VALUE_TYPE_OBJECT:
				{
					if(OS_VALUE_VARIANT(value).object->table){
						Core::GCArrayValue * arr = os->core->pushArrayValue(OS_VALUE_VARIANT(value).object->table->count);
						Core::Property * prop = OS_VALUE_VARIANT(value).object->table->first;
						for(int i = 0; prop; prop = prop->next, i++){
							os->vectorAddItem(arr->values, prop->index OS_DBG_FILEPOS);
						}
					}else{
						os->newArray();
					}
					return 1;
				}
			}
			return 0;
		}

		static int getValues(OS * os, int params, int, int, void*)
		{
			Core::Value value = os->core->getStackValue(-params-1);
			switch(OS_VALUE_TYPE(value)){
			case OS_VALUE_TYPE_ARRAY:
				{
					Core::GCArrayValue * arr = os->core->pushArrayValue(OS_VALUE_VARIANT(value).arr->values.count);
					for(int i = 0; i < OS_VALUE_VARIANT(value).arr->values.count; i++){
						os->vectorAddItem(arr->values, OS_VALUE_VARIANT(value).arr->values[i] OS_DBG_FILEPOS);
					}
					return 1;
				}

			case OS_VALUE_TYPE_OBJECT:
				{
					if(OS_VALUE_VARIANT(value).object->table){
						Core::GCArrayValue * arr = os->core->pushArrayValue(OS_VALUE_VARIANT(value).object->table->count);
						Core::Property * prop = OS_VALUE_VARIANT(value).object->table->first;
						for(int i = 0; prop; prop = prop->next, i++){
							os->vectorAddItem(arr->values, prop->value OS_DBG_FILEPOS);
						}
					}else{
						os->newArray();
					}
					return 1;
				}
			}
			return 0;
		}


		static int join(OS * os, int params, int, int, void*)
		{
			Core::Buffer buf(os);
			String str = params >= 1 ? os->toString(-params+0) : String(os);
			Core::Value self_var = os->core->getStackValue(-params-1);
			switch(OS_VALUE_TYPE(self_var)){
			case OS_VALUE_TYPE_ARRAY:
				for(int i = 0; i < OS_VALUE_VARIANT(self_var).arr->values.count; i++){
					if(i > 0){
						buf.append(str);
					}
					buf.append(os->core->valueToString(OS_VALUE_VARIANT(self_var).arr->values[i], true));
				}
				os->pushString(buf);
				return 1;

			case OS_VALUE_TYPE_OBJECT:
				if(OS_VALUE_VARIANT(self_var).object->table){
					Core::Property * prop = OS_VALUE_VARIANT(self_var).object->table->first;
					for(int i = 0; prop; prop = prop->next, i++){
						if(i > 0){
							buf.append(str);
						}
						buf.append(os->core->valueToString(prop->value, true));
					}
				}
				os->pushString(buf);
				return 1;
			}
			return 0;
		}

		static int clone(OS * os, int params, int, int, void*)
		{
			Core::Value val = os->core->getStackValue(-params-1);
			Core::GCValue * value, * new_value;
			switch(OS_VALUE_TYPE(val)){
			case OS_VALUE_TYPE_NULL:
			case OS_VALUE_TYPE_BOOL:
			case OS_VALUE_TYPE_NUMBER:
			case OS_VALUE_TYPE_STRING:
				os->core->pushValue(val);
				return 1;

			case OS_VALUE_TYPE_ARRAY:
				{
					OS_ASSERT(dynamic_cast<Core::GCArrayValue*>(OS_VALUE_VARIANT(val).value));
					value = OS_VALUE_VARIANT(val).value;
					Core::GCArrayValue * arr = (Core::GCArrayValue*)value;
					new_value = os->core->pushArrayValue(arr->values.count);
					new_value->prototype = value->prototype;
					Core::GCArrayValue * new_arr = (Core::GCArrayValue*)new_value;
					OS_MEMCPY(new_arr->values.buf, arr->values.buf, sizeof(Core::Value)*arr->values.count);
					new_arr->values.count = arr->values.count;
					break;
				}

			case OS_VALUE_TYPE_OBJECT:
				value = OS_VALUE_VARIANT(val).value;
				new_value = os->core->pushObjectValue(value->prototype);
				break;

			case OS_VALUE_TYPE_FUNCTION:
			case OS_VALUE_TYPE_USERDATA:
			case OS_VALUE_TYPE_USERPTR:
			case OS_VALUE_TYPE_CFUNCTION:
				value = OS_VALUE_VARIANT(val).value;
				new_value = os->core->pushValue(value);
				break;

			default:
				os->pushNull();
				return 1;
			}
			OS_ASSERT(new_value->type != OS_VALUE_TYPE_NULL);
			if(new_value != value && value->table && value->table->count > 0){
				new_value->table = os->core->newTable(OS_DBG_FILEPOS_START);
				os->core->copyTableProperties(new_value->table, value->table);
			}
			return 1;
		}

		static int cmp(OS * os, int params, int, int, void*)
		{
			if(params < 1) return 0;
			Core::Value left_value = os->core->getStackValue(-params - 1);
			Core::Value right_value = os->core->getStackValue(-params + 0);
			switch(OS_VALUE_TYPE(left_value)){
			case OS_VALUE_TYPE_NULL:
			case OS_VALUE_TYPE_BOOL:
			case OS_VALUE_TYPE_NUMBER:
			// case OS_VALUE_TYPE_STRING:
				switch(OS_VALUE_TYPE(right_value)){
				case OS_VALUE_TYPE_NULL:
				case OS_VALUE_TYPE_BOOL:
				case OS_VALUE_TYPE_NUMBER:
				// case OS_VALUE_TYPE_STRING:
					os->core->pushOpResultValue(Core::OP_COMPARE, left_value, right_value);
					return 1;
				}
				break;

			default:
				if(OS_VALUE_TYPE(left_value) == OS_VALUE_TYPE(right_value)){
					os->pushNumber(OS_VALUE_VARIANT(left_value).value > OS_VALUE_VARIANT(right_value).value ? 1 
						: (OS_VALUE_VARIANT(left_value).value < OS_VALUE_VARIANT(right_value).value ? -1 : 0));
					return 1;
				}
				break;
			}
			os->triggerError(String::format(os, OS_TEXT("attempt to compare '%s' with '%s'"), os->getTypeStr(-params + 0).toChar(), os->getTypeStr(-params + 1).toChar()));
			return 0;
		}
	};
	FuncDef list[] = {
		{core->strings->__cmp, Object::cmp},
		{OS_TEXT("getProperty"), Object::getProperty},
		{OS_TEXT("setProperty"), Object::setProperty},
		{OS_TEXT("__get@osValueId"), Object::getValueId},
		{core->strings->__len, Object::length},
		{core->strings->__iter, Object::iterator},
		{OS_TEXT("reverseIter"), Object::reverseIterator},
		{core->strings->func_valueOf, Object::valueOf},
		{core->strings->func_clone, Object::clone},
		{OS_TEXT("sort"), Object::sort},
		{core->strings->func_push, Object::push},
		{OS_TEXT("pop"), Object::pop},
		{OS_TEXT("hasOwnProperty"), Object::hasOwnProperty},
		{OS_TEXT("hasProperty"), Object::hasProperty},
		{OS_TEXT("hasProperties"), Object::hasProperties},
		{OS_TEXT("merge"), Object::merge},
		{OS_TEXT("join"), Object::join},
		{OS_TEXT("__get@keys"), Object::getKeys},
		{OS_TEXT("__get@values"), Object::getValues},
		{}
	};
	core->pushValue(core->prototypes[Core::PROTOTYPE_OBJECT]);
	setFuncs(list);
	pop();
}

void OS::initArrayClass()
{
	struct Array
	{
		static int sub(OS * os, int params, int, int, void*)
		{
			int start, len, size;
			Core::Value self_var = os->core->getStackValue(-params-1);
			switch(OS_VALUE_TYPE(self_var)){
			case OS_VALUE_TYPE_ARRAY:
				OS_ASSERT(dynamic_cast<Core::GCArrayValue*>(OS_VALUE_VARIANT(self_var).arr));
				size = OS_VALUE_VARIANT(self_var).arr->values.count;
				break;

			default:
				return 0;
			}
			switch(params){
			case 0:
				os->core->pushValue(self_var);
				return 1;

			case 1:
				start = os->toInt(-params);
				len = size;
				break;

			default:
				start = os->toInt(-params);
				len = os->toInt(-params+1);
			}
			if(start < 0){
				start = size + start;
				if(start < 0){
					start = 0;
				}
			}
			if(start >= size){
				os->newArray();
				return 1;
			}
			if(len < 0){
				len = size - start + len;
			}
			if(len <= 0){
				os->newArray();
				return 1;
			}
			if(start + len > size){
				len = size - start;
			}
			if(!start && len == size){
				os->core->pushValue(self_var);
				return 1;
			}
			Core::GCArrayValue * arr = os->core->pushArrayValue(len);
			for(int i = 0; i < len; i++){
				os->vectorAddItem(arr->values, OS_VALUE_VARIANT(self_var).arr->values[start+i] OS_DBG_FILEPOS);
			}
			return 1;
		}
	};
	FuncDef list[] = {
		{OS_TEXT("sub"), Array::sub},
		{}
	};
	core->pushValue(core->prototypes[Core::PROTOTYPE_ARRAY]);
	setFuncs(list);
	pop();
}

namespace ObjectScript {

template <> struct CtypeName<OS::Core::Buffer>{ static const OS_CHAR * getName(){ return OS_TEXT("Buffer"); } };
template <> struct CtypeValue<OS::Core::Buffer*>: public CtypeUserClass<OS::CoreBuffer*>{};
template <> struct UserDataDestructor<OS::Core::Buffer>
{
	static void dtor(ObjectScript::OS * os, void * data, void * user_param)
	{
		OS_ASSERT(data && dynamic_cast<OS::Core::Buffer*>((OS::Core::Buffer*)data));
		OS::Core::Buffer * buf = (OS::Core::Buffer*)data;
		buf->~Buffer();
		os->free(buf);
	}
};

} // namespace ObjectScript

void OS::initBufferClass()
{
	struct Lib
	{
		static int __construct(OS * os, int params, int, int, void * user_param)
		{
			Core::Buffer * self = new (os->malloc(sizeof(Core::Buffer) OS_DBG_FILEPOS)) Core::Buffer(os);
			for(int i = 0; i < params; i++){
				self->append(os->toString(-params + i));
			}
			CtypeValue<Core::Buffer*>::push(os, self);
			return 1;
		}

		static int append(OS * os, int params, int, int, void * user_param)
		{
			OS_GET_SELF(Core::Buffer*);
			for(int i = 0; i < params; i++){
				self->append(os->toString(-params + i));
			}
			CtypeValue<Core::Buffer*>::push(os, self);
			return 1;
		}

		static int valueOf(OS * os, int params, int, int, void * user_param)
		{
			if(os->isUserdata(CtypeId<Core::Buffer>::getId(), -params-1)){
				int offs = os->getAbsoluteOffs(-params-1);
				os->pushStackValue(offs);
				os->getPrototype();
				os->getProperty(os->core->strings->func_valueOf);
				os->pushStackValue(offs);
				return os->call(0, 1);
			}
			OS_GET_SELF(Core::Buffer*);
			os->pushString(self->toString());
			return 1;
		}

		static int len(OS * os, int params, int, int, void * user_param)
		{
			OS_GET_SELF(Core::Buffer*);
			os->pushNumber(self->buffer.count);
			return 1;
		}
	};

	OS::FuncDef funcs[] = {
		{OS_TEXT("__construct"), Lib::__construct},
		{OS_TEXT("append"), Lib::append},
		{core->strings->func_valueOf, Lib::valueOf},
		{core->strings->__len, Lib::len},
		// {OS_TEXT("printf"), Lib::printf},
		{}
	};
	registerUserClass<Core::Buffer>(this, funcs);

	getGlobal(CtypeName<Core::Buffer>::getName());
	core->pushValue(core->prototypes[Core::PROTOTYPE_STRING]);
	setPrototype(CtypeId<Core::Buffer>::getId());
}

void OS::initStringClass()
{
	struct String
	{
		static int length(OS * os, int params, int, int, void*)
		{
			os->pushNumber(os->toString(-params-1).getLen());
			return 1;
		}

		static int sub(OS * os, int params, int, int, void*)
		{
			int start, len;
			OS::String str = os->toString(-params-1);
			int size = str.getLen();
			switch(params){
			case 0:
				os->pushStackValue(-params-1);
				return 1;

			case 1:
				start = os->toInt(-params);
				len = size;
				break;

			default:
				start = os->toInt(-params);
				len = os->toInt(-params+1);
			}
			if(start < 0){
				start = size + start;
				if(start < 0){
					start = 0;
				}
			}
			if(start >= size){
				os->pushString(OS_TEXT(""));
				return 1;
			}
			if(len < 0){
				len = size - start + len;
			}
			if(len <= 0){
				os->pushString(OS_TEXT(""));
				return 1;
			}
			if(start + len > size){
				len = size - start;
			}
			if(!start && len == size){
				os->pushStackValue(-params-1);
				return 1;
			}
			os->pushString(str.toChar() + start, len);
			return 1;
		}

		static int replace(OS * os, int params, int, int, void*)
		{
			OS::String subject = os->toString(-params-1);
			int subject_len = subject.getLen();
			if(params >= 1){
				OS::String search = os->toString(-params);
				int search_len = search.getLen();
				if(search_len > 0 && search_len <= subject_len){
					OS::String replace = params >= 2 ? os->toString(-params+1) : OS::String(os);
					int replace_len = replace.getLen();

					const OS_CHAR * subject_str = subject.toChar();
					const OS_CHAR * search_str = search.toChar();
					
					OS::Core::Buffer buf(os);
					bool found = false;
					int start = 0;
					for(int i = 0; i < subject_len-search_len+1;){
						if(OS_MEMCMP(subject_str + i, search_str, sizeof(OS_CHAR)*search_len) == 0){
							buf.append(subject_str + start, i - start);
							buf.append(replace, replace_len);
							i += search_len;
							start = i;
							found = true;
						}else{
							i++;
						}
					}
					if(found){
						buf.append(subject_str + start, subject_len - start);
						os->pushString(buf);
						return 1;
					}
				}
			}
			os->pushString(subject);
			return 1;
		}

		static int find(OS * os, int params, int, int, void*)
		{
			OS::String subject = os->toString(-params-1);
			int subject_len = subject.getLen();
			if(params >= 1){
				OS::String search = os->toString(-params);
				int search_len = search.getLen();
				if(search_len > 0 && search_len <= subject_len){
					const OS_CHAR * subject_str = subject.toChar();
					const OS_CHAR * search_str = search.toChar();
					int end = subject_len - search_len;
					int i = params >= 2 ? os->toInt(-params+1) : 0;
					for(; i <= end;){
						if(OS_MEMCMP(subject_str + i, search_str, sizeof(OS_CHAR)*search_len) == 0){
							os->pushNumber(i);
							return 1;
						}
					}
				}
			}
			return 0;
		}

		static int cmp(OS * os, int params, int, int, void*)
		{
			if(params < 1) return 0;
			OS::String left = os->toString(-params - 1);
			OS::String right = os->toString(-params + 0);
			os->pushNumber(left.cmp(right));
			return 1;
		}

		static int trim(OS * os, int params, int, int, void*)
		{
			OS::String str = os->toString(-params - 1); // this
			bool trim_left = params >= 1 ? os->toBool(-params + 0) : true;
			os->pushString(str.trim(trim_left, params >= 2 ? os->toBool(-params + 1) : trim_left));
			return 1;
		}
		
		static int upper(OS * os, int params, int, int, void*)
		{
			OS::String self = os->toString(-params - 1); // this
			const OS_CHAR * str = self.toChar();
			int len = self.getLen();
			for(int i = 0; i < len; i++){
				OS_CHAR c = OS_CHAR_UPPER(str[i]);
				if(c != str[i]){
					OS::Core::Buffer buf(os);
					buf.append(str, i);
					buf.append(c);
					for(i++; i < len; i++){
						buf.append(OS_CHAR_UPPER(str[i]));
					}
					os->pushString(buf);
					return 1;
				}
			}
			os->pushString(self);
			return 1;
		}
		
		static int lower(OS * os, int params, int, int, void*)
		{
			OS::String self = os->toString(-params - 1); // this
			const OS_CHAR * str = self.toChar();
			int len = self.getLen();
			for(int i = 0; i < len; i++){
				OS_CHAR c = OS_CHAR_LOWER(str[i]);
				if(c != str[i]){
					OS::Core::Buffer buf(os);
					buf.append(str, i);
					buf.append(c);
					for(i++; i < len; i++){
						buf.append(OS_CHAR_LOWER(str[i]));
					}
					os->pushString(buf);
					return 1;
				}
			}
			os->pushString(self);
			return 1;
		}
		
		static int split(OS * os, int params, int, int, void*)
		{
			int offs = os->getAbsoluteOffs(-params-1);
			
			os->newArray();
			int count = 0;
			
			OS::String subject = os->toString(offs);
			int subject_len = subject.getLen();
			if(params >= 1){
				OS::String search = os->toString(offs+1);
				int search_len = search.getLen();
				if(search_len > 0 && search_len <= subject_len){
					const OS_CHAR * subject_str = subject.toChar();
					const OS_CHAR * search_str = search.toChar();
					
					int max_count = params >= 2 ? os->toInt(offs+2) : INT_MAX;
					if(max_count > 0){
						bool found = false;
						int start = 0;
						for(int i = 0; i < subject_len-search_len+1;){
							if(OS_MEMCMP(subject_str + i, search_str, sizeof(OS_CHAR)*search_len) == 0){
								os->pushStackValue();
								os->pushNumber(count++);
								os->pushString(subject_str + start, i - start);
								os->setProperty();
								i += search_len;
								start = i;
								found = true;
								if(--max_count == 0){
									break;
								}
							}else{
								i++;
							}
						}
						if(found){
							os->pushStackValue();
							os->pushNumber(count++);
							os->pushString(subject_str + start, subject_len - start);
							os->setProperty();
							return 1;
						}
					}
				}
			}
			os->pushStackValue();
			os->pushNumber(0);
			os->pushString(subject);
			os->setProperty();
			return 1;
		}
	};
	FuncDef list[] = {
		{core->strings->__cmp, String::cmp},
		{core->strings->__len, String::length},
		{OS_TEXT("sub"), String::sub},
		{OS_TEXT("find"), String::find},
		{OS_TEXT("replace"), String::replace},
		{OS_TEXT("trim"), String::trim},
		{OS_TEXT("upper"), String::upper},
		{OS_TEXT("lower"), String::lower},
		{OS_TEXT("split"), String::split},
		{}
	};
	core->pushValue(core->prototypes[Core::PROTOTYPE_STRING]);
	setFuncs(list);
	pop();
}

OS::Core::File::File(OS * p_os)
{
	os = p_os; // ->retain();
	f = NULL;
}

OS::Core::File::~File()
{
	close();
	// os->release();
}

bool OS::Core::File::open(const OS_CHAR * filename, const OS_CHAR * mode)
{
	close();
	f = os->openFile(filename, mode);
	return isOpen();
}

void OS::Core::File::close()
{
	os->closeFile(f);
	f = NULL;
}

int OS::Core::File::getSize() const
{
	return os->getFileSize(f);
}

int OS::Core::File::getPos() const
{
	return os->seekFile(f, 0, SEEK_CUR);
}

void OS::Core::File::setPos(int pos)
{
	os->seekFile(f, pos, SEEK_SET);
}

bool OS::Core::File::isOpen() const
{
	return f != NULL;
}

OS::Core::String OS::Core::File::read()
{
	return read(getSize() - getPos());
}

OS::Core::String OS::Core::File::read(int len)
{
	if(!f || len <= 0){
		return OS::Core::String(os);
	}

	Core::Buffer buf(os);
	buf.reserveCapacity(len);
	int read_len = os->readFile(buf.buffer.buf, len, f);
	if(read_len == 0){
		// TODO:: error ?!
	}
	buf.buffer.count = read_len;
	return buf.toString();
}

int OS::Core::File::write(const void * data, int len)
{
	return os->writeFile(data, len, f);
}

int OS::Core::File::write(const Core::String& str)
{
	return write(str.toChar(), str.getLen() * sizeof(OS_CHAR));
}

namespace ObjectScript {

template <> struct CtypeName<OS::Core::File>{ static const OS_CHAR * getName(){ return OS_TEXT("File"); } };
template <> struct CtypeValue<OS::Core::File*>: public CtypeUserClass<OS::CoreFile*>{};
template <> struct UserDataDestructor<OS::Core::File>
{ 
	static void dtor(ObjectScript::OS * os, void * data, void * user_param)
	{
		OS_ASSERT(data && dynamic_cast<OS::Core::File*>((OS::Core::File*)data));
		OS::Core::File * buf = (OS::Core::File*)data;
		buf->~File();
		os->free(buf);
	}
};

} // namespace ObjectScript

void OS::initFileClass()
{
	struct Lib
	{
		static int __construct(OS * os, int params, int, int, void * user_param)
		{
			Core::File * self = new (os->malloc(sizeof(Core::File) OS_DBG_FILEPOS)) Core::File(os);
			if(params >= 2){
				self->open(os->toString(-params), os->toString(-params+1));
			}else if(params >= 1){
				self->open(os->toString(-params));
			}
			CtypeValue<Core::File*>::push(os, self);
			return 1;
		}
		
		static int open(OS * os, int params, int, int, void * user_param)
		{
			OS_GET_SELF(Core::File*);
			if(params >= 2){
				os->pushBool(self->open(os->toString(-params), os->toString(-params+1)));
			}else if(params >= 1){
				os->pushBool(self->open(os->toString(-params)));
			}else{
				self->close();
			}
			return 0;
		}
		
		static int read(OS * os, int params, int, int, void * user_param)
		{
			OS_GET_SELF(Core::File*);
			if(params > 0){
				os->pushString(self->read(os->toInt(-params)));
			}else{
				os->pushString(self->read());
			}
			return 1;
		}
		
		static int write(OS * os, int params, int, int, void * user_param)
		{
			OS_GET_SELF(Core::File*);
			if(params >= 2){
				String str = os->toString(-params);
				int size = os->toInt(-params+1);
				if(size > str.getDataSize()){
					size = str.getDataSize();
				}
				os->pushNumber(self->write((void*)str.toChar(), size));
				return 1;
			}else if(params >= 1){
				os->pushNumber(self->write(os->toString(-params)));
				return 1;
			}
			return 0;
		}
	};

	OS::FuncDef funcs[] = {
		{OS_TEXT("__construct"), Lib::__construct},
		{OS_TEXT("open"), Lib::open},
		def(OS_TEXT("close"), &Core::File::close),
		{OS_TEXT("read"), Lib::read},
		{OS_TEXT("write"), Lib::write},
		def(OS_TEXT("__get@isOpen"), &Core::File::isOpen),
		def(OS_TEXT("__get@size"), &Core::File::getSize),
		def(OS_TEXT("__get@pos"), &Core::File::getPos),
		def(OS_TEXT("__set@pos"), &Core::File::setPos),
		{}
	};
	registerUserClass<Core::File>(this, funcs);
}

void OS::initFunctionClass()
{
	struct Function
	{
		static int apply(OS * os, int params, int, int need_ret_values, void*)
		{
			int offs = os->getAbsoluteOffs(-params);
			os->pushStackValue(offs-1); // self as func
			if(params < 1){
				os->pushNull();
				return os->call(0, need_ret_values);
			}
			os->pushStackValue(offs); // first param - new this

			Core::Value array_var = os->core->getStackValue(offs+1);
			if(OS_VALUE_TYPE(array_var) == OS_VALUE_TYPE_ARRAY){
				int count = OS_VALUE_VARIANT(array_var).arr->values.count;
				for(int i = 0; i < count; i++){
					os->core->pushValue(OS_VALUE_VARIANT(array_var).arr->values[i]);
				}
				return os->call(count, need_ret_values);
			}
			return os->call(0, need_ret_values);
		}

		static int call(OS * os, int params, int, int need_ret_values, void*)
		{
			return os->call(params-1, need_ret_values);
		}

		static int applyEnv(OS * os, int params, int, int need_ret_values, void *)
		{
			Core::Value save_env;
			Core::Value func = os->core->getStackValue(-params-1);
			if(OS_VALUE_TYPE(func) == OS_VALUE_TYPE_FUNCTION){
				save_env = OS_VALUE_VARIANT(func).func->env;
				OS_VALUE_VARIANT(func).func->env = os->core->getStackValue(-params).getGCValue();
			}
			os->remove(-params);
			int r = apply(os, params-1, 0, need_ret_values, NULL);
			if(OS_VALUE_TYPE(func) == OS_VALUE_TYPE_FUNCTION){
				OS_VALUE_VARIANT(func).func->env = save_env;
			}
			return r;
		}

		static int callEnv(OS * os, int params, int, int need_ret_values, void *)
		{
			Core::Value save_env;
			Core::Value func = os->core->getStackValue(-params-1);
			if(OS_VALUE_TYPE(func) == OS_VALUE_TYPE_FUNCTION){
				save_env = OS_VALUE_VARIANT(func).func->env;
				OS_VALUE_VARIANT(func).func->env = os->core->getStackValue(-params).getGCValue();
			}
			os->remove(-params);
			int r = call(os, params-1, 0, need_ret_values, NULL);
			if(OS_VALUE_TYPE(func) == OS_VALUE_TYPE_FUNCTION){
				OS_VALUE_VARIANT(func).func->env = save_env;
			}
			return r;
		}
	};
	FuncDef list[] = {
		{OS_TEXT("apply"), Function::apply},
		{OS_TEXT("applyEnv"), Function::applyEnv},
		{OS_TEXT("call"), Function::call},
		{OS_TEXT("callEnv"), Function::callEnv},
		{}
	};
	core->pushValue(core->prototypes[Core::PROTOTYPE_FUNCTION]);
	setFuncs(list);
	pop();
}

/*
The following functions are based on a C++ class MTRand by
Richard J. Wagner. For more information see the web page at
http://www-personal.engin.umich.edu/~wagnerr/MersenneTwister.html

It's port from PHP framework.
*/

#define OS_RAND_N             RAND_STATE_SIZE      /* length of state vector */
#define OS_RAND_M             (397)                /* a period parameter */
#define OS_RAND_hiBit(u)      ((u) & 0x80000000U)  /* mask all but highest   bit of u */
#define OS_RAND_loBit(u)      ((u) & 0x00000001U)  /* mask all but lowest    bit of u */
#define OS_RAND_loBits(u)     ((u) & 0x7FFFFFFFU)  /* mask     the highest   bit of u */
#define OS_RAND_mixBits(u, v) (OS_RAND_hiBit(u)|OS_RAND_loBits(v)) /* move hi bit of u to hi bit of v */

#define OS_RAND_twist(m,u,v)  (m ^ (OS_RAND_mixBits(u,v)>>1) ^ ((OS_U32)(-(OS_INT32)(OS_RAND_loBit(u))) & 0x9908b0dfU))
#define OS_RAND_MAX 0x7FFFFFFF		/* (1<<31) - 1 */ 

#define OS_RAND_RANGE(__n, __min, __max, __tmax) \
	(__n) = (__min) + (long) ((double) ( (double) (__max) - (__min) + 1.0) * ((__n) / ((__tmax) + 1.0)))

#if defined _MSC_VER && !defined IW_SDK
#include <windows.h>
#define OS_RAND_GENERATE_SEED() (((long) (time(0) * GetCurrentProcessId())) ^ ((long) (1000000.0)))
// #elif !defined IW_SDK
// #define OS_RAND_GENERATE_SEED() (((long) (time(0) * getpid())) ^ ((long) (1000000.0)))
#else
#define OS_RAND_GENERATE_SEED() (((long) (time(0))) ^ ((long) (1000000.0)))
#endif 

void OS::Core::randInitialize(OS_U32 seed)
{
	rand_seed = seed;

	OS_U32 * s = rand_state;
	OS_U32 * r = s;

	*s++ = seed & 0xffffffffU;
	for(int i = 1; i < OS_RAND_N; ++i ) {
		*s++ = ( 1812433253U * ( *r ^ (*r >> 30) ) + i ) & 0xffffffffU;
		r++;
	}

	randReload();
}

void OS::Core::randReload()
{
	/* Generate N new values in state
	Made clearer and faster by Matthew Bellew (matthew.bellew@home.com) */

	OS_U32 * state = rand_state;
	OS_U32 * p = state;
	int i;

	for(i = OS_RAND_N - OS_RAND_M; i--; ++p){
		*p = OS_RAND_twist(p[OS_RAND_M], p[0], p[1]);
	}
	for(i = OS_RAND_M; --i; ++p){
		*p = OS_RAND_twist(p[OS_RAND_M-OS_RAND_N], p[0], p[1]);
	}
	*p = OS_RAND_twist(p[OS_RAND_M-OS_RAND_N], p[0], state[0]);
	rand_left = OS_RAND_N;
	rand_next = state;
}

double OS::Core::getRand()
{
	/* Pull a 32-bit integer from the generator state
	Every other access function simply transforms the numbers extracted here */

	if(!rand_left){
		if(!rand_next){
			randInitialize(OS_RAND_GENERATE_SEED());
		}else{
			randReload();
		}
	}
	--rand_left;

	OS_U32 s1 = *rand_next++;
	s1 ^= (s1 >> 11);
	s1 ^= (s1 <<  7) & 0x9d2c5680U;
	s1 ^= (s1 << 15) & 0xefc60000U;
	return (double)((s1 ^ (s1 >> 18))>>1) / (double)OS_RAND_MAX;
}

double OS::Core::getRand(double up)
{
	return ::floor(getRand()*(up-1) + 0.5);
}

double OS::Core::getRand(double min, double max)
{
	return getRand() * (max - min) + min;
}

#define OS_MATH_PI 3.1415926535897932384626433832795
#define OS_RADIANS_PER_DEGREE (OS_MATH_PI/180.0)

void OS::initMathModule()
{
	struct Math
	{
		static int minmax(OS * os, int params, OS_EOpcode opcode)
		{
			OS_ASSERT(params >= 0);
			if(params <= 1){
				return params;
			}
			int params_offs = os->getAbsoluteOffs(-params);
			os->pushStackValue(params_offs); // save temp result
			for(int i = 1; i < params; i++){
				os->pushStackValue(-1); // copy temp result
				os->pushStackValue(params_offs + i);
				os->runOp(opcode); // remove params & push op result
				if(!os->toBool()){
					os->pop(2); // remove op result and temp result
					os->pushStackValue(params_offs + i); // save temp result
					continue;
				}
				os->pop();
			}
			return 1;
		}

		static int min_func(OS * os, int params, int, int, void*)
		{
			return minmax(os, params, OP_LOGIC_LE);
		}

		static int max_func(OS * os, int params, int, int, void*)
		{
			return minmax(os, params, OP_LOGIC_GE);
		}

		static double abs(double p)
		{
			return ::fabs(p);
		}

		static double ceil(double p)
		{
			return ::ceil(p);
		}

		static double floor(double p)
		{
			return ::floor(p);
		}

		static double round(double a, int precision)
		{
			if(precision <= 0){
				if(precision < 0){
					double p = 10.0;
					for(int i = -precision-1; i > 0; i--){
						p *= 10.0;
					}
					return ::floor(a / p + 0.5) * p;
				}
				return ::floor(a + 0.5);
			}
			double p = 10.0;
			for(int i = precision-1; i > 0; i--){
				p *= 10.0;
			}
			return ::floor(a * p + 0.5) / p;
		}

		static double sin(double p)
		{
			return ::sin(p);
		}

		static double sinh(double p)
		{
			return ::sinh(p);
		}

		static double cos(double p)
		{
			return ::cos(p);
		}

		static double cosh(double p)
		{
			return ::cosh(p);
		}

		static double tan(double p)
		{
			return ::tan(p);
		}

		static double tanh(double p)
		{
			return ::tanh(p);
		}

		static double acos(double p)
		{
			return ::acos(p);
		}

		static double asin(double p)
		{
			return ::asin(p);
		}

		static double atan(double p)
		{
			return ::atan(p);
		}

		static double atan2(double y, double x)
		{
			return ::atan2(y, x);
		}

		static double exp(double p)
		{
			return ::exp(p);
		}

		static int frexp(OS * os, int params, int, int, void*)
		{
			if(!params) return 0;
			int e;
			os->pushNumber(::frexp((double)os->toNumber(-params), &e));
			os->pushNumber(e);
			return 2;
		}

		static double ldexp(double x, int y)
		{
			return ::ldexp(x, y);
		}

		static double pow(double x, double y)
		{
			return ::pow(x, y);
		}

		static int random(OS * os, int params, int, int, void*)
		{
			OS::Core * core = os->core;
			switch(params){
			case 0:
				os->pushNumber(core->getRand());
				return 1;

			case 1:
				os->pushNumber(core->getRand(os->toNumber(-params)));
				return 1;

			case 2:
			default:
				os->pushNumber(core->getRand(os->toNumber(-params), os->toNumber(-params+1)));
				return 1;
			}
			return 0;
		}

		static int getrandseed(OS * os, int params, int, int, void*)
		{
			os->pushNumber((OS_NUMBER)os->core->rand_seed);
			return 1;
		}

		static int setrandseed(OS * os, int params, int, int, void*)
		{
			if(!params) return 0;
			os->core->rand_seed = (OS_U32)os->toNumber(-params);
			return 0;
		}

		static double fmod(double x, double y)
		{
			return ::fmod(x, y);
		}

		static int modf(OS * os, int params, int, int, void*)
		{
			if(!params) return 0;
			double ip;
			double fp = ::modf(os->toNumber(-params), &ip);
			os->pushNumber(ip);
			os->pushNumber(fp);
			return 2;
		}

		static double sqrt(double p)
		{
			return ::sqrt(p);
		}

		static int log(OS * os, int params, int, int, void*)
		{
			if(!params) return 0;
			double x = os->toNumber(-params);
			OS_NUMBER base;
			if(params > 1 && os->isNumber(-params+1, &base)){
				if(base == 10){
					os->pushNumber(::log10(x));
				}else{
					os->pushNumber(::log(x)/::log((double)base));
				}
			}else{
				os->pushNumber(::log(x));
			}
			return 1;
		}

		static double deg(double p)
		{
			return p / OS_RADIANS_PER_DEGREE;
		}
		
		static double rad(double p)
		{
			return p * OS_RADIANS_PER_DEGREE;
		}
	};
	FuncDef list[] = {
		{OS_TEXT("min"), Math::min_func},
		{OS_TEXT("max"), Math::max_func},
		def(OS_TEXT("abs"), Math::abs),
		def(OS_TEXT("ceil"), Math::ceil),
		def(OS_TEXT("floor"), Math::floor),
		def(OS_TEXT("round"), Math::round),
		def(OS_TEXT("sin"), Math::sin),
		def(OS_TEXT("sinh"), Math::sinh),
		def(OS_TEXT("cos"), Math::cos),
		def(OS_TEXT("cosh"), Math::cosh),
		def(OS_TEXT("tan"), Math::tan),
		def(OS_TEXT("tanh"), Math::tanh),
		def(OS_TEXT("acos"), Math::acos),
		def(OS_TEXT("asin"), Math::asin),
		def(OS_TEXT("atan"), Math::atan),
		def(OS_TEXT("atan2"), Math::atan2),
		def(OS_TEXT("exp"), Math::exp),
		{OS_TEXT("frexp"), Math::frexp},
		def(OS_TEXT("ldexp"), Math::ldexp),
		def(OS_TEXT("pow"), Math::pow),
		{OS_TEXT("random"), Math::random},
		{OS_TEXT("__get@randseed"), Math::getrandseed},
		{OS_TEXT("__set@randseed"), Math::setrandseed},
		def(OS_TEXT("fmod"), Math::fmod),
		{OS_TEXT("modf"), Math::modf},
		def(OS_TEXT("sqrt"), Math::sqrt),
		{OS_TEXT("log"), Math::log},
		def(OS_TEXT("deg"), Math::deg),
		def(OS_TEXT("rad"), Math::rad),
		{}
	};
	NumberDef numbers[] = {
		{OS_TEXT("PI"), (OS_NUMBER)OS_MATH_PI},
		{OS_TEXT("MAX_NUMBER"), OS_MAX_NUMBER},
		{}
	};

	getModule(OS_TEXT("math"));
	setFuncs(list);
	setNumbers(numbers);
	pop();
}

void OS::initGCModule()
{
	struct GC
	{
		static int getAllocatedBytes(OS * os, int params, int, int, void*)
		{
			os->pushNumber(os->getAllocatedBytes());
			return 1;
		}
		static int getMaxAllocatedBytes(OS * os, int params, int, int, void*)
		{
			os->pushNumber(os->getMaxAllocatedBytes());
			return 1;
		}
		static int getCachedBytes(OS * os, int params, int, int, void*)
		{
			os->pushNumber(os->getCachedBytes());
			return 1;
		}
		static int getNumObjects(OS * os, int params, int, int, void*)
		{
			os->pushNumber(os->core->values.count);
			return 1;
		}
		static int getNumCreatedObjects(OS * os, int params, int, int, void*)
		{
			os->pushNumber(os->core->num_created_values);
			return 1;
		}
		static int getNumDestroyedObjects(OS * os, int params, int, int, void*)
		{
			os->pushNumber(os->core->num_destroyed_values);
			return 1;
		}
	};
	FuncDef list[] = {
		{OS_TEXT("__get@allocatedBytes"), GC::getAllocatedBytes},
		{OS_TEXT("__get@maxAllocatedBytes"), GC::getMaxAllocatedBytes},
		{OS_TEXT("__get@cachedBytes"), GC::getCachedBytes},
		{OS_TEXT("__get@numObjects"), GC::getNumObjects},
		{OS_TEXT("__get@numCreatedObjects"), GC::getNumCreatedObjects},
		{OS_TEXT("__get@numDestroyedObjects"), GC::getNumDestroyedObjects},
		{}
	};

	getModule(OS_TEXT("GC"));
	setFuncs(list);
	pop();
}

void OS::initLangTokenizerModule()
{
	struct LangTokenizer
	{
		enum {
			TOKEN_TYPE_STRING,
			TOKEN_TYPE_NUMBER,
			TOKEN_TYPE_NAME,
			TOKEN_TYPE_OPERATOR
		};

		static int getTokenType(Core::TokenType type)
		{
			switch(type){
			case Core::Tokenizer::NAME:
				return TOKEN_TYPE_NAME;

			case Core::Tokenizer::STRING:
				return TOKEN_TYPE_STRING;

			case Core::Tokenizer::NUMBER:
				return TOKEN_TYPE_NUMBER;
			}
			return TOKEN_TYPE_OPERATOR;
		}

		static void pushTokensAsObject(OS * os, Core::Tokenizer& tokenizer)
		{
			os->newArray();
			int count = tokenizer.getNumTokens();
			for(int i = 0; i < count; i++){
				os->pushStackValue(-1);
				os->newObject();
				{
					Core::TokenData * token = tokenizer.getToken(i);

					os->pushStackValue(-1);
					os->pushString(OS_TEXT("str"));
					os->pushString(token->str);
					os->setProperty();
#if 0
					os->pushStackValue(-1);
					os->pushString(OS_TEXT("line"));
					os->pushNumber(token->line+1);
					os->setProperty();

					os->pushStackValue(-1);
					os->pushString(OS_TEXT("pos"));
					os->pushNumber(token->pos+1);
					os->setProperty();
#endif
					os->pushStackValue(-1);
					os->pushString(OS_TEXT("type"));
					os->pushNumber(getTokenType(token->type));
					os->setProperty();
				}
				os->addProperty();
			}
		}

		static int parseText(OS * os, int params, int, int, void*)
		{
			if(params < 1){
				return 0;
			}
			String str = os->toString(-params);
			if(str.getDataSize() == 0){
				return 0;
			}
			Core::Tokenizer tokenizer(os);
			tokenizer.parseText(str.toChar(), str.getLen(), String(os),
				params >= 2 ? (OS_ESourceCodeType)os->toInt(-params+1) : OS_SOURCECODE_AUTO,
				params >= 3 ? os->toBool(-params+2) : true);
			pushTokensAsObject(os, tokenizer);
			return 1;
		}

		static int parseFile(OS * os, int params, int, int, void*)
		{
			if(params < 1){
				return 0;
			}
			String filename = os->resolvePath(os->toString(-params));
			if(filename.getDataSize() == 0){
				return 0;
			}
			Core::FileStreamReader file(os, filename);
			if(!file.f){
				return 0;
			}
			Core::MemStreamWriter file_data(os);
			file_data.writeFromStream(&file);

			Core::Tokenizer tokenizer(os);
			tokenizer.parseText((OS_CHAR*)file_data.buffer.buf, file_data.buffer.count, filename,
				params >= 2 ? (OS_ESourceCodeType)os->toInt(-params+1) : OS_SOURCECODE_AUTO,
				params >= 3 ? os->toBool(-params+2) : true);

			pushTokensAsObject(os, tokenizer);
			return 1;
		}
	};

	FuncDef list[] = {
		{OS_TEXT("parseText"), LangTokenizer::parseText},
		{OS_TEXT("parseFile"), LangTokenizer::parseFile},
		{}
	};

	NumberDef numbers[] = {
		{OS_TEXT("TOKEN_TYPE_STRING"), LangTokenizer::TOKEN_TYPE_STRING},
		{OS_TEXT("TOKEN_TYPE_NUMBER"), LangTokenizer::TOKEN_TYPE_NUMBER},
		{OS_TEXT("TOKEN_TYPE_NAME"), LangTokenizer::TOKEN_TYPE_NAME},
		{OS_TEXT("TOKEN_TYPE_OPERATOR"), LangTokenizer::TOKEN_TYPE_OPERATOR},
		{}
	};

	getModule(OS_TEXT("LangTokenizer"));
	setFuncs(list);
	setNumbers(numbers);
	pop();
}

#define OS_AUTO_TEXT(exp) OS_TEXT(#exp)

void OS::initPreScript()
{
	eval(OS_AUTO_TEXT(
		// it's ObjectScript code here
		function Object.__get@length(){ return #this }
		function Function.__iter(){
			if(this === Function){
				return super()
			}
			return this
		}

		modules_loaded = {}
		function require(filename, required, source_code_type, check_utf8_bom){
			filename = resolvePath(filename)
				return filename && (modules_loaded.rawget(filename) 
					|| function(){
						modules_loaded[filename] = {} // block recursive require
						modules_loaded[filename] = compileFile(filename, required, source_code_type, check_utf8_bom)()
						return modules_loaded[filename]
					}())
		}
	));
}

void OS::initPostScript()
{
	eval(OS_AUTO_TEXT(
		// it's ObjectScript code here
		Object.__setempty = Object.push
		Object.__getempty = Object.pop
		
		function Buffer.printf(){
			this.append(sprintf.apply(_E, arguments))
		}
	));
}

int OS::Core::syncRetValues(int need_ret_values, int cur_ret_values)
{
	if(cur_ret_values > need_ret_values){
		pop(cur_ret_values - need_ret_values);
	}else{ 
		for(; cur_ret_values < need_ret_values; cur_ret_values++){
			pushNull();
		}
	}
	return need_ret_values;
}

OS::Core::GCObjectValue * OS::Core::initObjectInstance(GCObjectValue * object)
{
	struct Lib {
		static GCObjectValue * initObjectInstance_r(Core * core, GCObjectValue * object, GCValue * prototype)
		{
			if(prototype->prototype){
				initObjectInstance_r(core, object, prototype->prototype);
			}
			Value value;
			if(core->getPropertyValue(value, prototype, PropertyIndex(core->strings->__object, PropertyIndex::KeepStringIndex()), false)){
				GCValue * object_props = value.getGCValue();
				if(object_props && object_props->table){
					Property * prop = object_props->table->first;
					for(; prop; prop = prop->next){
						core->pushCloneValue(prop->value);
						core->setPropertyValue(object, *prop, core->stack_values.lastElement(), true);
						core->pop();
					}
				}
			}

			return object;
		}
	};
	if(object->prototype){
		Lib::initObjectInstance_r(this, object, object->prototype);
	}
	return object;
}

void OS::Core::pushArguments(StackFunction * stack_func)
{
	if(!stack_func->arguments){
		int i;
		GCArrayValue * args = pushArrayValue(stack_func->num_params - PRE_VARS + (stack_func->rest_arguments ? stack_func->rest_arguments->values.count : 0));
		Locals * func_locals = stack_func->locals;
		int num_params = stack_func->num_params;
		for(i = PRE_VARS; i < num_params; i++){
			allocator->vectorAddItem(args->values, func_locals->values[i] OS_DBG_FILEPOS);
		}
		if(stack_func->rest_arguments){
			GCArrayValue * arr = stack_func->rest_arguments;
			for(i = 0; i < arr->values.count; i++){
				setPropertyValue(args, PropertyIndex(args->values.count), arr->values[i], false);
			}
		}
		stack_func->arguments = args;
	}else{
		pushValue(stack_func->arguments);
	}
}

void OS::Core::pushArgumentsWithNames(StackFunction * stack_func)
{
	int i;
	GCObjectValue * args = pushObjectValue();
	Locals * func_locals = stack_func->locals;
	FunctionDecl * func_decl = stack_func->func->func_decl;
	int num_params = stack_func->num_params;
	for(i = PRE_VARS; i < num_params; i++){ // skip func & this
		setPropertyValue(args, PropertyIndex(func_decl->locals[i].name.string, PropertyIndex::KeepStringIndex()), func_locals->values[i], false);
	}
	if(num_params < func_decl->num_params){
		for(; i < func_decl->num_params; i++){
			setPropertyValue(args, PropertyIndex(func_decl->locals[i].name.string, PropertyIndex::KeepStringIndex()), Value(), false);
		}
	}else if(stack_func->rest_arguments){
		GCArrayValue * arr = stack_func->rest_arguments;
		for(i = 0; i < arr->values.count; i++){
			setPropertyValue(args, Value(args->table ? args->table->next_index : 0), arr->values[i], false);
		}
	}
}

void OS::Core::pushRestArguments(StackFunction * stack_func)
{
	if(!stack_func->rest_arguments){
		stack_func->rest_arguments = pushArrayValue();
	}else{
		pushValue(stack_func->rest_arguments);
	}
}

void OS::Core::pushBackTrace(int skip_funcs, int max_trace_funcs)
{
	GCArrayValue * arr = pushArrayValue();

	String function_str(allocator, OS_TEXT("function"));
	String name_str(allocator, OS_TEXT("name"));
	String file_str(allocator, OS_TEXT("file"));
	String line_str(allocator, OS_TEXT("line"));
	String pos_str(allocator, OS_TEXT("pos"));
	String token_str(allocator, OS_TEXT("token"));
	String object_str(allocator, OS_TEXT("object"));
	String arguments_str(allocator, OS_TEXT("arguments"));
	String core_str(allocator, OS_TEXT("<<CORE>>"));
	String lambda_str(allocator, OS_TEXT("<<lambda>>"));

	if(skip_funcs < 0){
		skip_funcs = 0;
	}
	for(int i = call_stack_funcs.count-1-skip_funcs; i >= 0 && arr->values.count < max_trace_funcs; i--){
		StackFunction * stack_func = call_stack_funcs.buf + i;

		Program * prog = stack_func->func->prog;
		if((!stack_func->func->name || core_str == stack_func->func->name) && !prog->filename.getDataSize()){
			continue;
		}

		GCObjectValue * obj = pushObjectValue();
		setPropertyValue(obj, PropertyIndex(name_str, PropertyIndex::KeepStringIndex()), stack_func->func->name ? stack_func->func->name : lambda_str.string, false);
		setPropertyValue(obj, PropertyIndex(function_str, PropertyIndex::KeepStringIndex()), stack_func->func, false);

		const String& filename = prog->filename.getDataSize() ? prog->filename : core_str;
		setPropertyValue(obj, PropertyIndex(file_str, PropertyIndex::KeepStringIndex()), filename.string, false);

		Program::DebugInfoItem * debug_info = NULL;
		if(prog->filename.getDataSize() && prog->debug_info.count > 0){
			int opcode_pos = stack_func->opcodes - prog->opcodes.buf; // .getPos() + stack_func->func->func_decl->opcodes_pos;
			debug_info = prog->getDebugInfo(opcode_pos);
		}
		setPropertyValue(obj, PropertyIndex(line_str, PropertyIndex::KeepStringIndex()), debug_info ? debug_info->line : Value(), false);
		setPropertyValue(obj, PropertyIndex(pos_str, PropertyIndex::KeepStringIndex()), debug_info ? debug_info->pos : Value(), false);
		setPropertyValue(obj, PropertyIndex(token_str, PropertyIndex::KeepStringIndex()), debug_info ? debug_info->token.string : Value(), false);

		setPropertyValue(obj, PropertyIndex(object_str, PropertyIndex::KeepStringIndex()), stack_func->locals->values[PRE_VAR_THIS], false);

		pushArgumentsWithNames(stack_func);
		setPropertyValue(obj, PropertyIndex(arguments_str, PropertyIndex::KeepStringIndex()), stack_values.lastElement(), false);
		pop(); // remove args

		setPropertyValue(arr, Value(arr->values.count), obj, false);
		pop(); // remove obj
	}
}

int OS::Core::call(int start_pos, int call_params, int ret_values, GCValue * self_for_proto, bool allow_only_enter_func)
{
	OS_ASSERT(start_pos >= OS_TOP_STACK_NULL_VALUES && call_params >= 2 && start_pos + call_params <= stack_values.count);
	Value& func = stack_values[start_pos];
	switch(OS_VALUE_TYPE(func)){
	case OS_VALUE_TYPE_FUNCTION:
		{
			OS_ASSERT(dynamic_cast<GCFunctionValue*>(OS_VALUE_VARIANT(func).func));
			GCFunctionValue * func_value = OS_VALUE_VARIANT(func).func;
			FunctionDecl * func_decl = func_value->func_decl;

			reserveStackValues(start_pos + (func_decl->stack_size > ret_values ? func_decl->stack_size : ret_values));
			
			if(call_stack_funcs.capacity < call_stack_funcs.count+1){
				call_stack_funcs.capacity = call_stack_funcs.capacity > 0 ? call_stack_funcs.capacity*2 : 8;
				OS_ASSERT(call_stack_funcs.capacity >= call_stack_funcs.count+1);

				StackFunction * new_buf = (StackFunction*)malloc(sizeof(StackFunction)*call_stack_funcs.capacity OS_DBG_FILEPOS);
				OS_MEMCPY(new_buf, call_stack_funcs.buf, sizeof(StackFunction) * call_stack_funcs.count);
				free(call_stack_funcs.buf);
				call_stack_funcs.buf = new_buf;
			}

			// it should be here before call_stack_funcs.count++ !!! don't move it
			GCArrayValue * rest_arguments;
			if(call_params > func_decl->num_params){
				int num_extra_params = call_params - func_decl->num_params;
				rest_arguments = newArrayValue(num_extra_params);
				OS_MEMCPY(rest_arguments->values.buf, stack_values.buf + start_pos + func_decl->num_params, sizeof(Value) * num_extra_params);
				rest_arguments->values.count = num_extra_params;
				call_params = func_decl->num_params;
			}else{
				rest_arguments = NULL;
			}

			StackFunction * stack_func = (StackFunction*)(call_stack_funcs.buf + call_stack_funcs.count++);
			stack_func->func = func_value;

			stack_func->rest_arguments = rest_arguments;
			stack_func->arguments = NULL;

			stack_func->locals_stack_pos = start_pos;
			stack_func->num_params = call_params;
			
			Locals * func_locals = (Locals*)(malloc(sizeof(Locals) + sizeof(Locals*) * func_decl->func_depth OS_DBG_FILEPOS));
			func_locals->prog = func_value->prog->retain();
			func_locals->func_decl = func_decl;
			func_locals->values = stack_values.buf + stack_func->locals_stack_pos;
			func_locals->is_stack_locals = true;
			func_locals->num_parents = func_decl->func_depth;
			func_locals->ref_count = 1;
			func_locals->gc_time = -1;
			if(func_decl->func_depth > 0){
				OS_ASSERT(func_value->locals && func_value->locals->num_parents == func_decl->func_depth-1);
				Locals ** parents = func_locals->getParents();
				parents[0] = func_value->locals->retain();
				if(func_decl->func_depth > 1){
					OS_MEMCPY(parents+1, func_value->locals->getParents(), sizeof(Locals*) * (func_decl->func_depth-1));
				}
			}
			stack_func->locals = func_locals;

			int clear_values = func_decl->stack_size - call_params;
			if(clear_values > 0){
				OS_SET_NULL_VALUES(func_locals->values + call_params, clear_values); 
			}

			stack_func->need_ret_values = ret_values;
			stack_func->opcodes = func_value->prog->opcodes.buf + func_decl->opcodes_pos;

			if(self_for_proto){
				stack_func->self_for_proto = self_for_proto;
			}else if(!(stack_func->self_for_proto = func_locals->values[PRE_VAR_THIS].getGCValue())){
				switch(OS_VALUE_TYPE(*func_locals->values)){
				case OS_VALUE_TYPE_BOOL:
					stack_func->self_for_proto = prototypes[PROTOTYPE_BOOL];
					break;

				case OS_VALUE_TYPE_NUMBER:
					stack_func->self_for_proto = prototypes[PROTOTYPE_NUMBER];
					break;
				}
			}

			stack_func->caller_stack_size = stack_values.count;
			
			OS_ASSERT(stack_func->locals_stack_pos + func_decl->stack_size <= stack_values.capacity);
			stack_values.count = stack_func->locals_stack_pos + func_decl->stack_size;

			func_locals->values[func_decl->num_params + POST_VAR_ENV] = func_value->env;
		#ifdef OS_GLOBAL_VAR_ENABLED
			func_locals->values[func_decl->num_params + POST_VAR_GLOBALS] = global_vars;
		#endif

			reloadStackFunctionCache();

			if(allow_only_enter_func){
				return 0;
			}
			return execute();
		}

	case OS_VALUE_TYPE_CFUNCTION:
		{
			OS_ASSERT(dynamic_cast<GCCFunctionValue*>(OS_VALUE_VARIANT(func).cfunc));
			OS_ASSERT(start_pos + call_params <= stack_values.count);
			int save_stack_size = stack_values.count;
			GCCFunctionValue * cfunc_value = OS_VALUE_VARIANT(func).cfunc;
			if(cfunc_value->num_closure_values > 0){
				reserveStackValues(start_pos + call_params + cfunc_value->num_closure_values);
				Value * closure_values = (Value*)(cfunc_value + 1);
				OS_MEMCPY(stack_values.buf + start_pos + call_params, closure_values, sizeof(Value)*cfunc_value->num_closure_values);
			}
			stack_values.count = start_pos + call_params + cfunc_value->num_closure_values;
			int cur_ret_values = cfunc_value->func(allocator, call_params - 2, cfunc_value->num_closure_values, ret_values, cfunc_value->user_param);
#if 0		// does store closure values back?
			if(cfunc_value->num_closure_values > 0){
				Value * closure_values = (Value*)(cfunc_value + 1);
				OS_MEMCPY(closure_values, stack_values.buf + start_pos + call_params, sizeof(Value)*cfunc_value->num_closure_values);
			}
#endif
			if(ret_values == 1){
				if(cur_ret_values > 0){
					stack_values.buf[start_pos] = stack_values.buf[stack_values.count - cur_ret_values];
				}else{
					OS_SET_VALUE_NULL(stack_values.buf[start_pos]);
				}
			}else if(ret_values > 0){
				Value * stack_func_locals = stack_values.buf + stack_values.count - cur_ret_values;
				if(ret_values <= cur_ret_values){
					OS_MEMMOVE(stack_values.buf + start_pos, stack_func_locals, sizeof(Value) * ret_values);
				}else{
					if(cur_ret_values > 0){
						OS_MEMMOVE(stack_values.buf + start_pos, stack_func_locals, sizeof(Value) * cur_ret_values);
					}
					OS_SET_NULL_VALUES(stack_values.buf + start_pos + cur_ret_values, ret_values - cur_ret_values);
				}
			}
			if(save_stack_size > stack_values.count){
				OS_ASSERT(save_stack_size <= stack_values.capacity);
				OS_SET_NULL_VALUES(stack_values.buf + stack_values.count, save_stack_size - stack_values.count);
			}
			stack_values.count = save_stack_size;
			return ret_values;
		}

	case OS_VALUE_TYPE_OBJECT:
		{
			GCValue * object = initObjectInstance(pushObjectValue(OS_VALUE_VARIANT(func).value));
			object->is_object_instance = true;
			object->external_ref_count++;

			bool prototype_enabled = true;
			if(getPropertyValue(stack_values.buf[start_pos], func, PropertyIndex(strings->__construct, PropertyIndex::KeepStringIndex()), prototype_enabled)
				&& stack_values.buf[start_pos].isFunction())
			{
				stack_values.buf[start_pos + 1] = object;
				call(start_pos, call_params, ret_values);
			}
			OS_ASSERT(start_pos + ret_values <= stack_values.count);
			if(ret_values > 0){
				stack_values.buf[start_pos] = object;
				if(ret_values > 1){					
					OS_SET_NULL_VALUES(stack_values.buf + start_pos + 1, ret_values - 1);
				}
			}
			object->external_ref_count--;
			return ret_values;
		}

	case OS_VALUE_TYPE_USERDATA:
	case OS_VALUE_TYPE_USERPTR:
		{
			Value func_value = func;
			bool prototype_enabled = true;
			Value func;
			if(getPropertyValue(func, func_value, PropertyIndex(strings->__construct, PropertyIndex::KeepStringIndex()), prototype_enabled)
				&& func.isFunction())
			{
				stack_values.buf[start_pos + 0] = func;
				stack_values.buf[start_pos + 1] = func_value;
				return call(start_pos, call_params, ret_values);
			}
			break;
		}
	}
	OS_ASSERT(start_pos + ret_values <= stack_values.count);
	OS_SET_NULL_VALUES(stack_values.buf + start_pos, ret_values);
	return ret_values;
}

int OS::Core::call(int params, int ret_values, GCValue * self_for_proto, bool allow_only_enter_func)
{
	params += 2;
	int start_pos = stack_values.count - params;
	ret_values = call(start_pos, params, ret_values, self_for_proto, allow_only_enter_func);
	stack_values.count = start_pos + ret_values;
	return ret_values;
}

bool OS::compileFile(const String& p_filename, bool required, OS_ESourceCodeType source_code_type, bool check_utf8_bom)
{
	String filename = resolvePath(p_filename);
	String compiled_filename = getCompiledFilename(filename);
	bool sourcecode_file_exist = isFileExist(filename);
	bool compiled_file_exist = isFileExist(compiled_filename);
	if(compiled_file_exist && sourcecode_file_exist){
		if(core->settings.primary_compiled_file || checkFileUsage(filename, compiled_filename) == LOAD_COMPILED_FILE){
			sourcecode_file_exist = false;
		}else{
			compiled_file_exist = false;
		}
	}
	if(!sourcecode_file_exist && !compiled_file_exist){
		if(required){
			core->error(OS_E_ERROR, String::format(this, OS_TEXT("required filename %s is not exist"), p_filename.toChar()));
			return false;
		}
		core->error(OS_E_WARNING, String::format(this, OS_TEXT("filename %s is not exist"), p_filename.toChar()));
		return false;
	}
	if(!sourcecode_file_exist){
		OS_ASSERT(compiled_file_exist);
		Core::Program * prog = new (malloc(sizeof(Core::Program) OS_DBG_FILEPOS)) Core::Program(this);
		prog->filename = compiled_filename;

		Core::FileStreamReader prog_file_reader(this, compiled_filename);
		Core::MemStreamWriter prog_file_data(this);
		prog_file_data.writeFromStream(&prog_file_reader);
		Core::MemStreamReader prog_reader(NULL, prog_file_data.buffer.buf, prog_file_data.getSize());

		String debug_info_filename = getDebugInfoFilename(filename);
		if(isFileExist(debug_info_filename)){
			Core::FileStreamReader debug_info_file_reader(this, debug_info_filename);
			Core::MemStreamWriter debug_info_file_data(this);
			debug_info_file_data.writeFromStream(&debug_info_file_reader);
			Core::MemStreamReader debug_info_reader(NULL, debug_info_file_data.buffer.buf, debug_info_file_data.getSize());
			if(!prog->loadFromStream(&prog_reader, &debug_info_reader)){
				prog->release();
				return false;
			}
		}else if(!prog->loadFromStream(&prog_reader, NULL)){
			prog->release();
			return false;
		}
		prog->pushStartFunction();
		prog->release();
		return true;
	}

	Core::FileStreamReader file(this, filename);
	if(!file.f){
		core->error(OS_E_ERROR, String::format(this, OS_TEXT("error open filename %s"), p_filename.toChar()));
		return false;
	}

	Core::MemStreamWriter file_data(this);
	file_data.writeFromStream(&file);

	if(source_code_type == OS_SOURCECODE_AUTO && getFilenameExt(filename) == OS_EXT_TEMPLATE){
		source_code_type = OS_SOURCECODE_TEMPLATE;
	}

	Core::Tokenizer tokenizer(this);
	tokenizer.parseText((OS_CHAR*)file_data.buffer.buf, file_data.buffer.count, filename, source_code_type, check_utf8_bom);

	Core::Compiler compiler(&tokenizer);
	return compiler.compile();
}

bool OS::compile(const String& str, OS_ESourceCodeType source_code_type, bool check_utf8_bom)
{
	if(str.getDataSize() == 0){
		return false;
	}
	Core::Tokenizer tokenizer(this);
	tokenizer.parseText(str.toChar(), str.getLen(), String(this), source_code_type, check_utf8_bom);

	Core::Compiler compiler(&tokenizer);
	return compiler.compile();
}

bool OS::compile(OS_ESourceCodeType source_code_type, bool check_utf8_bom)
{
	return compile(popString(), source_code_type, check_utf8_bom);
}

int OS::call(int params, int ret_values)
{
	return core->call(params, ret_values);
}

int OS::eval(const OS_CHAR * str, int params, int ret_values, OS_ESourceCodeType source_code_type, bool check_utf8_bom)
{
	return eval(String(this, str), params, ret_values, source_code_type, check_utf8_bom);
}

int OS::eval(const String& str, int params, int ret_values, OS_ESourceCodeType source_code_type, bool check_utf8_bom)
{
	compile(str, source_code_type, check_utf8_bom);
	pushNull();
	move(-2, 2, -2-params);
	return core->call(params, ret_values);
}

int OS::require(const OS_CHAR * filename, bool required, int ret_values, OS_ESourceCodeType source_code_type, bool check_utf8_bom)
{
	return require(String(this, filename), required, ret_values, source_code_type, check_utf8_bom);
}

int OS::require(const String& filename, bool required, int ret_values, OS_ESourceCodeType source_code_type, bool check_utf8_bom)
{
	getGlobal(core->strings->func_require);
	pushGlobals();
	pushString(filename);
	pushBool(required);
	pushNumber(source_code_type);
	pushBool(check_utf8_bom);
	return call(4, ret_values);
}

int OS::getSetting(OS_ESettings setting)
{
	switch(setting){
	case OS_SETTING_CREATE_DEBUG_INFO:
		return core->settings.create_debug_info;

	case OS_SETTING_CREATE_DEBUG_OPCODES:
		return core->settings.create_debug_opcodes;

	case OS_SETTING_CREATE_DEBUG_EVAL_OPCODES:
		return core->settings.create_debug_eval_opcodes;

	case OS_SETTING_CREATE_COMPILED_FILE:
		return core->settings.create_compiled_file;

	case OS_SETTING_PRIMARY_COMPILED_FILE:
		return core->settings.primary_compiled_file;
	}
	return -1;
}

int OS::setSetting(OS_ESettings setting, int value)
{
	struct Lib {
		static int ret(bool& cur_value, int new_value)
		{
			int old = cur_value;
			cur_value = new_value ? true : false;
			return old;
		}	
	};

	switch(setting){
	case OS_SETTING_CREATE_DEBUG_INFO:
		return Lib::ret(core->settings.create_debug_info, value);

	case OS_SETTING_CREATE_DEBUG_OPCODES:
		return Lib::ret(core->settings.create_debug_opcodes, value);

	case OS_SETTING_CREATE_DEBUG_EVAL_OPCODES:
		return Lib::ret(core->settings.create_debug_eval_opcodes, value);

	case OS_SETTING_CREATE_COMPILED_FILE:
		return Lib::ret(core->settings.create_compiled_file, value);

	case OS_SETTING_PRIMARY_COMPILED_FILE:
		return Lib::ret(core->settings.primary_compiled_file, value);

	default:
		OS_ASSERT(false);
	}
	return -1;
}

int OS::gc()
{
	return core->gcStep();
}

void OS::gcFull()
{
	core->gcFull();
}

void OS::triggerError(int code, const OS_CHAR * message)
{
	core->error(code, message);
}

void OS::triggerError(int code, const String& message)
{
	core->error(code, message);
}

void OS::triggerError(const OS_CHAR * message)
{
	core->error(OS_E_ERROR, message);
}

void OS::triggerError(const String& message)
{
	core->error(OS_E_ERROR, message);
}

// =====================================================================
// =====================================================================
// =====================================================================

#define OS_QSORT_CUTOFF 8

static void qsortSwap(char *a, char *b, unsigned width)
{
	char tmp;

	if(a != b) {
		if(width == sizeof(void*)){
			void * tmp = *(void**)a;
			*(void**)a = *(void**)b;
			*(void**)b = tmp;
			return;
		}
		if(width >= 16 && width <= 256){
			void * tmp = alloca(width);
			OS_MEMCPY(tmp, a, width);
			OS_MEMCPY(a, b, width);
			OS_MEMCPY(b, tmp, width);
			return;
		}
		while(width--){
			tmp = *a;
			*a++ = *b;
			*b++ = tmp;
		}
	}
}

static void qsortShortsort(OS * os, char *lo, char *hi, unsigned width, int (*comp)(OS*, const void *, const void *, void*), void * user_params)
{
	char *p, *max;

	while (hi > lo) {
		max = lo;
		for (p = lo + width; p <= hi; p += width) if (comp(os, p, max, user_params) > 0) max = p;
		qsortSwap(max, hi, width);
		hi -= width;
	}
}

void OS::qsort(void *base, unsigned num, unsigned width, int (*comp)(OS*, const void *, const void *, void*), void * user_params)
{
	char *lo, *hi;
	char *mid;
	char *l, *h;
	unsigned size;
	char *lostk[30], *histk[30];
	int stkptr;

	if (num < 2 || width == 0) return;
	stkptr = 0;

	lo = (char*)base;
	hi = (char*)base + width * (num - 1);

recurse:
	size = (hi - lo) / width + 1;

	if (size <= OS_QSORT_CUTOFF) {
		qsortShortsort(this, lo, hi, width, comp, user_params);
	} else {
		mid = lo + (size / 2) * width;
		qsortSwap(mid, lo, width);

		l = lo;
		h = hi + width;

		for (;;) {
			do { l += width; } while (l <= hi && comp(this, l, lo, user_params) <= 0);
			do { h -= width; } while (h > lo && comp(this, h, lo, user_params) >= 0);
			if (h < l) break;
			qsortSwap(l, h, width);
		}

		qsortSwap(lo, h, width);

		if (h - 1 - lo >= hi - l) {
			if (lo + width < h) {
				lostk[stkptr] = lo;
				histk[stkptr] = h - width;
				++stkptr;
			}

			if (l < hi) {
				lo = l;
				goto recurse;
			}
		} else {
			if (l < hi) {
				lostk[stkptr] = l;
				histk[stkptr] = hi;
				++stkptr;
			}

			if (lo + width < h) {
				hi = h - width;
				goto recurse;
			}
		}
	}

	--stkptr;
	if (stkptr >= 0) {
		lo = lostk[stkptr];
		hi = histk[stkptr];
		goto recurse;
	}
}

// =====================================================================
// =====================================================================
// =====================================================================

static FunctionDataChain * function_data_first = NULL;

FunctionDataChain::FunctionDataChain()
{ 
	next = NULL;
	ptr = NULL;
	data_size = 0;
	// data_hash = 0;
}

FunctionDataChain::~FunctionDataChain()
{
}

FunctionDataChain * FunctionDataChain::find()
{
	FunctionDataChain * cur = function_data_first;
	for(; cur; cur = cur->next){
		if(cur->data_size == data_size && OS_MEMCMP(cur->ptr, ptr, data_size) == 0){
			return cur;
		}
	}
	return NULL;
}

void FunctionDataChain::registerFunctionData()
{
	next = function_data_first;
	function_data_first = this;
}

void ObjectScript::finalizeAllBinds()
{
	while(function_data_first){
		FunctionDataChain * cur = function_data_first;
		function_data_first = cur->next;
		delete cur;
	}
}

struct FunctionDataFinalizer
{
	~FunctionDataFinalizer(){ ObjectScript::finalizeAllBinds(); }
} __functionDataFinalizer__;

// =====================================================================
// =====================================================================
// =====================================================================
