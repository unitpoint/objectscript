#include "objectscript.h"

#ifdef _MSC_VER
#define DEBUG_BREAK __debugbreak()
#else
#include <signal.h>
#define DEBUG_BREAK raise(SIGTRAP)
// #define DEBUG_BREAK __builtin_trap()
#endif

using namespace ObjectScript;

// =====================================================================
// =====================================================================
// =====================================================================

static int __snprintf__(char * buf, size_t num, const char * format, ...)
{
	va_list va;
	va_start(va, format);
	int ret = OS_VSNPRINTF(buf, num, num/sizeof(OS_CHAR)-1, format, va);
	va_end(va);
	return ret;
}

static bool isnan(float a)
{
	volatile float b = a;
	return b != b;
}

static bool isnan(double a)
{
	volatile double b = a;
	return b != b;
}

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

static inline void parseSpaces(const OS_CHAR *& str)
{
	while(*str && OS_ISSPACE(*str))
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
		}
		if(prev_val > val){
			p_str = start;
			p_val = 0;
			return val;
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
			return val;
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
			return val;
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
			return val;
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
				if(!*str || OS_ISSPACE(*str) || OS_STRCHR(OS_TEXT("!@#$%^&*()-+={}[]\\|;:'\",<.>/?`~"), *str)){
					OS_INT32 spec_val;
					switch(i){
					case 0:
						spec_val = 0x7f800000;
						break;

					case 1:
						spec_val = 0xffc00000;
						break;

					case 2:
						spec_val = 0x7fc00000;
						break;

					default:
						OS_ASSERT(false);
					}
					result = (OS_FLOAT)fromLittleEndianByteOrder(*(float*)&spec_val);
					return true;
				}            
			}
			result = 0;
			return false;
		}

		OS_FLOAT m = 0.1;
		for(str++; *str >= OS_TEXT('0') && *str <= OS_TEXT('9'); str++, m *= 0.1){
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
			m = 1.0f;
			for(int i = 0; i < pow; i++){
				m *= 10.0f;
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

OS_CHAR * OS::Utils::numToStr(OS_CHAR * dst, OS_INT a)
{
	if(sizeof(OS_INT) == sizeof(int)){
		OS_SNPRINTF(dst, sizeof(*dst)*64, OS_TEXT("%i"), a);
	}else{
		OS_SNPRINTF(dst, sizeof(*dst)*64, OS_TEXT("%li"), (long int)a);
	}
	return dst;
}

OS_CHAR * OS::Utils::numToStr(OS_CHAR * dst, OS_FLOAT a, int precision)
{
	OS_CHAR fmt[128];
	if(precision <= 0) {
		if(precision < 0) {
#if 1
			OS_FLOAT p = 10.0f;
			for(int i = -precision-1; i > 0; i--){
				p *= 10.0f;
			}
#else
			OS_FLOAT p = OS_MATH_POW((OS_FLOAT)10.0, (OS_FLOAT)-precision);
#endif
			a = OS_MATH_FLOOR(a / p + 0.5f) * p;
		}
		OS_SNPRINTF(dst, sizeof(fmt), OS_TEXT("%.f"), a);
		return dst;
	}
	if(precision == OS_AUTO_PRECISION){
		OS_SNPRINTF(dst, sizeof(fmt), OS_TEXT("%g"), a);
		return dst;
	}
	
	OS_SNPRINTF(fmt, sizeof(fmt), OS_TEXT("%%.%dg"), precision);
	int n = OS_SNPRINTF(dst, sizeof(fmt), fmt, a);

	OS_ASSERT(n >= 1 && !OS_STRSTR(dst, OS_TEXT(".")) || dst[n-1] != '0');
	// while(n > 0 && dst[n-1] == '0') dst[--n] = (OS_CHAR)0;
	// if(n > 0 && dst[n-1] == '.') dst[--n] = (OS_CHAR)0;

	return dst;
}

OS_INT OS::Utils::strToInt(const OS_CHAR * str)
{
	return (OS_INT)strToFloat(str);
}

OS_FLOAT OS::Utils::strToFloat(const OS_CHAR* str)
{
	OS_FLOAT fval;
	if(parseFloat(str, fval) && !*str){
		return fval;
	}
	return 0;
}

#define OS_KEY_HASH_START_VALUE 5381
#define OS_ADD_KEY_HASH_VALUE hash = ((hash << 5) + hash) + *buf++

int OS::Utils::keyToHash(const void * buf, int size)
{
	return OS::Utils::addKeyToHash(OS_KEY_HASH_START_VALUE, buf, size);
}

int OS::Utils::keyToHash(const void * buf1, int size1, const void * buf2, int size2)
{
	int hash = OS::Utils::addKeyToHash(OS_KEY_HASH_START_VALUE, buf1, size1);
	return OS::Utils::addKeyToHash(hash, buf2, size2);
}

int OS::Utils::addKeyToHash(int hash, const void * p_buf, int size)
{
	const OS_BYTE * buf = (const OS_BYTE*)p_buf;
	for(; size >= 8; size -= 8) {
		OS_ADD_KEY_HASH_VALUE;
		OS_ADD_KEY_HASH_VALUE;
		OS_ADD_KEY_HASH_VALUE;
		OS_ADD_KEY_HASH_VALUE;
		OS_ADD_KEY_HASH_VALUE;
		OS_ADD_KEY_HASH_VALUE;
		OS_ADD_KEY_HASH_VALUE;
		OS_ADD_KEY_HASH_VALUE;
	}
	switch(size) {
	case 7: OS_ADD_KEY_HASH_VALUE;
	case 6: OS_ADD_KEY_HASH_VALUE;
	case 5: OS_ADD_KEY_HASH_VALUE;
	case 4: OS_ADD_KEY_HASH_VALUE;
	case 3: OS_ADD_KEY_HASH_VALUE;
	case 2: OS_ADD_KEY_HASH_VALUE;
	case 1: OS_ADD_KEY_HASH_VALUE;
	}
	return hash;
}

int OS::Utils::cmp(const void * buf1, int len1, const void * buf2, int len2)
{
	int len = len1 < len2 ? len1 : len2;
	int cmp = OS_MEMCMP(buf1, buf2, len);
	return cmp ? cmp : len1 - len2;
}

int OS::Utils::cmp(const void * buf1, int len1, const void * buf2, int len2, int maxLen)
{
	return cmp(buf1, len1 < maxLen ? len1 : maxLen, buf2, len2 < maxLen ? len2 : maxLen);
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

/*
OS::Core::String::String(OS * os, const void * buf1, int size1, const void * buf2, int size2, const void * buf3, int size3)
{
	string = os->core->newStringValue(buf1, size1, buf2, size2, buf3, size3);
	string->external_ref_count++;
#ifdef OS_DEBUG
	this->str = string->toChar();
#endif
}
*/

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
	}
}

struct OS_VaListDtor
{
	va_list * va;

	OS_VaListDtor(va_list * p_va){ va = p_va; }
	~OS_VaListDtor(){ va_end(*va); }
};

OS::Core::String OS::Core::String::format(OS * allocator, int temp_buf_size, const OS_CHAR * fmt, ...)
{
	va_list va;
	va_start(va, fmt);
	OS_VaListDtor va_dtor(&va);
	return String(allocator->core->newStringValueVa(temp_buf_size, fmt, va));
}

OS::Core::String OS::Core::String::formatVa(OS * allocator, int temp_buf_size, const OS_CHAR * fmt, va_list va)
{
	return String(allocator->core->newStringValueVa(temp_buf_size, fmt, va));
}

OS::Core::String OS::Core::String::format(OS * allocator, const OS_CHAR * fmt, ...)
{
	va_list va;
	va_start(va, fmt);
	OS_VaListDtor va_dtor(&va);
	return String(allocator->core->newStringValueVa(OS_DEF_FMT_BUF_SIZE, fmt, va));
}

OS::Core::String OS::Core::String::formatVa(OS * allocator, const OS_CHAR * fmt, va_list va)
{
	return String(allocator->core->newStringValueVa(OS_DEF_FMT_BUF_SIZE, fmt, va));
}

OS::Core::String& OS::Core::String::operator=(const String& b)
{
	if(string != b.string){
		OS_ASSERT(string->external_ref_count > 0);
		string->external_ref_count--;
		string = b.string;
		string->external_ref_count++;
	}
	return *this;
}

bool OS::Core::String::operator==(const String& b) const
{
	return string == b.string;
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

OS_FLOAT OS::Core::String::toFloat() const
{
	OS_FLOAT val;
	if(string->isFloat(&val)){
		return val;
	}
	return 0;
}

// =====================================================================

OS::Core::StringBuffer::StringBuffer(OS * p_allocator)
{
	allocator = p_allocator;
}

OS::Core::StringBuffer::~StringBuffer()
{
	allocator->vectorClear(*this);
}

OS::Core::StringBuffer& OS::Core::StringBuffer::append(OS_CHAR c)
{
	allocator->vectorAddItem(*this, c);
	return *this;
}

OS::Core::StringBuffer& OS::Core::StringBuffer::append(const OS_CHAR * str)
{
	return append(str, OS_STRLEN(str));
}

OS::Core::StringBuffer& OS::Core::StringBuffer::append(const OS_CHAR * str, int len)
{
	allocator->vectorReserveCapacity(*this, count + len);
	OS_MEMCPY(buf + count, str, len * sizeof(OS_CHAR));
	count += len;
	return *this;
}

OS::Core::StringBuffer& OS::Core::StringBuffer::append(const String& str)
{
	return append(str.toChar(), str.getLen());
}

OS::Core::StringBuffer& OS::Core::StringBuffer::append(const StringBuffer& buf)
{
	return append(buf.buf, buf.count);
}

OS::Core::StringBuffer& OS::Core::StringBuffer::operator+=(const String& str)
{
	return append(str);
}

OS::Core::StringBuffer::operator OS::Core::String() const
{
	return toString();
}

OS::Core::String OS::Core::StringBuffer::toString() const
{
	return String(allocator, buf, count);
}

OS::Core::GCStringValue * OS::Core::StringBuffer::toGCStringValue() const
{
	return allocator->core->newStringValue(buf, count);
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

OS::String::String(OS * allocator, const Core::String& str): super(str)
{
	this->allocator = allocator->retain();
}

OS::String::String(OS * allocator, const OS_CHAR * str): super(allocator, str)
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

/*
OS::String::String(OS * allocator, const void * buf1, int size1, const void * buf2, int size2, const void * buf3, int size3): super(allocator, buf1, size1, buf2, size2, buf3, size3)
{
	this->allocator = allocator->retain();
}
*/

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
	string = NULL;
	allocator->release();
}

OS::String& OS::String::operator=(const Core::String& str)
{
	if(string != str.string){
		OS_ASSERT(string->external_ref_count > 0);
		string->external_ref_count--;
		string = str.string;
		string->external_ref_count++;
	}
	return *this;
}

OS::String& OS::String::operator=(const String& str)
{
	OS_ASSERT(allocator == str.allocator);
	if(string != str.string){
		OS_ASSERT(string->external_ref_count > 0);
		string->external_ref_count--;
		string = str.string;
		string->external_ref_count++;
	}
	return *this;
}

OS::String& OS::String::operator+=(const String& str)
{
	return *this = allocator->core->newStringValue(*this, str);
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
		// case DOT_NAME:  return OS_TEXT("DOT_NAME");
	case IDENTIFER:  return OS_TEXT("IDENTIFER");
	case DOT_IDENTIFER:  return OS_TEXT("DOT_IDENTIFER");
	case STRING:    return OS_TEXT("STRING");

	case NUMBER:   return OS_TEXT("NUMBER");
	// case NUM_VECTOR_3:  return OS_TEXT("NUM_VECTOR_3");
	// case NUM_VECTOR_4:  return OS_TEXT("NUM_VECTOR_4");

	case OPERATOR:        return OS_TEXT("OPERATOR");
	case BINARY_OPERATOR: return OS_TEXT("BINARY_OPERATOR");

	case OPERATOR_INDIRECT: return OS_TEXT("OPERATOR_INDIRECT");
	case OPERATOR_CONCAT:  return OS_TEXT("OPERATOR_CONCAT");

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

		// case PRE_PROCESSOR: return OS_TEXT("PRE_PROCESSOR");

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
	// vec3 = NULL;
}

OS * OS::Core::Tokenizer::TokenData::getAllocator() const
{
	return text_data->getAllocator();
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

		case OS::Core::Tokenizer::OPERATOR_INDIRECT:  // .
		case OS::Core::Tokenizer::OPERATOR_CONCAT: // ..

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

int __cdecl OS::Core::Tokenizer::CompareOperatorDesc(const void * a, const void * b) 
{
	const OperatorDesc * op0 = (const OperatorDesc*)a;
	const OperatorDesc * op1 = (const OperatorDesc*)b;
	return (int)OS_STRLEN(op1->name) - (int)OS_STRLEN(op0->name);
}

void OS::Core::Tokenizer::initOperatorsTable()
{
	if(!operator_initialized){
		qsort(operator_desc, operator_count, sizeof(operator_desc[0]), CompareOperatorDesc);
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

OS * OS::Core::Tokenizer::TextData::getAllocator()
{
	return allocator; // filename.getAllocator();
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
		OS * allocator = getAllocator();
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

	text_data = new (allocator->malloc(sizeof(TextData))) TextData(allocator);
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
	// allocator->vectorClear(lines);
	text_data->release();
}

void OS::Core::Tokenizer::insertToken(int i, TokenData * token)
{
	getAllocator()->vectorInsertAtIndex(tokens, i, token);
}

bool OS::Core::Tokenizer::parseText(const OS_CHAR * text, int len, const String& filename)
{
	OS_ASSERT(text_data->lines.count == 0);

	OS * allocator = getAllocator();

	// text_data->release();
	// text_data = new (allocator->malloc(sizeof(TextData))) TextData(allocator);
	text_data->filename = filename;

	const OS_CHAR * str = text;
	const OS_CHAR * str_end = str + len;
	while(str < str_end)
	{
		const OS_CHAR * line_end = OS_STRCHR(str, OS_TEXT('\n'));
		if(line_end){
			allocator->vectorAddItem(text_data->lines, String(allocator, str, line_end - str, false, true));
			str = line_end+1;
		}else{
			allocator->vectorAddItem(text_data->lines, String(allocator, str, str_end - str, false, true));
			break;
		}
	}
	return parseLines();
}

void OS::Core::Tokenizer::TokenData::setFloat(OS_FLOAT value)
{
	float_value = value;
}

OS::Core::Tokenizer::TokenData * OS::Core::Tokenizer::addToken(const String& str, TokenType type, int line, int pos)
{
	OS * allocator = getAllocator();
	TokenData * token = new (allocator->malloc(sizeof(TokenData))) TokenData(text_data, str, type, line, pos);
	allocator->vectorAddItem(tokens, token);
	return token;
}

OS::Core::TokenType OS::Core::Tokenizer::parseNum(const OS_CHAR *& str, OS_FLOAT& fval, bool parse_end_spaces)
{
	if(Utils::parseFloat(str, fval) && (!*str || OS_ISSPACE(*str) || OS_STRCHR(OS_TEXT("!@#$%^&*()-+={}[]\\|;:'\",<.>/?`~"), *str))){
		if(parse_end_spaces){
			parseSpaces(str);
		}
		return NUMBER;
	}
	return ERROR_TOKEN;
}

bool OS::Core::Tokenizer::parseLines()
{
	OS * allocator = getAllocator();
	cur_line = cur_pos = 0;
	for(; cur_line < text_data->lines.count; cur_line++){
		// parse line
		const OS_CHAR * line_start = text_data->lines[cur_line].toChar();
		const OS_CHAR * str = line_start;

		cur_pos = 0;
		for(;;){
			// skip spaces
			parseSpaces(str);
			if(!*str){
				break;
			}

			if(*str == OS_TEXT('"') || *str == OS_TEXT('\'')){ // begin string
				StringBuffer s(allocator);
				OS_CHAR closeChar = *str;
				const OS_CHAR * tokenStart = str;
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
				addToken(s, STRING, cur_line, tokenStart - line_start);
				continue;
			}

			if(*str == OS_TEXT('/')){
				if(str[1] == OS_TEXT('/')){ // begin line comment
					if(settings.save_comments){
						addToken(String(allocator, str), COMMENT_LINE, cur_line, str - line_start);
					}
					break;
				}
				if(str[1] == OS_TEXT('*')){ // begin multi line comment
					StringBuffer comment(allocator);
					comment.append(str, 2);
					int startLine = cur_line;
					int startPos = str - line_start;
					for(str += 2;;){
						const OS_CHAR * end = OS_STRSTR(str, OS_TEXT("*/"));
						if(end){
							if(settings.save_comments){
								comment.append(str, (int)(end+2 - str));
								addToken(comment, COMMENT_MULTI_LINE, startLine, startPos);
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

			if(*str == OS_TEXT('_') || *str == OS_TEXT('$') || *str == OS_TEXT('@') 
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
				addToken(name, type, cur_line, name_start - line_start);
				continue;
			}
			// parse operator
			int i;
			for(i = 0; i < operator_count; i++){
				size_t len = OS_STRLEN(operator_desc[i].name);
				if(OS_STRNCMP(str, operator_desc[i].name, len) == 0)
				{
					addToken(String(allocator, str, (int)len), operator_desc[i].type, cur_line, str - line_start);
					str += len;
					break;
				}
			}
			if(i < operator_count){
				continue;
			}

			{
				OS_FLOAT fval;
				const OS_CHAR * tokenStart = str;
				TokenType type = parseNum(str, fval, true);
				if(type != ERROR_TOKEN){
					TokenData * token = addToken(String(allocator, tokenStart, str - tokenStart, false, true), type, cur_line, tokenStart - line_start);
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

OS::Core::Compiler::Expression * OS::Core::Compiler::ExpressionList::add(Expression * exp)
{
	allocator->vectorAddItem(*this, exp);
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
}

OS::Core::Compiler::Expression::Expression(ExpressionType p_type, TokenData * p_token, Expression * e1): list(p_token->getAllocator())
{
	token = p_token->retain();
	type = p_type;
	list.add(e1);
	ret_values = 0;
	active_locals = 0;
}

OS::Core::Compiler::Expression::Expression(ExpressionType p_type, TokenData * p_token, Expression * e1, Expression * e2): list(p_token->getAllocator())
{
	token = p_token->retain();
	type = p_type;
	list.add(e1);
	list.add(e2);
	ret_values = 0;
	active_locals = 0;
}

OS::Core::Compiler::Expression::Expression(ExpressionType p_type, TokenData * p_token, Expression * e1, Expression * e2, Expression * e3): list(p_token->getAllocator())
{
	token = p_token->retain();
	type = p_type;
	list.add(e1);
	list.add(e2);
	list.add(e3);
	ret_values = 0;
	active_locals = 0;
}


OS::Core::Compiler::Expression::~Expression()
{
	token->release();
}

bool OS::Core::Compiler::Expression::isConstValue() const
{
	switch(type){
	// case EXP_TYPE_CODE_LIST:
	// 	return list.count ? list[list.count-1]->isValue() : false;

	case EXP_TYPE_CONST_STRING:
	case EXP_TYPE_CONST_NUMBER:
	case EXP_TYPE_CONST_NULL:
	case EXP_TYPE_CONST_TRUE:
	case EXP_TYPE_CONST_FALSE:
		// case EXP_TYPE_NAME:
		// case EXP_TYPE_CALL:
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
	// case EXP_TYPE_CODE_LIST:
	// 	return list.count ? list[list.count-1]->isWriteable() : false;

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
		// case EXP_TYPE_INC:	// ++
		//case EXP_TYPE_DEC:	// --
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

void OS::Core::Compiler::Expression::debugPrint(StringBuffer& out, OS::Core::Compiler * compiler, int depth)
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
	case EXP_TYPE_NOP:
		out += String::format(allocator, OS_TEXT("%snop\n"), spaces);
		break;

	case EXP_TYPE_CODE_LIST:
		type_name = OS::Core::Compiler::getExpName(type);
		out += String::format(allocator, OS_TEXT("%sbegin %s\n"), spaces, type_name);
		for(i = 0; i < list.count; i++){
			if(i > 0){
				out.append(OS_TEXT("\n"));
			}
			// OS_ASSERT(i+1 == list.count ? list[i]->ret_values == ret_values : list[i]->ret_values == 0);
			list[i]->debugPrint(out, compiler, depth+1);
		}
		out += String::format(allocator, OS_TEXT("%send %s ret values %d\n"), spaces, type_name, ret_values);
		break;

	case EXP_TYPE_IF:
		OS_ASSERT(list.count == 2 || list.count == 3);
		out += String::format(allocator, OS_TEXT("%sbegin if\n"), spaces);
			out += String::format(allocator, OS_TEXT("%s  begin bool exp\n"), spaces);
				list[0]->debugPrint(out, compiler, depth+2);
			out += String::format(allocator, OS_TEXT("%s  end bool exp\n"), spaces);
			out += String::format(allocator, OS_TEXT("%s  begin then\n"), spaces);
				list[1]->debugPrint(out, compiler, depth+2);
			out += String::format(allocator, OS_TEXT("%s  end then\n"), spaces);
			if(list.count == 3){
				out += String::format(allocator, OS_TEXT("%s  begin else\n"), spaces);
					list[2]->debugPrint(out, compiler, depth+2);
				out += String::format(allocator, OS_TEXT("%s  end else\n"), spaces);
			}
		out += String::format(allocator, OS_TEXT("%send if ret values %d\n"), spaces, ret_values);
		break;

	case EXP_TYPE_CONST_NUMBER:
	case EXP_TYPE_CONST_STRING:
		{
			const OS_CHAR * end = OS_TEXT("");
			switch(token->getType()){
			case Tokenizer::NUMBER: type_name = OS_TEXT("number "); break;
			case Tokenizer::STRING: type_name = OS_TEXT("string \""); end = OS_TEXT("\""); break;
			case Tokenizer::NAME: type_name = OS_TEXT("string \""); end = OS_TEXT("\""); break;
			default: type_name = OS_TEXT("???"); break;
			}
			out += String::format(allocator, OS_TEXT("%spush const %s%s%s\n"), spaces, type_name, token->str.toChar(), end);
		}
		break;

	case EXP_TYPE_CONST_NULL:
	case EXP_TYPE_CONST_TRUE:
	case EXP_TYPE_CONST_FALSE:
		out += String::format(allocator, OS_TEXT("%s%s\n"), spaces, getExpName(type));
		break;

	case EXP_TYPE_NAME:
		out += String::format(allocator, OS_TEXT("%sname %s\n"), spaces, token->str.toChar());
		break;

	case EXP_TYPE_PARAMS:
		out += String::format(allocator, OS_TEXT("%sbegin params %d\n"), spaces, list.count);
		for(i = 0; i < list.count; i++){
			if(i > 0){
				out += String::format(allocator, OS_TEXT("%s  ,\n"), spaces);
			}
			list[i]->debugPrint(out, compiler, depth+1);
		}
		out += String::format(allocator, OS_TEXT("%send params ret values %d\n"), spaces, ret_values);
		break;

	case EXP_TYPE_ARRAY:
		out += String::format(allocator, OS_TEXT("%sbegin array %d\n"), spaces, list.count);
		for(i = 0; i < list.count; i++){
			if(i > 0){
				out += String::format(allocator, OS_TEXT("%s  ,\n"), spaces);
			}
			list[i]->debugPrint(out, compiler, depth+1);
		}
		out += String::format(allocator, OS_TEXT("%send array\n"), spaces);
		break;

	case EXP_TYPE_OBJECT:
		out += String::format(allocator, OS_TEXT("%sbegin object %d\n"), spaces, list.count);
		for(i = 0; i < list.count; i++){
			if(i > 0){
				out += String::format(allocator, OS_TEXT("%s  ,\n"), spaces);
			}
			list[i]->debugPrint(out, compiler, depth+1);
		}
		out += String::format(allocator, OS_TEXT("%send object\n"), spaces);
		break;

	case EXP_TYPE_OBJECT_SET_BY_NAME:
		OS_ASSERT(list.count == 1);
		out += String::format(allocator, OS_TEXT("%sbegin set by name\n"), spaces);
			list[0]->debugPrint(out, compiler, depth+1);
		out += String::format(allocator, OS_TEXT("%send set by name: [%s]\n"), spaces, token->str.toChar());
		break;

	case EXP_TYPE_OBJECT_SET_BY_INDEX:
		OS_ASSERT(list.count == 1);
		out += String::format(allocator, OS_TEXT("%sbegin set by index\n"), spaces);
			list[0]->debugPrint(out, compiler, depth+1);
		out += String::format(allocator, OS_TEXT("%send set by index: [%d]\n"), spaces, (int)token->getFloat());
		break;

	case EXP_TYPE_OBJECT_SET_BY_EXP:
		OS_ASSERT(list.count == 2);
		out += String::format(allocator, OS_TEXT("%sbegin set by exp\n"), spaces);
			list[0]->debugPrint(out, compiler, depth+1);
			list[1]->debugPrint(out, compiler, depth+1);
		out += String::format(allocator, OS_TEXT("%send set by exp\n"), spaces);
		break;

	case EXP_TYPE_OBJECT_SET_BY_AUTO_INDEX:
		OS_ASSERT(list.count == 1);
		out += String::format(allocator, OS_TEXT("%sbegin set like array\n"), spaces);
			list[0]->debugPrint(out, compiler, depth+1);
		out += String::format(allocator, OS_TEXT("%send set like array\n"), spaces);
		break;

	case EXP_TYPE_FUNCTION:
		{
			// OS_ASSERT(list.count >= 1);
			Scope * scope = dynamic_cast<Scope*>(this);
			OS_ASSERT(scope);
			out += String::format(allocator, OS_TEXT("%sbegin function\n"), spaces);
			if(scope->num_locals > 0){
				out += String::format(allocator, OS_TEXT("%s  begin locals, total %d\n"), spaces, scope->num_locals);
				for(i = 0; i < scope->locals.count; i++){
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
					out.append(OS_TEXT("\n"));
				}
				list[i]->debugPrint(out, compiler, depth+1);
			}
			out += String::format(allocator, OS_TEXT("%send function\n"), spaces);
			break;
		}

	case EXP_TYPE_SCOPE:
	case EXP_TYPE_LOOP_SCOPE:
		{
			// OS_ASSERT(list.count >= 1);
			Scope * scope = dynamic_cast<Scope*>(this);
			OS_ASSERT(scope);
			const OS_CHAR * exp_name = OS::Core::Compiler::getExpName(type);
			out += String::format(allocator, OS_TEXT("%sbegin %s\n"), spaces, exp_name);
			if(scope->locals.count > 0){
				out += String::format(allocator, OS_TEXT("%s  begin locals %d\n"), spaces, scope->locals.count);
				for(i = 0; i < scope->locals.count; i++){
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
					out.append(OS_TEXT("\n"));
				}
				list[i]->debugPrint(out, compiler, depth+1);
			}
			out += String::format(allocator, OS_TEXT("%send %s ret values %d\n"), spaces, exp_name, ret_values);
			break;
		}

	case EXP_TYPE_RETURN:
		if(list.count > 0){
			out += String::format(allocator, OS_TEXT("%sbegin return\n"), spaces);
			for(i = 0; i < list.count; i++){
				if(i > 0){
					out += String::format(allocator, OS_TEXT("%s  ,\n"), spaces);
				}
				list[i]->debugPrint(out, compiler, depth+1);
			}
			out += String::format(allocator, OS_TEXT("%send return values %d\n"), spaces, ret_values);
		}else{
			out += String::format(allocator, OS_TEXT("%sreturn\n"), spaces);
		}
		break;

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

	case EXP_TYPE_DEBUG_LOCALS:
		out += String::format(allocator, OS_TEXT("%sbegin debug locals\n"), spaces);
		for(i = 0; i < list.count; i++){
			if(i > 0){
				out += String::format(allocator, OS_TEXT("%s  ,\n"), spaces);
			}
			list[i]->debugPrint(out, compiler, depth+1);
		}
		out += String::format(allocator, OS_TEXT("%send debug locals\n"), spaces);
		break;

	case EXP_TYPE_TAIL_CALL:
		OS_ASSERT(list.count == 2);
		out += String::format(allocator, OS_TEXT("%sbegin %s\n"), spaces, OS::Core::Compiler::getExpName(type));
			list[0]->debugPrint(out, compiler, depth+1);
			list[1]->debugPrint(out, compiler, depth+1);
		out += String::format(allocator, OS_TEXT("%send %s\n"), spaces, OS::Core::Compiler::getExpName(type));
		break;

	case EXP_TYPE_TAIL_CALL_METHOD:
		OS_ASSERT(list.count == 2);
		out += String::format(allocator, OS_TEXT("%sbegin %s\n"), spaces, OS::Core::Compiler::getExpName(type));
			list[0]->debugPrint(out, compiler, depth+1);
			list[1]->debugPrint(out, compiler, depth+1);
		out += String::format(allocator, OS_TEXT("%send %s\n"), spaces, OS::Core::Compiler::getExpName(type));
		break;

	case EXP_TYPE_CALL:
	case EXP_TYPE_CALL_AUTO_PARAM:
		OS_ASSERT(list.count == 2);
		out += String::format(allocator, OS_TEXT("%sbegin %s\n"), spaces, OS::Core::Compiler::getExpName(type));
			list[0]->debugPrint(out, compiler, depth+1);
			list[1]->debugPrint(out, compiler, depth+1);
		out += String::format(allocator, OS_TEXT("%send %s ret values %d\n"), spaces, OS::Core::Compiler::getExpName(type), ret_values);
		break;

	case EXP_TYPE_CALL_DIM:
	// case EXP_TYPE_GET_DIM:
	case EXP_TYPE_CALL_METHOD:
	case EXP_TYPE_GET_PROPERTY:
	case EXP_TYPE_GET_PROPERTY_AUTO_CREATE:
	// case EXP_TYPE_GET_PROPERTY_DIM:
	// case EXP_TYPE_SET_ENV_VAR_DIM:
	case EXP_TYPE_EXTENDS:
		OS_ASSERT(list.count == 2);
		out += String::format(allocator, OS_TEXT("%sbegin %s\n"), spaces, OS::Core::Compiler::getExpName(type));
			list[0]->debugPrint(out, compiler, depth+1);
			list[1]->debugPrint(out, compiler, depth+1);
		out += String::format(allocator, OS_TEXT("%send %s ret values %d\n"), spaces, OS::Core::Compiler::getExpName(type), ret_values);
		break;

	case EXP_TYPE_DELETE:
		OS_ASSERT(list.count == 2);
		out += String::format(allocator, OS_TEXT("%sbegin %s\n"), spaces, OS::Core::Compiler::getExpName(type));
			list[0]->debugPrint(out, compiler, depth+1);
			list[1]->debugPrint(out, compiler, depth+1);
		out += String::format(allocator, OS_TEXT("%send %s\n"), spaces, OS::Core::Compiler::getExpName(type));
		break;

	case EXP_TYPE_CLONE:
		OS_ASSERT(list.count == 1);
		out += String::format(allocator, OS_TEXT("%sbegin %s\n"), spaces, OS::Core::Compiler::getExpName(type));
			list[0]->debugPrint(out, compiler, depth+1);
		out += String::format(allocator, OS_TEXT("%send %s\n"), spaces, OS::Core::Compiler::getExpName(type));
		break;

	case EXP_TYPE_VALUE:
		OS_ASSERT(list.count == 1);
		out += String::format(allocator, OS_TEXT("%sbegin %s\n"), spaces, OS::Core::Compiler::getExpName(type));
			list[0]->debugPrint(out, compiler, depth+1);
		out += String::format(allocator, OS_TEXT("%send %s\n"), spaces, OS::Core::Compiler::getExpName(type));
		break;

	case EXP_TYPE_POP_VALUE:
		OS_ASSERT(list.count == 1);
		out += String::format(allocator, OS_TEXT("%sbegin %s\n"), spaces, OS::Core::Compiler::getExpName(type));
			list[0]->debugPrint(out, compiler, depth+1);
		out += String::format(allocator, OS_TEXT("%send %s ret values %d\n"), spaces, OS::Core::Compiler::getExpName(type), ret_values);
		break;

	case EXP_TYPE_TYPE_OF:
	case EXP_TYPE_VALUE_OF:
	case EXP_TYPE_NUMBER_OF:
	case EXP_TYPE_STRING_OF:
	case EXP_TYPE_ARRAY_OF:
	case EXP_TYPE_OBJECT_OF:
	case EXP_TYPE_USERDATA_OF:
	case EXP_TYPE_FUNCTION_OF:
	case EXP_TYPE_PLUS:			// +
	case EXP_TYPE_NEG:			// -
	case EXP_TYPE_LENGTH:		// #
	case EXP_TYPE_LOGIC_BOOL:	// !!
	case EXP_TYPE_LOGIC_NOT:	// !
	case EXP_TYPE_BIT_NOT:		// ~
	case EXP_TYPE_PRE_INC:		// ++
	case EXP_TYPE_PRE_DEC:		// --
	case EXP_TYPE_POST_INC:		// ++
	case EXP_TYPE_POST_DEC:		// --
		{
			OS_ASSERT(list.count == 1);
			const OS_CHAR * exp_name = OS::Core::Compiler::getExpName(type);
			out += String::format(allocator, OS_TEXT("%sbegin %s\n"), spaces, exp_name);
				list[0]->debugPrint(out, compiler, depth+1);
			out += String::format(allocator, OS_TEXT("%send %s\n"), spaces, exp_name);
			break;
		}

	case EXP_TYPE_INDIRECT:
	case EXP_TYPE_ASSIGN:
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
		// case EXP_TYPE_BIT_NOT: // ~
	case EXP_TYPE_BIT_AND_ASSIGN: // &=
	case EXP_TYPE_BIT_OR_ASSIGN:  // |=
	case EXP_TYPE_BIT_XOR_ASSIGN: // ^=
	case EXP_TYPE_BIT_NOT_ASSIGN: // ~=
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
			out += String::format(allocator, OS_TEXT("%sbegin %s\n"), spaces, exp_name);
				list[0]->debugPrint(out, compiler, depth+1);
				list[1]->debugPrint(out, compiler, depth+1);
			out += String::format(allocator, OS_TEXT("%send %s\n"), spaces, exp_name);
			break;
		}

	case EXP_TYPE_NEW_LOCAL_VAR:
		{
			OS_ASSERT(list.count == 0);
			String info = String::format(allocator, OS_TEXT("(%d %d%s)"),
				local_var.index, local_var.up_count, 
				local_var.type == LOCAL_GENERIC ? OS_TEXT(" param") : (local_var.type == LOCAL_TEMP ? OS_TEXT(" temp") : OS_TEXT("")));
			out += String::format(allocator, OS_TEXT("%snew local var %s %s\n"), spaces, token->str.toChar(), info.toChar());
			break;
		}

	case EXP_TYPE_GET_THIS:
	case EXP_TYPE_GET_ARGUMENTS:
	case EXP_TYPE_GET_REST_ARGUMENTS:
		{
			OS_ASSERT(list.count == 0);
			const OS_CHAR * exp_name = OS::Core::Compiler::getExpName(type);
			out += String::format(allocator, OS_TEXT("%s%s\n"), spaces, exp_name);
			break;
		}

	case EXP_TYPE_GET_LOCAL_VAR:
	case EXP_TYPE_GET_LOCAL_VAR_AUTO_CREATE:
		{
			OS_ASSERT(list.count == 0);
			const OS_CHAR * exp_name = OS::Core::Compiler::getExpName(type);
			String info = String::format(allocator, OS_TEXT("(%d %d%s)"),
				local_var.index, local_var.up_count, 
				local_var.type == LOCAL_GENERIC ? OS_TEXT(" param") : (local_var.type == LOCAL_TEMP ? OS_TEXT(" temp") : OS_TEXT("")));
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

	case EXP_TYPE_SET_LOCAL_VAR:
		{
			OS_ASSERT(list.count == 1);
			const OS_CHAR * exp_name = OS::Core::Compiler::getExpName(type);
			String info = String::format(allocator, OS_TEXT("(%d %d%s)"),
				local_var.index, local_var.up_count, 
				local_var.type == LOCAL_GENERIC ? OS_TEXT(" param") : (local_var.type == LOCAL_TEMP ? OS_TEXT(" temp") : OS_TEXT("")));
			out += String::format(allocator, OS_TEXT("%sbegin %s\n"), spaces, exp_name);
				list[0]->debugPrint(out, compiler, depth+1);
			out += String::format(allocator, OS_TEXT("%send %s %s %s\n"), spaces, exp_name, token->str.toChar(), info.toChar());
			break;
		}

	case EXP_TYPE_SET_ENV_VAR:
		{
			OS_ASSERT(list.count == 1);
			const OS_CHAR * exp_name = OS::Core::Compiler::getExpName(type);
			out += String::format(allocator, OS_TEXT("%sbegin %s\n"), spaces, exp_name);
				list[0]->debugPrint(out, compiler, depth+1);
			out += String::format(allocator, OS_TEXT("%send %s %s\n"), spaces, exp_name, token->str.toChar());
			break;
		}

	case EXP_TYPE_SET_PROPERTY:
	// case EXP_TYPE_SET_PROPERTY_DIM:
	case EXP_TYPE_SET_DIM:
		{
			OS_ASSERT(list.count == 3);
			const OS_CHAR * exp_name = OS::Core::Compiler::getExpName(type);
			out += String::format(allocator, OS_TEXT("%sbegin %s\n"), spaces, exp_name);
				list[0]->debugPrint(out, compiler, depth+1);
				list[1]->debugPrint(out, compiler, depth+1);
				list[2]->debugPrint(out, compiler, depth+1);
			out += String::format(allocator, OS_TEXT("%send %s ret values %d\n"), spaces, exp_name, ret_values);
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
		OS_ASSERT(prop->value.type == OS_VALUE_TYPE_NUMBER);
		return (int)prop->value.v.number;
	}
	prop = new (malloc(sizeof(Property))) Property(index);
	prop->value = ValueData(strings_table->count);
	allocator->core->addTableProperty(strings_table, prop);
	allocator->vectorAddItem(strings, str);
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

int OS::Core::Compiler::cacheNumber(OS_FLOAT num)
{
	PropertyIndex index(num);
	Property * prop = prog_numbers_table->get(index);
	if(prop){
		OS_ASSERT(prop->value.type == OS_VALUE_TYPE_NUMBER);
		return (int)prop->value.v.number;
	}
	prop = new (malloc(sizeof(Property))) Property(index);
	prop->value = ValueData(prog_numbers_table->count);
	allocator->core->addTableProperty(prog_numbers_table, prop);
	allocator->vectorAddItem(prog_numbers, num);
	OS_ASSERT(prog_numbers_table->count == prog_numbers.count);
	return prog_numbers_table->count-1;
}

void OS::Core::Compiler::writeDebugInfo(Expression * exp)
{
	if(prog_debug_info){
		prog_num_debug_infos++;
		prog_debug_info->writeUVariable(prog_opcodes->getPos());
		prog_debug_info->writeUVariable(exp->token->line+1);
		prog_debug_info->writeUVariable(exp->token->pos+1);
		prog_debug_info->writeUVariable(cacheDebugString(exp->token->str));
	}
}

bool OS::Core::Compiler::writeOpcodes(Scope * scope, ExpressionList& list)
{
	for(int i = 0; i < list.count; i++){
		if(!writeOpcodes(scope, list[i])){
			return false;
		}
	}
	return true;
}

bool OS::Core::Compiler::writeOpcodes(Scope * scope, Expression * exp)
{
	int i;
	writeDebugInfo(exp);
	switch(exp->type){
	default:
		{
			ExpressionType exp_type = exp->type;
			OS_ASSERT(false);
			break;
		}

	case EXP_TYPE_NOP:
		break;

	case EXP_TYPE_NEW_LOCAL_VAR:
		break;

	case EXP_TYPE_VALUE:
	case EXP_TYPE_CODE_LIST:
		if(!writeOpcodes(scope, exp->list)){
			return false;
		}
		break;

	case EXP_TYPE_CONST_NUMBER:
		OS_ASSERT(exp->list.count == 0);
		prog_opcodes->writeByte(Program::OP_PUSH_NUMBER);
		prog_opcodes->writeUVariable(cacheNumber(exp->token->getFloat()));
		break;

	case EXP_TYPE_CONST_STRING:
		OS_ASSERT(exp->list.count == 0);
		prog_opcodes->writeByte(Program::OP_PUSH_STRING);
		prog_opcodes->writeUVariable(cacheString(exp->token->str));
		break;

	case EXP_TYPE_CONST_NULL:
		OS_ASSERT(exp->list.count == 0);
		prog_opcodes->writeByte(Program::OP_PUSH_NULL);
		break;

	case EXP_TYPE_CONST_TRUE:
		OS_ASSERT(exp->list.count == 0);
		prog_opcodes->writeByte(Program::OP_PUSH_TRUE);
		break;

	case EXP_TYPE_CONST_FALSE:
		OS_ASSERT(exp->list.count == 0);
		prog_opcodes->writeByte(Program::OP_PUSH_FALSE);
		break;

	case EXP_TYPE_FUNCTION:
		{
			Scope * scope = dynamic_cast<Scope*>(exp);
			OS_ASSERT(scope);
			prog_opcodes->writeByte(Program::OP_PUSH_FUNCTION);

			int func_index = scope->func_index; // prog_functions.indexOf(scope);
			OS_ASSERT(func_index >= 0);
			prog_opcodes->writeUVariable(func_index);

			allocator->vectorReserveCapacity(scope->locals_compiled, scope->num_locals);
			scope->locals_compiled.count = scope->num_locals;

			scope->opcodes_pos = prog_opcodes->getPos();
			if(!writeOpcodes(scope, exp->list)){
				return false;
			}
			prog_opcodes->writeByte(Program::OP_RETURN);
			prog_opcodes->writeByte(0);
			scope->opcodes_size = prog_opcodes->getPos() - scope->opcodes_pos;

			for(i = 0; i < scope->locals.count; i++){
				Scope::LocalVar& var = scope->locals[i];
				Scope::LocalVarCompiled& var_scope = scope->locals_compiled[var.index];
				var_scope.cached_name_index = cacheString(var.name);
				var_scope.start_code_pos = scope->opcodes_pos;
				var_scope.end_code_pos = prog_opcodes->getPos();
			}
			break;
		}

	case EXP_TYPE_SCOPE:
	case EXP_TYPE_LOOP_SCOPE:
		{
			Scope * scope = dynamic_cast<Scope*>(exp);
			OS_ASSERT(scope);
			int start_code_pos = prog_opcodes->getPos();
			if(!writeOpcodes(scope, exp->list)){
				return false;
			}
			if(exp->type == EXP_TYPE_LOOP_SCOPE){
				prog_opcodes->writeByte(Program::OP_JUMP);
				prog_opcodes->writeInt32(start_code_pos - prog_opcodes->getPos() - sizeof(OS_INT32));

				scope->fixLoopBreaks(start_code_pos, prog_opcodes->getPos(), prog_opcodes);
			}else{
				OS_ASSERT(scope->loop_breaks.count == 0);
			}
			for(i = 0; i < scope->locals.count; i++){
				Scope::LocalVar& var = scope->locals[i];
				Scope::LocalVarCompiled& var_scope = scope->function->locals_compiled[var.index];
				var_scope.cached_name_index = cacheString(var.name);
				var_scope.start_code_pos = start_code_pos;
				var_scope.end_code_pos = prog_opcodes->getPos();
			}
			break;
		}

	case EXP_TYPE_IF:
		{
			OS_ASSERT(exp->list.count == 2 || exp->list.count == 3);
			if(!writeOpcodes(scope, exp->list[0])){
				return false;
			}
			prog_opcodes->writeByte(Program::OP_IF_NOT_JUMP);
			
			int if_not_jump_pos = prog_opcodes->getPos();
			prog_opcodes->writeInt32(0);

			if(!writeOpcodes(scope, exp->list[1])){
				return false;
			}

			int if_not_jump_to = prog_opcodes->getPos();
			if(exp->list.count == 3 && exp->list[2]->list.count > 0){
				prog_opcodes->writeByte(Program::OP_JUMP);

				int jump_pos = prog_opcodes->getPos();
				prog_opcodes->writeInt32(0);
				
				if_not_jump_to = prog_opcodes->getPos();
				if(!writeOpcodes(scope, exp->list[2])){
					return false;
				}
				prog_opcodes->writeInt32AtPos(prog_opcodes->getPos() - jump_pos - sizeof(OS_INT32), jump_pos);
			}
			prog_opcodes->writeInt32AtPos(if_not_jump_to - if_not_jump_pos - sizeof(OS_INT32), if_not_jump_pos);
			break;
		}

	case EXP_TYPE_LOGIC_AND: // &&
	case EXP_TYPE_LOGIC_OR:  // ||
		{
			OS_ASSERT(exp->list.count == 2);
			if(!writeOpcodes(scope, exp->list[0])){
				return false;
			}
			prog_opcodes->writeByte(Program::getOpcodeType(exp->type));
			
			int op_jump_pos = prog_opcodes->getPos();
			prog_opcodes->writeInt32(0);

			if(!writeOpcodes(scope, exp->list[1])){
				return false;
			}

			int op_jump_to = prog_opcodes->getPos();
			prog_opcodes->writeInt32AtPos(op_jump_to - op_jump_pos - sizeof(OS_INT32), op_jump_pos);
			break;
		}

	case EXP_TYPE_EXTENDS:
		OS_ASSERT(exp->list.count == 2);
		if(!writeOpcodes(scope, exp->list)){
			return false;
		}
		prog_opcodes->writeByte(Program::OP_EXTENDS);
		break;

	case EXP_TYPE_CLONE:
		OS_ASSERT(exp->list.count == 1);
		if(!writeOpcodes(scope, exp->list)){
			return false;
		}
		prog_opcodes->writeByte(Program::OP_CLONE);
		break;

	case EXP_TYPE_DELETE:
		OS_ASSERT(exp->list.count == 2);
		if(!writeOpcodes(scope, exp->list)){
			return false;
		}
		prog_opcodes->writeByte(Program::OP_DELETE_PROP);
		break;

	case EXP_TYPE_OBJECT:
		// OS_ASSERT(exp->list.count >= 0);
		prog_opcodes->writeByte(Program::OP_PUSH_NEW_OBJECT);
		if(!writeOpcodes(scope, exp->list)){
			return false;
		}
		break;

	case EXP_TYPE_OBJECT_SET_BY_AUTO_INDEX:
		OS_ASSERT(exp->list.count == 1);
		if(!writeOpcodes(scope, exp->list)){
			return false;
		}
		prog_opcodes->writeByte(Program::OP_OBJECT_SET_BY_AUTO_INDEX);
		break;

	case EXP_TYPE_OBJECT_SET_BY_EXP:
		OS_ASSERT(exp->list.count == 2);
		if(!writeOpcodes(scope, exp->list)){
			return false;
		}
		prog_opcodes->writeByte(Program::OP_OBJECT_SET_BY_EXP);
		break;

	case EXP_TYPE_OBJECT_SET_BY_INDEX:
		OS_ASSERT(exp->list.count == 1);
		if(!writeOpcodes(scope, exp->list)){
			return false;
		}
		prog_opcodes->writeByte(Program::OP_OBJECT_SET_BY_INDEX);
		// prog_opcodes->writeInt64(exp->token->getInt());
		prog_opcodes->writeUVariable(cacheNumber(exp->token->getFloat()));
		break;

	case EXP_TYPE_OBJECT_SET_BY_NAME:
		OS_ASSERT(exp->list.count == 1);
		if(!writeOpcodes(scope, exp->list)){
			return false;
		}
		prog_opcodes->writeByte(Program::OP_OBJECT_SET_BY_NAME);
		prog_opcodes->writeUVariable(cacheString(exp->token->str));
		break;

	case EXP_TYPE_GET_ENV_VAR:
		OS_ASSERT(exp->list.count == 0);
		prog_opcodes->writeByte(Program::OP_PUSH_ENV_VAR);
		prog_opcodes->writeUVariable(cacheString(exp->token->str));
		break;

	case EXP_TYPE_GET_ENV_VAR_AUTO_CREATE:
		OS_ASSERT(exp->list.count == 0);
		prog_opcodes->writeByte(Program::OP_PUSH_ENV_VAR_AUTO_CREATE);
		prog_opcodes->writeUVariable(cacheString(exp->token->str));
		break;

	case EXP_TYPE_SET_ENV_VAR:
		OS_ASSERT(exp->list.count > 0);
		if(!writeOpcodes(scope, exp->list)){
			return false;
		}
		prog_opcodes->writeByte(Program::OP_SET_ENV_VAR);
		prog_opcodes->writeUVariable(cacheString(exp->token->str));
		break;

	case EXP_TYPE_GET_THIS:
	case EXP_TYPE_GET_ARGUMENTS:
	case EXP_TYPE_GET_REST_ARGUMENTS:
		OS_ASSERT(exp->list.count == 0);
		prog_opcodes->writeByte(Program::getOpcodeType(exp->type));
		break;

	case EXP_TYPE_GET_LOCAL_VAR:
		OS_ASSERT(exp->list.count == 0);
		if(!exp->local_var.up_count){
			prog_opcodes->writeByte(Program::OP_PUSH_LOCAL_VAR);
			prog_opcodes->writeUVariable(exp->local_var.index);
		}else{
			prog_opcodes->writeByte(Program::OP_PUSH_UP_LOCAL_VAR);
			prog_opcodes->writeUVariable(exp->local_var.index);
			prog_opcodes->writeByte(exp->local_var.up_count);
		}
		break;

	case EXP_TYPE_GET_LOCAL_VAR_AUTO_CREATE:
		OS_ASSERT(exp->list.count == 0);
		if(!exp->local_var.up_count){
			prog_opcodes->writeByte(Program::OP_PUSH_LOCAL_VAR_AUTO_CREATE);
			prog_opcodes->writeUVariable(exp->local_var.index);
		}else{
			prog_opcodes->writeByte(Program::OP_PUSH_UP_LOCAL_VAR_AUTO_CREATE);
			prog_opcodes->writeUVariable(exp->local_var.index);
			prog_opcodes->writeByte(exp->local_var.up_count);
		}
		break;

	case EXP_TYPE_SET_LOCAL_VAR:
		OS_ASSERT(exp->list.count > 0);
		if(!writeOpcodes(scope, exp->list)){
			return false;
		}
		if(!exp->local_var.up_count){
			prog_opcodes->writeByte(Program::OP_SET_LOCAL_VAR);
			prog_opcodes->writeUVariable(exp->local_var.index);
		}else{
			prog_opcodes->writeByte(Program::OP_SET_UP_LOCAL_VAR);
			prog_opcodes->writeUVariable(exp->local_var.index);
			prog_opcodes->writeByte(exp->local_var.up_count);
		}
		break;

	case EXP_TYPE_CALL:
	case EXP_TYPE_CALL_AUTO_PARAM:
		OS_ASSERT(exp->list.count == 2);
		OS_ASSERT(exp->list[1]->type == EXP_TYPE_PARAMS);
		if(!writeOpcodes(scope, exp->list)){
			return false;
		}
		prog_opcodes->writeByte(Program::getOpcodeType(exp->type));
		prog_opcodes->writeByte(exp->list[1]->ret_values); // params number
		prog_opcodes->writeByte(exp->ret_values);
		break;

	case EXP_TYPE_TAIL_CALL:
		OS_ASSERT(exp->list.count == 2);
		OS_ASSERT(exp->list[1]->type == EXP_TYPE_PARAMS);
		if(!writeOpcodes(scope, exp->list)){
			return false;
		}
		prog_opcodes->writeByte(Program::getOpcodeType(exp->type));
		prog_opcodes->writeByte(exp->list[1]->ret_values); // params number
		break;

	// case EXP_TYPE_GET_DIM:
	case EXP_TYPE_CALL_METHOD:
		OS_ASSERT(exp->list.count == 2);
		OS_ASSERT(exp->list[1]->type == EXP_TYPE_PARAMS);
		if(!writeOpcodes(scope, exp->list)){
			return false;
		}
		prog_opcodes->writeByte(Program::getOpcodeType(exp->type));
		prog_opcodes->writeByte(exp->list[1]->ret_values-1); // params number
		prog_opcodes->writeByte(exp->ret_values);
		break;

	case EXP_TYPE_TAIL_CALL_METHOD:
		OS_ASSERT(exp->list.count == 2);
		OS_ASSERT(exp->list[1]->type == EXP_TYPE_PARAMS);
		if(!writeOpcodes(scope, exp->list)){
			return false;
		}
		prog_opcodes->writeByte(Program::getOpcodeType(exp->type));
		prog_opcodes->writeByte(exp->list[1]->ret_values-1); // params number
		break;

	case EXP_TYPE_GET_PROPERTY:
		OS_ASSERT(exp->list.count == 2);
		if(!writeOpcodes(scope, exp->list)){
			return false;
		}
		prog_opcodes->writeByte(Program::OP_GET_PROPERTY);
		prog_opcodes->writeByte(exp->ret_values);
		break;

	case EXP_TYPE_GET_PROPERTY_AUTO_CREATE:
		OS_ASSERT(exp->list.count == 2);
		if(!writeOpcodes(scope, exp->list)){
			return false;
		}
		prog_opcodes->writeByte(Program::OP_GET_PROPERTY_AUTO_CREATE);
		prog_opcodes->writeByte(exp->ret_values);
		break;

	case EXP_TYPE_SET_PROPERTY:
		OS_ASSERT(exp->list.count == 3);
		if(!writeOpcodes(scope, exp->list)){
			return false;
		}
		prog_opcodes->writeByte(Program::OP_SET_PROPERTY);
		break;

	case EXP_TYPE_SET_DIM:
		OS_ASSERT(exp->list.count == 3);
		if(!writeOpcodes(scope, exp->list)){
			return false;
		}
		prog_opcodes->writeByte(Program::OP_SET_DIM);
		prog_opcodes->writeByte(exp->list[2]->list.count); // params
		break;

	case EXP_TYPE_PARAMS:
		if(!writeOpcodes(scope, exp->list)){
			return false;
		}
		break;

	case EXP_TYPE_RETURN:
		if(!writeOpcodes(scope, exp->list)){
			return false;
		}
		prog_opcodes->writeByte(Program::OP_RETURN);
		prog_opcodes->writeByte(exp->ret_values);
		break;

	case EXP_TYPE_BREAK:
		OS_ASSERT(exp->list.count == 0);
		prog_opcodes->writeByte(Program::OP_JUMP);
		scope->addLoopBreak(prog_opcodes->getPos(), Scope::LOOP_BREAK);
		prog_opcodes->writeInt32(0);
		break;

	case EXP_TYPE_CONTINUE:
		OS_ASSERT(exp->list.count == 0);
		prog_opcodes->writeByte(Program::OP_JUMP);
		scope->addLoopBreak(prog_opcodes->getPos(), Scope::LOOP_CONTINUE);
		prog_opcodes->writeInt32(0);
		break;

	case EXP_TYPE_DEBUGGER:
		{
			OS_ASSERT(exp->list.count == 0);
			prog_opcodes->writeByte(Program::OP_DEBUGGER);
			prog_opcodes->writeUVariable(exp->token->line + 1);
			prog_opcodes->writeUVariable(exp->token->pos + 1);
			prog_opcodes->writeUVariable(OS_DEBUGGER_SAVE_NUM_LINES);
			Core::String empty(allocator);
			for(int i = 0; i < OS_DEBUGGER_SAVE_NUM_LINES; i++){
				int j = exp->token->line - OS_DEBUGGER_SAVE_NUM_LINES/2 + i;
				if(j >= 0 && j < exp->token->text_data->lines.count){
					prog_opcodes->writeUVariable(cacheString(exp->token->text_data->lines[j]));
				}else{
					prog_opcodes->writeUVariable(cacheString(empty));
				}
			}
			break;
		}

	case EXP_TYPE_DEBUG_LOCALS:
		if(!writeOpcodes(scope, exp->list)){
			return false;
		}
		break;

	case EXP_TYPE_POP_VALUE:
		OS_ASSERT(exp->list.count == 1);
		if(!writeOpcodes(scope, exp->list)){
			return false;
		}
		prog_opcodes->writeByte(Program::OP_POP);
		break;

	case EXP_TYPE_TYPE_OF:
	case EXP_TYPE_VALUE_OF:
	case EXP_TYPE_NUMBER_OF:
	case EXP_TYPE_STRING_OF:
	case EXP_TYPE_ARRAY_OF:
	case EXP_TYPE_OBJECT_OF:
	case EXP_TYPE_USERDATA_OF:
	case EXP_TYPE_FUNCTION_OF:
	case EXP_TYPE_LOGIC_BOOL:
	case EXP_TYPE_LOGIC_NOT:
	case EXP_TYPE_BIT_NOT:
	case EXP_TYPE_PLUS:
	case EXP_TYPE_NEG:
	case EXP_TYPE_LENGTH:
		OS_ASSERT(exp->list.count == 1);
		if(!writeOpcodes(scope, exp->list)){
			return false;
		}
		prog_opcodes->writeByte(Program::getOpcodeType(exp->type));
		break;

	case EXP_TYPE_CONCAT:

	// case EXP_TYPE_LOGIC_AND:
	// case EXP_TYPE_LOGIC_OR:
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

	case EXP_TYPE_ADD:
	case EXP_TYPE_SUB:
	case EXP_TYPE_MUL:
	case EXP_TYPE_DIV:
	case EXP_TYPE_MOD:
	case EXP_TYPE_LSHIFT:
	case EXP_TYPE_RSHIFT:
	case EXP_TYPE_POW:
		OS_ASSERT(exp->list.count == 2);
		if(!writeOpcodes(scope, exp->list)){
			return false;
		}
		prog_opcodes->writeByte(Program::getOpcodeType(exp->type));
		break;
	}
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
	func_index = -1;
	parser_started = false;
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
	getAllocator()->vectorAddItem(scope->loop_breaks, loop_break);
	return true;
}

void OS::Core::Compiler::Scope::fixLoopBreaks(int scope_start_pos, int scope_end_pos, StreamWriter * writer)
{
	for(int i = 0; i < loop_breaks.count; i++){
		LoopBreak& loop_break = loop_breaks[i];
		if(loop_break.type == LOOP_BREAK){
			int offs = scope_end_pos - loop_break.pos - sizeof(OS_INT32);
			writer->writeInt32AtPos(offs, loop_break.pos);
		}else{
			int offs = scope_start_pos - loop_break.pos - sizeof(OS_INT32);
			writer->writeInt32AtPos(offs, loop_break.pos);
		}
	}
}

void OS::Core::Compiler::Scope::addLocalVar(const String& name)
{
	OS * allocator = getAllocator();
	LocalVar local_var(name, function->num_locals);
	allocator->vectorAddItem(locals, local_var);
	function->num_locals++;
}

void OS::Core::Compiler::Scope::addLocalVar(const String& name, LocalVarDesc& local_var)
{
	local_var.index = function->num_locals;
	local_var.up_count = 0;
	local_var.type = LOCAL_GENERIC;
	addLocalVar(name);
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
	prog_opcodes = NULL;
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
	allocator->deleteObj(prog_opcodes);
	// allocator->deleteObj(tokenizer);
}

bool OS::Core::Compiler::compile()
{
	OS_ASSERT(!prog_opcodes && !prog_strings_table && !prog_debug_strings_table && !prog_numbers_table);
	OS_ASSERT(!prog_functions.count && !prog_numbers.count && !prog_strings.count);
	
	Scope * scope = NULL;
	if(!readToken()){
		setError(ERROR_EXPECT_TOKEN, recent_token);
	}else{
		scope = expectTextExpression();
	}
	if(scope){
		Expression * exp = postProcessExpression(scope, scope);
		OS_ASSERT(exp->type == EXP_TYPE_FUNCTION);

		prog_strings_table = allocator->core->newTable();
		prog_numbers_table = allocator->core->newTable();
		prog_opcodes = new (malloc(sizeof(MemStreamWriter))) MemStreamWriter(allocator);

		const String& filename = tokenizer->getTextData()->filename;
		if(allocator->core->settings.create_debug_opcodes){
			Core::StringBuffer dump(allocator);
			exp->debugPrint(dump, this, 0);
			String dump_filename = filename.getDataSize() 
				? allocator->changeFilenameExt(filename, allocator->getFilenameExt(OS_DEBUG_OPCODES_FILENAME)) 
				: String(allocator, OS_DEBUG_OPCODES_FILENAME);
			FileStreamWriter(allocator, dump_filename).writeBytes(dump.buf, dump.count * sizeof(OS_CHAR));
		}
		bool create_debug_info = filename.getDataSize() && allocator->core->settings.create_debug_info;
		if(create_debug_info){
			prog_debug_strings_table = allocator->core->newTable();
			prog_debug_info = new (malloc(sizeof(MemStreamWriter))) MemStreamWriter(allocator);
		}
		if(!writeOpcodes(scope, exp)){
		
		}

		MemStreamWriter mem_writer(allocator);
		MemStreamWriter debuginfo_mem_writer(allocator);
		saveToStream(&mem_writer, create_debug_info ? &debuginfo_mem_writer : NULL);

		if(create_debug_info){
			FileStreamWriter(allocator, allocator->changeFilenameExt(filename, OS_COMPILED_EXT)).writeBytes(mem_writer.buffer.buf, mem_writer.buffer.count);
			FileStreamWriter(allocator, allocator->changeFilenameExt(filename, OS_DEBUG_INFO_EXT)).writeBytes(debuginfo_mem_writer.buffer.buf, debuginfo_mem_writer.buffer.count);
		}
		
		Program * prog = new (malloc(sizeof(Program))) Program(allocator);
		prog->filename = tokenizer->getTextData()->filename;

		MemStreamReader mem_reader(NULL, mem_writer.buffer.buf, mem_writer.buffer.count);
		MemStreamReader debuginfo_mem_reader(NULL, debuginfo_mem_writer.buffer.buf, debuginfo_mem_writer.buffer.count);
		prog->loadFromStream(&mem_reader, create_debug_info ? &debuginfo_mem_reader : NULL);

		prog->pushFunction();
		prog->release();
		
		allocator->deleteObj(exp);

		return true;
	}else{
		Core::StringBuffer dump(allocator);
		dump.append("Error");
		switch(error){
		default:
			dump.append(" unknown");
			break;

		case ERROR_SYNTAX:
			dump.append(" SYNTAX");
			break;

		case ERROR_NESTED_ROOT_BLOCK:
			dump.append(" NESTED_ROOT_BLOCK");
			break;

		case ERROR_LOCAL_VAL_NOT_DECLARED:
			dump.append(" LOCAL_VAL_NOT_DECLARED");
			break;

		case ERROR_VAR_ALREADY_EXIST:
			dump.append(" VAR_ALREADY_EXIST");
			break;

		case ERROR_EXPECT_TOKEN_TYPE:
			dump.append(" EXPECT_TOKEN_TYPE ");
			dump.append(Tokenizer::getTokenTypeName(expect_token_type));
			break;

		case ERROR_EXPECT_TOKEN_STR:
			dump.append(" EXPECT_TOKEN_STR ");
			dump += expect_token;
			break;

		case ERROR_EXPECT_TOKEN:
			dump.append(" EXPECT_TOKEN");
			break;

		case ERROR_EXPECT_VALUE:
			dump.append(" EXPECT_VALUE");
			break;

		case ERROR_EXPECT_WRITEABLE:
			dump.append(" EXPECT_WRITEABLE");
			break;

		case ERROR_EXPECT_EXPRESSION:
			dump.append(" EXPECT_EXPRESSION");
			break;

		case ERROR_EXPECT_FUNCTION_SCOPE:
			dump.append(" EXPECT_FUNCTION_SCOPE");
			break;

		case ERROR_EXPECT_SWITCH_SCOPE:
			dump.append(" EXPECT_SWITCH_SCOPE");
			break;

		case ERROR_FINISH_BINARY_OP:
			dump.append(" FINISH_BINARY_OP");
			break;

		case ERROR_FINISH_UNARY_OP:
			dump.append(" FINISH_UNARY_OP");
			break;
		}
		dump.append("\n");
		if(error_token){
			if(error_token->text_data->filename.getDataSize() > 0){
				dump += OS::Core::String::format(allocator, "filename %s\n", error_token->text_data->filename.toChar());
			}
			dump += OS::Core::String::format(allocator, "[%d] %s\n", error_token->line+1, error_token->text_data->lines[error_token->line].toChar());
			dump += OS::Core::String::format(allocator, "pos %d, token: %s\n", error_token->pos+1, error_token->str.toChar());
		}
		printf("%s", dump.toString().toChar());
		// FileStreamWriter(allocator, "test-data/debug-exp-dump.txt").writeBytes(dump.toChar(), dump.getDataSize());
		
		allocator->pushNull();
	}
	return false;
}

void * OS::Core::Compiler::malloc(int size)
{
	return allocator->malloc(size);
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

		// case Tokenizer::OPERATOR_INC: return EXP_TYPE_INC;
		// case Tokenizer::OPERATOR_DEC: return EXP_TYPE_DEC;

	// case Tokenizer::OPERATOR_QUESTION: return EXP_TYPE_QUESTION;
		// case Tokenizer::OPERATOR_COLON: return ;

	case Tokenizer::OPERATOR_BIT_AND: return EXP_TYPE_BIT_AND;
	case Tokenizer::OPERATOR_BIT_OR: return EXP_TYPE_BIT_OR;
	case Tokenizer::OPERATOR_BIT_XOR: return EXP_TYPE_BIT_XOR;
	case Tokenizer::OPERATOR_BIT_NOT: return EXP_TYPE_BIT_NOT;
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

	// case EXP_TYPE_QUESTION:    // ? :
	// 	return OP_LEVEL_2;

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
		return OP_LEVEL_13;

	case EXP_TYPE_PRE_INC:     // ++
	case EXP_TYPE_PRE_DEC:     // --
	case EXP_TYPE_POST_INC:    // ++
	case EXP_TYPE_POST_DEC:    // --
		return OP_LEVEL_14;

	case EXP_TYPE_TYPE_OF:
	case EXP_TYPE_VALUE_OF:
	case EXP_TYPE_NUMBER_OF:
	case EXP_TYPE_STRING_OF:
	case EXP_TYPE_ARRAY_OF:
	case EXP_TYPE_OBJECT_OF:
	case EXP_TYPE_USERDATA_OF:
	case EXP_TYPE_FUNCTION_OF:

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
	if(token_type == Tokenizer::CODE_SEPARATOR && recent_token && recent_token->getType() == token_type){
		return ungetToken();
	}

	if(readToken() && recent_token->getType() == token_type){
		return ungetToken();
	}
	ungetToken();

	TokenData * token = new (malloc(sizeof(TokenData))) TokenData(recent_token->text_data, String(allocator), token_type, recent_token->line, recent_token->pos);
	tokenizer->insertToken(next_token_index, token);
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
	if(!readToken() || recent_token->getType() != type){
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

OS::Core::Compiler::Expression * OS::Core::Compiler::expectExpressionValues(Expression * exp, int ret_values)
{
	if(exp->ret_values == ret_values || ret_values < 0){
		return exp;
	}
	switch(exp->type){
	case EXP_TYPE_CALL:
	case EXP_TYPE_CALL_AUTO_PARAM:
	case EXP_TYPE_CALL_DIM:
	// case EXP_TYPE_GET_DIM:
	case EXP_TYPE_CALL_METHOD:
	case EXP_TYPE_GET_PROPERTY:
	case EXP_TYPE_GET_PROPERTY_AUTO_CREATE:
	// case EXP_TYPE_GET_PROPERTY_DIM:
	case EXP_TYPE_INDIRECT:
	// case EXP_TYPE_GET_ENV_VAR_DIM:
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
			// case EXP_TYPE_GET_DIM:
			case EXP_TYPE_CALL_METHOD:
			case EXP_TYPE_GET_PROPERTY:
			case EXP_TYPE_GET_PROPERTY_AUTO_CREATE:
			// case EXP_TYPE_GET_PROPERTY_DIM:
			case EXP_TYPE_INDIRECT:
			// case EXP_TYPE_GET_ENV_VAR_DIM:
			case EXP_TYPE_TAIL_CALL: // ret values are not used for tail call
			case EXP_TYPE_TAIL_CALL_METHOD: // ret values are not used for tail call
				last_exp->ret_values = ret_values;
				exp->ret_values = ret_values;
				return exp;

			case EXP_TYPE_RETURN:
				last_exp = new (malloc(sizeof(Expression))) Expression(EXP_TYPE_CODE_LIST, last_exp->token, last_exp);
				exp->list[exp->list.count-1] = last_exp;
				last_exp->ret_values = ret_values;
				exp->ret_values = ret_values;
				return exp;
			}
		}
		break;

	case EXP_TYPE_RETURN:
		exp = new (malloc(sizeof(Expression))) Expression(EXP_TYPE_CODE_LIST, exp->token, exp);
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
				// case EXP_TYPE_GET_DIM:
				case EXP_TYPE_CALL_METHOD:
				case EXP_TYPE_GET_PROPERTY:
				case EXP_TYPE_GET_PROPERTY_AUTO_CREATE:
				// case EXP_TYPE_GET_PROPERTY_DIM:
				// case EXP_TYPE_GET_ENV_VAR_DIM:
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
		exp = new (malloc(sizeof(Expression))) Expression(EXP_TYPE_POP_VALUE, exp->token, exp);
		exp->ret_values = new_ret_values;
	}
	if(exp->ret_values < ret_values){
		if(exp->type != EXP_TYPE_PARAMS){
			exp = new (malloc(sizeof(Expression))) Expression(EXP_TYPE_PARAMS, exp->token, exp);
			exp->ret_values = exp->list[0]->ret_values;
		}
		while(exp->ret_values < ret_values){
			Expression * null_exp = new (malloc(sizeof(Expression))) Expression(EXP_TYPE_CONST_NULL, exp->token);
			null_exp->ret_values = 1;
			exp->list.add(null_exp);
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
	// case EXP_TYPE_GET_DIM:
	case EXP_TYPE_CALL_METHOD:
	case EXP_TYPE_GET_PROPERTY:
	case EXP_TYPE_GET_PROPERTY_AUTO_CREATE:
	// case EXP_TYPE_GET_PROPERTY_DIM:
	// case EXP_TYPE_GET_ENV_VAR_DIM:
	case EXP_TYPE_INDIRECT:
		{
			exp = new (malloc(sizeof(Expression))) Expression(EXP_TYPE_VALUE, exp->token, exp);
			exp->ret_values = 1;
			break;
		}
	}
	return exp;
}

OS::Core::Compiler::Expression * OS::Core::Compiler::newExpressionFromList(ExpressionList& list, int ret_values)
{
	Expression * exp;
	if(list.count == 1){
		exp = list[0];
		list.removeIndex(0);
	}else if(list.count == 0){
		TokenData * cur_token = ungetToken();
		readToken();
		exp = new (malloc(sizeof(Expression))) Expression(EXP_TYPE_CODE_LIST, cur_token);
	}else{
		int i;
		for(i = 0; i < list.count-1; i++){
			OS_ASSERT(list[i]->type != EXP_TYPE_CODE_LIST);
			list[i] = expectExpressionValues(list[i], 0);
		}
		exp = new (malloc(sizeof(Expression))) Expression(EXP_TYPE_CODE_LIST, list[0]->token);
		exp->list.swap(list);
		exp->ret_values = exp->list[exp->list.count-1]->ret_values;
	}
	return expectExpressionValues(exp, ret_values);
}

OS::Core::Compiler::Expression * OS::Core::Compiler::stepPass2(Scope * scope, Expression * exp)
{
	switch(exp->type){
	case EXP_TYPE_FUNCTION:
		{
			Scope * new_scope = dynamic_cast<Scope*>(exp);
			OS_ASSERT(new_scope && (new_scope->parent == scope || (!new_scope->parent && new_scope->type == EXP_TYPE_FUNCTION)));
			scope = new_scope;
			OS_ASSERT(prog_functions.indexOf(scope) < 0);
			scope->func_index = prog_functions.count;
			allocator->vectorAddItem(prog_functions, scope);
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
			Expression * obj_exp = new (malloc(sizeof(Expression))) Expression(EXP_TYPE_OBJECT, exp->token);
	
			Vector<String> vars;
			Scope * start_scope = scope;
			for(; scope; scope = scope->parent){
				for(int i = scope->locals.count-1; i >= 0; i--){
					const Scope::LocalVar& local_var = scope->locals[i];
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
					allocator->vectorAddItem(vars, local_var.name);

					TokenData * name_token = new (malloc(sizeof(TokenData))) TokenData(tokenizer->getTextData(), local_var.name, 
						Tokenizer::NAME, exp->token->line, exp->token->pos);

					Expression * var_exp = new (malloc(sizeof(Expression))) Expression(EXP_TYPE_GET_LOCAL_VAR, name_token);
					OS_ASSERT(scope->function);
					var_exp->active_locals = scope->function->num_locals;
					var_exp->ret_values = 1;
					found = findLocalVar(var_exp->local_var, start_scope, local_var.name, start_scope->function->num_locals, true);
					OS_ASSERT(found); // && var_exp->local_var.index == local_var.index);
					if(start_scope->function->max_up_count < var_exp->local_var.up_count){
						start_scope->function->max_up_count = var_exp->local_var.up_count;
					}

					Expression * obj_item_exp = new (malloc(sizeof(Expression))) Expression(EXP_TYPE_OBJECT_SET_BY_NAME, name_token, var_exp);
					allocator->vectorInsertAtIndex(obj_exp->list, 0, obj_item_exp);

					name_token->release();
				}
			}
			allocator->vectorClear(vars);
			obj_exp->ret_values = 1;

			scope = start_scope;
			exp->list.add(obj_exp);
		}
		break;

	case EXP_TYPE_PARAMS:
		{
			for(int i = exp->list.count-1; i >= 0; i--){
				Expression * sub_exp = stepPass2(scope, exp->list[i]);
				if(sub_exp->type == EXP_TYPE_PARAMS){
					// OS_ASSERT(false);
					ExpressionList list(allocator);
					int j;
					for(j = 0; j < i; j++){
						list.add(exp->list[j]);
					}
					for(j = 0; j < sub_exp->list.count; j++){
						list.add(sub_exp->list[j]);
					}
					for(j = i+1; j < exp->list.count; j++){
						list.add(exp->list[j]);
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
			exp->list[0] = stepPass2(scope, exp->list[0]);

			Expression * var_exp = exp->list[0];
			OS_ASSERT(var_exp->type == EXP_TYPE_GET_LOCAL_VAR);
			
			String temp_var_name = String(allocator, OS_TEXT("#temp")); // + String(allocator, (OS_INT)scope->function->num_locals+1);
			TokenData * temp_var_token = new (malloc(sizeof(TokenData))) TokenData(tokenizer->getTextData(), temp_var_name, Tokenizer::NAME, exp->token->line, exp->token->pos);
			
			TokenData * num_token = new (malloc(sizeof(TokenData))) TokenData(tokenizer->getTextData(), String(allocator, OS_TEXT("1")), Tokenizer::NUMBER, exp->token->line, exp->token->pos);
			num_token->setFloat(1);
			
			Expression * cur_var_exp = new (malloc(sizeof(Expression))) Expression(EXP_TYPE_GET_LOCAL_VAR, var_exp->token);
			cur_var_exp->ret_values = 1;
			cur_var_exp->local_var = var_exp->local_var;
			
			Expression * result_exp = new (malloc(sizeof(Expression))) Expression(EXP_TYPE_CODE_LIST, exp->token);
			Expression * copy_exp = new (malloc(sizeof(Expression))) Expression(EXP_TYPE_SET_LOCAL_VAR, temp_var_token, cur_var_exp);
			OS_ASSERT(!findLocalVar(copy_exp->local_var, scope, temp_var_name, scope->function->num_locals, false));
			scope->addLocalVar(temp_var_name, copy_exp->local_var);
			result_exp->list.add(copy_exp);

			cur_var_exp = new (malloc(sizeof(Expression))) Expression(EXP_TYPE_GET_LOCAL_VAR, var_exp->token);
			cur_var_exp->ret_values = 1;
			cur_var_exp->local_var = var_exp->local_var;

			Expression * num_exp = new (malloc(sizeof(Expression))) Expression(EXP_TYPE_CONST_NUMBER, num_token);
			num_exp->ret_values = 1;

			Expression * op_exp = new (malloc(sizeof(Expression))) Expression(exp->type == EXP_TYPE_POST_INC ? EXP_TYPE_ADD : EXP_TYPE_SUB, exp->token, cur_var_exp, num_exp);
			op_exp->ret_values = 1;

			Expression * set_exp = new (malloc(sizeof(Expression))) Expression(EXP_TYPE_SET_LOCAL_VAR, var_exp->token, op_exp);
			set_exp->local_var = var_exp->local_var;

			result_exp->list.add(set_exp);

			Expression * get_temp_var_exp = new (malloc(sizeof(Expression))) Expression(EXP_TYPE_GET_LOCAL_VAR, temp_var_token);
			get_temp_var_exp->ret_values = 1;
			get_temp_var_exp->local_var = copy_exp->local_var;

			result_exp->list.add(get_temp_var_exp);
			result_exp->ret_values = 1;
			
			temp_var_token->release();
			num_token->release();

			allocator->deleteObj(exp);
			return stepPass2(scope, result_exp);
		}
		exp->type = exp->type == EXP_TYPE_POST_INC ? EXP_TYPE_PRE_INC : EXP_TYPE_PRE_DEC;
		// no break

	case EXP_TYPE_PRE_INC:
	case EXP_TYPE_PRE_DEC:
		{
			OS_ASSERT(exp->list.count == 1);
			exp->list[0] = stepPass2(scope, exp->list[0]);
			
			Expression * var_exp = exp->list[0];
			OS_ASSERT(var_exp->type == EXP_TYPE_GET_LOCAL_VAR);
			
			TokenData * num_token = new (malloc(sizeof(TokenData))) TokenData(tokenizer->getTextData(), String(allocator, OS_TEXT("1")), Tokenizer::NUMBER, exp->token->line, exp->token->pos);
			num_token->setFloat(1);

			Expression * cur_var_exp = new (malloc(sizeof(Expression))) Expression(EXP_TYPE_GET_LOCAL_VAR, var_exp->token);
			cur_var_exp->ret_values = 1;
			cur_var_exp->local_var = var_exp->local_var;

			Expression * num_exp = new (malloc(sizeof(Expression))) Expression(EXP_TYPE_CONST_NUMBER, num_token);
			num_exp->ret_values = 1;

			Expression * op_exp = new (malloc(sizeof(Expression))) Expression(exp->type == EXP_TYPE_PRE_INC ? EXP_TYPE_ADD : EXP_TYPE_SUB, exp->token, cur_var_exp, num_exp);
			op_exp->ret_values = 1;

			Expression * set_exp = new (malloc(sizeof(Expression))) Expression(EXP_TYPE_SET_LOCAL_VAR, var_exp->token, op_exp);
			set_exp->local_var = var_exp->local_var;

			Expression * result_exp = new (malloc(sizeof(Expression))) Expression(EXP_TYPE_CODE_LIST, exp->token);
			result_exp->list.add(set_exp);

			if(exp->ret_values > 0){
				OS_ASSERT(exp->ret_values == 1);
				
				cur_var_exp = new (malloc(sizeof(Expression))) Expression(EXP_TYPE_GET_LOCAL_VAR, var_exp->token);
				cur_var_exp->ret_values = 1;
				cur_var_exp->local_var = var_exp->local_var;

				result_exp->list.add(cur_var_exp);
				result_exp->ret_values = 1;
			}
			allocator->deleteObj(exp);
			return stepPass2(scope, result_exp);
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
		if(exp->list.count == 1){
			Expression * sub_exp = exp->list[0] = stepPass2(scope, exp->list[0]);
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
		break;

	case EXP_TYPE_CALL:
	case EXP_TYPE_CALL_AUTO_PARAM:
		{
			OS_ASSERT(exp->list.count == 2);
			exp->list[0] = stepPass2(scope, exp->list[0]);
			exp->list[1] = stepPass2(scope, exp->list[1]);
			Expression * left_exp = exp->list[0];
			Expression * right_exp = exp->list[1];
			if(left_exp->type == EXP_TYPE_GET_PROPERTY){
				OS_ASSERT(left_exp->list.count == 2);
				OS_ASSERT(right_exp->type == EXP_TYPE_PARAMS);
				allocator->vectorInsertAtIndex(right_exp->list, 0, left_exp->list[1]);
				right_exp->ret_values += left_exp->list[1]->ret_values;
				left_exp->list[1] = right_exp;
				left_exp->type = EXP_TYPE_CALL_METHOD;
				left_exp->ret_values = exp->ret_values;
				allocator->vectorClear(exp->list);
				allocator->deleteObj(exp);
				return left_exp;
			}
			/* if(left_exp->type == EXP_TYPE_GET_DIM){
				OS_ASSERT(left_exp->list.count == 2);
				OS_ASSERT(right_exp->type == EXP_TYPE_PARAMS);
				if(left_exp->list[1]->list.count == 1){
					Expression * params = left_exp->list[1];
					OS_ASSERT(params->type == EXP_TYPE_PARAMS);
					allocator->vectorInsertAtIndex(right_exp->list, 0, params->list[0]);
					right_exp->ret_values += params->ret_values;
					left_exp->list[1] = right_exp;
					left_exp->type = EXP_TYPE_CALL_METHOD;
					left_exp->ret_values = exp->ret_values;
					allocator->vectorClear(params->list);
					allocator->deleteObj(params);
					allocator->vectorClear(exp->list);
					allocator->deleteObj(exp);
					return left_exp;
				}
			} */
			return exp;
		}

	case EXP_TYPE_SET_DIM:
		{
			OS_ASSERT(exp->list.count == 3);
			exp->list[0] = stepPass2(scope, exp->list[0]);
			exp->list[1] = stepPass2(scope, exp->list[1]);
			exp->list[2] = stepPass2(scope, exp->list[2]);
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
			break;
		}

	case EXP_TYPE_SET_PROPERTY:
		{
			exp->type;
			break;
		}

	case EXP_TYPE_CALL_DIM:
		{
			OS_ASSERT(exp->list.count == 2);
			exp->list[0] = stepPass2(scope, exp->list[0]);
			exp->list[1] = stepPass2(scope, exp->list[1]);
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
				TokenData * token = new (malloc(sizeof(TokenData))) TokenData(tokenizer->getTextData(), method_name, Tokenizer::NAME, name_exp->token->line, name_exp->token->pos);
				Expression * exp_method_name = new (malloc(sizeof(Expression))) Expression(EXP_TYPE_CONST_STRING, token);
				exp_method_name->ret_values = 1;
				token->release();

				allocator->vectorInsertAtIndex(params->list, 0, exp_method_name);
				params->ret_values++;

				exp->type = EXP_TYPE_CALL_METHOD;
			}
			return exp;
		}

	case EXP_TYPE_INDIRECT:
		{
			OS_ASSERT(exp->list.count == 2);
			Expression * left_exp = exp->list[0];
			Expression * right_exp = exp->list[1];
			left_exp = expectExpressionValues(left_exp, 1);
			right_exp = expectExpressionValues(right_exp, 1);
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
		exp->list[i] = stepPass2(scope, exp->list[i]);
	}
	return exp;
}

OS::Core::Compiler::Expression * OS::Core::Compiler::postProcessExpression(Scope * scope, Expression * exp)
{
	exp = stepPass2(scope, exp);
#if 1
	return exp;
#else
	OS_ASSERT(scope->type == EXP_TYPE_FUNCTION);
	prog_stack_size = 0;
	return stepPass3(scope, exp);
#endif
}

#if 0
OS::Core::Compiler::Expression * OS::Core::Compiler::stepPass3(Scope * scope, Expression * exp)
{
	for(int i = 0; i < exp->list.count; i++){
		exp->list[i] = stepPass3(scope, exp->list[i]);
	}
	switch(exp->type){
	default:
		OS_ASSERT(false);
		break;

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

	case EXP_TYPE_DEBUG_LOCALS:
		break;

	case EXP_TYPE_PARAMS:
		break;

	case EXP_TYPE_POST_INC:
	case EXP_TYPE_POST_DEC:
	case EXP_TYPE_PRE_INC:
	case EXP_TYPE_PRE_DEC:
		OS_ASSERT(false);
		break;

	case EXP_TYPE_NAME:
		OS_ASSERT(false);
		break;

	case EXP_TYPE_RETURN:
		break;

	case EXP_TYPE_CALL:
	case EXP_TYPE_CALL_AUTO_PARAM:
		break;

	case EXP_TYPE_SET_DIM:
		break;

	case EXP_TYPE_SET_PROPERTY:
		break;

	case EXP_TYPE_CALL_DIM:
		OS_ASSERT(false);
		break;

	case EXP_TYPE_INDIRECT:
		OS_ASSERT(false);
		break;
	}
	return exp;
}
#endif

OS::Core::Compiler::Scope * OS::Core::Compiler::expectTextExpression()
{
	OS_ASSERT(recent_token);

	Scope * scope = new (malloc(sizeof(Scope))) Scope(NULL, EXP_TYPE_FUNCTION, recent_token);
	// scope->function = scope;
	scope->parser_started = true;
	scope->ret_values = 1;

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
		if(!exp){
			break;
		}
		list.add(exp);
		if(!recent_token){
			break;
		}
		TokenType token_type = recent_token->getType();
		if(token_type == Tokenizer::END_ARRAY_BLOCK 
			|| token_type == Tokenizer::END_BRACKET_BLOCK
			|| token_type == Tokenizer::END_CODE_BLOCK)
		{
			break;
		}
		if(token_type == Tokenizer::CODE_SEPARATOR){
			if(!readToken()){
				break;
			}
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
	int ret_values = list.count == 1 && list[0]->ret_values > 0 && list[0]->type == EXP_TYPE_FUNCTION ? 1 : 0;
	exp = newExpressionFromList(list, ret_values);
	switch(exp->type){
	case EXP_TYPE_CODE_LIST:
		if(exp->list.count == 1 && exp->list[0]->type == EXP_TYPE_FUNCTION){
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
		OS_ASSERT(scope->num_locals == 0);
		allocator->deleteObj(scope);
		scope = dynamic_cast<Scope*>(exp);
		OS_ASSERT(scope);
		scope->parent = NULL;
		return scope;

	default:
		scope->list.add(exp);
	}
	return scope;
}

OS::Core::Compiler::Scope * OS::Core::Compiler::expectCodeExpression(Scope * parent, int ret_values)
{
	OS_ASSERT(recent_token && recent_token->getType() == Tokenizer::BEGIN_CODE_BLOCK);
	if(!expectToken()){
		allocator->deleteObj(parent);
		return NULL;
	}

	Scope * scope;
	// bool is_new_func;
	if(parent->type == EXP_TYPE_FUNCTION && !parent->parser_started){
		scope = parent;
		// is_new_func = true;
		parent->parser_started = true;
	}else{
		scope = new (malloc(sizeof(Scope))) Scope(parent, EXP_TYPE_SCOPE, recent_token);
		// scope->function = parent->function;
		// is_new_func = false;
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
		if(!exp){
			break;
		}
		list.add(exp);
		TokenType token_type = recent_token->getType();
		if(token_type == Tokenizer::END_ARRAY_BLOCK 
			|| token_type == Tokenizer::END_BRACKET_BLOCK
			|| token_type == Tokenizer::END_CODE_BLOCK)
		{
			break;
		}
		if(token_type == Tokenizer::CODE_SEPARATOR){
			if(!readToken()){
				break;
			}
		}
	}
	if(isError()){
		allocator->deleteObj(scope);
		return NULL;
	}
	if(!recent_token || recent_token->getType() != Tokenizer::END_CODE_BLOCK){
		setError(Tokenizer::END_CODE_BLOCK, recent_token);
		allocator->deleteObj(scope);
		return NULL;
	}
	readToken();
	
	if(list.count == 0){
		return scope;
	}
	exp = newExpressionFromList(list, ret_values);
	switch(exp->type){
	case EXP_TYPE_CODE_LIST:
		{
			scope->list.swap(exp->list);
			allocator->deleteObj(exp);
			break;
		}
		// no break

	default:
		scope->list.add(exp);
	}
	return scope;
}

OS::Core::Compiler::Expression * OS::Core::Compiler::expectObjectExpression(Scope * scope)
{
	OS_ASSERT(recent_token && recent_token->getType() == Tokenizer::BEGIN_CODE_BLOCK);
	struct Lib {
		Compiler * compiler;
		Expression * obj_exp;

		void * malloc(int size)
		{
			return compiler->malloc(size);
		}

		Lib(Compiler * p_compiler, int active_locals)
		{
			compiler = p_compiler;
			obj_exp = new (malloc(sizeof(Expression))) Expression(EXP_TYPE_OBJECT, compiler->recent_token);
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

	// TokenData * name_token, * save_token;
	for(readToken();;){
		Expression * exp = NULL;
		if(!recent_token){
			return lib.error(ERROR_SYNTAX, recent_token);
		}
		if(recent_token->getType() == Tokenizer::END_CODE_BLOCK){
			readToken();
			return lib.obj_exp;
		}
		TokenData * name_token = recent_token;
		if(name_token->getType() == Tokenizer::BEGIN_ARRAY_BLOCK){
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
			if(!recent_token || recent_token->getType() != Tokenizer::END_ARRAY_BLOCK){
				allocator->deleteObj(exp);
				return lib.error(Tokenizer::END_ARRAY_BLOCK, recent_token);
			}
			if(!readToken() || (recent_token->getType() != Tokenizer::OPERATOR_COLON && recent_token->getType() != Tokenizer::OPERATOR_ASSIGN)){
				allocator->deleteObj(exp);
				return lib.error(Tokenizer::OPERATOR_COLON, recent_token);
			}
			save_token = readToken();
			Expression * exp2 = expectSingleExpression(scope, p);
			if(!exp2){
				return isError() ? lib.error() : lib.error(ERROR_EXPECT_EXPRESSION, save_token);
			}
			exp2 = expectExpressionValues(exp2, 1);
			exp = new (malloc(sizeof(Expression))) Expression(EXP_TYPE_OBJECT_SET_BY_EXP, name_token, exp, exp2);
		}else if(isNextToken(Tokenizer::OPERATOR_COLON) || isNextToken(Tokenizer::OPERATOR_ASSIGN)){
			ExpressionType exp_type = EXP_TYPE_OBJECT_SET_BY_NAME;
			switch(name_token->getType()){
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
			exp = new (malloc(sizeof(Expression))) Expression(exp_type, name_token, exp);
		}else{
			exp = expectSingleExpression(scope, p);
			if(!exp){
				return isError() ? lib.error() : lib.error(ERROR_EXPECT_EXPRESSION, name_token);
			}
			exp = expectExpressionValues(exp, 1);
			exp = new (malloc(sizeof(Expression))) Expression(EXP_TYPE_OBJECT_SET_BY_AUTO_INDEX, name_token, exp);
		}
		OS_ASSERT(exp);
		lib.obj_exp->list.add(exp);
		if(recent_token && recent_token->getType() == Tokenizer::END_CODE_BLOCK){
			readToken();
			return lib.obj_exp;
		}
#if 11
		if(!recent_token){
			return lib.error(Tokenizer::END_CODE_BLOCK, recent_token);
		}
		switch(recent_token->getType()){
		case Tokenizer::PARAM_SEPARATOR:
		case Tokenizer::CODE_SEPARATOR:
			readToken();
		}
#else
		if(!recent_token || (recent_token->getType() != Tokenizer::PARAM_SEPARATOR
				&& recent_token->getType() != Tokenizer::CODE_SEPARATOR)){
			return lib.error(Tokenizer::PARAM_SEPARATOR, recent_token);
		}
		readToken();
#endif
	}
	return NULL; // shut up compiler
}

OS::Core::Compiler::Expression * OS::Core::Compiler::expectArrayExpression(Scope * scope)
{
	Expression * params = new (malloc(sizeof(Expression))) Expression(EXP_TYPE_ARRAY, recent_token);
	params->ret_values = 1;
	readToken();
	Params p = Params().setAllowBinaryOperator(true);
	for(;;){
		Expression * exp = expectSingleExpression(scope, p);
		if(!exp){
			if(isError()){
				allocator->deleteObj(params);
				return NULL;
			}
			if(!recent_token || recent_token->getType() != Tokenizer::END_ARRAY_BLOCK){
				setError(Tokenizer::END_ARRAY_BLOCK, recent_token);
				allocator->deleteObj(params);
				return NULL;
			}
			readToken();
			return params; // finishParamsExpression(scope, params);
		}
		exp = expectExpressionValues(exp, 1);
		params->list.add(exp);
		if(recent_token && recent_token->getType() == Tokenizer::END_ARRAY_BLOCK){
			readToken();
			return params; // finishParamsExpression(scope, params);
		}
#if 11
		if(!recent_token){
			setError(Tokenizer::END_ARRAY_BLOCK, recent_token);
			allocator->deleteObj(params);
			return NULL;
		}
		switch(recent_token->getType()){
		case Tokenizer::PARAM_SEPARATOR:
		case Tokenizer::CODE_SEPARATOR:
			readToken();
		}
#else
		if(!recent_token || (recent_token->getType() != Tokenizer::PARAM_SEPARATOR
				&& recent_token->getType() != Tokenizer::CODE_SEPARATOR)){
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

	// OS_ASSERT(recent_token->getType() == Tokenizer::PARAM_SEPARATOR);
	Expression * params = new (malloc(sizeof(Expression))) Expression(EXP_TYPE_PARAMS, recent_token);
	bool is_dim = recent_token->getType() == Tokenizer::BEGIN_ARRAY_BLOCK;
	TokenType end_exp_type = is_dim ? Tokenizer::END_ARRAY_BLOCK : Tokenizer::END_BRACKET_BLOCK;
	readToken();
	Params p = Params().setAllowBinaryOperator(true);
	for(;;){
		Expression * exp = expectSingleExpression(scope, p);
		if(!exp){
			if(isError()){
				allocator->deleteObj(params);
				return NULL;
			}
			if(!recent_token || recent_token->getType() != end_exp_type){
				setError(end_exp_type, recent_token);
				allocator->deleteObj(params);
				return NULL;
			}
			readToken();
			return Lib::calcParamsExpression(this, scope, params);
		}
		// exp = expectExpressionValues(exp, 1);
		params->list.add(exp);
		// params->ret_values += exp->ret_values;
		if(recent_token && recent_token->getType() == end_exp_type){
			readToken();
			return Lib::calcParamsExpression(this, scope, params);
		}
		if(!recent_token){ // || recent_token->getType() != Tokenizer::PARAM_SEPARATOR){
			// setError(Tokenizer::PARAM_SEPARATOR, recent_token);
			setError(end_exp_type, recent_token);
			allocator->deleteObj(params);
			return NULL;
		}
		if(recent_token->getType() == Tokenizer::PARAM_SEPARATOR || recent_token->getType() == Tokenizer::CODE_SEPARATOR){
			readToken();
		}
	}
	return NULL; // shut up compiler
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
	exp = new (malloc(sizeof(Expression))) Expression(EXP_TYPE_EXTENDS, save_token, exp, exp2);
	exp->ret_values = 1;
	return exp;
}

OS::Core::Compiler::Expression * OS::Core::Compiler::expectCloneExpression(Scope * scope)
{
	OS_ASSERT(recent_token && recent_token->str == allocator->core->strings->syntax_clone);
	TokenData * save_token = recent_token;
	if(!expectToken()){
		return NULL;
	}
	Expression * exp = expectSingleExpression(scope, Params());
	if(!exp){
		return NULL;
	}
	exp = expectExpressionValues(exp, 1);
	exp = new (malloc(sizeof(Expression))) Expression(EXP_TYPE_CLONE, save_token, exp);
	exp->ret_values = 1;
	return exp;
}

OS::Core::Compiler::Expression * OS::Core::Compiler::expectDeleteExpression(Scope * scope)
{
	OS_ASSERT(recent_token && recent_token->str == allocator->core->strings->syntax_delete);
	TokenData * save_token = recent_token;
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
		TokenData * token = new (malloc(sizeof(TokenData))) TokenData(tokenizer->getTextData(), method_name, Tokenizer::NAME, object->token->line, object->token->pos);
		Expression * exp_method_name = new (malloc(sizeof(Expression))) Expression(EXP_TYPE_CONST_STRING, token);
		exp_method_name->ret_values = 1;
		token->release();

		Expression * indirect = new (malloc(sizeof(Expression))) Expression(EXP_TYPE_INDIRECT, object->token, object, exp_method_name);
		exp->list[0] = indirect;
		exp->type = EXP_TYPE_CALL;
		exp->ret_values = 1;
		return exp;
	}
	setError(ERROR_SYNTAX, exp->token);
	allocator->deleteObj(exp);
	return NULL;
}

OS::Core::Compiler::Expression * OS::Core::Compiler::expectValueOfExpression(Scope * scope, ExpressionType exp_type)
{
	OS_ASSERT(recent_token);
	TokenData * save_token = recent_token;
	if(!expectToken()){
		return NULL;
	}
	Expression * exp = expectSingleExpression(scope, Params());
	if(!exp){
		return NULL;
	}
	exp = expectExpressionValues(exp, 1);
	exp = new (malloc(sizeof(Expression))) Expression(exp_type, save_token, exp);
	exp->ret_values = 1;
	return exp;
}

OS::Core::Compiler::Scope * OS::Core::Compiler::expectFunctionExpression(Scope * parent)
{
	Scope * scope = new (malloc(sizeof(Scope))) Scope(parent, EXP_TYPE_FUNCTION, recent_token);
	scope->function = scope;
	scope->ret_values = 1;
	if(!expectToken(Tokenizer::BEGIN_BRACKET_BLOCK)){
		allocator->deleteObj(scope);
		return NULL;
	}
	for(;;){
		if(!readToken()){
			setError(ERROR_SYNTAX, recent_token);
			allocator->deleteObj(scope);
			return NULL;
		}
		switch(recent_token->getType()){
		case Tokenizer::END_BRACKET_BLOCK:
			break;

		case Tokenizer::NAME:
			// scope->list.add(new (malloc(sizeof(Expression))) Expression(EXP_TYPE_NAME, recent_token));
			scope->addLocalVar(recent_token->str);
			scope->num_params++;
			if(!readToken()){
				setError(ERROR_SYNTAX, recent_token);
				allocator->deleteObj(scope);
				return NULL;
			}
			if(recent_token->getType() == Tokenizer::END_BRACKET_BLOCK){
				break;
			}
			if(recent_token->getType() == Tokenizer::PARAM_SEPARATOR){
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
	OS_ASSERT(recent_token && recent_token->getType() == Tokenizer::END_BRACKET_BLOCK);
	if(!expectToken(Tokenizer::BEGIN_CODE_BLOCK)){
		allocator->deleteObj(scope);
		return NULL;
	}
	
	scope = expectCodeExpression(scope);
	return scope;
}

OS::Core::Compiler::Expression * OS::Core::Compiler::expectVarExpression(Scope * scope)
{
	OS_ASSERT(recent_token && recent_token->str == allocator->core->strings->syntax_var);
	if(!expectToken(Tokenizer::NAME)){
		return NULL;
	}
	Expression * name_exp = new (malloc(sizeof(Expression))) Expression(EXP_TYPE_NAME, recent_token);
	name_exp->ret_values = 1;

	Expression * exp = new (malloc(sizeof(Expression))) Expression(EXP_TYPE_PARAMS, recent_token, name_exp);
	exp->ret_values = 1;
	while(readToken()){
		if(recent_token->getType() != Tokenizer::PARAM_SEPARATOR){
			break;
		}
		if(!expectToken(Tokenizer::NAME)){
			allocator->deleteObj(exp);
			return NULL;
		}
		name_exp = new (malloc(sizeof(Expression))) Expression(EXP_TYPE_NAME, recent_token);
		name_exp->ret_values = 1;

		exp->list.add(name_exp);
		exp->ret_values++;
	}
	if(recent_token->getType() == Tokenizer::OPERATOR_ASSIGN){
		bool is_finished;
		exp = finishBinaryOperator(scope, getOpcodeLevel(exp->type), exp, Params().setAllowParams(true), is_finished);
		OS_ASSERT(is_finished);
	}
	// Expression * exp = expectSingleExpression(scope, Params().setAllowParams(true).setAllowAssign(true)); // false, true, false, true, false);
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
							// setError(ERROR_VAR_ALREADY_EXIST, exp->token);
							// allocator->deleteObj(ret_exp);
							// return NULL;
							OS_ASSERT(true);
						}else{
							scope->addLocalVar(exp->token->str, exp->local_var);
						}
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
					// setError(ERROR_VAR_ALREADY_EXIST, exp->token);
					// allocator->deleteObj(ret_exp);
					// return NULL;
					OS_ASSERT(true);
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
				// setError(ERROR_VAR_ALREADY_EXIST, exp->token);
				// allocator->deleteObj(ret_exp);
				// return NULL;
				OS_ASSERT(true);
			}else{
				scope->addLocalVar(exp->token->str, exp->local_var);
			}
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
	
	Scope * scope = new (malloc(sizeof(Scope))) Scope(parent, EXP_TYPE_SCOPE, recent_token);
	if(!expectToken(Tokenizer::BEGIN_BRACKET_BLOCK) || !expectToken()){
		allocator->deleteObj(scope);
		return NULL;
	}
	Expression * exp = expectSingleExpression(scope); // , true, true, true, true, true);
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
	if(recent_token->getType() == Tokenizer::NAME && (exp->type == EXP_TYPE_PARAMS || exp->type == EXP_TYPE_NEW_LOCAL_VAR || exp->type == EXP_TYPE_NAME)){
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
			vars.add(exp);
		}
		exp = NULL;
		for(int i = 0; i < vars.count; i++){
			OS_ASSERT(vars[i]->type == EXP_TYPE_NAME || vars[i]->type == EXP_TYPE_NEW_LOCAL_VAR);
			Expression * name_exp = vars[i];
			if(name_exp->type == EXP_TYPE_NAME){
				scope->addLocalVar(name_exp->token->str, name_exp->local_var);
				OS_ASSERT(scope->function);
				name_exp->active_locals = scope->function->num_locals;
				name_exp->type = EXP_TYPE_NEW_LOCAL_VAR;
			}
		}
		if(!expectToken()){
			allocator->deleteObj(scope);
			return NULL;
		}
		exp = expectSingleExpression(scope, Params().setAllowBinaryOperator(true).setAllowAutoCall(true)); // true, false, false, false, true);
		if(!recent_token || recent_token->getType() != Tokenizer::END_BRACKET_BLOCK){
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
		Expression * body_exp;
		Scope * loop_scope = new (malloc(sizeof(Scope))) Scope(scope, EXP_TYPE_LOOP_SCOPE, recent_token);
		if(recent_token->getType() == Tokenizer::BEGIN_CODE_BLOCK){
			body_exp = expectCodeExpression(loop_scope);
		}else{
			body_exp = expectSingleExpression(loop_scope); // , true, false, false, true, true);
		}
		if(!body_exp){
			allocator->deleteObj(scope);
			allocator->deleteObj(exp);
			allocator->deleteObj(loop_scope);
			return NULL;
		}
		body_exp = expectExpressionValues(body_exp, 0);
		
		exp = new (malloc(sizeof(Expression))) Expression(EXP_TYPE_CALL_METHOD, exp->token, exp);
		{
			Expression * params = new (malloc(sizeof(Expression))) Expression(EXP_TYPE_PARAMS, exp->token);
				
			String method_name = allocator->core->strings->__iter;
			TokenData * token = new (malloc(sizeof(TokenData))) TokenData(tokenizer->getTextData(), method_name, Tokenizer::NAME, exp->token->line, exp->token->pos);
			Expression * exp_method_name = new (malloc(sizeof(Expression))) Expression(EXP_TYPE_CONST_STRING, token);
			exp_method_name->ret_values = 1;
			token->release();

			params->list.add(exp_method_name);
			params->ret_values = 1;
			exp->list.add(params);
		}
		exp = expectExpressionValues(exp, vars.count + 1);
		int num_locals = vars.count;

		// ExpressionList temp_vars(allocator);
		const int temp_count = 2;
		const OS_CHAR * temp_names[temp_count] = {
			OS_TEXT("#func"), /*OS_TEXT("#state"), OS_TEXT("#state2"), */ OS_TEXT("#valid")
		};
		for(int i = 0; i < temp_count; i++){
			String name(allocator, temp_names[i]);
			TokenData * token = new (malloc(sizeof(TokenData))) TokenData(tokenizer->getTextData(), name, Tokenizer::NAME, exp->token->line, exp->token->pos);
			Expression * name_exp = new (malloc(sizeof(Expression))) Expression(EXP_TYPE_NEW_LOCAL_VAR, token);
			// name_exp->ret_values = 1;
			vars.add(name_exp);
			token->release();

			scope->addLocalVar(name, name_exp->local_var);
			OS_ASSERT(scope->function);
			name_exp->active_locals = scope->function->num_locals;
			name_exp->local_var.type = LOCAL_TEMP;
		}
			
		ExpressionList list(allocator);

		// var func, state, state2 = (in_exp).__iter()
		{
			Expression * params = new (malloc(sizeof(Expression))) Expression(EXP_TYPE_PARAMS, exp->token);
			for(int i = num_locals; i < vars.count-1; i++){
				Expression * var_exp = vars[i];
				Expression * name_exp = new (malloc(sizeof(Expression))) Expression(EXP_TYPE_NAME, var_exp->token);
				OS_ASSERT(scope->function);
				name_exp->active_locals = scope->function->num_locals;
				name_exp->ret_values = 1;
				params->list.add(name_exp);
			}
			params->ret_values = params->list.count;

			String assing_operator(allocator, OS_TEXT("="));
			TokenData * assign_token = new (malloc(sizeof(TokenData))) TokenData(tokenizer->getTextData(), assing_operator, Tokenizer::OPERATOR_ASSIGN, exp->token->line, exp->token->pos);
			exp = newBinaryExpression(scope, EXP_TYPE_ASSIGN, assign_token, params, exp);
			OS_ASSERT(exp && exp->type == EXP_TYPE_SET_LOCAL_VAR && !exp->ret_values);
			assign_token->release();

			list.add(exp); exp = NULL;
		}
		/*
		begin loop
			var valid, k, v = func(state, state2)
			if(!valid) break

			body_exp

		end loop
		*/
		list.add(loop_scope);
		{
			// var valid, k, v
			Expression * params = new (malloc(sizeof(Expression))) Expression(EXP_TYPE_PARAMS, loop_scope->token);
			for(int i = 0; i < num_locals+1; i++){
				Expression * var_exp = !i ? vars.lastElement() : vars[i-1];
				Expression * name_exp = new (malloc(sizeof(Expression))) Expression(EXP_TYPE_NAME, var_exp->token);
				OS_ASSERT(scope->function);
				name_exp->active_locals = scope->function->num_locals;
				name_exp->ret_values = 1;
				params->list.add(name_exp);
			}
			params->ret_values = params->list.count;

			// func(state, state2)
			Expression * call_exp = new (malloc(sizeof(Expression))) Expression(EXP_TYPE_CALL, loop_scope->token);
			{
				Expression * var_exp = vars[num_locals]; // func
				Expression * name_exp = new (malloc(sizeof(Expression))) Expression(EXP_TYPE_NAME, var_exp->token);
				OS_ASSERT(scope->function);
				name_exp->active_locals = scope->function->num_locals;
				name_exp->ret_values = 1;
				call_exp->list.add(name_exp);

				Expression * params = new (malloc(sizeof(Expression))) Expression(EXP_TYPE_PARAMS, loop_scope->token);
				for(int i = num_locals+1; i < vars.count-1; i++){
					Expression * var_exp = vars[i];
					Expression * name_exp = new (malloc(sizeof(Expression))) Expression(EXP_TYPE_NAME, var_exp->token);
					OS_ASSERT(scope->function);
					name_exp->active_locals = scope->function->num_locals;
					name_exp->ret_values = 1;
					params->list.add(name_exp);
				}
				params->ret_values = params->list.count;
				call_exp->list.add(params);
			}
			call_exp->ret_values = params->list.count;

			// var valid, k, v = func(state, state2)
			String assing_operator(allocator, OS_TEXT("="));
			TokenData * assign_token = new (malloc(sizeof(TokenData))) TokenData(tokenizer->getTextData(), assing_operator, 
				Tokenizer::OPERATOR_ASSIGN, loop_scope->token->line, loop_scope->token->pos);
			exp = newBinaryExpression(scope, EXP_TYPE_ASSIGN, assign_token, params, call_exp);
			OS_ASSERT(exp && exp->type == EXP_TYPE_SET_LOCAL_VAR && !exp->ret_values);
			assign_token->release();

			loop_scope->list.add(exp); exp = NULL;
		}

		// if(!valid) break
		{
			Expression * var_exp = vars.lastElement(); // valid var
			Expression * name_exp = new (malloc(sizeof(Expression))) Expression(EXP_TYPE_NAME, var_exp->token);
			OS_ASSERT(scope->function);
			name_exp->active_locals = scope->function->num_locals;
			name_exp->ret_values = 1;

			Expression * not_exp = new (malloc(sizeof(Expression))) Expression(EXP_TYPE_LOGIC_NOT, loop_scope->token, name_exp);
			not_exp->ret_values = 1;

			Expression * break_exp = new (malloc(sizeof(Expression))) Expression(EXP_TYPE_BREAK, loop_scope->token);
			Expression * if_exp = new (malloc(sizeof(Expression))) Expression(EXP_TYPE_IF, loop_scope->token, not_exp, break_exp);
			loop_scope->list.add(if_exp);
		}
		loop_scope->list.add(body_exp);

		// assemble all exps
		scope->list.swap(vars);
		scope->list.add(newExpressionFromList(list, 0));
		return scope;
	}
	Expression * pre_exp = exp;
	if(recent_token->getType() != Tokenizer::CODE_SEPARATOR){
		setError(Tokenizer::CODE_SEPARATOR, recent_token);
		allocator->deleteObj(scope);
		allocator->deleteObj(pre_exp);
		return NULL;
	}
	readToken();
	Expression * bool_exp;
	if(recent_token->getType() == Tokenizer::CODE_SEPARATOR){
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
	if(recent_token->getType() != Tokenizer::CODE_SEPARATOR){
		setError(Tokenizer::CODE_SEPARATOR, recent_token);
		allocator->deleteObj(scope);
		allocator->deleteObj(pre_exp);
		allocator->deleteObj(bool_exp);
		return NULL;
	}
	readToken();
	Expression * post_exp = expectSingleExpression(scope, Params().setAllowAutoCall(true).setAllowBinaryOperator(true));
	if(!post_exp){
		allocator->deleteObj(scope);
		allocator->deleteObj(pre_exp);
		allocator->deleteObj(bool_exp);
		return NULL;
	}
	if(recent_token->getType() != Tokenizer::END_BRACKET_BLOCK){
		setError(Tokenizer::END_BRACKET_BLOCK, recent_token);
		allocator->deleteObj(scope);
		allocator->deleteObj(pre_exp);
		allocator->deleteObj(bool_exp);
		allocator->deleteObj(post_exp);
		return NULL;
	}
	readToken();

	Scope * loop_scope = new (malloc(sizeof(Scope))) Scope(scope, EXP_TYPE_LOOP_SCOPE, recent_token);
	Expression * body_exp = expectSingleExpression(loop_scope);
	if(!body_exp){
		allocator->deleteObj(scope);
		allocator->deleteObj(pre_exp);
		allocator->deleteObj(bool_exp);
		allocator->deleteObj(post_exp);
		return NULL;
	}
	if(bool_exp){
		bool_exp = expectExpressionValues(bool_exp, 1);
		Expression * not_exp = new (malloc(sizeof(Expression))) Expression(EXP_TYPE_LOGIC_NOT, bool_exp->token, bool_exp);
		not_exp->ret_values = 1;

		Expression * break_exp = new (malloc(sizeof(Expression))) Expression(EXP_TYPE_BREAK, bool_exp->token);
		Expression * if_exp = new (malloc(sizeof(Expression))) Expression(EXP_TYPE_IF, bool_exp->token, not_exp, break_exp);

		loop_scope->list.add(if_exp);
	}
	body_exp = expectExpressionValues(body_exp, 0);
	loop_scope->list.add(body_exp);

	post_exp = expectExpressionValues(post_exp, 0);
	loop_scope->list.add(post_exp);
	
	scope->list.add(pre_exp);
	scope->list.add(loop_scope);
	return scope;
}

OS::Core::Compiler::Expression * OS::Core::Compiler::expectDebugLocalsExpression(Scope * scope)
{
	OS_ASSERT(recent_token && recent_token->str == allocator->core->strings->syntax_debuglocals);
	
	Expression * exp = new (malloc(sizeof(Expression))) Expression(EXP_TYPE_DEBUG_LOCALS, recent_token);
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
	Expression * if_exp = expectSingleExpression(scope, Params().setAllowBinaryOperator(true));
	if(!if_exp){
		return NULL;
	}
	if(if_exp->ret_values < 1){
		setError(ERROR_EXPECT_VALUE, token);
		allocator->deleteObj(if_exp);
		return NULL;
	}
	if_exp = expectExpressionValues(if_exp, 1);
	if(!recent_token || recent_token->getType() != Tokenizer::END_BRACKET_BLOCK){
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
	Expression * then_exp;
	token = recent_token;
	if(recent_token->getType() == Tokenizer::BEGIN_CODE_BLOCK){
		then_exp = expectCodeExpression(scope);
	}else{
		then_exp = expectSingleExpression(scope);
	}
	if(!then_exp){
		allocator->deleteObj(if_exp);
		return NULL;
	}
	then_exp = expectExpressionValues(then_exp, 0);
	if(recent_token && recent_token->getType() == Tokenizer::NAME){
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
			if(recent_token->getType() == Tokenizer::BEGIN_CODE_BLOCK){
				else_exp = expectCodeExpression(scope);
			}else{
				else_exp = expectSingleExpression(scope);
			}
		}else{
			return new (malloc(sizeof(Expression))) Expression(EXP_TYPE_IF, if_exp->token, if_exp, then_exp);
		}
		if(!else_exp){
			allocator->deleteObj(if_exp);
			allocator->deleteObj(then_exp);
			return NULL;
		}
		else_exp = expectExpressionValues(else_exp, 0);
		return new (malloc(sizeof(Expression))) Expression(EXP_TYPE_IF, if_exp->token, if_exp, then_exp, else_exp);
	}
	return new (malloc(sizeof(Expression))) Expression(EXP_TYPE_IF, if_exp->token, if_exp, then_exp);
}

OS::Core::Compiler::Expression * OS::Core::Compiler::expectReturnExpression(Scope * scope)
{
	OS_ASSERT(recent_token && recent_token->str == allocator->core->strings->syntax_return);
	Expression * ret_exp = new (malloc(sizeof(Expression))) Expression(EXP_TYPE_RETURN, recent_token);
	if(!readToken()){
		setError(ERROR_SYNTAX, recent_token);
		allocator->deleteObj(ret_exp);
		return NULL;
	}
	switch(recent_token->getType()){
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
		ret_exp->list.add(exp);
		ret_exp->ret_values = exp->ret_values;
	}
	return ret_exp;
}

OS::Core::Compiler::Expression * OS::Core::Compiler::newBinaryExpression(Scope * scope, ExpressionType exp_type, TokenData * token, Expression * left_exp, Expression * right_exp)
{
	OS_ASSERT(token->isTypeOf(Tokenizer::BINARY_OPERATOR));
	if(left_exp->isConstValue() && right_exp->isConstValue()){
		struct Lib {
			Compiler * compiler;
			TokenData * token;

			void * malloc(int size)
			{
				return compiler->malloc(size);
			}

			Expression * newExpression(const String& str, Expression * left_exp, Expression * right_exp)
			{
				token = new (malloc(sizeof(TokenData))) TokenData(token->text_data, str, Tokenizer::STRING, token->line, token->pos);
				Expression * exp = new (malloc(sizeof(Expression))) Expression(EXP_TYPE_CONST_STRING, token);
				exp->ret_values = 1;
				token->release();
				compiler->allocator->deleteObj(left_exp);
				compiler->allocator->deleteObj(right_exp);
				return exp;
			}

			Expression * newExpression(OS_FLOAT val, Expression * left_exp, Expression * right_exp)
			{
				token = new (malloc(sizeof(TokenData))) TokenData(token->text_data, String(compiler->allocator, val), Tokenizer::NUMBER, token->line, token->pos);
				token->setFloat(val);
				Expression * exp = new (malloc(sizeof(Expression))) Expression(EXP_TYPE_CONST_NUMBER, token);
				exp->ret_values = 1;
				token->release();
				compiler->allocator->deleteObj(left_exp);
				compiler->allocator->deleteObj(right_exp);
				return exp;
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

		switch(exp_type){
		case EXP_TYPE_CONCAT:    // ..
			return lib.newExpression(String(allocator->core->newStringValue(left_exp->toString(), right_exp->toString())), left_exp, right_exp);

		// case EXP_TYPE_ANY_PARAMS:  // ...

		/*
		case EXP_TYPE_LOGIC_AND: // &&
			return lib.switchExpression(left_exp->toInt() == 0, left_exp, right_exp);

		case EXP_TYPE_LOGIC_OR:  // ||
			return lib.switchExpression(left_exp->toInt() != 0, left_exp, right_exp);
		*/

		/*
		case EXP_TYPE_LOGIC_PTR_EQ:  // ===
		case EXP_TYPE_LOGIC_PTR_NE:  // !==
		case EXP_TYPE_LOGIC_EQ:  // ==
		case EXP_TYPE_LOGIC_NE:  // !=
		case EXP_TYPE_LOGIC_GE:  // >=
		case EXP_TYPE_LOGIC_LE:  // <=
		case EXP_TYPE_LOGIC_GREATER: // >
		case EXP_TYPE_LOGIC_LESS:    // <
		*/
		// case EXP_TYPE_LOGIC_NOT:     // !

		// case EXP_TYPE_INC:     // ++
		// case EXP_TYPE_DEC:     // --

		// case EXP_TYPE_QUESTION:  // ?
		// case EXP_TYPE_COLON:     // :

		case EXP_TYPE_BIT_AND: // &
			return lib.newExpression(left_exp->toInt() & right_exp->toInt(), left_exp, right_exp);

		case EXP_TYPE_BIT_OR:  // |
			return lib.newExpression(left_exp->toInt() | right_exp->toInt(), left_exp, right_exp);

		case EXP_TYPE_BIT_XOR: // ^
			return lib.newExpression(left_exp->toInt() ^ right_exp->toInt(), left_exp, right_exp);

		// case EXP_TYPE_BIT_NOT: // ~
		case EXP_TYPE_ADD: // +
			return lib.newExpression(left_exp->toNumber() + right_exp->toNumber(), left_exp, right_exp);

		case EXP_TYPE_SUB: // -
			return lib.newExpression(left_exp->toNumber() - right_exp->toNumber(), left_exp, right_exp);

		case EXP_TYPE_MUL: // *
			return lib.newExpression(left_exp->toNumber() * right_exp->toNumber(), left_exp, right_exp);

		case EXP_TYPE_DIV: // /
			return lib.newExpression(left_exp->toNumber() / right_exp->toNumber(), left_exp, right_exp);

		case EXP_TYPE_MOD: // %
			return lib.newExpression(OS_MATH_FMOD(left_exp->toNumber(), right_exp->toNumber()), left_exp, right_exp);

		case EXP_TYPE_LSHIFT: // <<
			return lib.newExpression(left_exp->toInt() << right_exp->toInt(), left_exp, right_exp);

		case EXP_TYPE_RSHIFT: // >>
			return lib.newExpression(left_exp->toInt() >> right_exp->toInt(), left_exp, right_exp);

		case EXP_TYPE_POW: // **
			return lib.newExpression(OS_MATH_POW(left_exp->toNumber(), right_exp->toNumber()), left_exp, right_exp);
		}
	}
	switch(exp_type){
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
		left_exp->list.add(right_exp);
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
			right_exp = params->list[0];
			right_exp = newBinaryExpression(scope, exp_type, token, left_exp, right_exp);
			params->list[0] = right_exp;
			return params;
		}
	}
	left_exp = expectExpressionValues(left_exp, 1);
	right_exp = expectExpressionValues(right_exp, 1);
	Expression * exp = new (malloc(sizeof(Expression))) Expression(exp_type, token, left_exp, right_exp);
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
			Expression * name_exp = var_exp->list[0];
			Expression * params = var_exp->list[1];
			OS_ASSERT(params->type == EXP_TYPE_PARAMS);
			allocator->vectorInsertAtIndex(var_exp->list, 0, value_exp);
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
			Expression * exp = new (malloc(sizeof(Expression))) Expression(exp_type, var_exp->token, value_exp, var_exp_left, var_exp_right);
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
		var_exp->list.add(value_exp);
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
	return new (malloc(sizeof(Expression))) Expression(EXP_TYPE_ASSIGN, var_exp->token, var_exp, value_exp);
}

OS::Core::Compiler::Expression * OS::Core::Compiler::finishBinaryOperator(Scope * scope, OpcodeLevel prev_level, Expression * exp, 
	const Params& _p, bool& is_finished)
{
	Params p = Params(_p)
		.setAllowAssign(false)
		.setAllowBinaryOperator(false)
		.setAllowParams(false)
		.setAllowAutoCall(false)
		.setAllowRootBlocks(false);

	TokenData * binary_operator = recent_token;
	OS_ASSERT(binary_operator->isTypeOf(Tokenizer::BINARY_OPERATOR));
	readToken();
	Expression * exp2 = expectSingleExpression(scope, p); // false, allow_param, false, false, false);
	if(!exp2){
		/* if(!isError()){
			return exp;
		} */
		is_finished = true;
		allocator->deleteObj(exp);
		return NULL;
	}
	// exp2 = expectExpressionValues(exp2, 1);
	if(!recent_token || !recent_token->isTypeOf(Tokenizer::BINARY_OPERATOR)){
		// return new (malloc(sizeof(Expression))) Expression(getExpressionType(binary_operator->getType()), binary_operator, exp, exp2);
		is_finished = true;
		return newBinaryExpression(scope, getExpressionType(binary_operator->getType()), binary_operator, exp, exp2);
	}
	ExpressionType left_exp_type = getExpressionType(binary_operator->getType());
	ExpressionType right_exp_type = getExpressionType(recent_token->getType());
	OpcodeLevel left_level = getOpcodeLevel(left_exp_type);
	OpcodeLevel right_level = getOpcodeLevel(right_exp_type);
	if(left_level == right_level){
		// exp = new (malloc(sizeof(Expression))) Expression(left_exp_type, binary_operator, exp, exp2);
		exp = newBinaryExpression(scope, left_exp_type, binary_operator, exp, exp2);
		/* if(!recent_token || !recent_token->isTypeOf(Tokenizer::BINARY_OPERATOR)){
			return exp;
		} */
		return finishBinaryOperator(scope, prev_level, exp, p, is_finished);
	}
	if(left_level > right_level){
		// exp = new (malloc(sizeof(Expression))) Expression(left_exp_type, binary_operator, exp, exp2);
		exp = newBinaryExpression(scope, left_exp_type, binary_operator, exp, exp2);
		if(prev_level >= right_level){
			is_finished = false;
			return exp;
		}
		/* if(!recent_token || !recent_token->isTypeOf(Tokenizer::BINARY_OPERATOR)){
			return exp;
		} */
		return finishBinaryOperator(scope, prev_level, exp, p, is_finished);
	}
	exp2 = finishBinaryOperator(scope, left_level, exp2, p, is_finished);
	if(!exp2){
		allocator->deleteObj(exp);
		return NULL;
	}
	exp = newBinaryExpression(scope, left_exp_type, binary_operator, exp, exp2);
	if(is_finished){ // !recent_token || !recent_token->isTypeOf(Tokenizer::BINARY_OPERATOR)){
		return exp;
	}
	return finishBinaryOperator(scope, prev_level, exp, p, is_finished);
}

OS::Core::Compiler::Expression * OS::Core::Compiler::finishValueExpression(Scope * scope, Expression * exp, const Params& _p)
{
	bool is_finished;
	Params p = Params(_p)
		.setAllowRootBlocks(false);
	for(;; p.allow_auto_call = false){
		if(!recent_token){
			return exp;
		}
		Expression * exp2;
		TokenData * token = recent_token;
		TokenType token_type = token->getType();
		switch(token_type){
		case Tokenizer::OPERATOR_INDIRECT:    // .
			// setError(ERROR_SYNTAX, token);
			// return NULL;
			token = expectToken(Tokenizer::NAME);
			if(!token){
				allocator->deleteObj(exp);
				return NULL;
			}
			exp2 = new (malloc(sizeof(Expression))) Expression(EXP_TYPE_NAME, token);
			exp2->ret_values = 1;
			OS_ASSERT(scope->function);
			exp2->active_locals = scope->function->num_locals;
			exp = new (malloc(sizeof(Expression))) Expression(EXP_TYPE_INDIRECT, exp2->token, exp, exp2);
			exp->ret_values = 1;
			readToken();
			continue;

		// post ++, post --
		case Tokenizer::OPERATOR_INC:
		case Tokenizer::OPERATOR_DEC:
			if(exp->type != EXP_TYPE_NAME){
				return exp;
				/* setError(ERROR_SYNTAX, recent_token);
				allocator->deleteObj(exp);
				return NULL; */
			}
			OS_ASSERT(exp->ret_values == 1);
			if(!findLocalVar(exp->local_var, scope, exp->token->str, exp->active_locals, true)){
				setError(ERROR_LOCAL_VAL_NOT_DECLARED, exp->token);
				allocator->deleteObj(exp);
				return NULL;
			}
			exp->type = EXP_TYPE_GET_LOCAL_VAR;
			exp = new (malloc(sizeof(Expression))) Expression(token_type == Tokenizer::OPERATOR_INC ? EXP_TYPE_POST_INC : EXP_TYPE_POST_DEC, exp->token, exp);
			exp->ret_values = 1;
			readToken();
			return finishValueExpression(scope, exp, Params(p).setAllowAutoCall(false));

		case Tokenizer::OPERATOR_CONCAT:    // ..
		// case Tokenizer::REST_ARGUMENTS:  // ...

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

		// case Tokenizer::OPERATOR_INC:     // ++
		// case Tokenizer::OPERATOR_DEC:     // --

		// case Tokenizer::OPERATOR_QUESTION:  // ?
		// case Tokenizer::OPERATOR_COLON:     // :

		case Tokenizer::OPERATOR_BIT_AND: // &
		case Tokenizer::OPERATOR_BIT_OR:  // |
		case Tokenizer::OPERATOR_BIT_XOR: // ^
		case Tokenizer::OPERATOR_BIT_NOT: // ~
		case Tokenizer::OPERATOR_ADD: // +
		case Tokenizer::OPERATOR_SUB: // -
		case Tokenizer::OPERATOR_MUL: // *
		case Tokenizer::OPERATOR_DIV: // /
		case Tokenizer::OPERATOR_MOD: // %
		case Tokenizer::OPERATOR_LSHIFT: // <<
		case Tokenizer::OPERATOR_RSHIFT: // >>
		case Tokenizer::OPERATOR_POW: // **
			if(!p.allow_binary_operator){ // && token_type != Tokenizer::OPERATOR_INDIRECT){
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
			if(!p.allow_auto_call){
				return exp;
			}
			exp2 = expectObjectExpression(scope);
			if(!exp2){
				allocator->deleteObj(exp);
				return NULL;
			}
			OS_ASSERT(exp2->ret_values == 1);
			exp2 = new (malloc(sizeof(Expression))) Expression(EXP_TYPE_PARAMS, exp2->token, exp2);
			exp2->ret_values = 1;
			exp = new (malloc(sizeof(Expression))) Expression(EXP_TYPE_CALL_AUTO_PARAM, token, exp, exp2);
			exp->ret_values = 1;
			// allow_auto_call = false;
			continue;

		default:
			if(!p.allow_auto_call){
				return exp;
			}
			if(token->getType() == Tokenizer::NAME){
				if(token->str == allocator->core->strings->syntax_var
					|| token->str == allocator->core->strings->syntax_function
					|| token->str == allocator->core->strings->syntax_return
					|| token->str == allocator->core->strings->syntax_if
					|| token->str == allocator->core->strings->syntax_else
					|| token->str == allocator->core->strings->syntax_elseif
					|| token->str == allocator->core->strings->syntax_for
					|| token->str == allocator->core->strings->syntax_break
					|| token->str == allocator->core->strings->syntax_continue
					|| token->str == allocator->core->strings->syntax_in
					|| token->str == allocator->core->strings->syntax_class
					|| token->str == allocator->core->strings->syntax_enum
					|| token->str == allocator->core->strings->syntax_switch
					|| token->str == allocator->core->strings->syntax_case 
					|| token->str == allocator->core->strings->syntax_default
					|| token->str == allocator->core->strings->syntax_debugger
					|| token->str == allocator->core->strings->syntax_debuglocals
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
			exp2 = new (malloc(sizeof(Expression))) Expression(EXP_TYPE_PARAMS, exp2->token, exp2);
			exp2->ret_values = 1;
			exp = new (malloc(sizeof(Expression))) Expression(EXP_TYPE_CALL_AUTO_PARAM, token, exp, exp2);
			exp->ret_values = 1;
			// allow_auto_call = false;
			continue;

		case Tokenizer::BEGIN_BRACKET_BLOCK: // (
			exp2 = expectParamsExpression(scope);
			if(!exp2){
				allocator->deleteObj(exp);
				return NULL;
			}
			exp = new (malloc(sizeof(Expression))) Expression(EXP_TYPE_CALL, token, exp, exp2);
			exp->ret_values = 1;
			continue;

		case Tokenizer::BEGIN_ARRAY_BLOCK: // [
			exp2 = expectParamsExpression(scope);
			if(!exp2){
				allocator->deleteObj(exp);
				return NULL;
			}
			exp = new (malloc(sizeof(Expression))) Expression(EXP_TYPE_CALL_DIM, token, exp, exp2);
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
}

OS::Core::Compiler::Params::Params(const Params& p)
{
	OS_MEMCPY(this, &p, sizeof(p));
}

OS::Core::Compiler::Params& OS::Core::Compiler::Params::setAllowRootBlocks(bool val)
{
	allow_root_blocks = val;
	return *this;
}

OS::Core::Compiler::Params& OS::Core::Compiler::Params::setAllowBinaryOperator(bool val)
{
	allow_binary_operator = val;
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

OS::Core::Compiler::Expression * OS::Core::Compiler::expectSingleExpression(Scope * scope)
{
	return expectSingleExpression(scope, Params()
		.setAllowAssign(true)
		.setAllowAutoCall(true)
		.setAllowBinaryOperator(true)
		.setAllowParams(true)
		.setAllowRootBlocks(true));
}

OS::Core::Compiler::Expression * OS::Core::Compiler::expectSingleExpression(Scope * scope, const Params& p)
{
	TokenData * token = recent_token; // readToken();
	if(!token){
		setError(ERROR_EXPECT_EXPRESSION, token);
		return NULL;
	}
	Expression * exp;
	TokenType token_type = token->getType();
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
		exp = new (malloc(sizeof(Expression))) Expression(getUnaryExpressionType(token_type), exp->token, exp);
		exp->ret_values = 1;
		return finishValueExpression(scope, exp, Params(p).setAllowAutoCall(false)); // allow_binary_operator, allow_param, allow_assign, false);

	// pre ++, pre --
	case Tokenizer::OPERATOR_INC:
	case Tokenizer::OPERATOR_DEC:
		if(!expectToken(Tokenizer::NAME)){
			return NULL;
		}
		exp = new (malloc(sizeof(Expression))) Expression(EXP_TYPE_GET_LOCAL_VAR, recent_token);
		exp->ret_values = 1;
		exp->active_locals = scope->function->num_locals;
		if(!findLocalVar(exp->local_var, scope, exp->token->str, exp->active_locals, true)){
			setError(ERROR_LOCAL_VAL_NOT_DECLARED, exp->token);
			allocator->deleteObj(exp);
			return NULL;
		}
		exp = new (malloc(sizeof(Expression))) Expression(token_type == Tokenizer::OPERATOR_INC ? EXP_TYPE_PRE_INC : EXP_TYPE_PRE_DEC, exp->token, exp);
		exp->ret_values = 1;
		readToken();
		return finishValueExpression(scope, exp, Params(p).setAllowAutoCall(false));
	// end unary operators

	case Tokenizer::BEGIN_CODE_BLOCK:
		if(p.allow_root_blocks){
			return expectCodeExpression(scope);
		}
		exp = expectObjectExpression(scope);
		if(!exp){
			return NULL;
		}
		OS_ASSERT(exp->ret_values == 1);
		return finishValueExpression(scope, exp, Params(p).setAllowAssign(false).setAllowAutoCall(false));

	case Tokenizer::BEGIN_ARRAY_BLOCK:
		exp = expectArrayExpression(scope);
		if(!exp){
			return NULL;
		}
		OS_ASSERT(exp->ret_values == 1);
		return finishValueExpression(scope, exp, Params(p).setAllowAssign(false).setAllowAutoCall(false));

	case Tokenizer::BEGIN_BRACKET_BLOCK:
		readToken();
		exp = expectSingleExpression(scope, Params().setAllowBinaryOperator(true).setAllowAutoCall(true));
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
		switch(recent_token->getType()){
		case Tokenizer::END_BRACKET_BLOCK:
			readToken();
			return finishValueExpression(scope, exp, p);
		}
		setError(Tokenizer::END_BRACKET_BLOCK, recent_token);
		allocator->deleteObj(exp);
		return NULL;

	case Tokenizer::STRING:
		exp = new (malloc(sizeof(Expression))) Expression(EXP_TYPE_CONST_STRING, token);
		exp->ret_values = 1;
		readToken();
		return finishValueExpression(scope, exp, Params(p).setAllowAutoCall(false));

	case Tokenizer::NUMBER:
		exp = new (malloc(sizeof(Expression))) Expression(EXP_TYPE_CONST_NUMBER, token);
		exp->ret_values = 1;
		readToken();
		return finishValueExpression(scope, exp, Params(p).setAllowAutoCall(false));

	case Tokenizer::REST_ARGUMENTS:
		exp = new (malloc(sizeof(Expression))) Expression(EXP_TYPE_GET_REST_ARGUMENTS, token);
		exp->ret_values = 1;
		readToken();
		return finishValueExpression(scope, exp, Params(p).setAllowAutoCall(false));

	case Tokenizer::CODE_SEPARATOR:
	case Tokenizer::END_ARRAY_BLOCK:
	case Tokenizer::END_BRACKET_BLOCK:
	case Tokenizer::END_CODE_BLOCK:
		return new (malloc(sizeof(Expression))) Expression(EXP_TYPE_NOP, token);

	case Tokenizer::NAME:
		if(token->str == allocator->core->strings->syntax_var){
			if(!p.allow_root_blocks){
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
			return finishValueExpression(scope, exp, p);
		}
		if(token->str == allocator->core->strings->syntax_return){
			if(!p.allow_root_blocks){
				setError(ERROR_NESTED_ROOT_BLOCK, token);
				return NULL;
			}
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
		if(token->str == allocator->core->strings->syntax_this){
			exp = new (malloc(sizeof(Expression))) Expression(EXP_TYPE_GET_THIS, token);
			exp->ret_values = 1;
			readToken();
			return finishValueExpression(scope, exp, p);
		}
		if(token->str == allocator->core->strings->syntax_arguments){
			exp = new (malloc(sizeof(Expression))) Expression(EXP_TYPE_GET_ARGUMENTS, token);
			exp->ret_values = 1;
			readToken();
			return finishValueExpression(scope, exp, Params(p).setAllowAutoCall(false));
		}
		if(token->str == allocator->core->strings->syntax_null){
			exp = new (malloc(sizeof(Expression))) Expression(EXP_TYPE_CONST_NULL, token);
			exp->ret_values = 1;
			readToken();
			return finishValueExpression(scope, exp, Params(p).setAllowAutoCall(false));
		}
		if(token->str == allocator->core->strings->syntax_true){
			token->setFloat(1);
			exp = new (malloc(sizeof(Expression))) Expression(EXP_TYPE_CONST_TRUE, token);
			exp->ret_values = 1;
			readToken();
			return finishValueExpression(scope, exp, Params(p).setAllowAutoCall(false));
		}
		if(token->str == allocator->core->strings->syntax_false){
			exp = new (malloc(sizeof(Expression))) Expression(EXP_TYPE_CONST_FALSE, token);
			exp->ret_values = 1;
			readToken();
			return finishValueExpression(scope, exp, Params(p).setAllowAutoCall(false));
		}
		if(token->str == allocator->core->strings->syntax_extends){
			exp = expectExtendsExpression(scope);
			if(!exp){
				return NULL;
			}
			return finishValueExpression(scope, exp, Params(p).setAllowAutoCall(false));
		}
		if(token->str == allocator->core->strings->syntax_clone){
			exp = expectCloneExpression(scope);
			if(!exp){
				return NULL;
			}
			return finishValueExpression(scope, exp, Params(p).setAllowAutoCall(false));
		}
		if(token->str == allocator->core->strings->syntax_delete){
			if(!p.allow_root_blocks){
				setError(ERROR_NESTED_ROOT_BLOCK, token);
				return NULL;
			}
			return expectDeleteExpression(scope);
		}
		if(token->str == allocator->core->strings->syntax_typeof){
			exp = expectValueOfExpression(scope, EXP_TYPE_TYPE_OF);
			if(!exp){
				return NULL;
			}
			return finishValueExpression(scope, exp, Params(p).setAllowAutoCall(false));
		}
		if(token->str == allocator->core->strings->syntax_valueof){
			exp = expectValueOfExpression(scope, EXP_TYPE_VALUE_OF);
			if(!exp){
				return NULL;
			}
			return finishValueExpression(scope, exp, Params(p).setAllowAutoCall(false));
		}
		if(token->str == allocator->core->strings->syntax_booleanof){
			exp = expectValueOfExpression(scope, EXP_TYPE_LOGIC_BOOL);
			if(!exp){
				return NULL;
			}
			return finishValueExpression(scope, exp, Params(p).setAllowAutoCall(false));
		}
		if(token->str == allocator->core->strings->syntax_numberof){
			exp = expectValueOfExpression(scope, EXP_TYPE_NUMBER_OF);
			if(!exp){
				return NULL;
			}
			return finishValueExpression(scope, exp, Params(p).setAllowAutoCall(false));
		}
		if(token->str == allocator->core->strings->syntax_stringof){
			exp = expectValueOfExpression(scope, EXP_TYPE_STRING_OF);
			if(!exp){
				return NULL;
			}
			return finishValueExpression(scope, exp, Params(p).setAllowAutoCall(false));
		}
		if(token->str == allocator->core->strings->syntax_arrayof){
			exp = expectValueOfExpression(scope, EXP_TYPE_ARRAY_OF);
			if(!exp){
				return NULL;
			}
			return finishValueExpression(scope, exp, Params(p).setAllowAutoCall(false));
		}
		if(token->str == allocator->core->strings->syntax_objectof){
			exp = expectValueOfExpression(scope, EXP_TYPE_OBJECT_OF);
			if(!exp){
				return NULL;
			}
			return finishValueExpression(scope, exp, Params(p).setAllowAutoCall(false));
		}
		if(token->str == allocator->core->strings->syntax_userdataof){
			exp = expectValueOfExpression(scope, EXP_TYPE_USERDATA_OF);
			if(!exp){
				return NULL;
			}
			return finishValueExpression(scope, exp, Params(p).setAllowAutoCall(false));
		}
		if(token->str == allocator->core->strings->syntax_functionof){
			exp = expectValueOfExpression(scope, EXP_TYPE_FUNCTION_OF);
			if(!exp){
				return NULL;
			}
			return finishValueExpression(scope, exp, Params(p).setAllowAutoCall(false));
		}
		if(token->str == allocator->core->strings->syntax_break){
			if(!p.allow_root_blocks){
				setError(ERROR_NESTED_ROOT_BLOCK, token);
				return NULL;
			}
			readToken();
			return new (malloc(sizeof(Expression))) Expression(EXP_TYPE_BREAK, token);
		}
		if(token->str == allocator->core->strings->syntax_continue){
			if(!p.allow_root_blocks){
				setError(ERROR_NESTED_ROOT_BLOCK, token);
				return NULL;
			}
			readToken();
			return new (malloc(sizeof(Expression))) Expression(EXP_TYPE_CONTINUE, token);
		}
		if(token->str == allocator->core->strings->syntax_debugger){
			if(!p.allow_root_blocks){
				setError(ERROR_NESTED_ROOT_BLOCK, token);
				return NULL;
			}
			readToken();
			return new (malloc(sizeof(Expression))) Expression(EXP_TYPE_DEBUGGER, token);
		}
		if(token->str == allocator->core->strings->syntax_debuglocals){
			exp = expectDebugLocalsExpression(scope);
			if(!exp){
				return NULL;
			}
			return finishValueExpression(scope, exp, Params(p).setAllowAutoCall(false));
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
		exp = new (malloc(sizeof(Expression))) Expression(EXP_TYPE_NAME, token);
		exp->ret_values = 1;
		OS_ASSERT(scope->function);
		exp->active_locals = scope->function->num_locals;
		readToken();
		return finishValueExpression(scope, exp, p);
	}
	setError(ERROR_EXPECT_EXPRESSION, token);
	return NULL;
}

void OS::Core::Compiler::debugPrintSourceLine(StringBuffer& out, TokenData * token)
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

	case EXP_TYPE_GET_PROPERTY:
		return OS_TEXT("get property");

	case EXP_TYPE_GET_PROPERTY_AUTO_CREATE:
		return OS_TEXT("get property auto create");

	case EXP_TYPE_SET_DIM:
		return OS_TEXT("set dim");

	case EXP_TYPE_POP_VALUE:
		return OS_TEXT("pop");

	case EXP_TYPE_CALL:
	case EXP_TYPE_CALL_AUTO_PARAM:
		return OS_TEXT("call");

	case EXP_TYPE_CALL_DIM:
		return OS_TEXT("dim");

	// case EXP_TYPE_GET_DIM:
	//	return OS_TEXT("get dim");

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

	case EXP_TYPE_CLONE:
		return OS_TEXT("clone");

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
		return OS_TEXT("push arguments");

	case EXP_TYPE_GET_REST_ARGUMENTS:
		return OS_TEXT("push rest arguments");

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

	case EXP_TYPE_SET_ENV_VAR:
		return OS_TEXT("set env var");

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

	case EXP_TYPE_TYPE_OF:
		return OS_TEXT("typeof");

	case EXP_TYPE_VALUE_OF:
		return OS_TEXT("valueof");

	case EXP_TYPE_NUMBER_OF:
		return OS_TEXT("numberof");

	case EXP_TYPE_STRING_OF:
		return OS_TEXT("stringof");

	case EXP_TYPE_ARRAY_OF:
		return OS_TEXT("arrayof");

	case EXP_TYPE_OBJECT_OF:
		return OS_TEXT("objectof");

	case EXP_TYPE_USERDATA_OF:
		return OS_TEXT("userdataof");

	case EXP_TYPE_FUNCTION_OF:
		return OS_TEXT("functionof");

		/*
		case EXP_TYPE_INC:     // ++
		return OS_TEXT("operator ++");

		case EXP_TYPE_DEC:     // --
		return OS_TEXT("operator --");
		*/

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

OS::Core::FunctionDecl::FunctionDecl() // Program * p_prog)
{
	// prog = p_prog;
	parent_func_index = -1;
	locals = NULL;
	num_locals = 0;
	num_params = 0;
	max_up_count = 0;
	opcodes_pos = 0;
	opcodes_size = 0;
}

OS::Core::FunctionDecl::~FunctionDecl()
{
	OS_ASSERT(!locals); // && !prog);
}

// =====================================================================

OS::Core::Program::Program(OS * allocator): filename(allocator)
{
	this->allocator = allocator;
	ref_count = 1;
	opcodes = NULL;
	const_numbers = NULL;
	const_strings = NULL;
	num_numbers = 0;
	num_strings = 0;
	gc_time = -1;
}

OS::Core::Program::~Program()
{
	OS_ASSERT(ref_count == 0);
	int i;
	// values could be already destroyed by gc or will be destroyed soon
	/* for(i = num_numbers+num_strings-1; i >= 0; i--){
		allocator->core->releaseValue(const_values[i]);
	} */
	allocator->free(const_numbers);
	const_numbers = NULL;

	allocator->free(const_strings);
	const_strings = NULL;

	for(i = 0; i < num_functions; i++){
		FunctionDecl * func = functions + i;
		for(int j = 0; j < func->num_locals; j++){
			func->locals[j].~LocalVar();
		}
		allocator->free(func->locals);
		func->locals = NULL;
		// func->prog = NULL;
		func->~FunctionDecl();
	}
	allocator->free(functions);
	functions = NULL;

	allocator->deleteObj(opcodes);
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

	// writer->writeUVariable(prog_numbers.count);
	writer->writeUVariable(int_count);
	writer->writeUVariable(float_count);
	writer->writeUVariable(double_count);
	writer->writeUVariable(prog_strings.count);
	writer->writeUVariable(prog_functions.count);
	writer->writeUVariable(prog_opcodes->getPos());

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
		writer->writeUVariable(func_scope->num_locals);
		writer->writeUVariable(func_scope->num_params);
		writer->writeUVariable(func_scope->max_up_count);
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

	writer->writeBytes(prog_opcodes->buffer.buf, prog_opcodes->buffer.count);

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
	OS_ASSERT(!opcodes && !const_numbers && !num_numbers 
		&& !const_strings && !num_strings && !debug_info.count);

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

	const_numbers = (OS_FLOAT*)allocator->malloc(sizeof(OS_FLOAT) * num_numbers);
	const_strings = (GCStringValue**)allocator->malloc(sizeof(GCStringValue*) * num_strings);
	
	int num_index = 0;
	for(i = 0; i < int_count; i++){
		num_index += reader->readUVariable();
		OS_ASSERT(num_index >= 0 && num_index < num_numbers);
		OS_FLOAT number = (OS_FLOAT)reader->readUVariable();
		const_numbers[num_index] = number;
	}
	for(num_index = 0, i = 0; i < float_count; i++){
		num_index += reader->readUVariable();
		OS_ASSERT(num_index >= 0 && num_index < num_numbers);
		OS_FLOAT number = (OS_FLOAT)reader->readFloat();
		const_numbers[num_index] = number;
	}
	for(num_index = 0, i = 0; i < double_count; i++){
		num_index += reader->readUVariable();
		OS_ASSERT(num_index >= 0 && num_index < num_numbers);
		OS_FLOAT number = (OS_FLOAT)reader->readDouble();
		const_numbers[num_index] = number;
	}
	StringBuffer buf(allocator);
	for(i = 0; i < num_strings; i++){
		int data_size = reader->readUVariable();
		allocator->vectorReserveCapacity(buf, data_size/sizeof(OS_CHAR));
		reader->readBytes((void*)buf.buf, data_size);
		buf.count = data_size/sizeof(OS_CHAR);
		const_strings[i] = buf.toGCStringValue();
	}

	functions = (FunctionDecl*)allocator->malloc(sizeof(FunctionDecl) * num_functions);
	for(i = 0; i < num_functions; i++){
		FunctionDecl * func = functions + i;
		new (func) FunctionDecl();
		func->parent_func_index = reader->readUVariable() - 1;
		func->num_locals = reader->readUVariable();
		func->num_params = reader->readUVariable();
		func->max_up_count = reader->readUVariable();
		func->opcodes_pos = reader->readUVariable();
		func->opcodes_size = reader->readUVariable();

		func->locals = (FunctionDecl::LocalVar*)allocator->malloc(sizeof(FunctionDecl::LocalVar) * func->num_locals);
		for(int j = 0; j < func->num_locals; j++){
			int cached_name_index = reader->readUVariable();
			OS_ASSERT(cached_name_index >= 0 && cached_name_index < num_strings);
			FunctionDecl::LocalVar * local_var = func->locals + j;
			String var_name = allocator->core->valueToString(const_strings[cached_name_index]);
			new (local_var) FunctionDecl::LocalVar(var_name);
			local_var->start_code_pos = reader->readUVariable() + func->opcodes_pos;
			local_var->end_code_pos = reader->readUVariable() + func->opcodes_pos;
		}
	}

	opcodes = new (allocator->malloc(sizeof(MemStreamReader))) MemStreamReader(allocator, opcodes_size);
	reader->readBytes(opcodes->buffer, opcodes_size);

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
		StringBuffer buf(allocator);
		for(i = 0; i < num_strings; i++){
			int data_size = debuginfo_reader->readUVariable();
			allocator->vectorReserveCapacity(buf, data_size/sizeof(OS_CHAR));
			debuginfo_reader->readBytes((void*)buf.buf, data_size);
			buf.count = data_size/sizeof(OS_CHAR);
			allocator->vectorAddItem(strings, buf.toString());
		}
		for(i = 0; i < num_debug_infos; i++){
			int end_opcode_offs = debuginfo_reader->readUVariable();
			int line = debuginfo_reader->readUVariable();
			int pos = debuginfo_reader->readUVariable();
			int string_index = debuginfo_reader->readUVariable();
			allocator->vectorAddItem(debug_info, DebugInfoItem(end_opcode_offs, line, pos, strings[string_index]));
		}
		allocator->vectorClear(strings);
	}

	return true;
}

OS::Core::Program::DebugInfoItem::DebugInfoItem(int p_opcode_offs, int p_line, int p_pos, const String& p_token): token(p_token)
{
	opcode_offs = p_opcode_offs;
	line = p_line;
	pos = p_pos;
}

void OS::Core::Program::pushFunction()
{
	int opcode = opcodes->readByte();
	if(opcode != OP_PUSH_FUNCTION){
		OS_ASSERT(false);
		allocator->pushNull();
		return;
	}

	int func_index = opcodes->readUVariable();
	OS_ASSERT(func_index == 0); // func_index >= 0 && func_index < num_functions);
	FunctionDecl * func_decl = functions + func_index;
	OS_ASSERT(func_decl->max_up_count == 0);

	GCFunctionValue * func_value = allocator->core->pushFunctionValue(NULL, this, func_decl);

	allocator->core->gcProcessGreyProgram(this);

	OS_ASSERT(func_decl->opcodes_pos == opcodes->pos);
	opcodes->movePos(func_decl->opcodes_size);
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

OS::Core::Program::OpcodeType OS::Core::Program::getOpcodeType(Compiler::ExpressionType exp_type)
{
	switch(exp_type){
	case Compiler::EXP_TYPE_CALL: return OP_CALL;
	case Compiler::EXP_TYPE_CALL_AUTO_PARAM: return OP_CALL;
	// case Compiler::EXP_TYPE_GET_DIM: return OP_GET_DIM;
	case Compiler::EXP_TYPE_CALL_METHOD: return OP_CALL_METHOD;
	case Compiler::EXP_TYPE_TAIL_CALL: return OP_TAIL_CALL;
	case Compiler::EXP_TYPE_TAIL_CALL_METHOD: return OP_TAIL_CALL_METHOD;

	case Compiler::EXP_TYPE_GET_THIS: return OP_PUSH_THIS;
	case Compiler::EXP_TYPE_GET_ARGUMENTS: return OP_PUSH_ARGUMENTS;
	case Compiler::EXP_TYPE_GET_REST_ARGUMENTS: return OP_PUSH_REST_ARGUMENTS;

	case Compiler::EXP_TYPE_TYPE_OF: return OP_TYPE_OF;
	case Compiler::EXP_TYPE_VALUE_OF: return OP_VALUE_OF;
	case Compiler::EXP_TYPE_NUMBER_OF: return OP_NUMBER_OF;
	case Compiler::EXP_TYPE_STRING_OF: return OP_STRING_OF;
	case Compiler::EXP_TYPE_ARRAY_OF: return OP_ARRAY_OF;
	case Compiler::EXP_TYPE_OBJECT_OF: return OP_OBJECT_OF;
	case Compiler::EXP_TYPE_USERDATA_OF: return OP_USERDATA_OF;
	case Compiler::EXP_TYPE_FUNCTION_OF: return OP_FUNCTION_OF;

	case Compiler::EXP_TYPE_LOGIC_BOOL: return OP_LOGIC_BOOL;
	case Compiler::EXP_TYPE_LOGIC_NOT: return OP_LOGIC_NOT;
	case Compiler::EXP_TYPE_BIT_NOT: return OP_BIT_NOT;
	case Compiler::EXP_TYPE_PLUS: return OP_PLUS;
	case Compiler::EXP_TYPE_NEG: return OP_NEG;
	case Compiler::EXP_TYPE_LENGTH: return OP_LENGTH;

	case Compiler::EXP_TYPE_CONCAT: return OP_CONCAT;

	case Compiler::EXP_TYPE_LOGIC_AND: return OP_LOGIC_AND;
	case Compiler::EXP_TYPE_LOGIC_OR: return OP_LOGIC_OR;
	case Compiler::EXP_TYPE_LOGIC_PTR_EQ: return OP_LOGIC_PTR_EQ;
	case Compiler::EXP_TYPE_LOGIC_PTR_NE: return OP_LOGIC_PTR_NE;
	case Compiler::EXP_TYPE_LOGIC_EQ: return OP_LOGIC_EQ;
	case Compiler::EXP_TYPE_LOGIC_NE: return OP_LOGIC_NE;
	case Compiler::EXP_TYPE_LOGIC_GE: return OP_LOGIC_GE;
	case Compiler::EXP_TYPE_LOGIC_LE: return OP_LOGIC_LE;
	case Compiler::EXP_TYPE_LOGIC_GREATER: return OP_LOGIC_GREATER;
	case Compiler::EXP_TYPE_LOGIC_LESS: return OP_LOGIC_LESS;

	case Compiler::EXP_TYPE_BIT_AND: return OP_BIT_AND;
	case Compiler::EXP_TYPE_BIT_OR: return OP_BIT_OR;
	case Compiler::EXP_TYPE_BIT_XOR: return OP_BIT_XOR;
	
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
	return OP_UNKNOWN;
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

void OS::Core::StreamWriter::readFromStream(StreamReader * reader)
{
	int size = reader->getSize() - reader->getPos();
	int buf_size = 1024 * 16;
	void * buf = allocator->malloc(buf_size < size ? buf_size : size);
	OS_ASSERT(buf || !size);
	for(; size > 0; size -= buf_size){
		int chunk_size = buf_size <= size ? buf_size : size;
		reader->readBytes(buf, chunk_size);
		writeBytes(buf, chunk_size);
	}
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

void OS::Core::MemStreamWriter::writeBytes(const void * buf, int len)
{
	// int pos = buffer.count;
	allocator->vectorReserveCapacity(buffer, pos + len);
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
		allocator->vectorAddItem(buffer, (OS_BYTE)value);
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

OS::Core::FileStreamWriter::FileStreamWriter(OS * allocator, const char * filename): StreamWriter(allocator)
{
	errno_t err = fopen_s(&f, filename, "wb");
	(void)err;
	OS_ASSERT(f);
}

OS::Core::FileStreamWriter::FileStreamWriter(OS * allocator, FILE * f): StreamWriter(allocator)
{
	this->f = f;
	OS_ASSERT(f);
}

OS::Core::FileStreamWriter::~FileStreamWriter()
{
	if(f){
		fclose(f);
	}
}

int OS::Core::FileStreamWriter::getPos() const
{
	return f ? ftell(f) : 0;
}

void OS::Core::FileStreamWriter::setPos(int new_pos)
{
	OS_ASSERT(new_pos >= 0 && new_pos <= getSize());
	if(f){
		fseek(f, new_pos, SEEK_SET);
	}
}

int OS::Core::FileStreamWriter::getSize() const
{
	if(f){
		int save_pos = getPos();
		fseek(f, 0, SEEK_END);
		int size = ftell(f);
		fseek(f, save_pos, SEEK_SET);
		return size;
	}
	return 0;
}

void OS::Core::FileStreamWriter::writeBytes(const void * buf, int len)
{
	if(f){
		fwrite(buf, len, 1, f);
	}
}

void OS::Core::FileStreamWriter::writeBytesAtPos(const void * buf, int len, int pos)
{
	int save_pos = getPos();
	fseek(f, pos, SEEK_SET);
	writeBytes(buf, len);
	fseek(f, save_pos, SEEK_SET);
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
	buffer = (OS_BYTE*)allocator->malloc(buf_size);
	size = buf_size;
	pos = 0;
}

OS::Core::MemStreamReader::MemStreamReader(OS * allocator, OS_BYTE * buf, int buf_size): StreamReader(allocator)
{
	buffer = buf;
	size = buf_size;
	pos = 0;
}

OS::Core::MemStreamReader::~MemStreamReader()
{
	if(allocator){
		allocator->free(buffer);
	}
}

int OS::Core::MemStreamReader::getPos() const
{
	return pos;
}

void OS::Core::MemStreamReader::setPos(int new_pos)
{
	OS_ASSERT(new_pos >= 0 && new_pos <= size);
	pos = new_pos;
}

int OS::Core::MemStreamReader::getSize() const
{
	return size;
}

void OS::Core::MemStreamReader::movePos(int len)
{
	OS_ASSERT(pos+len >= 0 && pos+len <= size);
	pos += len;
}

bool OS::Core::MemStreamReader::checkBytes(void * src, int len)
{
	OS_ASSERT(pos >= 0 && pos+len <= size);
	bool r = OS_MEMCMP(buffer+pos, src, len) == 0;
	pos += len;
	return r;
}

void * OS::Core::MemStreamReader::readBytes(void * dst, int len)
{
	OS_ASSERT(pos >= 0 && pos+len <= size);
	OS_MEMCPY(dst, buffer+pos, len);
	pos += len;
	return dst;
}

void * OS::Core::MemStreamReader::readBytesAtPos(void * dst, int len, int pos)
{
	OS_ASSERT(pos >= 0 && pos+len <= size);
	OS_MEMCPY(dst, buffer+pos, len);
	return dst;
}

OS_BYTE OS::Core::MemStreamReader::readByte()
{
	OS_ASSERT(pos >= 0 && pos+(int)sizeof(OS_BYTE) <= size);
	return buffer[pos++];
}

OS_BYTE OS::Core::MemStreamReader::readByteAtPos(int pos)
{
	OS_ASSERT(pos >= 0 && pos+(int)sizeof(OS_BYTE) <= size);
	return buffer[pos];
}

// =====================================================================

OS::Core::FileStreamReader::FileStreamReader(OS * allocator, const char * filename): StreamReader(allocator)
{
	errno_t err = fopen_s(&f, filename, "rb");
	(void)err;
	// OS_ASSERT(f);
}

OS::Core::FileStreamReader::FileStreamReader(OS * allocator, FILE * f): StreamReader(allocator)
{
	this->f = f;
	OS_ASSERT(f);
}

OS::Core::FileStreamReader::~FileStreamReader()
{
	if(f){
		fclose(f);
	}
}

int OS::Core::FileStreamReader::getPos() const
{
	return f ? ftell(f) : 0;
}

void OS::Core::FileStreamReader::setPos(int new_pos)
{
	OS_ASSERT(new_pos >= 0 && new_pos <= getSize());
	if(f){
		fseek(f, new_pos, SEEK_SET);
	}
}

int OS::Core::FileStreamReader::getSize() const
{
	if(f){
		int save_pos = getPos();
		fseek(f, 0, SEEK_END);
		int size = ftell(f);
		fseek(f, save_pos, SEEK_SET);
		return size;
	}
	return 0;
}

void OS::Core::FileStreamReader::movePos(int len)
{
	if(f){
		fseek(f, len, SEEK_CUR);
	}
}

bool OS::Core::FileStreamReader::checkBytes(void * src, int len)
{
	void * buf = alloca(len);
	readBytes(buf, len);
	return OS_MEMCMP(buf, src, len) == 0;
}

void * OS::Core::FileStreamReader::readBytes(void * buf, int len)
{
	if(f){
		fread(buf, len, 1, f);
	}else{
		OS_MEMSET(buf, 0, len);
	}
	return buf;
}

void * OS::Core::FileStreamReader::readBytesAtPos(void * buf, int len, int pos)
{
	int save_pos = getPos();
	fseek(f, pos, SEEK_SET);
	readBytes(buf, len);
	fseek(f, save_pos, SEEK_SET);
	return buf;
}

// =====================================================================
// =====================================================================
// =====================================================================

static bool isDecString(const OS_CHAR * str, int len)
{
	OS_INT val;
	const OS_CHAR * end = str + len;
	return parseSimpleDec(str, val) && str == end;
}

OS::Core::PropertyIndex::PropertyIndex(const PropertyIndex& p_index): index(p_index.index)
{
}

OS::Core::PropertyIndex::PropertyIndex(const ValueData& p_index): index(p_index)
{
	convertStringToNumber();
}

OS::Core::PropertyIndex::PropertyIndex(const ValueData& p_index, const KeepStringIndex&): index(p_index)
{
	OS_ASSERT(index.type != OS_VALUE_TYPE_STRING || PropertyIndex(p_index).index.type == OS_VALUE_TYPE_STRING);
}

OS::Core::PropertyIndex::PropertyIndex(GCStringValue * p_index): index(p_index)
{
	convertStringToNumber();
}

OS::Core::PropertyIndex::PropertyIndex(GCStringValue * p_index, const KeepStringIndex&): index(p_index)
{
	// OS_ASSERT(index.type != OS_VALUE_TYPE_STRING || PropertyIndex(p_index).index.type == OS_VALUE_TYPE_STRING);
}

OS::Core::PropertyIndex::PropertyIndex(const String& p_index): index(p_index.string)
{
	convertStringToNumber();
}

OS::Core::PropertyIndex::PropertyIndex(const String& p_index, const KeepStringIndex&): index(p_index.string)
{
	// OS_ASSERT(index.type != OS_VALUE_TYPE_STRING || PropertyIndex(p_index).index.type == OS_VALUE_TYPE_STRING);
}

void OS::Core::PropertyIndex::convertStringToNumber()
{
	if(index.type == OS_VALUE_TYPE_STRING){
		OS_FLOAT val;
		const OS_CHAR * str = index.v.string->toChar();
		if(*str >= OS_TEXT('0') && *str <= OS_TEXT('9')){
			const OS_CHAR * end = str + index.v.string->getLen();
			if(parseSimpleFloat(str, val)){
				if(*str == OS_TEXT('.')){
					OS_FLOAT m = 0.1;
					for(str++; *str >= OS_TEXT('0') && *str <= OS_TEXT('9'); str++, m *= 0.1){
						val += (OS_FLOAT)(*str - OS_TEXT('0')) * m;
					}
				}
				if(str == end){
					index.v.number = val;
					index.type = OS_VALUE_TYPE_NUMBER;
					// OS_ASSERT((OS_INT)index.v.number == val);
				}
			}
		}
	}
}

bool OS::Core::PropertyIndex::isEqual(const PropertyIndex& b) const
{
	switch(index.type){
	case OS_VALUE_TYPE_NULL:
		return b.index.type == OS_VALUE_TYPE_NULL;

	// case OS_VALUE_TYPE_BOOL:
	//	return b.index.type == OS_VALUE_TYPE_BOOL && index.v.boolean == b.index.v.boolean;

	case OS_VALUE_TYPE_NUMBER:
		return b.index.type == OS_VALUE_TYPE_NUMBER && index.v.number == b.index.v.number;
	}
	return index.type == b.index.type && index.v.value == b.index.v.value;
}

bool OS::Core::PropertyIndex::isEqual(int hash, const void * b, int size) const
{
	if(index.type == OS_VALUE_TYPE_STRING){
		return index.v.string->hash == hash 
			&& index.v.string->data_size == size
			&& OS_MEMCMP(index.v.string->toMemory(), b, size) == 0;
	}
	return false;
}

bool OS::Core::PropertyIndex::isEqual(int hash, const void * buf1, int size1, const void * buf2, int size2) const
{
	if(index.type == OS_VALUE_TYPE_STRING){
		int src_size = index.v.string->data_size;
		if(index.v.string->hash != hash || src_size != size1 + size2){
			return false;
		}
		const OS_BYTE * src = index.v.string->toBytes();
		return Utils::cmp(src, size1, buf1, size1) == 0
			&& Utils::cmp(src + size1, size2, buf2, size2) == 0;
	}
	return false;
}

int OS::Core::PropertyIndex::getHash() const
{
	switch(index.type){
	/*
	case OS_VALUE_TYPE_NULL:
		return 0;

	case OS_VALUE_TYPE_BOOL:
		return index.v.boolean;
	*/

	case OS_VALUE_TYPE_NUMBER:
		// return (int)index.v.number;
		OS_ASSERT(sizeof(int) <= sizeof(index.v.number));
		return ((int*)((OS_BYTE*)&index.v.number + sizeof(index.v.number)))[-1];

	case OS_VALUE_TYPE_STRING:
		return index.v.string->hash;
	}
	// all other values share same area with index.v.value so just use it as hash
	return (int)index.v.value;
}

// =====================================================================

OS::Core::Property::Property(const PropertyIndex& index): PropertyIndex(index)
{
	hash_next = NULL;
	prev = NULL;
	next = NULL;
}

OS::Core::Property::Property(const ValueData& index): PropertyIndex(index)
{
	hash_next = NULL;
	prev = NULL;
	next = NULL;
}

OS::Core::Property::Property(const ValueData& index, const KeepStringIndex& keep): PropertyIndex(index, keep)
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
}

OS::Core::Table::IteratorState::~IteratorState()
{
	OS_ASSERT(!table && !prop && !next);
}

OS::Core::Table::Table()
{
	head_mask = 0;
	heads = NULL;
	next_id = 0;
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
	iter->prop = first;
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

OS::Core::Table * OS::Core::newTable()
{
	return new (malloc(sizeof(Table))) Table();
}

void OS::Core::deleteTable(Table * table)
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
	table->~Table();
	free(table);
}

void OS::Core::addTableProperty(Table * table, Property * prop)
{
	OS_ASSERT(prop->next == NULL);
	OS_ASSERT(!table->get(*prop));

	if((table->count >> 1) >= table->head_mask){
		int new_size = table->heads ? (table->head_mask+1) * 2 : OS_DEF_VAR_HASH_SIZE;
		int alloc_size = sizeof(Property*)*new_size;
		Property ** new_heads = (Property**)malloc(alloc_size);
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

	if(prop->index.type == OS_VALUE_TYPE_NUMBER && table->next_id <= prop->index.v.number){
		table->next_id = (OS_INT)prop->index.v.number + 1;
	}

	table->count++;
}

bool OS::Core::deleteTableProperty(Table * table, const PropertyIndex& index)
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
					iter->prop = cur->next;
				}
			}

			cur->next = NULL;
			cur->prev = NULL;
			cur->hash_next = NULL;
			// cur->value.clear();

			table->count--;

			cur->~Property();
			free(cur);

			return true;
		}
	}  
	return false;
}

void OS::Core::deleteValueProperty(GCValue * table_value, const PropertyIndex& index, bool prototype_enabled, bool del_method_enabled)
{
	Table * table = table_value->table;
	if(table && deleteTableProperty(table, index)){
		if(table_value->type == OS_VALUE_TYPE_ARRAY){
			reorderTableNumericKeys(table);
		}
		return;
	}
	if(prototype_enabled){
		GCValue * cur_value = table_value;
		while(cur_value->prototype){
			cur_value = cur_value->prototype;
			Table * cur_table = cur_value->table;
			if(cur_table && deleteTableProperty(cur_table, index)){
				if(cur_value->type == OS_VALUE_TYPE_ARRAY){
					reorderTableNumericKeys(cur_table);
				}
				return;
			}
		}
	}
	if(index.index.type == OS_VALUE_TYPE_STRING && strings->syntax_prototype == index.index.v.string){
		return;
	}
	if(del_method_enabled){
		ValueData func_value;
		if(getPropertyValue(func_value, table_value, PropertyIndex(strings->__del, PropertyIndex::KeepStringIndex()), prototype_enabled)
			&& func_value.isFunction())
		{
			pushValueData(func_value);
			pushValue(table_value);
			pushValueData(index.index);
			call(1, 0);
		}
	}
}

void OS::Core::deleteValueProperty(ValueData& table_value, const PropertyIndex& index, bool prototype_enabled, bool del_method_enabled)
{
	switch(table_value.type){
	case OS_VALUE_TYPE_NULL:
		return;

	case OS_VALUE_TYPE_BOOL:
		if(prototype_enabled){
			return deleteValueProperty(prototypes[PROTOTYPE_BOOL], index, prototype_enabled, del_method_enabled);
		}
		return;

	case OS_VALUE_TYPE_NUMBER:
		if(prototype_enabled){
			return deleteValueProperty(prototypes[PROTOTYPE_NUMBER], index, prototype_enabled, del_method_enabled);
		}
		return;

	case OS_VALUE_TYPE_STRING:
		if(prototype_enabled){
			return deleteValueProperty(prototypes[PROTOTYPE_STRING], index, prototype_enabled, del_method_enabled);
		}
		return;

	case OS_VALUE_TYPE_ARRAY:
	case OS_VALUE_TYPE_OBJECT:
	case OS_VALUE_TYPE_USERDATA:
	case OS_VALUE_TYPE_USERPTR:
	case OS_VALUE_TYPE_FUNCTION:
	case OS_VALUE_TYPE_CFUNCTION:
		return deleteValueProperty(table_value.v.value, index, prototype_enabled, del_method_enabled);
	}
}

void OS::Core::reorderTableNumericKeys(Table * table)
{
	OS_ASSERT(table);
	int i = 0;
	for(Property * prop = table->first; prop; prop = prop->next, i++){
		if(prop->index.type == OS_VALUE_TYPE_NUMBER){
			prop->index.v.number = (OS_FLOAT)i;
		}
	}
	table->next_id = i;
}

void OS::Core::reorderTableKeys(Table * table)
{
	OS_ASSERT(table);
	int i = 0;
	for(Property * prop = table->first; prop; prop = prop->next, i++){
		prop->index.v.number = (OS_FLOAT)i;
	}
	table->next_id = i;
}

void OS::Core::initTableProperties(Table * dst, Table * src)
{
	OS_ASSERT(dst->count == 0);
	for(Property * prop = src->first; prop; prop = prop->next){
		setTableValue(dst, PropertyIndex(*prop), prop->value);
	}
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
	parent_inctance = NULL;
	prog = NULL;
	func_decl = NULL;
}

OS::Core::GCFunctionValue::~GCFunctionValue()
{
	OS_ASSERT(!parent_inctance);
	OS_ASSERT(!prog && !func_decl);
}

OS::Core::GCFunctionValue * OS::Core::pushFunctionValue(FunctionRunningInstance * func_running, Program * prog, FunctionDecl * func_decl)
{
	GCFunctionValue * func_value = new (allocator->malloc(sizeof(GCFunctionValue))) GCFunctionValue();
	func_value->type = OS_VALUE_TYPE_FUNCTION;
	func_value->prototype = allocator->core->prototypes[PROTOTYPE_FUNCTION];
	func_value->parent_inctance = func_running ? func_running->retain() : NULL;
	func_value->prog = prog->retain();
	func_value->func_decl = func_decl;
	func_value->env = allocator->core->global_vars;
	registerValue(func_value);
	pushValue(func_value);
	return func_value;
}

void OS::Core::clearFunctionValue(GCFunctionValue * func_value)
{
	OS_ASSERT(func_value->prog && func_value->func_decl); // && func_data->env); //  && func_data->self

	// value could be already destroyed by gc or will be destroyed soon
	// releaseValue(func_data->env);
	func_value->env.clear();
	
	// releaseValue(func_data->self);
	// func_data->self = NULL;
	OS_ASSERT(func_value->func_decl);

	if(func_value->parent_inctance){
		releaseFunctionRunningInstance(func_value->parent_inctance);
		func_value->parent_inctance = NULL;
	}
	func_value->func_decl = NULL;
	
	func_value->prog->release();
	func_value->prog = NULL;
	
	// func_value->~GCFunctionValue();
	// free(func_value);
}

// =====================================================================

OS::Core::FunctionRunningInstance::FunctionRunningInstance()
{
	func = NULL;
	self = NULL;

#ifdef FUNC_VAL_ONE_PARENT
	parent_inctances = NULL;
#else
	parent_inctance = NULL;
#endif
	// num_parent_inctances = 0;
	caller_stack_pos = 0;
	locals_stack_pos = 0;
	opcode_stack_pos = 0;
	bottom_stack_pos = 0;

	locals = NULL;
	num_params = 0;
	num_extra_params = 0;
	// initial_stack_size = 0;
	need_ret_values = 0;

	arguments = NULL;
	rest_arguments = NULL;

	opcodes_pos = 0;
	ref_count = 1;

	gc_time = -1;
}

OS::Core::FunctionRunningInstance::~FunctionRunningInstance()
{
	OS_ASSERT(!ref_count);
	OS_ASSERT(!func && !self && !locals && !arguments && !rest_arguments);
#ifdef FUNC_VAL_ONE_PARENT
	OS_ASSERT(!parent_inctances);
#else
	OS_ASSERT(!parent_inctance);
#endif
}

OS::Core::FunctionRunningInstance * OS::Core::FunctionRunningInstance::retain()
{
	ref_count++;
	return this;
}

// =====================================================================

OS::Core::ValueData::ValueData()
{
	v.value = NULL;
	type = OS_VALUE_TYPE_NULL;
}

OS::Core::ValueData::ValueData(bool val)
{
	v.boolean = val;
	type = OS_VALUE_TYPE_BOOL;
}

OS::Core::ValueData::ValueData(OS_FLOAT val)
{
	v.number = val;
	type = OS_VALUE_TYPE_NUMBER;
}

OS::Core::ValueData::ValueData(int val)
{
	v.number = (OS_FLOAT)val;
	type = OS_VALUE_TYPE_NUMBER;
}

OS::Core::ValueData::ValueData(int val, const WeakRef&)
{
	v.value_id = val;
	type = OS_VALUE_TYPE_WEAKREF;
}

OS::Core::ValueData::ValueData(GCValue * val)
{
	if(val){
		v.value = val;
		type = val->type;
	}else{
		v.value = NULL;
		type = OS_VALUE_TYPE_NULL;
	}
}

OS::Core::ValueData& OS::Core::ValueData::operator=(GCValue * val)
{
	if(val){
		v.value = val;
		type = val->type;
	}else{
		v.value = NULL;
		type = OS_VALUE_TYPE_NULL;
	}
	return *this;
}
				
void OS::Core::ValueData::clear()
{
	v.value = NULL;
	type = OS_VALUE_TYPE_NULL;
}

OS::Core::GCValue * OS::Core::ValueData::getGCValue() const
{
	switch(type){
	case OS_VALUE_TYPE_STRING:
	case OS_VALUE_TYPE_ARRAY:
	case OS_VALUE_TYPE_OBJECT:
	case OS_VALUE_TYPE_FUNCTION:
	case OS_VALUE_TYPE_CFUNCTION:
	case OS_VALUE_TYPE_USERDATA:
	case OS_VALUE_TYPE_USERPTR:
		OS_ASSERT(v.value);
		return v.value;
	}
	return NULL;
}

bool OS::Core::ValueData::isFunction() const
{
	switch(type){
	case OS_VALUE_TYPE_FUNCTION:
	case OS_VALUE_TYPE_CFUNCTION:
		return true;
	}
	return false;
}

bool OS::Core::ValueData::isUserData() const
{
	switch(type){
	case OS_VALUE_TYPE_USERDATA:
	case OS_VALUE_TYPE_USERPTR:
		return true;
	}
	return false;
}

// =====================================================================

OS::Core::ValueDataRetained::ValueDataRetained(): super()
{
}

OS::Core::ValueDataRetained::ValueDataRetained(bool val): super(val)
{
	// retain();
}

OS::Core::ValueDataRetained::ValueDataRetained(OS_FLOAT val): super(val)
{
	// retain();
}

OS::Core::ValueDataRetained::ValueDataRetained(int val): super(val)
{
	// retain();
}

OS::Core::ValueDataRetained::ValueDataRetained(int val, const WeakRef& wr): super(val, wr)
{
	// retain();
}

OS::Core::ValueDataRetained::ValueDataRetained(GCValue * val): super(val)
{
	// retain();
	if(val){
		val->external_ref_count++;
	}
}

OS::Core::ValueDataRetained::ValueDataRetained(const ValueData& b): super(b)
{
	retain();
}

OS::Core::ValueDataRetained::~ValueDataRetained()
{
	release();
}

OS::Core::ValueDataRetained& OS::Core::ValueDataRetained::operator=(ValueData& b)
{
	release();
	super::operator=(b);
	retain();
	return *this;
}

				
void OS::Core::ValueDataRetained::clear()
{
	release();
	super::clear();
}

void OS::Core::ValueDataRetained::retain()
{
	switch(type){
	case OS_VALUE_TYPE_STRING:
	case OS_VALUE_TYPE_ARRAY:
	case OS_VALUE_TYPE_OBJECT:
	case OS_VALUE_TYPE_FUNCTION:
	case OS_VALUE_TYPE_CFUNCTION:
	case OS_VALUE_TYPE_USERDATA:
	case OS_VALUE_TYPE_USERPTR:
		OS_ASSERT(v.value);
		v.value->external_ref_count++;
		break;
	}
}

void OS::Core::ValueDataRetained::release()
{
	switch(type){
	case OS_VALUE_TYPE_STRING:
	case OS_VALUE_TYPE_ARRAY:
	case OS_VALUE_TYPE_OBJECT:
	case OS_VALUE_TYPE_FUNCTION:
	case OS_VALUE_TYPE_CFUNCTION:
	case OS_VALUE_TYPE_USERDATA:
	case OS_VALUE_TYPE_USERPTR:
		OS_ASSERT(v.value && v.value->external_ref_count > 0);
		v.value->external_ref_count--;
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
	gc_color = GC_WHITE;
	type = OS_VALUE_TYPE_NULL;
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

OS::Core::GCStringValue * OS::Core::GCStringValue::alloc(OS * allocator, const void * buf, int data_size)
{
	OS_ASSERT(data_size >= 0);
	int alloc_size = data_size + sizeof(GCStringValue) + sizeof(wchar_t) + sizeof(wchar_t)/2;
	GCStringValue * string = new (allocator->malloc(alloc_size)) GCStringValue(data_size);
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

OS::Core::GCStringValue * OS::Core::GCStringValue::alloc(OS * allocator, const void * buf1, int len1, const void * buf2, int len2)
{
	OS_ASSERT(len1 >= 0 && len2 >= 0);
	int alloc_size = len1 + len2 + sizeof(GCStringValue) + sizeof(wchar_t) + sizeof(wchar_t)/2;
	GCStringValue * string = new (allocator->malloc(alloc_size)) GCStringValue(len1 + len2);
	string->type = OS_VALUE_TYPE_STRING;
	string->prototype = allocator->core->prototypes[PROTOTYPE_STRING];
	OS_BYTE * data_buf = string->toBytes();
	OS_MEMCPY(data_buf, buf1, len1); data_buf += len1;
	OS_MEMCPY(data_buf, buf2, len2); data_buf += len2;
	OS_MEMSET(data_buf, 0, sizeof(wchar_t) + sizeof(wchar_t)/2);
	string->calcHash();
	allocator->core->registerValue(string);
#ifdef OS_DEBUG
	string->str = string->toChar();
#endif
	return string;
}

/*
OS::Core::GCStringValue * OS::Core::GCStringValue::alloc(OS * allocator, const void * buf1, int len1, 
	const void * buf2, int len2, const void * buf3, int len3)
{
	OS_ASSERT(len1 >= 0 && len2 >= 0 && len3 >= 0);
	int alloc_size = len1 + len2 + len3 + sizeof(GCStringValue) + sizeof(wchar_t) + sizeof(wchar_t)/2;
	GCStringValue * string = new (allocator->malloc(alloc_size)) GCStringValue(len1 + len2 + len3);
	string->type = OS_VALUE_TYPE_STRING;
	string->prototype = allocator->core->prototypes[PROTOTYPE_STRING];
	OS_BYTE * data_buf = string->toBytes();
	OS_MEMCPY(data_buf, buf1, len1); data_buf += len1;
	OS_MEMCPY(data_buf, buf2, len2); data_buf += len2;
	OS_MEMCPY(data_buf, buf3, len3); data_buf += len3;
	OS_MEMSET(data_buf, 0, sizeof(wchar_t) + sizeof(wchar_t)/2);
	string->calcHash();
	allocator->core->registerValue(string);
	return string;
}
*/

OS::Core::GCStringValue * OS::Core::GCStringValue::alloc(OS * allocator, GCStringValue * a, GCStringValue * b)
{
	return alloc(allocator, a->toMemory(), a->data_size, b->toMemory(), b->data_size);
}

/*
OS::Core::GCStringValue * OS::Core::GCStringValue::alloc(OS * allocator, GCStringValue * a, GCStringValue * b, GCStringValue * c)
{
	return alloc(allocator, a->toMemory(), a->data_size, b->toMemory(), b->data_size, c->toMemory(), c->data_size);
}
*/

bool OS::Core::GCStringValue::isFloat(OS_FLOAT* p_val) const
{
	const OS_CHAR * str = toChar();
	const OS_CHAR * end = str + getLen();
	OS_FLOAT val;
	if(Utils::parseFloat(str, val) && str == end){
		if(p_val) *p_val = val;
		return true;
	}
	if(p_val) *p_val = 0;
	return false;
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

bool OS::Core::valueToBool(const ValueData& val)
{
	switch(val.type){
	case OS_VALUE_TYPE_NULL:
		return false;

	case OS_VALUE_TYPE_BOOL:
		return val.v.boolean;

	case OS_VALUE_TYPE_NUMBER:
		// return val->value.number && !isnan(val->value.number);
		return !isnan(val.v.number);

	// case OS_VALUE_TYPE_STRING:
	//	return val->value.string_data->data_size > 0;

	// case OS_VALUE_TYPE_OBJECT:
	// case OS_VALUE_TYPE_ARRAY:
	// 	return val->table ? val->table->count : 0;
	}
	return true;
}

OS_INT OS::Core::valueToInt(const ValueData& val, bool valueof_enabled)
{
	return (OS_INT)valueToNumber(val, valueof_enabled);
}

OS_INT OS::Core::Compiler::Expression::toInt()
{
	return (OS_INT)toNumber();
}

OS_FLOAT OS::Core::Compiler::Expression::toNumber()
{
	switch(type){
	case EXP_TYPE_CONST_NULL:
		return 0;

	case EXP_TYPE_CONST_STRING:
		return token->str.toFloat();

	case EXP_TYPE_CONST_NUMBER:
		return token->getFloat();

	case EXP_TYPE_CONST_TRUE:
		return 1;

	case EXP_TYPE_CONST_FALSE:
		return 0;
	}
	OS_ASSERT(false);
	return 0;
}

OS_FLOAT OS::Core::valueToNumber(const ValueData& val, bool valueof_enabled)
{
	switch(val.type){
	case OS_VALUE_TYPE_NULL:
		return 0;

	case OS_VALUE_TYPE_BOOL:
		return val.v.boolean;

	case OS_VALUE_TYPE_NUMBER:
		return val.v.number;

	case OS_VALUE_TYPE_STRING:
		{
			OS_FLOAT fval;
			if(val.v.string->isFloat(&fval)){
				return fval;
			}
			return 0;
		}
	}
	if(valueof_enabled){
		pushValueOf(val);
		struct Pop { Core * core; ~Pop(){ core->pop(); } } pop = {this};
		return valueToNumber(stack_values.lastElement(), false);
	}
	return 0;
}

bool OS::Core::isValueNumber(const ValueData& val, OS_FLOAT * out)
{
	switch(val.type){
	/*
	case OS_VALUE_TYPE_NULL:
		if(out){
			*out = 0;
		}
		return true;
	*/

	case OS_VALUE_TYPE_BOOL:
		if(out){
			*out = val.v.boolean;
		}
		return true;

	case OS_VALUE_TYPE_NUMBER:
		if(out){
			*out = val.v.number;
		}
		return true;

	case OS_VALUE_TYPE_STRING:
		return val.v.string->isFloat(out);
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

OS::Core::String OS::Core::valueToString(const ValueData& val, bool valueof_enabled)
{
	switch(val.type){
	case OS_VALUE_TYPE_NULL:
		// return String(allocator);
		return strings->syntax_null;

	case OS_VALUE_TYPE_BOOL:
		// return val->value.boolean ? String(allocator, OS_TEXT("1")) : String(allocator);
		return val.v.boolean ? strings->syntax_true : strings->syntax_false;

	case OS_VALUE_TYPE_NUMBER:
		return String(allocator, val.v.number, OS_DEF_PRECISION);

	case OS_VALUE_TYPE_STRING:
		return String(val.v.string);
	}
	if(valueof_enabled){
		/*
		Value * func = getPropertyValue(val, PropertyIndex(strings->__tostring, PropertyIndex::KeepStringIndex()), prototype_enabled);
		if(func){
			pushValue(func);
			pushValue(val);
			call(0, 1);
			OS_ASSERT(stack_values.count > 0);
			struct Pop { Core * core; ~Pop(){ core->pop(); } } pop = {this};
			return valueToString(stack_values.lastElement(), false);
		}
		*/
		pushValueOf(val);
		struct Pop { Core * core; ~Pop(){ core->pop(); } } pop = {this};
		return valueToString(stack_values.lastElement(), false);
	}
	return String(allocator);
}

bool OS::Core::isValueString(const ValueData& val, String * out)
{
	switch(val.type){
	case OS_VALUE_TYPE_NULL:
		if(out){
			// *out = String(allocator);
			*out = strings->syntax_null;
		}
		return true;

	case OS_VALUE_TYPE_BOOL:
		if(out){
			// *out = String(allocator, val->value.boolean ? OS_TEXT("1") : OS_TEXT(""));
			*out = val.v.boolean ? strings->syntax_true : strings->syntax_false;
		}
		return true;

	case OS_VALUE_TYPE_NUMBER:
		if(out){
			*out = String(allocator, val.v.number, OS_DEF_PRECISION);
		}
		return true;

	case OS_VALUE_TYPE_STRING:
		if(out){
			*out = String(val.v.string);
		}
		return true;

	// case OS_VALUE_TYPE_OBJECT:
	// case OS_VALUE_TYPE_ARRAY:
	// 	return String(this, (OS_INT)(val->table ? val->table->count : 0));
	}
	if(out){
		*out = String(allocator);
	}
	return false;
}

bool OS::Core::isValueString(const ValueData& val, OS::String * out)
{
	switch(val.type){
	case OS_VALUE_TYPE_NULL:
		if(out){
			// *out = String(allocator);
			*out = strings->syntax_null;
		}
		return true;

	case OS_VALUE_TYPE_BOOL:
		if(out){
			// *out = String(allocator, val->value.boolean ? OS_TEXT("1") : OS_TEXT(""));
			*out = val.v.boolean ? strings->syntax_true : strings->syntax_false;
		}
		return true;

	case OS_VALUE_TYPE_NUMBER:
		if(out){
			*out = String(allocator, val.v.number, OS_DEF_PRECISION);
		}
		return true;

	case OS_VALUE_TYPE_STRING:
		if(out){
			*out = String(val.v.string);
		}
		return true;

	// case OS_VALUE_TYPE_OBJECT:
	// case OS_VALUE_TYPE_ARRAY:
	// 	return String(this, (OS_INT)(val->table ? val->table->count : 0));
	}
	if(out){
		*out = String(allocator);
	}
	return false;
}

// =====================================================================
// =====================================================================
// =====================================================================

OS::Core::Values::Values()
{
	head_mask = 0; // OS_DEF_VALUES_HASH_SIZE-1;
	heads = NULL; // new Value*[OS_DEF_VALUES_HASH_SIZE];
	// OS_ASSERT(heads);
	next_id = 1;
	count = 0;
}
OS::Core::Values::~Values()
{
	OS_ASSERT(count == 0);
	OS_ASSERT(!heads);
	// delete [] heads;
}

void OS::Core::registerValue(GCValue * value)
{
	value->value_id = values.next_id++;

	if((values.count>>1) >= values.head_mask){
		int new_size = values.heads ? (values.head_mask+1) * 2 : OS_DEF_VALUES_HASH_SIZE;
		int alloc_size = sizeof(GCValue*) * new_size;
		GCValue ** new_heads = (GCValue**)malloc(alloc_size); // new Value*[new_size];
		OS_ASSERT(new_heads);
		OS_MEMSET(new_heads, 0, alloc_size);

		GCValue ** old_heads = values.heads;
		int old_mask = values.head_mask;

		values.heads = new_heads;
		values.head_mask = new_size-1;

		if(old_heads){
			for(int i = 0; i <= old_mask; i++){
				for(GCValue * value = old_heads[i], * next; value; value = next){
					next = value->hash_next;
					int slot = value->value_id & values.head_mask;
					value->hash_next = values.heads[slot];
					values.heads[slot] = value;
				}
			}
			// delete [] old_heads;
			free(old_heads);
		}
		/* if(gc_values_head_index >= 0){
			// restart gc ASAP
			gc_values_head_index = -1;
			gc_start_next_values = 0;
		} */
	}

	int slot = value->value_id & values.head_mask;
	value->hash_next = values.heads[slot];
	values.heads[slot] = value;
	values.count++;

	num_created_values++;

	gcAddToGreyList(value);
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
			values.count--;
			value->hash_next = NULL;
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
						if(values.heads[i] == value){
							values.heads[i] = next;
						}
						values.count--;
						value->hash_next = NULL;
						deleteValue(value);
					}
				}
		#endif
			}
			if(!values.count || !del_ref_counted_also){
				break;
			}
		}
		if(values.count == 0){
			free(values.heads);
			values.heads = NULL;
			values.head_mask = 0;
			values.next_id = 1;
		}
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
	__constructor(allocator, OS_TEXT("__constructor")),
	// __destructor(allocator, OS_TEXT("__destructor")),
	__get(allocator, OS_TEXT("__get")),
	__set(allocator, OS_TEXT("__set")),
	__del(allocator, OS_TEXT("__del")),
	__getempty(allocator, OS_TEXT("__getempty")),
	__setempty(allocator, OS_TEXT("__setempty")),
	__delempty(allocator, OS_TEXT("__delempty")),
	__getdim(allocator, OS_TEXT("__getdim")),
	__setdim(allocator, OS_TEXT("__setdim")),
	__deldim(allocator, OS_TEXT("__deldim")),
	__cmp(allocator, OS_TEXT("__cmp")),
	__iter(allocator, OS_TEXT("__iter")),
	// __tostring(allocator, OS_TEXT("__tostring")),
	__valueof(allocator, OS_TEXT("__valueof")),
	__booleanof(allocator, OS_TEXT("__booleanof")),
	__numberof(allocator, OS_TEXT("__numberof")),
	__stringof(allocator, OS_TEXT("__stringof")),
	__arrayof(allocator, OS_TEXT("__arrayof")),
	__objectof(allocator, OS_TEXT("__objectof")),
	__userdataof(allocator, OS_TEXT("__userdataof")),
	__functionof(allocator, OS_TEXT("__functionof")),
	__clone(allocator, OS_TEXT("__clone")),
	__concat(allocator, OS_TEXT("__concat")),
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

	typeof_null(allocator, OS_TEXT("null")),
	typeof_boolean(allocator, OS_TEXT("boolean")),
	typeof_number(allocator, OS_TEXT("number")),
	typeof_string(allocator, OS_TEXT("string")),
	typeof_object(allocator, OS_TEXT("object")),
	typeof_array(allocator, OS_TEXT("array")),
	typeof_userdata(allocator, OS_TEXT("userdata")),
	typeof_function(allocator, OS_TEXT("function")),

	syntax_typeof(allocator, OS_TEXT("typeof")),
	syntax_valueof(allocator, OS_TEXT("valueof")),
	syntax_booleanof(allocator, OS_TEXT("booleanof")),
	syntax_numberof(allocator, OS_TEXT("numberof")),
	syntax_stringof(allocator, OS_TEXT("stringof")),
	syntax_arrayof(allocator, OS_TEXT("arrayof")),
	syntax_objectof(allocator, OS_TEXT("objectof")),
	syntax_userdataof(allocator, OS_TEXT("userdataof")),
	syntax_functionof(allocator, OS_TEXT("functionof")),
	syntax_extends(allocator, OS_TEXT("extends")),
	syntax_clone(allocator, OS_TEXT("clone")),
	syntax_delete(allocator, OS_TEXT("delete")),
	syntax_prototype(allocator, OS_TEXT("prototype")),
	syntax_var(allocator, OS_TEXT("var")),
	syntax_this(allocator, OS_TEXT("this")),
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
	syntax_debugger(allocator, OS_TEXT("debugger")),
	syntax_debuglocals(allocator, OS_TEXT("debuglocals")),

	__dummy__(0)
{
}

// =====================================================================
// =====================================================================
// =====================================================================

OS::MemoryManager::~MemoryManager()
{
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
	
	stat_malloc_count = 0;
	stat_free_count = 0;

	registerPageDesc(sizeof(Core::GCObjectValue), OS_MEMORY_MANAGER_PAGE_BLOCKS);
	registerPageDesc(sizeof(Core::GCStringValue), OS_MEMORY_MANAGER_PAGE_BLOCKS);
	registerPageDesc(sizeof(Core::GCUserDataValue), OS_MEMORY_MANAGER_PAGE_BLOCKS);
	registerPageDesc(sizeof(Core::GCFunctionValue), OS_MEMORY_MANAGER_PAGE_BLOCKS);
	registerPageDesc(sizeof(Core::GCCFunctionValue), OS_MEMORY_MANAGER_PAGE_BLOCKS);

	registerPageDesc(sizeof(Core::Property), OS_MEMORY_MANAGER_PAGE_BLOCKS);
	registerPageDesc(sizeof(Core::FunctionRunningInstance) + sizeof(void*)*4, OS_MEMORY_MANAGER_PAGE_BLOCKS);
	// registerPageDesc(sizeof(Core::FunctionRunningInstance) + sizeof(void*)*8, OS_MEMORY_MANAGER_PAGE_BLOCKS);
	// registerPageDesc(sizeof(Core::FunctionRunningInstance) + sizeof(void*)*16, OS_MEMORY_MANAGER_PAGE_BLOCKS);
	// registerPageDesc(sizeof(Core::FunctionRunningInstance) + sizeof(void*)*32, OS_MEMORY_MANAGER_PAGE_BLOCKS);
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
	OS_ASSERT(!allocated_bytes && !cached_bytes);
}

void OS::SmartMemoryManager::sortPageDesc()
{
	struct Lib {
		static int __cdecl comparePageDesc(const void * pa, const void * pb)
		{
			PageDesc * a = (PageDesc*)pa;
			PageDesc * b = (PageDesc*)pb;
			return a->block_size - b->block_size;
		}
	};
	qsort(page_desc, num_page_desc, sizeof(page_desc[0]), Lib::comparePageDesc);
}

void OS::SmartMemoryManager::registerPageDesc(int block_size, int num_blocks)
{
	if(num_page_desc == MAX_PAGE_COUNT){
		return;
	}
	int i;
	for(i = 0; i < num_page_desc; i++){
		if(page_desc[i].block_size == block_size){
			if(page_desc[i].num_blocks < num_blocks){
				page_desc[i].num_blocks = num_blocks;
				page_desc[i].allocated_bytes = sizeof(Page) + (sizeof(MemBlock) + block_size) * num_blocks;
			}
			return;
		}
	}
	page_desc[i].block_size = block_size;
	page_desc[i].num_blocks = num_blocks;
	page_desc[i].allocated_bytes = sizeof(Page) + (sizeof(MemBlock) + block_size) * num_blocks;
	num_page_desc++;
}

void * OS::SmartMemoryManager::allocFromCachedBlock(int i)
{
	OS_ASSERT(i >= 0 && i < num_page_desc);
	CachedBlock * cached_block = cached_blocks[i];
	OS_ASSERT(cached_block);
	cached_blocks[i] = cached_block->next;
	Page * page = cached_block->page;
	OS_ASSERT(page->num_cached_blocks > 0);
	page->num_cached_blocks--;
	MemBlock * mem_block = (MemBlock*)cached_block;
	mem_block->page = page;
	mem_block->block_size = page_desc[i].block_size;
	cached_bytes -= mem_block->block_size + sizeof(MemBlock);
	void * p = mem_block + 1;
	OS_MEMSET(p, 0, mem_block->block_size);
	return p;
}

void * OS::SmartMemoryManager::allocFromPageType(int i)
{
	OS_ASSERT(i >= 0 && i < num_page_desc);
	if(cached_blocks[i]){
		return allocFromCachedBlock(i);
	}

	int allocated_bytes = page_desc[i].allocated_bytes;
	Page * page = (Page*)stdAlloc(allocated_bytes);
	page->index = i;
	page->next_page = pages[i];
	pages[i] = page;
	page->num_cached_blocks = page_desc[i].num_blocks;
	cached_bytes += allocated_bytes;

	OS_BYTE * page_blocks = (OS_BYTE*)(page + 1);
	for(int j = 0; j < page_desc[i].num_blocks; j++){
		CachedBlock * cached_block = (CachedBlock*)page_blocks;
		cached_block->page = page;
		cached_block->next = cached_blocks[i];
		cached_blocks[i] = cached_block;
		page_blocks += sizeof(MemBlock) + page_desc[i].block_size;
	}

	return allocFromCachedBlock(i);
}

void OS::SmartMemoryManager::freeMemBlock(MemBlock * mem_block)
{
	Page * page = mem_block->page;
	int size = mem_block->block_size;
	cached_bytes += size + sizeof(MemBlock);
	CachedBlock * cached_block = (CachedBlock*)mem_block;
	cached_block->page = page;
	cached_block->next = cached_blocks[page->index];
#ifdef OS_DEBUG
	OS_MEMSET(cached_block+1, 0xde, size - (sizeof(CachedBlock) - sizeof(MemBlock)));
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

void * OS::SmartMemoryManager::stdAlloc(int size)
{
	// real_malloc_count++;
	int * p = (int*)::malloc(size + sizeof(int));
	if(!p && cached_bytes > 0){
		freeCachedMemory(0);
		p = (int*)::malloc(size + sizeof(int));
		if(!p){
			return NULL;
		}
	}
	p[0] = size | 0x80000000;
	allocated_bytes += size + sizeof(int);
	if(max_allocated_bytes < allocated_bytes){
		max_allocated_bytes = allocated_bytes;
	}
	OS_MEMSET(p+1, 0, size);
	return p+1;
}

void OS::SmartMemoryManager::stdFree(void * ptr)
{
	int * p = (int*)ptr - 1;
	OS_ASSERT(p[0] & 0x80000000);
	int size = p[0] & ~0x80000000;
	allocated_bytes -= size + sizeof(int);
#ifdef OS_DEBUG
	OS_MEMSET(ptr, 0xde, size);
#endif
	::free(p);
}

void * OS::SmartMemoryManager::malloc(int size)
{
	if(size <= 0){
		return NULL;
	}
	stat_malloc_count++;
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
				return allocFromPageType(i);
			}
		}
	}
#endif
	return stdAlloc(size);
}

void OS::SmartMemoryManager::free(void * ptr)
{
	if(!ptr){
		return;
	}
	int * p = (int*)ptr - 1;
	int size = p[0];
	if(size & 0x80000000){
		stat_free_count++;
		stdFree(ptr); // p, size & ~0x80000000);
		return;
	}
	MemBlock * mem_block = (MemBlock*)ptr - 1;
	OS_ASSERT(mem_block->block_size == size);
	freeMemBlock(mem_block);
	if(!(++stat_free_count % 1024) && cached_bytes > allocated_bytes / 2){
		freeCachedMemory(cached_bytes / 2);
	}
}

int OS::SmartMemoryManager::getPointerSize(void * ptr)
{
	if(ptr){
		return ((int*)ptr)[-1];
	}
	return 0;
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

OS::OS(MemoryManager * manager)
{
	ref_count = 1;
	memory_manager = manager;
	if(!memory_manager){
		memory_manager = new SmartMemoryManager();
	}
	core = new (malloc(sizeof(Core))) Core(this);
}

OS::~OS()
{
	OS_ASSERT(ref_count == 0);
	OS_ASSERT(!core);
	// deleteObj(core);
	delete memory_manager;
}

void * OS::malloc(int size)
{
	return memory_manager->malloc(size);
}

void OS::free(void * p)
{
	memory_manager->free(p);
}

void * OS::Core::malloc(int size)
{
	return allocator->malloc(size);
}

void OS::Core::free(void * p)
{
	allocator->free(p);
}

int OS::getPointerSize(void * p)
{
	return memory_manager->getPointerSize(p);
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

/*
OS::Core::OpcodeTime::OpcodeTime()
{
	opcode = Program::OP_UNKNOWN;
	time = 0;
}
*/

OS::Core::Core(OS * p_allocator)
{
	allocator = p_allocator;
	strings = NULL;
	OS_MEMSET(prototypes, 0, sizeof(prototypes));
	
	global_vars = NULL;
	user_pool = NULL;

	num_created_values = 0;
	num_destroyed_values = 0;

	settings.create_debug_info = false;
	settings.create_debug_opcodes = false;
	settings.recompile_sourcecode = false;

	gcInitGreyList();

	/*
	if(!opcode_usage_initialized){
		opcode_usage_initialized = true;
		for(int i = 0; i < Program::OPCODE_COUNT; i++){
			opcode_usage[i].opcode = (Program::OpcodeType)i;
		}
	}
	*/
}

OS::Core::~Core()
{
	OS_ASSERT(!strings && !global_vars && !user_pool);
	for(int i = 0; i < PROTOTYPE_COUNT; i++){
		OS_ASSERT(!prototypes[i]);
	}
}

OS * OS::create(MemoryManager * manager)
{
	OS * os = new OS(manager);
	if(os->init()){
		return os;
	}
	delete os;
	return NULL;
}

bool OS::init()
{
	if(core->init()){
		initGlobalFunctions();
		initObjectClass();
		initArrayClass();
		initStringClass();
		initFunctionClass();
		initMathLibrary();
		initScript();
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
	string_values_table = newTable();
	for(int i = 0; i < PROTOTYPE_COUNT; i++){
		prototypes[i] = newObjectValue(NULL);
		prototypes[i]->type = OS_VALUE_TYPE_OBJECT;
	}
	global_vars = newObjectValue();
	user_pool = newObjectValue();
	
	prototypes[PROTOTYPE_BOOL]->prototype = prototypes[PROTOTYPE_OBJECT];
	prototypes[PROTOTYPE_NUMBER]->prototype = prototypes[PROTOTYPE_OBJECT];
	prototypes[PROTOTYPE_STRING]->prototype = prototypes[PROTOTYPE_OBJECT];
	prototypes[PROTOTYPE_ARRAY]->prototype = prototypes[PROTOTYPE_OBJECT];
	prototypes[PROTOTYPE_FUNCTION]->prototype = prototypes[PROTOTYPE_OBJECT];

	strings = new (malloc(sizeof(Strings))) Strings(allocator);

	setGlobalValue(OS_TEXT("Boolean"), ValueData(prototypes[PROTOTYPE_BOOL]), false, false);
	setGlobalValue(OS_TEXT("Number"), ValueData(prototypes[PROTOTYPE_NUMBER]), false, false);
	setGlobalValue(OS_TEXT("String"), ValueData(prototypes[PROTOTYPE_STRING]), false, false);
	setGlobalValue(OS_TEXT("Object"), ValueData(prototypes[PROTOTYPE_OBJECT]), false, false);
	setGlobalValue(OS_TEXT("Array"), ValueData(prototypes[PROTOTYPE_ARRAY]), false, false);
	setGlobalValue(OS_TEXT("Function"), ValueData(prototypes[PROTOTYPE_FUNCTION]), false, false);

	return true;
}

void OS::Core::shutdown()
{
	// freeAutoreleaseValues();
	// vectorClear(autorelease_values);
	stack_values.count = 0;
	
	free(stack_values.buf);
	stack_values.buf = NULL;
	stack_values.capacity = 0;
	stack_values.count = 0;

	while(call_stack_funcs.count > 0){
		FunctionRunningInstance * func_running = call_stack_funcs[--call_stack_funcs.count];
		releaseFunctionRunningInstance(func_running);
	}
	allocator->vectorClear(call_stack_funcs);
	// vectorClear(cache_values);

	gcResetGreyList();
	
	global_vars = NULL;
	user_pool = NULL;

	deleteValues(false);
	for(int i = 0; i < PROTOTYPE_COUNT; i++){
		// releaseValue(prototypes[i]);
		prototypes[i] = NULL;
	}
	// vectorClear(stack_values); // !!!

	allocator->deleteObj(strings);
	deleteValues(false);
	deleteTable(string_values_table);
	string_values_table = NULL;
#ifdef OS_DEBUG
	deleteValues(false);
	for(int i = 0; i <= values.head_mask; i++){
		for(GCValue * value = values.heads[i]; value; value = value->hash_next){
			GCValue * leak_value = value;
		}
	}
#endif
	deleteValues(true);
}

OS::Core::String OS::changeFilenameExt(const OS_CHAR * filename, const OS_CHAR * ext)
{
	int len = OS_STRLEN(filename);
	for(int i = len-1; i >= 0; i--){
		if(filename[i] == OS_TEXT('.')){
			return Core::String(this, filename, i, ext, OS_STRLEN(ext));
		}
		if(filename[i] == OS_TEXT('\\') || filename[i] == OS_TEXT('/')){
			break;
		}
	}
	return Core::String(this, filename, len, ext, OS_STRLEN(ext));
}

OS::Core::String OS::getFilenameExt(const OS_CHAR * filename)
{
	int len = OS_STRLEN(filename);
	for(int i = len-1; i >= 0; i--){
		if(filename[i] == OS_TEXT('.')){
			return Core::String(this, filename+i, len-i);
		}
		if(filename[i] == OS_TEXT('\\') || filename[i] == OS_TEXT('/')){
			break;
		}
	}
	return Core::String(this, OS_TEXT(""));
}

void OS::Core::error(int code, const OS_CHAR * message)
{
	error(code, String(allocator, message));
}

void OS::Core::error(int code, const String& message)
{
	FunctionRunningInstance * func_running = call_stack_funcs.lastElement();
	GCFunctionValue * func_value = func_running->func;
	Program * prog = func_value->prog;
	Program::DebugInfoItem * debug_info = NULL;
	if(prog->debug_info.count > 0){
		int opcodes_offs = func_running->opcodes_pos;
		for(int i = 0; i < prog->debug_info.count; i++){
			if(prog->debug_info[i].opcode_offs == opcodes_offs){
				debug_info = &prog->debug_info[i];
			}
			if(prog->debug_info[i].opcode_offs > opcodes_offs){
				break;
			}
		}
	}
	const OS_CHAR * error_type = NULL;
	switch(code){
	case OS_WARNING:
		error_type = OS_TEXT("WARNING");
		break;

	default:
	case OS_ERROR:
		error_type = OS_TEXT("ERROR");
		code = OS_ERROR;
		break;
	}
	if(debug_info){
		printf("[%s] %s (line: %d, pos: %d, token: %s, filename: %s)\n", error_type, message.toChar(), debug_info->line, debug_info->pos, 
			debug_info->token.toChar(), prog->filename.toChar());
	}else{
		printf("[%s] %s\n", error_type, message.toChar());
	}
}

void OS::Core::gcInitGreyList()
{
	gc_grey_list_first = NULL;
	gc_grey_root_initialized = false;
	gc_values_head_index = -1;
	gc_time = 0;
	gc_grey_added_count = 0;
	// gc_grey_removed_count = 0;
	gc_start_values_mult = 2.0f;
	gc_step_size_mult = 0.5f;
	gc_start_next_values = 16;
	gc_step_size = 0;
}

void OS::Core::gcResetGreyList()
{
	while(gc_grey_list_first){
		gcRemoveFromGreyList(gc_grey_list_first);
	}
	gc_grey_root_initialized = false;
	// OS_ASSERT(gc_grey_list.gc_grey_next == (Value*)&gc_grey_list);
	// OS_ASSERT(gc_grey_list.gc_grey_prev == (Value*)&gc_grey_list);
}

void OS::Core::gcAddToGreyList(ValueData& val)
{
	switch(val.type){
	case OS_VALUE_TYPE_STRING:
	case OS_VALUE_TYPE_ARRAY:
	case OS_VALUE_TYPE_OBJECT:
	case OS_VALUE_TYPE_USERDATA:
	case OS_VALUE_TYPE_USERPTR:
	case OS_VALUE_TYPE_FUNCTION:
	case OS_VALUE_TYPE_CFUNCTION:
		gcAddToGreyList(val.v.value);
		break;
	}
}

void OS::Core::gcAddToGreyList(GCValue * value)
{
	if(value->gc_color != GCValue::GC_WHITE){
		return;
	}
	// OS_ASSERT(!value->gc_grey_next && !value->gc_grey_prev);
	OS_ASSERT(!value->gc_grey_next);
	// gc_grey_list.insertEnd(value);
	value->gc_grey_next = gc_grey_list_first;
	gc_grey_list_first = value;
	value->gc_color = GCValue::GC_GREY;
	gc_grey_added_count++;
}

void OS::Core::gcRemoveFromGreyList(GCValue * value)
{
	// OS_ASSERT(value->gc_grey_next && value->gc_grey_prev);
	OS_ASSERT(value->gc_color == GCValue::GC_GREY);
	OS_ASSERT(gc_grey_list_first == value);
	// gc_grey_list.remove(value);
	gc_grey_list_first = value->gc_grey_next;
	value->gc_grey_next = NULL;
	value->gc_color = GCValue::GC_BLACK;
	// gc_grey_removed_count++;
}

void OS::Core::gcProcessGreyList(int step_size)
{
	for(; step_size > 0 && gc_grey_list_first; step_size--){
		gcProcessGreyValue(gc_grey_list_first);
	}
}

void OS::Core::gcProcessGreyTable(Table * table)
{
	Property * prop = table->first, * prop_next;
	for(; prop; prop = prop_next){
		prop_next = prop->next;
		if(prop->value.type == OS_VALUE_TYPE_WEAKREF){
			if(!values.get(prop->value.v.value_id)){
				PropertyIndex index = *prop;
				deleteTableProperty(table, index);
				continue;
			}
		}
		gcAddToGreyList(prop->value);
	}
}

void OS::Core::gcProcessStringsCacheTable()
{
	gcProcessGreyTable(string_values_table);
}

void OS::Core::gcProcessGreyProgram(Program * prog)
{
	if(prog->gc_time == gc_time){
		return;
	}
	prog->gc_time = gc_time;
	for(int i = 0; i < prog->num_strings; i++){
		gcAddToGreyList(prog->const_strings[i]);
	}
}

void OS::Core::gcProcessGreyFunctionValue(GCFunctionValue * func_value)
{
	OS_ASSERT(func_value->type == OS_VALUE_TYPE_FUNCTION);
	gcAddToGreyList(func_value->env);

	if(func_value->parent_inctance){
		gcProcessGreyFunctionRunning(func_value->parent_inctance);
	}
	gcProcessGreyProgram(func_value->prog);
}

void OS::Core::gcProcessGreyFunctionRunning(FunctionRunningInstance * func_running)
{
	OS_ASSERT(func_running->func && func_running->func->type == OS_VALUE_TYPE_FUNCTION);
	if(func_running->gc_time == gc_time){
		return;
	}
	func_running->gc_time = gc_time;

	gcAddToGreyList(func_running->func);

	if(func_running->self){
		gcAddToGreyList(func_running->self);
	}

	int i;
	FunctionDecl * func_decl = func_running->func->func_decl;

	for(i = 0; i < func_decl->max_up_count; i++){
		gcProcessGreyFunctionRunning(func_running->parent_inctances[i]);
	}

	int num_locals = func_decl->num_locals;
	for(i = 0; i < num_locals; i++){
		gcAddToGreyList(func_running->locals[i]);
	}
	if(func_running->arguments){
		gcAddToGreyList(func_running->arguments);
	}
	if(func_running->rest_arguments){
		gcAddToGreyList(func_running->rest_arguments);
	}
}


void OS::Core::gcProcessGreyValue(GCValue * value)
{
	OS_ASSERT(value->gc_color == GCValue::GC_GREY);
	gcRemoveFromGreyList(value);
	if(value->prototype){
		gcAddToGreyList(value->prototype);
	}
	if(value->table){
		gcProcessGreyTable(value->table);
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

	case OS_VALUE_TYPE_ARRAY:
	case OS_VALUE_TYPE_OBJECT:
		OS_ASSERT(dynamic_cast<GCObjectValue*>(value));
		break;

	case OS_VALUE_TYPE_FUNCTION:
		OS_ASSERT(dynamic_cast<GCFunctionValue*>(value));
		gcProcessGreyFunctionValue((GCFunctionValue*)value);
		break;

	case OS_VALUE_TYPE_CFUNCTION:
		{
			OS_ASSERT(dynamic_cast<GCCFunctionValue*>(value));
			GCCFunctionValue * func_value = (GCCFunctionValue*)value;
			ValueData * upvalues = (ValueData*)(func_value + 1);
			for(int i = 0; i < func_value->num_upvalues; i++){
				gcAddToGreyList(upvalues[i]);
			}
			break;
		}

	case OS_VALUE_TYPE_USERDATA:
	case OS_VALUE_TYPE_USERPTR:
		OS_ASSERT(dynamic_cast<GCUserDataValue*>(value));
		break;
	}
}

int OS::Core::gcStep()
{
	return OS_GC_PHASE_MARK;

	if(values.count == 0){
		return OS_GC_PHASE_MARK;
	}
	gc_time++;

	int step_size = gc_step_size;
	if(gc_values_head_index >= 0){
		OS_ASSERT(gc_values_head_index <= values.head_mask);
		int i = gc_values_head_index;
		step_size += step_size/16;
		for(; i <= values.head_mask && step_size > 0; i++){
			for(GCValue * value = values.heads[i], * next; value; value = next, step_size--){
				next = value->hash_next;
				if(value->gc_color == GCValue::GC_WHITE && !value->external_ref_count){
					OS_ASSERT(!isValueUsed(value));
					deleteValue(value);
					if(gc_values_head_index < 0){
						return OS_GC_PHASE_MARK;
					}
				}else if(value->gc_color == GCValue::GC_BLACK){
					value->gc_color = GCValue::GC_WHITE;
				}
			}
		}
		if(i <= values.head_mask){
			gc_values_head_index = i;
			return OS_GC_PHASE_SWEEP;
		}
		gc_values_head_index = -1;
		gc_start_next_values = (int)((float)values.count * gc_start_values_mult);

		return OS_GC_PHASE_MARK;
	}
	if(!gc_grey_root_initialized){
		gc_grey_root_initialized = true;
		gc_step_size = (int)((float)values.count * gc_step_size_mult);
		
		// int old_count = gc_grey_added_count;
		gcAddToGreyList(global_vars);
		gcAddToGreyList(user_pool);
		int i;
		for(i = 0; i < PROTOTYPE_COUNT; i++){
			gcAddToGreyList(prototypes[i]);
		}
		gcProcessStringsCacheTable();
		// step_size -= gc_grey_added_count - old_count;
	}
	int i;
	for(i = 0; i < stack_values.count; i++){
		gcAddToGreyList(stack_values[i]);
	}
	for(i = 0; i < call_stack_funcs.count; i++){
		gcProcessGreyFunctionRunning(call_stack_funcs[i]);
	}
	gcProcessGreyList(step_size);
	if(!gc_grey_list_first){
		gc_grey_root_initialized = false;
		gc_values_head_index = 0;
		return OS_GC_PHASE_SWEEP;
	}
	return OS_GC_PHASE_MARK;
}

void OS::Core::gcFinishSweepPhase()
{
	if(values.count == 0){
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
	if(values.count == 0){
		return;
	}
	while(gc_values_head_index < 0){
		gc_step_size = values.count * 2;
		gcStep();
	}
}

void OS::Core::gcStepIfNeeded()
{
	if(gc_values_head_index >= 0 || gc_grey_root_initialized){
		gcStep();
	}else if(gc_start_next_values <= values.count){
		gcFinishSweepPhase();
		gcStep();
	}
}

void OS::Core::gcFull()
{
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

void OS::Core::clearValue(ValueData& val)
{
	switch(val.type){
	case OS_VALUE_TYPE_NULL:
		return;

	case OS_VALUE_TYPE_BOOL:
		val.v.boolean = 0;
		break;

	case OS_VALUE_TYPE_NUMBER:
		val.v.number = 0;
		break;

	case OS_VALUE_TYPE_WEAKREF:
		val.v.value_id = 0;
		break;

	default:
		val.v.value = 0;
	}
	val.type = OS_VALUE_TYPE_NULL;
}

void OS::Core::clearValue(GCValue * val)
{
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
			OS_ASSERT(dynamic_cast<GCUserDataValue*>(val));
			GCUserDataValue * userdata = (GCUserDataValue*)val;
			if(userdata->dtor){
				userdata->dtor(allocator, userdata->ptr, userdata->user_param);
				userdata->dtor = NULL;
			}
			userdata->ptr = NULL;
			userdata->crc = 0;
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
			func_value->num_upvalues = 0;
			break;
		}

	case OS_VALUE_TYPE_ARRAY:
	case OS_VALUE_TYPE_OBJECT:
		OS_ASSERT(dynamic_cast<GCObjectValue*>(val));
		break;
	}
	if(val->table){
		// when object is destroying, some properties could be already destroyed
		// so destructor can't use self properties and can break gc process
		// so destructors are disabled
		/* {
			bool prototype_enabled = true;
			Value * func = getPropertyValue(val, PropertyIndex(strings->__destructor, PropertyIndex::KeepStringIndex()), prototype_enabled);
			if(func){
				pushValue(func);
				pushValue(val);
				call(0, 0);
			}
		} */

		Table * table = val->table;
		val->table = NULL;
		deleteTable(table);
	}
	if(val->prototype){
		// prototype could be already destroyed by gc or will be destroyed soon
		val->prototype = NULL;
	}
	val->type = OS_VALUE_TYPE_UNKNOWN;
}

bool OS::Core::isValueUsed(GCValue * val)
{
	struct Lib {
		Core * core;
		GCValue * val;

		bool findAt(ValueData& cur)
		{
			GCValue * value = cur.getGCValue();
			return value && findAt(value);
		}

		bool findAt(FunctionRunningInstance * func_running)
		{
			if(findAt(func_running->func)){
				return true;
			}
			if(func_running->self && findAt(func_running->self)){
				return true;
			}
			FunctionDecl * func_decl = func_running->func->func_decl;
			for(int i = 0; i < func_decl->num_locals; i++){
				if(findAt(func_running->locals[i])){
					return true;
				}
			}
			if(func_running->arguments && findAt(func_running->arguments)){
				return true;
			}
			if(func_running->rest_arguments && findAt(func_running->rest_arguments)){
				return true;
			}
			return false;
		}

		bool findAt(GCValue * cur)
		{
			if(cur == val){
				return true;
			}
			if(cur == val->prototype){
				return true;
			}
			if(cur->table){
				Property * prop = cur->table->first;
				for(; prop; prop = prop->next){
					if(findAt(prop->value)){
						return true;
					}
				}
			}
			switch(cur->type){
			case OS_VALUE_TYPE_FUNCTION:
				{
					OS_ASSERT(dynamic_cast<GCFunctionValue*>(cur));
					GCFunctionValue * func_value = (GCFunctionValue*)cur;
					if(findAt(func_value->env)){
						return true;
					}
					if(func_value->parent_inctance && findAt(func_value->parent_inctance)){
						return true;
					}
					for(int i = 0; i < func_value->prog->num_strings; i++){
						if(findAt(func_value->prog->const_strings[i])){
							return true;
						}
					}
					break;
				}

			case OS_VALUE_TYPE_CFUNCTION:
				{
					OS_ASSERT(dynamic_cast<GCCFunctionValue*>(cur));
					GCCFunctionValue * func_value = (GCCFunctionValue*)cur;
					ValueData * upvalues = (ValueData*)(func_value + 1);
					for(int i = 0; i < func_value->num_upvalues; i++){
						if(findAt(upvalues[i])){
							return true;
						}
					}
					break;
				}
			}
			return false;
		}

	} lib = {this, val};

	if(lib.findAt(global_vars)){
		return true;
	}
	int i;
	for(i = 0; i < stack_values.count; i++){
		if(lib.findAt(stack_values[i])){
			return true;
		}
	}
	for(i = 0; i < call_stack_funcs.count; i++){
		if(lib.findAt(call_stack_funcs[i])){
			return true;
		}
	}
	return false;
}

void OS::Core::deleteValue(GCValue * val)
{
	OS_ASSERT(val);
	OS_ASSERT(val->gc_color != GCValue::GC_GREY);
	unregisterValue(val->value_id);
	clearValue(val);
	val->~GCValue();
	free(val);
	num_destroyed_values++;
}

OS::Core::Property * OS::Core::setTableValue(Table * table, PropertyIndex& index, ValueData& value)
{
	OS_ASSERT(table);

	// TODO: correct ???
	gcAddToGreyList(value);

	Property * prop = table->get(index);
	if(prop){
		prop->value = value;
		return prop;
	}
	prop = new (malloc(sizeof(Property))) Property(index);
	prop->value = value;
	addTableProperty(table, prop);
	return prop;
}

void OS::Core::setPropertyValue(GCValue * table_value, PropertyIndex& index, ValueData& value, bool prototype_enabled, bool setter_enabled)
{
#ifdef OS_DEBUG
	if(index.index.type == OS_VALUE_TYPE_NULL){
		error(OS_WARNING, OS_TEXT("object set null index"));
	}
#endif
	// TODO: correct ???
	gcAddToGreyList(value);

	Property * prop = NULL;
	Table * table = table_value->table;
	if(table && (prop = table->get(index))){
		prop->value = value;
		return;
	}

	if(prototype_enabled){
		GCValue * cur_value = table_value;
		while(cur_value->prototype){
			cur_value = cur_value->prototype;
			Table * cur_table = cur_value->table;
			if(cur_table && (prop = cur_table->get(index))){
				prop->value = value;
				return;
			}
		}
	}

	if(index.index.type == OS_VALUE_TYPE_STRING && strings->syntax_prototype == index.index.v.string){
		switch(table_value->type){
		case OS_VALUE_TYPE_STRING:
		case OS_VALUE_TYPE_ARRAY:
		case OS_VALUE_TYPE_OBJECT:
		case OS_VALUE_TYPE_FUNCTION:
			table_value->prototype = value.v.value;
			break;

		case OS_VALUE_TYPE_USERDATA:
		case OS_VALUE_TYPE_USERPTR:
		case OS_VALUE_TYPE_CFUNCTION:
			// TODO: warning???
			break;
		}
		return;
	}

	if(setter_enabled){
		ValueData func;
		// GCValue * self = table_value.v.value;
		if(index.index.type == OS_VALUE_TYPE_STRING){
			const void * buf1 = strings->__set.toChar();
			int size1 = strings->__set.getDataSize();
			const void * buf2 = OS_TEXT("@");
			int size2 = sizeof(OS_CHAR);
			const void * buf3 = index.index.v.string->toChar();
			int size3 = index.index.v.string->getDataSize();
			OS_BYTE * buf = (OS_BYTE*)alloca(size1 + size2 + size3 + sizeof(OS_CHAR));
			OS_MEMCPY(buf, buf1, size1);
			OS_MEMCPY(buf+size1, buf2, size2);
			OS_MEMCPY(buf+size1+size2, buf3, size3);
			buf[size1+size2+size3] = (OS_CHAR)0;
			GCStringValue * name = newStringValue(buf, (size1 + size2 + size3) / sizeof(OS_CHAR));
			if(getPropertyValue(func, table_value, PropertyIndex(name, PropertyIndex::KeepStringIndex()), prototype_enabled)){
				pushValueData(func);
				pushValue(table_value);
				pushValueData(value);
				call(1, 0);
				return;
			}
		}
		if(getPropertyValue(func, table_value, PropertyIndex(strings->__set, PropertyIndex::KeepStringIndex()), prototype_enabled)){
			pushValueData(func);
			pushValue(table_value);
			pushValueData(index.index);
			pushValueData(value);
			call(2, 0);
			return;
		}
	}

	if(!table){
		table_value->table = table = newTable();
	}
	prop = new (malloc(sizeof(Property))) Property(index);
	prop->value = value;
	addTableProperty(table, prop);
	// setTableValue(table, index, value);
}

void OS::Core::setPropertyValue(ValueData& table_value, PropertyIndex& index, ValueData& value, bool prototype_enabled, bool setter_enabled)
{
	switch(table_value.type){
	case OS_VALUE_TYPE_NULL:
		return;

	case OS_VALUE_TYPE_BOOL:
		if(prototype_enabled){
			return setPropertyValue(prototypes[PROTOTYPE_BOOL], index, value, prototype_enabled, setter_enabled);
		}
		return;

	case OS_VALUE_TYPE_NUMBER:
		if(prototype_enabled){
			return setPropertyValue(prototypes[PROTOTYPE_NUMBER], index, value, prototype_enabled, setter_enabled);
		}
		return;

	case OS_VALUE_TYPE_STRING:
		if(prototype_enabled){
			return setPropertyValue(prototypes[PROTOTYPE_STRING], index, value, prototype_enabled, setter_enabled);
		}
		return;

	case OS_VALUE_TYPE_ARRAY:
	case OS_VALUE_TYPE_OBJECT:
	case OS_VALUE_TYPE_USERDATA:
	case OS_VALUE_TYPE_USERPTR:
	case OS_VALUE_TYPE_FUNCTION:
	case OS_VALUE_TYPE_CFUNCTION:
		return setPropertyValue(table_value.v.value, index, value, prototype_enabled, setter_enabled);
	}
}

void OS::Core::pushPrototype(const ValueData& val)
{
	switch(val.type){
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
		pushValue(val.v.value);
		return;
	}
}

void OS::Core::setPrototype(const ValueData& val, const ValueData& proto)
{
	switch(val.type){
	case OS_VALUE_TYPE_NULL:
	case OS_VALUE_TYPE_BOOL:
	case OS_VALUE_TYPE_NUMBER:
		return;

	case OS_VALUE_TYPE_STRING:
	case OS_VALUE_TYPE_ARRAY:
	case OS_VALUE_TYPE_OBJECT:
	case OS_VALUE_TYPE_USERDATA:
	case OS_VALUE_TYPE_USERPTR:
	case OS_VALUE_TYPE_FUNCTION:
	case OS_VALUE_TYPE_CFUNCTION:
		val.v.value->prototype = proto.getGCValue();
		return;
	}
}

/*
OS::Core::ValueData OS::Core::newBoolValue(bool val)
{
	return ValueData(val);
}

OS::Core::ValueData OS::Core::newNumberValue(OS_FLOAT val)
{
	return ValueData(val);
}
*/

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
		while(len > 0 && OS_ISSPACE(*str)){
			str++;
			len--;
		}
	}
	if(trim_right){
		while(len > 0 && OS_ISSPACE(str[len-1])){
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
		while(len > 0 && OS_ISSPACE(*str)){
			str++;
			len--;
			changed = true;
		}
	}
	if(trim_right){
		while(len > 0 && OS_ISSPACE(str[len-1])){
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
	Property * prop;
	if(string_values_table->heads){
		int hash = Utils::keyToHash((OS_BYTE*)buf, size);
		prop = string_values_table->heads[hash & string_values_table->head_mask];
		for(; prop; prop = prop->hash_next){
			if(prop->isEqual(hash, buf, size)){
				OS_ASSERT(prop->value.type == OS_VALUE_TYPE_WEAKREF);
				GCValue * value = values.get(prop->value.v.value_id);
				if(value){
					OS_ASSERT(value->type == OS_VALUE_TYPE_STRING);
					OS_ASSERT(dynamic_cast<GCStringValue*>(value));
					return (GCStringValue*)value;
				}
				PropertyIndex index = *prop;
				deleteTableProperty(string_values_table, index);
			}
		}
	}
	GCStringValue * value = GCStringValue::alloc(allocator, buf, size);
	PropertyIndex index(value, PropertyIndex::KeepStringIndex());
	prop = new (malloc(sizeof(Property))) Property(index);
	prop->value = ValueData(value->value_id, WeakRef());
	addTableProperty(string_values_table, prop);
	return value;
}

OS::Core::GCStringValue * OS::Core::newStringValue(const void * buf1, int size1, const void * buf2, int size2)
{
	Property * prop;
	if(string_values_table->heads){
		int hash = Utils::keyToHash(buf1, size1, buf2, size2);
		prop = string_values_table->heads[hash & string_values_table->head_mask];
		for(; prop; prop = prop->hash_next){
			if(prop->isEqual(hash, buf1, size1, buf2, size2)){
				OS_ASSERT(prop->value.type == OS_VALUE_TYPE_WEAKREF);
				GCValue * value = values.get(prop->value.v.value_id);
				if(value){
					OS_ASSERT(value->type == OS_VALUE_TYPE_STRING);
					OS_ASSERT(dynamic_cast<GCStringValue*>(value));
					return (GCStringValue*)value;
				}
				PropertyIndex index = *prop;
				deleteTableProperty(string_values_table, index);
			}
		}
	}
	GCStringValue * value = GCStringValue::alloc(allocator, buf1, size1, buf2, size2);
	PropertyIndex index(value, PropertyIndex::KeepStringIndex());
	prop = new (malloc(sizeof(Property))) Property(index);
	prop->value = ValueData(value->value_id, WeakRef());
	addTableProperty(string_values_table, prop);
	return value;
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

OS::Core::GCStringValue * OS::Core::newStringValue(int temp_buf_size, const OS_CHAR * fmt, ...)
{
	va_list va;
	va_start(va, fmt);
	OS_CHAR * buf = (OS_CHAR*)allocator->malloc(temp_buf_size * sizeof(OS_CHAR));
	OS_VSNPRINTF(buf, sizeof(OS_CHAR) * temp_buf_size, temp_buf_size-1, fmt, va);
	GCStringValue * result = newStringValue(buf);
	allocator->free(buf);
	va_end(va);
	return result;
}

OS::Core::GCStringValue * OS::Core::newStringValueVa(int temp_buf_size, const OS_CHAR * fmt, va_list va)
{
	OS_CHAR * buf = (OS_CHAR*)allocator->malloc(temp_buf_size * sizeof(OS_CHAR));
	OS_VSNPRINTF(buf, sizeof(OS_CHAR) * temp_buf_size, temp_buf_size-1, fmt, va);
	GCStringValue * result = newStringValue(buf);
	allocator->free(buf);
	return result;
}

OS::Core::GCCFunctionValue * OS::Core::newCFunctionValue(OS_CFunction func, void * user_param)
{
	return newCFunctionValue(func, 0, user_param);
}

OS::Core::GCCFunctionValue * OS::Core::newCFunctionValue(OS_CFunction func, int num_upvalues, void * user_param)
{
	OS_ASSERT(stack_values.count >= num_upvalues);
	if(!func){
		return NULL;
	}
	GCCFunctionValue * res = new (malloc(sizeof(GCCFunctionValue) + sizeof(ValueData) * num_upvalues)) GCCFunctionValue();
	res->prototype = prototypes[PROTOTYPE_FUNCTION];
	res->func = func;
	res->user_param = user_param;
	res->num_upvalues = num_upvalues;
	ValueData * upvalues = (ValueData*)(res + 1);
	for(int i = 0; i < num_upvalues; i++){
		upvalues[i] = stack_values[stack_values.count - num_upvalues + i];
	}
	res->type = OS_VALUE_TYPE_CFUNCTION;
	pop(num_upvalues);
	registerValue(res);
	return res;
}

OS::Core::GCUserDataValue * OS::Core::newUserDataValue(int crc, int data_size, OS_UserDataDtor dtor, void * user_param)
{
	GCUserDataValue * res = new (malloc(sizeof(GCUserDataValue) + data_size)) GCUserDataValue();
	res->crc = crc;
	res->dtor = dtor;
	res->user_param = user_param;
	res->ptr = res + 1;
	res->type = OS_VALUE_TYPE_USERDATA;
	registerValue(res);
	return res;
}

OS::Core::GCUserDataValue * OS::Core::newUserPointerValue(int crc, void * data, OS_UserDataDtor dtor, void * user_param)
{
	GCUserDataValue * res = new (malloc(sizeof(GCUserDataValue))) GCUserDataValue();
	res->crc = crc;
	res->dtor = dtor;
	res->user_param = user_param;
	res->ptr = data;
	res->type = OS_VALUE_TYPE_USERPTR;
	registerValue(res);
	return res;
}

OS::Core::GCObjectValue * OS::Core::newObjectValue()
{
	return newObjectValue(prototypes[PROTOTYPE_OBJECT]);
}

OS::Core::GCObjectValue * OS::Core::newObjectValue(GCValue * prototype)
{
	GCObjectValue * res = new (malloc(sizeof(GCObjectValue))) GCObjectValue();
	res->prototype = prototype;
	res->type = OS_VALUE_TYPE_OBJECT;
	registerValue(res);
	return res;
}

OS::Core::GCObjectValue * OS::Core::newArrayValue()
{
	return newObjectValue(prototypes[PROTOTYPE_ARRAY]);
}

void OS::Core::pushValueData(const ValueData val)
{
	reserveStackValues(stack_values.count+1);
	stack_values.buf[stack_values.count++] = val;
}

void OS::Core::pushNull()
{
	pushValueData(ValueData());
}

void OS::Core::pushStackValue(int offs)
{
	pushValueData(getStackValue(offs));
}

void OS::Core::copyValue(int raw_from, int raw_to)
{
	reserveStackValues(raw_to+1);
	stack_values.buf[raw_to] = stack_values.buf[raw_from];
}

void OS::Core::pushTrue()
{
	pushValueData(ValueData(true));
}

void OS::Core::pushFalse()
{
	pushValueData(ValueData(false));
}

void OS::Core::pushBool(bool val)
{
	pushValueData(ValueData(val));
}

void OS::Core::pushNumber(OS_INT val)
{
	pushValueData(ValueData((OS_FLOAT)val));
}

void OS::Core::pushNumber(OS_FLOAT val)
{
	pushValueData(ValueData(val));
}

OS::Core::GCStringValue * OS::Core::pushStringValue(const String& val)
{
	return pushValue(newStringValue(val));
}

OS::Core::GCStringValue * OS::Core::pushStringValue(const OS_CHAR * val)
{
	return pushValue(newStringValue(val));
}

OS::Core::GCCFunctionValue * OS::Core::pushCFunctionValue(OS_CFunction func, void * user_param)
{
	return pushValue(newCFunctionValue(func, user_param));
}

OS::Core::GCCFunctionValue * OS::Core::pushCFunctionValue(OS_CFunction func, int upvalues, void * user_param)
{
	return pushValue(newCFunctionValue(func, upvalues, user_param));
}

OS::Core::GCUserDataValue * OS::Core::pushUserDataValue(int crc, int data_size, OS_UserDataDtor dtor, void * user_param)
{
	return pushValue(newUserDataValue(crc, data_size, dtor, user_param));
}

OS::Core::GCUserDataValue * OS::Core::pushUserPointerValue(int crc, void * data, OS_UserDataDtor dtor, void * user_param)
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

OS::Core::GCObjectValue * OS::Core::pushArrayValue()
{
	return pushValue(newArrayValue());
}

void OS::Core::pushTypeOf(const ValueData& val)
{
	switch(val.type){
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

bool OS::Core::pushNumberOf(const ValueData& val)
{
	if(val.type == OS_VALUE_TYPE_NUMBER){
		pushValueData(val);
		return true;
	}
	OS_FLOAT number;
	if(isValueNumber(val, &number)){
		pushNumber(number);
		return true;
	}
	pushNull();
	return false;
}

bool OS::Core::pushStringOf(const ValueData& val)
{
	if(val.type == OS_VALUE_TYPE_STRING){
		pushValueData(val);
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

bool OS::Core::pushValueOf(const ValueData& val)
{
	switch(val.type){
	case OS_VALUE_TYPE_NULL:
	case OS_VALUE_TYPE_NUMBER:
	case OS_VALUE_TYPE_BOOL:
	case OS_VALUE_TYPE_STRING:
		pushValueData(val);
		return true;
	}
	bool prototype_enabled = true;
	ValueData func;
	if(getPropertyValue(func, val.v.value, PropertyIndex(strings->__valueof, PropertyIndex::KeepStringIndex()), prototype_enabled)
		&& func.isFunction())
	{
		pushValueData(func);
		pushValueData(val);
		call(0, 1);
		switch(stack_values.lastElement().type){
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

OS::Core::GCObjectValue * OS::Core::pushArrayOf(ValueData& val)
{
	GCObjectValue * object;
	switch(val.type){
	// case OS_VALUE_TYPE_NULL:
	// 	return pushNull(); // pushArrayValue();

	case OS_VALUE_TYPE_BOOL:
	case OS_VALUE_TYPE_NUMBER:
	case OS_VALUE_TYPE_STRING:
		setPropertyValue(object = pushArrayValue(), PropertyIndex(ValueData((OS_FLOAT)0)), val, true, true);
		return object;

	case OS_VALUE_TYPE_ARRAY:
		return pushValue(val.v.object);

	case OS_VALUE_TYPE_OBJECT:
		object = pushArrayValue();
		if(val.v.object->table && val.v.object->table->count > 0){
			object->table = newTable();
			initTableProperties(object->table, val.v.object->table);
			reorderTableKeys(object->table);
		}
		return object;
	}
	pushNull();
	return NULL;
}

OS::Core::GCObjectValue * OS::Core::pushObjectOf(ValueData& val)
{
	GCObjectValue * object;
	switch(val.type){
	// case OS_VALUE_TYPE_NULL:
	// 	return pushObjectValue();

	case OS_VALUE_TYPE_BOOL:
	case OS_VALUE_TYPE_NUMBER:
	case OS_VALUE_TYPE_STRING:
		setPropertyValue(object = pushObjectValue(), PropertyIndex(ValueData((OS_FLOAT)0)), val, true, true);
		return object;

	case OS_VALUE_TYPE_ARRAY:
	case OS_VALUE_TYPE_OBJECT:
		return pushValue(val.v.object);
	}
	pushNull();
	return NULL;
}

OS::Core::GCUserDataValue * OS::Core::pushUserDataOf(ValueData& val)
{
	switch(val.type){
	case OS_VALUE_TYPE_USERDATA:
	case OS_VALUE_TYPE_USERPTR:
		return pushValue(val.v.userdata);
	}
	pushNull();
	return NULL;
}

bool OS::Core::pushFunctionOf(ValueData& val)
{
	switch(val.type){
	case OS_VALUE_TYPE_FUNCTION:
	case OS_VALUE_TYPE_CFUNCTION:
		pushValueData(val);
		return true;
	}
	pushNull();
	return false;
}

void OS::Core::pushCloneValue(ValueData& val)
{
	GCValue * value, * new_value;
	switch(val.type){
	case OS_VALUE_TYPE_NULL:
	case OS_VALUE_TYPE_BOOL:
	case OS_VALUE_TYPE_NUMBER:
	case OS_VALUE_TYPE_STRING:
		pushValueData(val);
		return;

	case OS_VALUE_TYPE_ARRAY:
		value = val.v.value;
		new_value = pushArrayValue();
		new_value->prototype = value->prototype;
		break;

	case OS_VALUE_TYPE_OBJECT:
		value = val.v.value;
		new_value = pushObjectValue(value->prototype);
		break;

	case OS_VALUE_TYPE_FUNCTION:
	case OS_VALUE_TYPE_USERDATA:
	case OS_VALUE_TYPE_USERPTR:
	case OS_VALUE_TYPE_CFUNCTION:
		value = val.v.value;
		new_value = pushValue(value);
		break;
				
	default:
		pushNull();
		return;
	}
	OS_ASSERT(new_value->type != OS_VALUE_TYPE_NULL);
	if(new_value != value && value->table){
		new_value->table = newTable();
		initTableProperties(new_value->table, value->table);
	}
	// removeStackValue(-2);

	switch(new_value->type){
	case OS_VALUE_TYPE_ARRAY:
	case OS_VALUE_TYPE_OBJECT:
	case OS_VALUE_TYPE_USERDATA:
	case OS_VALUE_TYPE_USERPTR:
		{
			bool prototype_enabled = true;
			ValueData func;
			if(getPropertyValue(func, new_value, 
				PropertyIndex(strings->__clone, PropertyIndex::KeepStringIndex()), prototype_enabled)
				&& func.isFunction())
			{
				pushValueData(func);
				pushValue(new_value);
				call(0, 1);
				OS_ASSERT(stack_values.count >= 1);
				removeStackValue(-2);
			}
		}
	}
}

void OS::Core::pushOpResultValue(int opcode, ValueData& value)
{
	struct Lib
	{
		Core * core;

		void pushSimpleOpcodeValue(int opcode, ValueData& value)
		{
			switch(opcode){
			case Program::OP_BIT_NOT:
				return core->pushNumber((OS_FLOAT)(~core->valueToInt(value)));

			case Program::OP_PLUS:
				if(value.type == OS_VALUE_TYPE_NUMBER){
					return core->pushValueData(value);
				}
				return core->pushNumber(core->valueToNumber(value));

			case Program::OP_NEG:
				return core->pushNumber(-core->valueToNumber(value));

			case Program::OP_LENGTH:
				// return core->pushNumber(core->valueToString(value).getDataSize() / sizeof(OS_CHAR));
				return pushObjectMethodOpcodeValue(core->strings->__len, value);
			}
			return core->pushNull();
		}

		void pushObjectMethodOpcodeValue(const String& method_name, ValueData& value)
		{
			bool prototype_enabled = true;
			ValueData func;
			if(core->getPropertyValue(func, value, 
				PropertyIndex(method_name, PropertyIndex::KeepStringIndex()), prototype_enabled)
				&& func.isFunction())
			{
				core->pushValueData(func);
				core->pushValueData(value);
				core->call(0, 1);
				return;
			}
			return core->pushNull();
		}

		void pushObjectOpcodeValue(int opcode, ValueData& value)
		{
			switch(opcode){
			case Program::OP_BIT_NOT:
				return pushObjectMethodOpcodeValue(core->strings->__bitnot, value);

			case Program::OP_PLUS:
				return pushObjectMethodOpcodeValue(core->strings->__plus, value);

			case Program::OP_NEG:
				return pushObjectMethodOpcodeValue(core->strings->__neg, value);

			case Program::OP_LENGTH:
				return pushObjectMethodOpcodeValue(core->strings->__len, value);
			}
			return core->pushNull();
		}

		void pushUnaryOpcodeValue(int opcode, ValueData& value)
		{
			switch(value.type){
			case OS_VALUE_TYPE_NULL:
			case OS_VALUE_TYPE_NUMBER:
			case OS_VALUE_TYPE_BOOL:
			case OS_VALUE_TYPE_STRING:
				return pushSimpleOpcodeValue(opcode, value);

			case OS_VALUE_TYPE_ARRAY:
			case OS_VALUE_TYPE_OBJECT:
			case OS_VALUE_TYPE_USERDATA:
			case OS_VALUE_TYPE_USERPTR:
				return pushObjectOpcodeValue(opcode, value);
			}
			return core->pushNull();
		}
	} lib = {this};
	return lib.pushUnaryOpcodeValue(opcode, value);
}

void OS::Core::pushOpResultValue(int opcode, ValueData& left_value, ValueData& right_value)
{
	struct Lib
	{
		Core * core;

		bool isEqualExactly(ValueData& left_value, ValueData& right_value)
		{
			if(left_value.type == right_value.type){ // && left_value->prototype == right_value->prototype){
				switch(left_value.type){
				case OS_VALUE_TYPE_NULL:
					return true;

				case OS_VALUE_TYPE_NUMBER:
					return left_value.v.number == right_value.v.number;

				case OS_VALUE_TYPE_BOOL:
					return left_value.v.boolean == right_value.v.boolean;

				case OS_VALUE_TYPE_STRING:
					// the same strings are always share one instance, so check only value_id

				case OS_VALUE_TYPE_ARRAY:
				case OS_VALUE_TYPE_OBJECT:
				case OS_VALUE_TYPE_USERDATA:
				case OS_VALUE_TYPE_USERPTR:
				case OS_VALUE_TYPE_FUNCTION:
				case OS_VALUE_TYPE_CFUNCTION:
					return left_value.v.value == right_value.v.value;
				
				case OS_VALUE_TYPE_WEAKREF:
					return left_value.v.value_id == right_value.v.value_id;
				}
			}
			return false;
		}

		int compareNumbers(OS_FLOAT num1, OS_FLOAT num2)
		{
			if(num1 > num2){
				return 1;
			}
			if(num1 < num2){
				return -1;
			}
			return 0;
		}

		int compareStrings(GCStringValue * left_string_data, OS_FLOAT right_number)
		{
			OS_CHAR buf[128];
			Utils::numToStr(buf, right_number);
			return left_string_data->cmp(buf);
		}

		int compareStrings(GCStringValue * left_string_data, GCStringValue * right_string_data)
		{
			return left_string_data->cmp(right_string_data);
		}

		int compareObjectToValue(ValueData& left_value, ValueData& right_value)
		{
			GCValue * left = left_value.v.value;
			switch(left->type){
			case OS_VALUE_TYPE_STRING:
				{
					OS_ASSERT(dynamic_cast<GCStringValue*>(left));
					GCStringValue * string = (GCStringValue*)left;
					return compareStringToValue(left_value, string, right_value);
				}

			case OS_VALUE_TYPE_ARRAY:
			case OS_VALUE_TYPE_OBJECT:
			case OS_VALUE_TYPE_USERDATA:
			case OS_VALUE_TYPE_USERPTR:
				switch(right_value.type){
				case OS_VALUE_TYPE_NULL:
					return 1;

				case OS_VALUE_TYPE_STRING:
				case OS_VALUE_TYPE_NUMBER:
				case OS_VALUE_TYPE_BOOL:
				case OS_VALUE_TYPE_ARRAY:
				case OS_VALUE_TYPE_OBJECT:
				case OS_VALUE_TYPE_USERDATA:
				case OS_VALUE_TYPE_USERPTR:
					{
						bool prototype_enabled = true;
						ValueData func;
						if(core->getPropertyValue(func, left, 
							PropertyIndex(core->strings->__cmp, PropertyIndex::KeepStringIndex()), prototype_enabled)
							&& func.isFunction())
						{
							core->pushValueData(func);
							core->pushValueData(left);
							core->pushValueData(right_value);
							core->call(1, 1);
							OS_ASSERT(core->stack_values.count >= 1);
							struct Pop { Core * core; ~Pop(){ core->pop(); } } pop = {core};
							const ValueData& value = core->stack_values.lastElement();
							if(value.type == OS_VALUE_TYPE_NUMBER){
								return (int)value.v.number;
							}
						}
						if(right_value.type != OS_VALUE_TYPE_STRING && right_value.type != OS_VALUE_TYPE_NUMBER && right_value.type != OS_VALUE_TYPE_BOOL){
							GCValue * right = right_value.v.value;
							OS_ASSERT(right->type == right_value.type);
							if(left->prototype != right->prototype){
								switch(right->type){
								case OS_VALUE_TYPE_ARRAY:
								case OS_VALUE_TYPE_OBJECT:
								case OS_VALUE_TYPE_USERDATA:
								case OS_VALUE_TYPE_USERPTR:
									if(core->getPropertyValue(func, right, 
										PropertyIndex(core->strings->__cmp, PropertyIndex::KeepStringIndex()), prototype_enabled)
										&& func.isFunction())
									{
										core->pushValueData(func);
										core->pushValueData(right_value);
										core->pushValueData(left);
										core->call(1, 1);
										OS_ASSERT(core->stack_values.count >= 1);
										struct Pop { Core * core; ~Pop(){ core->pop(); } } pop = {core};
										const ValueData& value = core->stack_values.lastElement();
										if(value.type == OS_VALUE_TYPE_NUMBER){
											return -(int)value.v.number;
										}
									}
								}
							}
						}
						core->pushValueOf(ValueData(left));
						ValueData left_value = core->stack_values.lastElement();
						struct Pop { Core * core; ~Pop(){ core->pop(); } } pop = {core};
						return compareValues(left_value, right_value);
					}
				}
				break;
			}
			// generic compare
			return 1; // left->value_id - (int)right_value;
		}

		int compareNumberToValue(ValueData& left_value, OS_FLOAT left_number, ValueData& right_value)
		{
			switch(right_value.type){
			case OS_VALUE_TYPE_NULL:
				return 1;

			case OS_VALUE_TYPE_NUMBER:
				return compareNumbers(left_number, right_value.v.number);

			case OS_VALUE_TYPE_BOOL:
				return compareNumbers(left_number, right_value.v.boolean);

			case OS_VALUE_TYPE_STRING:
				return -compareStrings(right_value.v.string, left_number);
			}
			return -compareObjectToValue(right_value, left_value);
		}

		int compareStringToValue(ValueData& left_value, GCStringValue * left_string_data, ValueData& right_value)
		{
			switch(right_value.type){
			case OS_VALUE_TYPE_NULL:
				return 1;

			case OS_VALUE_TYPE_NUMBER:
				return compareStrings(left_string_data, right_value.v.number);

			case OS_VALUE_TYPE_BOOL:
				return compareStrings(left_string_data, right_value.v.boolean);

			case OS_VALUE_TYPE_STRING:
				return compareStrings(left_string_data, right_value.v.string);
			}
			return -compareObjectToValue(right_value, left_value);
		}

		int compareValues(ValueData& left_value, ValueData& right_value)
		{
			switch(left_value.type){
			case OS_VALUE_TYPE_NULL:
				return right_value.type == OS_VALUE_TYPE_NULL ? 0 : -1;

			case OS_VALUE_TYPE_NUMBER:
				return compareNumberToValue(left_value, left_value.v.number, right_value);

			case OS_VALUE_TYPE_BOOL:
				return compareNumberToValue(left_value, left_value.v.boolean, right_value);

			// case OS_VALUE_TYPE_STRING:
			// 	return compareStringToValue(left_value->v.string_data, right_value);
			}
			return compareObjectToValue(left_value, right_value);
		}

		void pushSimpleOpcodeValue(int opcode, ValueData& left_value, ValueData& right_value)
		{
			switch(opcode){
			case Program::OP_CONCAT:
				core->pushStringValue(core->newStringValue(core->valueToString(left_value), core->valueToString(right_value)));
				return;

			case Program::OP_BIT_AND:
				return core->pushNumber(core->valueToInt(left_value) & core->valueToInt(right_value));

			case Program::OP_BIT_OR:
				return core->pushNumber(core->valueToInt(left_value) | core->valueToInt(right_value));

			case Program::OP_BIT_XOR:
				return core->pushNumber(core->valueToInt(left_value) ^ core->valueToInt(right_value));

			case Program::OP_ADD: // +
				return core->pushNumber(core->valueToNumber(left_value) + core->valueToNumber(right_value));

			case Program::OP_SUB: // -
				return core->pushNumber(core->valueToNumber(left_value) - core->valueToNumber(right_value));

			case Program::OP_MUL: // *
				return core->pushNumber(core->valueToNumber(left_value) * core->valueToNumber(right_value));

			case Program::OP_DIV: // /
				{
					OS_FLOAT right = core->valueToNumber(right_value);
					if(!right){
						// TODO: exception???
						return core->pushNumber(0.0);
					}
					return core->pushNumber(core->valueToNumber(left_value) / right);
				}

			case Program::OP_MOD: // %
				{
					OS_FLOAT right = core->valueToNumber(right_value);
					if(!right){
						// TODO: exception???
						return core->pushNumber(0.0);
					}
					return core->pushNumber(OS_MATH_FMOD(core->valueToNumber(left_value), right));
				}

			case Program::OP_LSHIFT: // <<
				return core->pushNumber(core->valueToInt(left_value) << core->valueToInt(right_value));

			case Program::OP_RSHIFT: // >>
				return core->pushNumber(core->valueToInt(left_value) >> core->valueToInt(right_value));

			case Program::OP_POW: // **
				return core->pushNumber(OS_MATH_POW(core->valueToNumber(left_value), core->valueToNumber(right_value)));
			}
			core->pushNull();
		}

		void pushObjectMethodOpcodeValue(int opcode, const String& method_name, ValueData& left_value, ValueData& right_value, GCValue * object, bool is_left_side)
		{
			bool prototype_enabled = true;
			ValueData func;
			if(core->getPropertyValue(func, object, 
				PropertyIndex(method_name, PropertyIndex::KeepStringIndex()), prototype_enabled)
				&& func.isFunction())
			{
				core->pushValueData(func);
				core->pushValueData(object);
				core->pushValueData(left_value);
				core->pushValueData(right_value);
				core->pushValueData(is_left_side ? right_value : left_value);
				core->call(3, 1);
				return;
			}
			ValueData& other_value = is_left_side ? right_value : left_value;
			switch(other_value.type){
			case OS_VALUE_TYPE_ARRAY:
			case OS_VALUE_TYPE_OBJECT:
			case OS_VALUE_TYPE_USERDATA:
			case OS_VALUE_TYPE_USERPTR:
				{
					GCValue * other = other_value.v.value;
					if(object->prototype == other->prototype){
						core->pushNull();
						return;
					}
					if(core->getPropertyValue(func, other, 
						PropertyIndex(method_name, PropertyIndex::KeepStringIndex()), prototype_enabled)
						&& func.isFunction())
					{
						core->pushValueData(func);
						core->pushValueData(other_value);
						core->pushValueData(left_value);
						core->pushValueData(right_value);
						core->pushValueData(!is_left_side ? right_value : left_value);
						core->call(3, 1);
						return;
					}
				}
			}
			{
				core->pushValueOf(left_value);
				ValueData left_value = core->stack_values.lastElement();
				pushBinaryOpcodeValue(opcode, left_value, right_value);
				core->removeStackValue(-2);
			}
		}

		void pushObjectOpcodeValue(int opcode, ValueData& left_value, ValueData& right_value, GCValue * object, bool is_left_side)
		{
			switch(opcode){
			case Program::OP_CONCAT:
				return pushObjectMethodOpcodeValue(opcode, core->strings->__concat, left_value, right_value, object, is_left_side);

			case Program::OP_BIT_AND:
				return pushObjectMethodOpcodeValue(opcode, core->strings->__bitand, left_value, right_value, object, is_left_side);

			case Program::OP_BIT_OR:
				return pushObjectMethodOpcodeValue(opcode, core->strings->__bitor, left_value, right_value, object, is_left_side);

			case Program::OP_BIT_XOR:
				return pushObjectMethodOpcodeValue(opcode, core->strings->__bitxor, left_value, right_value, object, is_left_side);

			case Program::OP_ADD: // +
				return pushObjectMethodOpcodeValue(opcode, core->strings->__add, left_value, right_value, object, is_left_side);

			case Program::OP_SUB: // -
				return pushObjectMethodOpcodeValue(opcode, core->strings->__sub, left_value, right_value, object, is_left_side);

			case Program::OP_MUL: // *
				return pushObjectMethodOpcodeValue(opcode, core->strings->__mul, left_value, right_value, object, is_left_side);

			case Program::OP_DIV: // /
				return pushObjectMethodOpcodeValue(opcode, core->strings->__div, left_value, right_value, object, is_left_side);

			case Program::OP_MOD: // %
				return pushObjectMethodOpcodeValue(opcode, core->strings->__mod, left_value, right_value, object, is_left_side);

			case Program::OP_LSHIFT: // <<
				return pushObjectMethodOpcodeValue(opcode, core->strings->__lshift, left_value, right_value, object, is_left_side);

			case Program::OP_RSHIFT: // >>
				return pushObjectMethodOpcodeValue(opcode, core->strings->__rshift, left_value, right_value, object, is_left_side);

			case Program::OP_POW: // **
				return pushObjectMethodOpcodeValue(opcode, core->strings->__pow, left_value, right_value, object, is_left_side);
			}
			core->pushNull();
		}

		void pushBinaryOpcodeValue(int opcode, ValueData& left_value, ValueData& right_value)
		{
			switch(left_value.type){
			case OS_VALUE_TYPE_NULL:
			case OS_VALUE_TYPE_NUMBER:
			case OS_VALUE_TYPE_BOOL:
			case OS_VALUE_TYPE_STRING:
				switch(right_value.type){
				case OS_VALUE_TYPE_NULL:
				case OS_VALUE_TYPE_NUMBER:
				case OS_VALUE_TYPE_BOOL:
				case OS_VALUE_TYPE_STRING:
					return pushSimpleOpcodeValue(opcode, left_value, right_value);

				case OS_VALUE_TYPE_ARRAY:
				case OS_VALUE_TYPE_OBJECT:
				case OS_VALUE_TYPE_USERDATA:
				case OS_VALUE_TYPE_USERPTR:
					return pushObjectOpcodeValue(opcode, left_value, right_value, right_value.v.value, false);
				}
				break;

			case OS_VALUE_TYPE_ARRAY:
			case OS_VALUE_TYPE_OBJECT:
			case OS_VALUE_TYPE_USERDATA:
			case OS_VALUE_TYPE_USERPTR:
				switch(right_value.type){
				case OS_VALUE_TYPE_NULL:
				case OS_VALUE_TYPE_NUMBER:
				case OS_VALUE_TYPE_BOOL:
				case OS_VALUE_TYPE_STRING:
					// return pushObjectOpcodeValue(opcode, left_value, right_value, left_value.v.value, true);

				case OS_VALUE_TYPE_ARRAY:
				case OS_VALUE_TYPE_OBJECT:
				case OS_VALUE_TYPE_USERDATA:
				case OS_VALUE_TYPE_USERPTR:
					return pushObjectOpcodeValue(opcode, left_value, right_value, left_value.v.value, true);
				}
			}
			core->pushNull();
		}
	} lib = {this};

	switch(opcode){
	case Program::OP_LOGIC_PTR_EQ:
		return pushBool(lib.isEqualExactly(left_value, right_value));

	case Program::OP_LOGIC_PTR_NE:
		return pushBool(!lib.isEqualExactly(left_value, right_value));

	case Program::OP_LOGIC_EQ:
		if(left_value.type == OS_VALUE_TYPE_NUMBER && right_value.type == OS_VALUE_TYPE_NUMBER){
			return pushBool(left_value.v.number == right_value.v.number);
		}
		return pushBool(lib.compareValues(left_value, right_value) == 0);

	case Program::OP_LOGIC_NE:
		if(left_value.type == OS_VALUE_TYPE_NUMBER && right_value.type == OS_VALUE_TYPE_NUMBER){
			return pushBool(left_value.v.number != right_value.v.number);
		}
		return pushBool(lib.compareValues(left_value, right_value) != 0);

	case Program::OP_LOGIC_GE:
		if(left_value.type == OS_VALUE_TYPE_NUMBER && right_value.type == OS_VALUE_TYPE_NUMBER){
			return pushBool(left_value.v.number >= right_value.v.number);
		}
		return pushBool(lib.compareValues(left_value, right_value) >= 0);

	case Program::OP_LOGIC_LE:
		if(left_value.type == OS_VALUE_TYPE_NUMBER && right_value.type == OS_VALUE_TYPE_NUMBER){
			return pushBool(left_value.v.number <= right_value.v.number);
		}
		return pushBool(lib.compareValues(left_value, right_value) <= 0);

	case Program::OP_LOGIC_GREATER:
		if(left_value.type == OS_VALUE_TYPE_NUMBER && right_value.type == OS_VALUE_TYPE_NUMBER){
			return pushBool(left_value.v.number > right_value.v.number);
		}
		return pushBool(lib.compareValues(left_value, right_value) > 0);

	case Program::OP_LOGIC_LESS:
		if(left_value.type == OS_VALUE_TYPE_NUMBER && right_value.type == OS_VALUE_TYPE_NUMBER){
			return pushBool(left_value.v.number < right_value.v.number);
		}
		return pushBool(lib.compareValues(left_value, right_value) < 0);

	case Program::OP_CONCAT:
		if(left_value.type == OS_VALUE_TYPE_STRING && right_value.type == OS_VALUE_TYPE_STRING){
			pushStringValue(newStringValue(left_value.v.string, right_value.v.string));
		}
		break;

	case Program::OP_BIT_AND:
		if(left_value.type == OS_VALUE_TYPE_NUMBER && right_value.type == OS_VALUE_TYPE_NUMBER){
			return pushNumber((OS_INT)left_value.v.number & (OS_INT)right_value.v.number);
		}
		break;

	case Program::OP_BIT_OR:
		if(left_value.type == OS_VALUE_TYPE_NUMBER && right_value.type == OS_VALUE_TYPE_NUMBER){
			return pushNumber((OS_INT)left_value.v.number | (OS_INT)right_value.v.number);
		}
		break;

	case Program::OP_BIT_XOR:
		if(left_value.type == OS_VALUE_TYPE_NUMBER && right_value.type == OS_VALUE_TYPE_NUMBER){
			return pushNumber((OS_INT)left_value.v.number ^ (OS_INT)right_value.v.number);
		}
		break;

	case Program::OP_ADD: // +
		if(left_value.type == OS_VALUE_TYPE_NUMBER && right_value.type == OS_VALUE_TYPE_NUMBER){
			return pushNumber(left_value.v.number + right_value.v.number);
		}
		break;

	case Program::OP_SUB: // -
		if(left_value.type == OS_VALUE_TYPE_NUMBER && right_value.type == OS_VALUE_TYPE_NUMBER){
			return pushNumber(left_value.v.number - right_value.v.number);
		}
		break;

	case Program::OP_MUL: // *
		if(left_value.type == OS_VALUE_TYPE_NUMBER && right_value.type == OS_VALUE_TYPE_NUMBER){
			return pushNumber(left_value.v.number * right_value.v.number);
		}
		break;

	case Program::OP_DIV: // /
		if(left_value.type == OS_VALUE_TYPE_NUMBER && right_value.type == OS_VALUE_TYPE_NUMBER){
			if(!right_value.v.number){
				// TODO: exception???
				return pushNumber(0.0);
			}
			return pushNumber(left_value.v.number / right_value.v.number);
		}
		break;

	case Program::OP_MOD: // %
		if(left_value.type == OS_VALUE_TYPE_NUMBER && right_value.type == OS_VALUE_TYPE_NUMBER){
			if(!right_value.v.number){
				// TODO: exception???
				return pushNumber(0.0);
			}
			return pushNumber(OS_MATH_FMOD(left_value.v.number, right_value.v.number));
		}
		break;

	case Program::OP_LSHIFT: // <<
		if(left_value.type == OS_VALUE_TYPE_NUMBER && right_value.type == OS_VALUE_TYPE_NUMBER){
			return pushNumber((OS_INT)left_value.v.number << (OS_INT)right_value.v.number);
		}
		break;

	case Program::OP_RSHIFT: // >>
		if(left_value.type == OS_VALUE_TYPE_NUMBER && right_value.type == OS_VALUE_TYPE_NUMBER){
			return pushNumber((OS_INT)left_value.v.number >> (OS_INT)right_value.v.number);
		}
		break;

	case Program::OP_POW: // **
		if(left_value.type == OS_VALUE_TYPE_NUMBER && right_value.type == OS_VALUE_TYPE_NUMBER){
			return pushNumber(OS_MATH_POW(left_value.v.number, right_value.v.number));
		}
		break;
	}
	return lib.pushBinaryOpcodeValue(opcode, left_value, right_value);
}

void OS::Core::setGlobalValue(const String& name, ValueData& value, bool prototype_enabled, bool setter_enabled)
{
	setPropertyValue(global_vars, Core::PropertyIndex(name), value, prototype_enabled, setter_enabled);
}

void OS::Core::setGlobalValue(const OS_CHAR * name, ValueData& value, bool prototype_enabled, bool setter_enabled)
{
	setGlobalValue(String(allocator, name), value, prototype_enabled, setter_enabled);
}

int OS::Core::getStackOffs(int offs)
{
	return offs <= 0 ? stack_values.count + offs + 1 : offs;
}

OS::Core::ValueData OS::Core::getStackValue(int offs)
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
	OS_ASSERT(false);
	return ValueData();
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
		stack_values.capacity = stack_values.capacity > 0 ? stack_values.capacity*2 : 4;
		if(stack_values.capacity < new_capacity){
			stack_values.capacity = new_capacity; // (capacity+3) & ~3;
		}
		ValueData * new_buf = (ValueData*)malloc(sizeof(ValueData)*stack_values.capacity);
		OS_MEMCPY(new_buf, stack_values.buf, sizeof(ValueData) * stack_values.count);
		free(stack_values.buf);
		stack_values.buf = new_buf;

		for(int i = 0; i < call_stack_funcs.count; i++){
			FunctionRunningInstance * func_running = call_stack_funcs[i];
			func_running->locals = stack_values.buf + func_running->locals_stack_pos;
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
		OS_MEMMOVE(stack_values.buf + start, stack_values.buf + end, sizeof(ValueData) * (stack_values.count - end));
		stack_values.count -= end - start;
	}
	gcStepIfNeeded();
}

void OS::Core::removeStackValue(int offs)
{
	removeStackValues(offs, 1);
}

void OS::Core::removeAllStackValues()
{
	stack_values.count = 0;
	gcStepIfNeeded();
}

void OS::Core::pop(int count)
{
	if(count >= stack_values.count){
		OS_ASSERT(count == stack_values.count);
		stack_values.count = 0;
	}else{
		stack_values.count -= count;
	}
	gcStepIfNeeded();
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
	ValueData * temp_values = (ValueData*)alloca(sizeof(ValueData) * count);
	OS_MEMCPY(temp_values, stack_values.buf + offs, sizeof(ValueData) * count);
	if(new_offs > offs){
		OS_MEMMOVE(stack_values.buf + offs, stack_values.buf + offs+count, sizeof(ValueData) * (new_offs - offs));
	}else{
		OS_MEMMOVE(stack_values.buf + new_offs+count, stack_values.buf + new_offs, sizeof(ValueData) * (offs - new_offs));
	}
	OS_MEMCPY(stack_values.buf + new_offs, temp_values, sizeof(ValueData) * count);
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

	ValueData value = stack_values[offs];
	if(new_offs > offs){
		OS_MEMMOVE(stack_values.buf + offs, stack_values.buf + offs+1, sizeof(ValueData) * (new_offs - offs));
	}else{
		OS_MEMMOVE(stack_values.buf + new_offs+1, stack_values.buf + new_offs, sizeof(ValueData) * (offs - new_offs));
	}
	stack_values[new_offs] = value;
}

void OS::Core::insertValue(const ValueData val, int offs)
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
		OS_MEMMOVE(stack_values.buf + offs+1, stack_values.buf + offs, sizeof(ValueData) * count);
	}
	stack_values[offs] = val;
}

void OS::pushNull()
{
	core->pushNull();
}

/*
void OS::pushNumber(OS_INT16 val)
{
	core->pushNumber((OS_FLOAT)val);
}

void OS::pushNumber(OS_INT32 val)
{
	core->pushNumber((OS_FLOAT)val);
}

void OS::pushNumber(OS_INT64 val)
{
	core->pushNumber((OS_FLOAT)val);
}
*/

void OS::pushNumber(OS_FLOAT val)
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

void OS::pushString(const Core::String& val)
{
	core->pushStringValue(val);
}

void OS::pushCFunction(OS_CFunction func, void * user_param)
{
	core->pushCFunctionValue(func, user_param);
}

void OS::pushCFunction(OS_CFunction func, int upvalues, void * user_param)
{
	core->pushCFunctionValue(func, upvalues, user_param);
}

void * OS::pushUserData(int crc, int data_size, OS_UserDataDtor dtor, void * user_param)
{
	Core::GCUserDataValue * userdata = core->pushUserDataValue(crc, data_size, dtor, user_param);
	return userdata ? userdata->ptr : NULL;
}

void * OS::pushUserPointer(int crc, void * data, OS_UserDataDtor dtor, void * user_param)
{
	Core::GCUserDataValue * userdata = core->pushUserPointerValue(crc, data, dtor, user_param);
	return userdata ? userdata->ptr : NULL;
}

void OS::newObject()
{
	core->pushObjectValue();
}

void OS::newArray()
{
	core->pushArrayValue();
}

/*
void OS::pushValue(const Value& value)
{
	core->pushValueAutoNull(value.value);
}
*/

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
	core->pushValueData(core->values.get(id));
}

/*
void OS::getValueOf()
{
	Core::Value * value = core->getStackValue(-1);
	if(value){
		core->pushValueOf(value);
		remove(-2);
	}else{
		pushNull();
	}
}

void OS::getNumberOf()
{
	Core::Value * value = core->getStackValue(-1);
	if(value){
		core->pushNumberOf(value);
		remove(-2);
	}else{
		pushNull();
	}
}

void OS::getStringOf()
{
	Core::Value * value = core->getStackValue(-1);
	if(value){
		core->pushStringOf(value);
		remove(-2);
	}else{
		pushNull();
	}
}

void OS::getBooleanOf()
{
	Core::Value * value = core->getStackValue(-1);
	if(value){
		core->pushConstBoolValue(core->valueToBool(value));
		remove(-2);
	}else{
		pushNull();
	}
}
*/

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

OS_FLOAT OS::toNumber(int offs, bool valueof_enabled)
{
	return core->valueToNumber(core->getStackValue(offs), valueof_enabled);
}

int OS::toInt(int offs, bool valueof_enabled)
{
	return (int)toNumber(offs, valueof_enabled);
}

bool OS::isNumber(int offs, OS_FLOAT * out)
{
	return core->isValueNumber(core->getStackValue(offs), out);
}

OS::String OS::toString(int offs, bool valueof_enabled)
{
	return String(this, core->valueToString(core->getStackValue(offs), valueof_enabled));
}

bool OS::isString(int offs, String * out)
{
	return core->isValueString(core->getStackValue(offs), out);
}

OS_EValueType OS::getType(int offs)
{
	return core->getStackValue(offs).type;
}

OS_EValueType OS::getTypeById(int id)
{
	Core::GCValue * val = core->values.get(id);
	return val ? val->type : OS_VALUE_TYPE_NULL;
}

bool OS::isType(OS_EValueType type, int offs)
{
	return core->getStackValue(offs).type == type;
}

bool OS::isNull(int offs)
{
	return isType(OS_VALUE_TYPE_NULL, offs);
}

bool OS::isObject(int offs)
{
	const Core::ValueData& val = core->getStackValue(offs);
	switch(val.type){
	case OS_VALUE_TYPE_OBJECT:
	case OS_VALUE_TYPE_ARRAY:
		return true;
	}
	return false;
}

bool OS::isUserData(int offs)
{
	Core::ValueData val = core->getStackValue(offs);
	switch(val.type){
	case OS_VALUE_TYPE_USERDATA:
	case OS_VALUE_TYPE_USERPTR:
		return true;
	}
	return false;
}

void * OS::toUserData(int crc)
{
	return toUserData(-1, crc);
}

void * OS::toUserData(int offs, int crc)
{
	Core::ValueData val = core->getStackValue(offs);
	switch(val.type){
	case OS_VALUE_TYPE_USERDATA:
	case OS_VALUE_TYPE_USERPTR:
		if(val.v.userdata->crc == crc){
			return val.v.userdata->ptr;
		}
	}
	return NULL;
}

/*
bool OS::Value::isObject() const
{
	if(value){
		switch(value->type){
		case OS_VALUE_TYPE_OBJECT:
		case OS_VALUE_TYPE_ARRAY:
			return true;
		}
	}
	return false;
}
*/

bool OS::isArray(int offs)
{
	return isType(OS_VALUE_TYPE_ARRAY, offs);
}

/*
bool OS::Value::isArray() const
{
	return isType(OS_VALUE_TYPE_ARRAY);
}
*/

bool OS::isFunction(int offs)
{
	return core->getStackValue(offs).isFunction();
}

bool OS::Core::isValueInstanceOf(GCValue * val, GCValue * prototype_val)
{
	// vectorClear(cache_values);
	// vectorAddItem(cache_values, val);
	while(val != prototype_val){
		val = val->prototype;
		if(!val){ // || cache_values.contains(val)){ // prevent recurse
			return false;
		}
		if(val->type == OS_VALUE_TYPE_ARRAY){
			int i = 0;
		}
		// vectorAddItem(cache_values, val);
	}
	return true;
}

bool OS::Core::isValueInstanceOf(const ValueData& val, const ValueData& prototype_val)
{
	GCValue * object = val.getGCValue();
	GCValue * proto = prototype_val.getGCValue();
	return object && proto && isValueInstanceOf(object, proto);
}

bool OS::isInstanceOf(int value_offs, int prototype_offs)
{
	return core->isValueInstanceOf(core->getStackValue(value_offs), core->getStackValue(prototype_offs));
}

/*
bool OS::Value::isInstanceOf(const Value& prototype_value) const
{
	if(value && prototype_value.value && allocator == prototype_value.allocator){
		return allocator->core->isValueInstanceOf(value, prototype_value.value);
	}
	return false;
}
*/

void OS::setProperty(bool prototype_enabled, bool setter_enabled)
{
	if(core->stack_values.count >= 3){
		Core::ValueData object = core->stack_values[core->stack_values.count - 3];
		Core::ValueData index = core->stack_values[core->stack_values.count - 2];
		Core::ValueData value = core->stack_values[core->stack_values.count - 1];
		core->setPropertyValue(object, Core::PropertyIndex(index), value, prototype_enabled, setter_enabled);
		pop(3);
	}else{
		// error
		pop(3);
	}
}

void OS::setProperty(const OS_CHAR * name, bool prototype_enabled, bool setter_enabled)
{
	getProperty(Core::String(this, name), prototype_enabled, setter_enabled);
}

void OS::setProperty(const Core::String& name, bool prototype_enabled, bool setter_enabled)
{
	pushString(name);
	move(-1, -2);
	setProperty(prototype_enabled, setter_enabled);
}

void OS::addProperty()
{
	pushStackValue(-2);
	runOp(OP_LENGTH);
	move(-2, -1);
	setProperty(false, false);
}

void OS::getPrototype()
{
	if(core->stack_values.count >= 1){
		core->pushPrototype(core->stack_values.lastElement());
	}else{
		pushNull();
	}
}

void OS::setPrototype()
{
	if(core->stack_values.count >= 2){
		Core::ValueData value = core->stack_values[core->stack_values.count - 2];
		Core::ValueData proto = core->stack_values[core->stack_values.count - 1];
		core->setPrototype(value, proto);
	}
	pop(2);
}

int OS::getId(int offs)
{
	Core::ValueData val = core->getStackValue(offs);
	switch(val.type){
	case OS_VALUE_TYPE_STRING:
	case OS_VALUE_TYPE_ARRAY:
	case OS_VALUE_TYPE_OBJECT:
	case OS_VALUE_TYPE_USERDATA:
	case OS_VALUE_TYPE_USERPTR:
	case OS_VALUE_TYPE_FUNCTION:
	case OS_VALUE_TYPE_CFUNCTION:
		return val.v.value->value_id;

	case OS_VALUE_TYPE_WEAKREF:
		return val.v.value_id;
	}
	return 0;
}

bool OS::Core::getPropertyValue(OS::Core::ValueData& result, Table * table, const PropertyIndex& index)
{
#ifdef OS_DEBUG
	if(index.index.type == OS_VALUE_TYPE_NULL){
		error(OS_WARNING, OS_TEXT("object get null index"));
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

bool OS::Core::getPropertyValue(OS::Core::ValueData& result, GCValue * table_value, PropertyIndex& index, bool prototype_enabled)
{
#ifdef OS_DEBUG
	if(index.index.type == OS_VALUE_TYPE_NULL){
		error(OS_WARNING, OS_TEXT("object get null index"));
	}
#endif
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
	if(index.index.type == OS_VALUE_TYPE_STRING && strings->syntax_prototype == index.index.v.string){
		result = table_value->prototype;
		return true;
	}
	return false;
}

bool OS::Core::getPropertyValue(ValueData& result, ValueData& table_value, PropertyIndex& index, bool prototype_enabled)
{
	switch(table_value.type){
	case OS_VALUE_TYPE_NULL:
		return false;

	case OS_VALUE_TYPE_BOOL:
		return prototype_enabled && getPropertyValue(result, prototypes[PROTOTYPE_BOOL], index, prototype_enabled);

	case OS_VALUE_TYPE_NUMBER:
		return prototype_enabled && getPropertyValue(result, prototypes[PROTOTYPE_NUMBER], index, prototype_enabled);

	case OS_VALUE_TYPE_STRING:
		return prototype_enabled && getPropertyValue(result, prototypes[PROTOTYPE_STRING], index, prototype_enabled);

	case OS_VALUE_TYPE_ARRAY:
	case OS_VALUE_TYPE_OBJECT:
	case OS_VALUE_TYPE_USERDATA:
	case OS_VALUE_TYPE_USERPTR:
	case OS_VALUE_TYPE_FUNCTION:
	case OS_VALUE_TYPE_CFUNCTION:
		return getPropertyValue(result, table_value.v.value, index, prototype_enabled);
	}
	return false;
}

void OS::Core::pushPropertyValue(GCValue * table_value, PropertyIndex& index, bool prototype_enabled, bool getter_enabled, bool auto_create)
{
	GCValue * self = table_value;
	for(;;){
		ValueData value;
		if(getPropertyValue(value, table_value, index, prototype_enabled)){
			return pushValueData(value);
		}
		if(getter_enabled){
			if(index.index.type == OS_VALUE_TYPE_STRING){
				const void * buf1 = strings->__set.toChar();
				int size1 = strings->__set.getDataSize();
				const void * buf2 = OS_TEXT("@");
				int size2 = sizeof(OS_CHAR);
				const void * buf3 = index.index.v.string->toChar();
				int size3 = index.index.v.string->getDataSize();
				OS_BYTE * buf = (OS_BYTE*)alloca(size1 + size2 + size3 + sizeof(OS_CHAR));
				OS_MEMCPY(buf, buf1, size1);
				OS_MEMCPY(buf+size1, buf2, size2);
				OS_MEMCPY(buf+size1+size2, buf3, size3);
				buf[size1+size2+size3] = (OS_CHAR)0;
				GCStringValue * getter_name = newStringValue(buf, (size1 + size2 + size3) / sizeof(OS_CHAR));
				if(getPropertyValue(value, table_value, PropertyIndex(getter_name, PropertyIndex::KeepStringIndex()), prototype_enabled)){
					pushValueData(value);
					pushValue(self);
					call(0, 1);
					return;
				}
			}
			if(getPropertyValue(value, table_value, PropertyIndex(strings->__get, PropertyIndex::KeepStringIndex()), prototype_enabled)){
				auto_create = false;
				if(value.type == OS_VALUE_TYPE_OBJECT){
					table_value = value.v.value;
					continue;
				}
				pushValueData(value);
				pushValue(self);
				pushValueData(index.index);
				call(1, 1);
				return;
			}
		}
		if(auto_create){
			setPropertyValue(self, index, ValueData(pushObjectValue()), false, false); 
			return;
		}
		break;
	}
	return pushNull();
}

void OS::Core::pushPropertyValue(ValueData& table_value, PropertyIndex& index, bool prototype_enabled, bool getter_enabled, bool auto_create)
{
	switch(table_value.type){
	case OS_VALUE_TYPE_NULL:
		break;

	case OS_VALUE_TYPE_BOOL:
		if(prototype_enabled){
			return pushPropertyValue(prototypes[PROTOTYPE_BOOL], index, prototype_enabled, getter_enabled, auto_create);
		}
		break;

	case OS_VALUE_TYPE_NUMBER:
		if(prototype_enabled){
			return pushPropertyValue(prototypes[PROTOTYPE_NUMBER], index, prototype_enabled, getter_enabled, auto_create);
		}
		break;

	case OS_VALUE_TYPE_STRING:
		if(prototype_enabled){
			return pushPropertyValue(prototypes[PROTOTYPE_STRING], index, prototype_enabled, getter_enabled, auto_create);
		}
		break;

	case OS_VALUE_TYPE_ARRAY:
	case OS_VALUE_TYPE_OBJECT:
	case OS_VALUE_TYPE_USERDATA:
	case OS_VALUE_TYPE_USERPTR:
	case OS_VALUE_TYPE_FUNCTION:
	case OS_VALUE_TYPE_CFUNCTION:
		return pushPropertyValue(table_value.v.value, index, prototype_enabled, getter_enabled, auto_create);
	}
	pushNull();
}

void OS::getProperty(bool prototype_enabled, bool getter_enabled)
{
	if(core->stack_values.count >= 2){
		Core::ValueData object = core->stack_values[core->stack_values.count - 2];
		Core::ValueData index = core->stack_values[core->stack_values.count - 1];
		core->pushPropertyValue(object, Core::PropertyIndex(index), prototype_enabled, getter_enabled, false);
		core->removeStackValues(-3, 2);
	}else{
		// error
		pop(2);
		pushNull();
	}
}

void OS::getProperty(const OS_CHAR * name, bool prototype_enabled, bool getter_enabled)
{
	getProperty(Core::String(this, name), prototype_enabled, getter_enabled);
}

void OS::getProperty(const Core::String& name, bool prototype_enabled, bool getter_enabled)
{
	pushString(name);
	getProperty(prototype_enabled, getter_enabled);
}

void OS::Core::releaseFunctionRunningInstance(OS::Core::FunctionRunningInstance * func_running)
{
	// OS_ASSERT(func_running->func && func_running->func->type == OS_VALUE_TYPE_FUNCTION);
	// OS_ASSERT(func_running->func->value.func->func_decl);
	// OS_ASSERT(func_running->self);
	// OS_ASSERT(func_running->func->value.func->parent_inctance != func_running);

	if(--func_running->ref_count > 0){
		if(func_running->locals_stack_pos >= 0){
			int count = func_running->opcode_stack_pos - func_running->locals_stack_pos;
			if(count > 0){
				ValueData * locals = (ValueData*)malloc(sizeof(ValueData) * count);
				OS_MEMCPY(locals, func_running->locals, sizeof(ValueData) * count);
				func_running->locals = locals;
			}else{
				func_running->locals = NULL;
			}
			func_running->locals_stack_pos = -1;
		}
		return;
	}

	// func_running->func could be already destroyed by gc or will be destroyed soon
	// FunctionDecl * func_decl = func_running->func->value.func->func_decl;
	// locals could be already destroyed by gc or will be destroyed soon
	/* for(i = 0; i < func_decl->num_locals; i++){
		releaseValue(func_running->locals[i]);
	} */
	// free(func_running->locals);
	if(func_running->locals_stack_pos < 0){
		free(func_running->locals);
	}
	func_running->locals = NULL;

	// value could be already destroyed by gc or will be destroyed soon
	// releaseValue(func_running->func); 
	func_running->func = NULL;

	// value could be already destroyed by gc or will be destroyed soon
	// releaseValue(func_running->self);
	func_running->self = NULL;

#ifdef FUNC_VAL_ONE_PARENT
	func_running->parent_inctances = NULL;
#else
	func_running->parent_inctance = NULL;
#endif

	func_running->arguments = NULL;
	func_running->rest_arguments = NULL;

	func_running->~FunctionRunningInstance();
	free(func_running);
}

void OS::Core::enterFunction(GCFunctionValue * func_value, GCValue * self, int params, int extra_remove_from_stack, int need_ret_values)
{
	OS_ASSERT(call_stack_funcs.count < OS_CALL_STACK_MAX_SIZE);
	OS_ASSERT(func_value->type == OS_VALUE_TYPE_FUNCTION);
	OS_ASSERT(stack_values.count >= params + extra_remove_from_stack);
	// OS_ASSERT(self);

	FunctionDecl * func_decl = func_value->func_decl;
	int i, num_extra_params = params > func_decl->num_params ? params - func_decl->num_params : 0;
	// int locals_mem_size = sizeof(ValueData) * (func_decl->num_locals + num_extra_params);
	int parents_mem_size = sizeof(FunctionRunningInstance*) * func_decl->max_up_count;
	
	FunctionRunningInstance * func_running = new (malloc(sizeof(FunctionRunningInstance) + parents_mem_size)) FunctionRunningInstance();
	func_running->func = func_value;
	func_running->self = self;
	
	func_running->caller_stack_pos = stack_values.count - params - extra_remove_from_stack;
	func_running->locals_stack_pos = func_running->caller_stack_pos + extra_remove_from_stack;
	func_running->opcode_stack_pos = func_running->locals_stack_pos + func_decl->num_locals + num_extra_params;
	func_running->bottom_stack_pos = func_running->opcode_stack_pos + 0; // how many opcodes use stack

	reserveStackValues(func_running->bottom_stack_pos + OS_RESERVE_STACK_SIZE);
	stack_values.count = func_running->bottom_stack_pos;

	func_running->num_params = params;
	func_running->num_extra_params = num_extra_params;
	func_running->locals = stack_values.buf + func_running->locals_stack_pos; // (ValueData*)(func_running + 1); // malloc(locals_mem_size);
	
	ValueData * extra_params = func_running->locals + func_decl->num_locals;
	if(num_extra_params > 0){
		OS_MEMCPY(extra_params, func_running->locals + (params - num_extra_params), sizeof(ValueData) * num_extra_params);
	}
	/* for(i = 0; i < num_extra_params; i++){
		extra_params[i] = func_running->locals[params - num_extra_params + i];
	} */
	int opcodes_vars = func_running->bottom_stack_pos - func_running->opcode_stack_pos;
	if(opcodes_vars > 0){
		OS_MEMSET(extra_params + num_extra_params, 0, sizeof(ValueData) * opcodes_vars);
	}
	/* for(; opcodes_vars > 0; opcodes_vars--, i++){
		extra_params[i] = ValueData();
	} */
	int func_params = func_decl->num_params < params ? func_decl->num_params : params;
	OS_ASSERT(func_params <= func_decl->num_locals);
	OS_MEMSET(func_running->locals + func_params, 0, sizeof(ValueData) * (func_decl->num_locals - func_params));
	/* for(i = func_params; i < func_decl->num_locals; i++){
		func_running->locals[i] = ValueData();
	} */

	func_running->parent_inctances = (FunctionRunningInstance**)(func_running + 1); // malloc(sizeof(FunctionRunningInstance*) * func_decl->max_up_count);
	FunctionRunningInstance * cur_parent = func_value->parent_inctance;
	for(i = 0; i < func_decl->max_up_count; i++){
		OS_ASSERT(cur_parent);
		func_running->parent_inctances[i] = cur_parent->retain();
		OS_ASSERT(cur_parent->func && cur_parent->func->type == OS_VALUE_TYPE_FUNCTION);
		cur_parent = cur_parent->func->parent_inctance;
	}

	func_running->need_ret_values = need_ret_values;
	func_running->opcodes_pos = func_decl->opcodes_pos;
	
	allocator->vectorAddItem(call_stack_funcs, func_running);

	gcProcessGreyFunctionRunning(func_running);
}

int OS::Core::leaveFunction()
{
	// Value * num_results = pop();
	return 0;
}

int OS::Core::execute()
{
restart:
	OS_ASSERT(call_stack_funcs.count > 0);
	FunctionRunningInstance * func_running = call_stack_funcs[call_stack_funcs.count-1];
	GCFunctionValue * func_value = func_running->func;
	FunctionDecl * func_decl = func_value->func_decl;
	ValueData env = func_value->env; // ? func_value_data->env : global_vars;
	Program * prog = func_value->prog;

	MemStreamReader opcodes(NULL, prog->opcodes->buffer + func_decl->opcodes_pos, func_decl->opcodes_size);
	opcodes.movePos(func_running->opcodes_pos - func_decl->opcodes_pos);

	int prog_num_numbers = prog->num_numbers;
	OS_FLOAT * prog_numbers = prog->const_numbers;

	int prog_num_strings = prog->num_strings;
	GCStringValue ** prog_strings = prog->const_strings;

	int caller_stack_pos = func_running->caller_stack_pos;

	OS * allocator = this->allocator;
	for(int opcodes_executed = 0;; opcodes_executed++){
		OS_ASSERT(stack_values.count >= func_running->bottom_stack_pos);
		func_running->opcodes_pos = opcodes.pos;
		if(opcodes_executed >= OS_INFINITE_LOOP_OPCODES){
			OS_ASSERT(false);
			// TODO: generate infinite loop error
			OS_ASSERT(stack_values.count >= caller_stack_pos);
			int ret_values = func_running->need_ret_values;
			syncStackRetValues(ret_values, 0);
			OS_ASSERT(call_stack_funcs.count > 0 && call_stack_funcs[call_stack_funcs.count-1] == func_running);
			call_stack_funcs.count--;
			releaseFunctionRunningInstance(func_running);
			removeStackValues(caller_stack_pos+1, stack_values.count - ret_values - caller_stack_pos);
			return ret_values;
		}
		int i;
		Program::OpcodeType opcode = (Program::OpcodeType)opcodes.readByte();
		switch(opcode){
		case Program::OP_DEBUGGER:
			{
				int line = opcodes.readUVariable();
				int pos = opcodes.readUVariable();
				int saved_lines = opcodes.readUVariable();
				const OS_CHAR * lines[OS_DEBUGGER_SAVE_NUM_LINES];
				OS_MEMSET(lines, 0, sizeof(lines));
				for(int i = 0; i < saved_lines; i++){
					int offs = opcodes.readUVariable();
					OS_ASSERT(offs >= 0 && offs < prog_num_strings);
					OS_ASSERT(prog_strings[offs]->type == OS_VALUE_TYPE_STRING);
					if(i < OS_DEBUGGER_SAVE_NUM_LINES){
						lines[i] = valueToString(prog_strings[offs]).toChar();
					}
				}
				DEBUG_BREAK;
				break;
			}

		case Program::OP_PUSH_NUMBER:
			i = opcodes.readUVariable();
			OS_ASSERT(i >= 0 && i < prog_num_numbers);
			pushValueData(ValueData(prog_numbers[i]));
			break;

		case Program::OP_PUSH_STRING:
			i = opcodes.readUVariable();
			OS_ASSERT(i >= 0 && i < prog_num_strings);
			OS_ASSERT(prog_strings[i]->type == OS_VALUE_TYPE_STRING);
			pushValue(prog_strings[i]);
			break;

		case Program::OP_PUSH_NULL:
			pushNull();
			break;

		case Program::OP_PUSH_TRUE:
			pushTrue();
			break;

		case Program::OP_PUSH_FALSE:
			pushFalse();
			break;

		case Program::OP_PUSH_FUNCTION:
			{
				int func_index = opcodes.readUVariable();
				OS_ASSERT(func_index > 0 && func_index < prog->num_functions);
				FunctionDecl * func_decl = prog->functions + func_index;
				allocator->core->pushFunctionValue(func_running, prog, func_decl);
	
				// OS_ASSERT(func_decl->opcodes_pos == opcodes.pos);
				opcodes.movePos(func_decl->opcodes_size);
				break;
			}

		case Program::OP_PUSH_NEW_OBJECT:
			pushObjectValue();
			break;

		case Program::OP_OBJECT_SET_BY_AUTO_INDEX:
			{
				OS_ASSERT(stack_values.count >= 2);
				ValueData object = stack_values[stack_values.count-2];
				ValueData value = stack_values[stack_values.count-1];
				OS_INT num_index = 0;
				switch(object.type){
				case OS_VALUE_TYPE_ARRAY:
				case OS_VALUE_TYPE_OBJECT:
					num_index = object.v.object->table ? object.v.object->table->next_id : 0;
					break;
				}
				setPropertyValue(object, PropertyIndex(ValueData((OS_FLOAT)num_index)), value, false, false);
				pop(); // keep object in stack
				break;
			}

		case Program::OP_OBJECT_SET_BY_EXP:
			{
				OS_ASSERT(stack_values.count >= 3);
				ValueData object = stack_values[stack_values.count - 3];
				ValueData value = stack_values[stack_values.count - 2];
				ValueData index = stack_values[stack_values.count - 1];
				setPropertyValue(object, Core::PropertyIndex(index), value, false, false);
				pop(2); // keep object in stack
				break;
			}

		case Program::OP_OBJECT_SET_BY_INDEX:
			{
				OS_ASSERT(stack_values.count >= 2);
				i = opcodes.readUVariable();
				OS_ASSERT(i >= 0 && i < prog_num_numbers);
				ValueData object = stack_values[stack_values.count-2];
				ValueData index = prog_numbers[i];
				ValueData value = stack_values[stack_values.count-1];
				setPropertyValue(object, Core::PropertyIndex(index), value, false, false);
				pop(); // keep object in stack
				break;
			}

		case Program::OP_OBJECT_SET_BY_NAME:
			{
				OS_ASSERT(stack_values.count >= 2);
				i = opcodes.readUVariable();
				OS_ASSERT(i >= 0 && i < prog_num_strings);
				OS_ASSERT(prog_strings[i]->type == OS_VALUE_TYPE_STRING);
				ValueData table_value = stack_values[stack_values.count-2];
				ValueData index_value = prog_strings[i];
				ValueData value = stack_values[stack_values.count-1];
				OS_ASSERT(index_value.type == OS_VALUE_TYPE_STRING);
				setPropertyValue(table_value, PropertyIndex(index_value.v.string, PropertyIndex::KeepStringIndex()), 
					value, false, false);
				pop(); // keep object in stack
				break;
			}

		case Program::OP_PUSH_ENV_VAR:
		case Program::OP_PUSH_ENV_VAR_AUTO_CREATE:
			{
				i = opcodes.readUVariable();
				OS_ASSERT(i >= 0 && i < prog_num_strings);
				OS_ASSERT(prog_strings[i]->type == OS_VALUE_TYPE_STRING);
				GCStringValue * name = prog_strings[i];
				// String name = valueToString(name_value);
				pushPropertyValue(env, PropertyIndex(name, PropertyIndex::KeepStringIndex()), 
					true, true, opcode == Program::OP_PUSH_ENV_VAR_AUTO_CREATE); 
				break;
			}

		case Program::OP_SET_ENV_VAR:
			{
				OS_ASSERT(stack_values.count >= 1);
				i = opcodes.readUVariable();
				OS_ASSERT(i >= 0 && i < prog_num_strings);
				OS_ASSERT(prog_strings[i]->type == OS_VALUE_TYPE_STRING);
				ValueData value = stack_values[stack_values.count-1];
				ValueData name_value = prog_strings[i];
				OS_ASSERT(name_value.type == OS_VALUE_TYPE_STRING);
				GCStringValue * name = name_value.v.string;
				// String name = valueToString(name_value);
				setPropertyValue(env, PropertyIndex(name, PropertyIndex::KeepStringIndex()), value, true, true);
				pop();
				break;
			}

		case Program::OP_PUSH_THIS:
			pushValue(func_running->self);
			break;

		case Program::OP_PUSH_ARGUMENTS:
			if(!func_running->arguments){
				int i;
				GCObjectValue * args = pushArrayValue();
				int num_params = func_running->num_params - func_running->num_extra_params;
				for(i = 0; i < num_params; i++){
					setPropertyValue(args, PropertyIndex(i), func_running->locals[i], false, false);
				}
				for(i = 0; i < func_running->num_extra_params; i++){
					ValueData arg = func_running->locals[i + func_decl->num_locals];
					setPropertyValue(args, PropertyIndex(i), arg, false, false);
				}
				func_running->arguments = args;
			}else{
				pushValue(func_running->arguments);
			}
			break;

		case Program::OP_PUSH_REST_ARGUMENTS:
			if(!func_running->rest_arguments){
				GCObjectValue * args = pushArrayValue();
				for(int i = 0; i < func_running->num_extra_params; i++){
					ValueData arg = func_running->locals[i + func_decl->num_locals];
					setPropertyValue(args, PropertyIndex(i), arg, false, false);
				}
				func_running->rest_arguments = args;
			}else{
				pushValue(func_running->rest_arguments);
			}
			break;

		case Program::OP_PUSH_LOCAL_VAR:
			{
				i = opcodes.readUVariable();
				if(i < func_decl->num_locals){
					pushValueData(func_running->locals[i]);
				}else{
					OS_ASSERT(false);
					pushNull();
				}
				break;
			}

		case Program::OP_PUSH_LOCAL_VAR_AUTO_CREATE:
			{
				i = opcodes.readUVariable();
				if(i < func_decl->num_locals){
					if(func_running->locals[i].type == OS_VALUE_TYPE_NULL){
						func_running->locals[i] = newObjectValue();
					}
					pushValueData(func_running->locals[i]);
				}else{
					OS_ASSERT(false);
					pushNull();
				}
				break;
			}

		case Program::OP_SET_LOCAL_VAR:
			{
				OS_ASSERT(stack_values.count >= 1);
				ValueData value = stack_values[stack_values.count-1];
				i = opcodes.readUVariable();
				if(i < func_decl->num_locals){
					func_running->locals[i] = value;
				}else{
					OS_ASSERT(false);
				}
				pop();
				break;
			}

		case Program::OP_PUSH_UP_LOCAL_VAR:
			{
				i = opcodes.readUVariable();
				int up_count = opcodes.readByte();
				if(up_count <= func_decl->max_up_count){
					FunctionRunningInstance * scope = func_running->parent_inctances[up_count-1];
					if(i < scope->func->func_decl->num_locals){
						pushValueData(scope->locals[i]);
						break;
					}
				}
				OS_ASSERT(false);
				pushNull();
				break;
			}

		case Program::OP_PUSH_UP_LOCAL_VAR_AUTO_CREATE:
			{
				i = opcodes.readUVariable();
				int up_count = opcodes.readByte();
				if(up_count <= func_decl->max_up_count){
					FunctionRunningInstance * scope = func_running->parent_inctances[up_count-1];
					if(i < scope->func->func_decl->num_locals){
						if(scope->locals[i].type == OS_VALUE_TYPE_NULL){
							scope->locals[i] = newObjectValue();
						}
						pushValueData(scope->locals[i]);
						break;
					}
				}
				OS_ASSERT(false);
				pushNull();
				break;
			}

		case Program::OP_SET_UP_LOCAL_VAR:
			{
				OS_ASSERT(stack_values.count >= 1);
				ValueData value = stack_values[stack_values.count-1];
				i = opcodes.readUVariable();
				int up_count = opcodes.readByte();
				if(up_count <= func_decl->max_up_count){
					FunctionRunningInstance * scope = func_running->parent_inctances[up_count-1];
					if(i < scope->func->func_decl->num_locals){
						scope->locals[i] = value;
						pop();
						break;
					}
				}
				OS_ASSERT(false);
				pop();
				break;
			}

		case Program::OP_IF_NOT_JUMP:
			{
				OS_ASSERT(stack_values.count >= 1);
				ValueData value = stack_values.lastElement();
				int offs = opcodes.readInt32();
				if(!valueToBool(value)){
					opcodes.movePos(offs);
				}
				pop();
				break;
			}

		case Program::OP_JUMP:
			{
				int offs = opcodes.readInt32();
				opcodes.movePos(offs);
				break;
			}

		case Program::OP_CALL:
			{
				int params = opcodes.readByte();
				int ret_values = opcodes.readByte();
				
				OS_ASSERT(stack_values.count >= 1 + params);
				// pushNull();
				// moveStackValue(-1, -params);
				insertValue(ValueData(), -params);
				call(params, ret_values);
				break;
			}

		case Program::OP_TAIL_CALL:
			{
				// OS_ASSERT(false);
				int params = opcodes.readByte();
				int ret_values = func_running->need_ret_values;
				
				OS_ASSERT(stack_values.count >= 1 + params);
				ValueData func_value = stack_values[stack_values.count-1-params];
				OS_ASSERT(call_stack_funcs.count > 0 && call_stack_funcs[call_stack_funcs.count-1] == func_running);
				switch(func_value.type){
				case OS_VALUE_TYPE_CFUNCTION:
					// pushNull();
					// moveStackValue(-1, -params);
					insertValue(ValueData(), -params);
					call(params, ret_values);
					break;

				case OS_VALUE_TYPE_FUNCTION:
					call_stack_funcs.count--;
					releaseFunctionRunningInstance(func_running);
					removeStackValues(caller_stack_pos+1, stack_values.count - 1-params - caller_stack_pos);
					enterFunction(func_value.v.func, NULL, params, 1, ret_values);
					goto restart;

				default:
					// TODO: warn or error here???
					pop(1+params);
					syncStackRetValues(ret_values, 0);
				}
				OS_ASSERT(stack_values.count == func_running->bottom_stack_pos + ret_values);
				// func_running->opcodes_pos = opcodes.getPos();
				OS_ASSERT(call_stack_funcs.count > 0 && call_stack_funcs[call_stack_funcs.count-1] == func_running);
				call_stack_funcs.count--;
				releaseFunctionRunningInstance(func_running);
				removeStackValues(caller_stack_pos+1, stack_values.count - ret_values - caller_stack_pos);
				return ret_values;
			}

		case Program::OP_CALL_METHOD:
			{
				int params = opcodes.readByte();
				int ret_values = opcodes.readByte();
				
				OS_ASSERT(stack_values.count >= 2 + params);
				ValueData table_value = stack_values[stack_values.count-2-params];
				ValueData index_value = stack_values[stack_values.count-1-params];
				pushPropertyValue(table_value, Core::PropertyIndex(index_value), true, true, false);
				ValueData value = stack_values.lastElement();
				// moveStackValue(-1, -1-params);
				GCValue * self = func_running->self;
				GCValue * call_self = table_value.getGCValue();
				if(call_self && (!self || self->prototype != call_self)){
					self = call_self;
				}
				pushValue(self);
				moveStackValues(-2, 2, -2-params);
				call(params, ret_values);
				removeStackValues(-2-ret_values, 2);
				break;
			}

		case Program::OP_TAIL_CALL_METHOD:
			{
				int params = opcodes.readByte();
				int ret_values = func_running->need_ret_values;
				
				OS_ASSERT(stack_values.count >= 2 + params);
				ValueData table_value = stack_values[stack_values.count-2-params];
				ValueData index_value = stack_values[stack_values.count-1-params];
				pushPropertyValue(table_value, Core::PropertyIndex(index_value), true, true, false);
				ValueData func_value = stack_values.lastElement();
				// moveStackValue(-1, -1-params);
				GCValue * self = func_running->self;
				GCValue * call_self = table_value.getGCValue();
				if(call_self && (!self || self->prototype != call_self)){
					self = call_self;
				}
				pushValue(self);
				moveStackValues(-2, 2, -2-params);

				OS_ASSERT(call_stack_funcs.count > 0 && call_stack_funcs[call_stack_funcs.count-1] == func_running);
				switch(func_value.type){
				case OS_VALUE_TYPE_CFUNCTION:
					call(params, ret_values);
					removeStackValues(-2-ret_values, 2);
					break;

				case OS_VALUE_TYPE_FUNCTION:
					call_stack_funcs.count--;
					releaseFunctionRunningInstance(func_running);
					removeStackValues(caller_stack_pos+1, stack_values.count - 4-params - caller_stack_pos);
					enterFunction(func_value.v.func, self, params, 4, ret_values);
					goto restart;

				default:
					// TODO: warn or error here???
					pop(4+params);
					syncStackRetValues(ret_values, 0);
				}
				OS_ASSERT(stack_values.count == func_running->bottom_stack_pos + ret_values);
				// func_running->opcodes_pos = opcodes.getPos();
				OS_ASSERT(call_stack_funcs.count > 0 && call_stack_funcs[call_stack_funcs.count-1] == func_running);
				call_stack_funcs.count--;
				releaseFunctionRunningInstance(func_running);
				removeStackValues(caller_stack_pos+1, stack_values.count - ret_values - caller_stack_pos);
				return ret_values;
			}

		case Program::OP_RETURN:
			{
				int cur_ret_values = opcodes.readByte();
				int ret_values = func_running->need_ret_values;
				syncStackRetValues(ret_values, cur_ret_values);
				OS_ASSERT(stack_values.count == func_running->bottom_stack_pos + ret_values);
				// func_running->opcodes_pos = opcodes.getPos();
				OS_ASSERT(call_stack_funcs.count > 0 && call_stack_funcs[call_stack_funcs.count-1] == func_running);
				call_stack_funcs.count--;
				releaseFunctionRunningInstance(func_running);
				removeStackValues(caller_stack_pos+1, stack_values.count - ret_values - caller_stack_pos);
				return ret_values;
			}

		case Program::OP_GET_PROPERTY:
		case Program::OP_GET_PROPERTY_AUTO_CREATE:
			{
				int ret_values = opcodes.readByte();
				OS_ASSERT(stack_values.count >= 2);
				ValueData table_arg = stack_values.buf[stack_values.count - 2];
				ValueData index_arg = stack_values.buf[stack_values.count - 1];
				pushPropertyValue(table_arg, Core::PropertyIndex(index_arg), true, true, opcode == Program::OP_GET_PROPERTY_AUTO_CREATE);
				removeStackValues(-3, 2);
				syncStackRetValues(ret_values, 1);
				break;
			}

		case Program::OP_SET_PROPERTY:
			{
				OS_ASSERT(stack_values.count >= 3);
				ValueData value_arg = stack_values.buf[stack_values.count - 3];
				ValueData table_arg = stack_values.buf[stack_values.count - 2];
				ValueData index_arg = stack_values.buf[stack_values.count - 1];
				setPropertyValue(table_arg, Core::PropertyIndex(index_arg), value_arg, true, true);
				pop(3);
				break;
			}

		case Program::OP_SET_DIM:
			{
				int params = opcodes.readByte();
				
				OS_ASSERT(stack_values.count >= 2 + params);
				moveStackValue(-2-params, -1-params); // put value to the first param
				params++;

				ValueData table_value = stack_values[stack_values.count-1-params];
				ValueData func;
				if(getPropertyValue(func, table_value, 
					PropertyIndex(strings->__setdim, PropertyIndex::KeepStringIndex()), true)
					&& func.isFunction())
				{
					pushValueData(func);
					pushValueData(table_value);
					moveStackValues(-2, 2, -2-params); // put func value before params
					call(params, 0);
					removeStackValue(-1); // remove table_value
				}else{
					pop(params+1);
				}
				break;
			}

		case Program::OP_EXTENDS:
			{
				OS_ASSERT(stack_values.count >= 2);
				ValueData left_value = stack_values[stack_values.count-2];
				ValueData right_value = stack_values[stack_values.count-1];
				switch(right_value.type){
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
					right_value.v.value->prototype = left_value.getGCValue();
					break;

				case OS_VALUE_TYPE_USERDATA:
				case OS_VALUE_TYPE_USERPTR:
				case OS_VALUE_TYPE_CFUNCTION:
					// TODO: warning???
					break;
				}
				removeStackValue(-2);
				break;
			}

		case Program::OP_CLONE:
			{
				OS_ASSERT(stack_values.count >= 1);
				ValueData value = stack_values[stack_values.count-1];
				pushCloneValue(value);
				removeStackValue(-2);
				break;
			}

		case Program::OP_DELETE_PROP:
			{
				OS_ASSERT(stack_values.count >= 2);
				ValueData table_value = stack_values[stack_values.count-2];
				ValueData index_value = stack_values[stack_values.count-1];
				deleteValueProperty(table_value, index_value, true, true);
				pop(2);
				break;
			}

		case Program::OP_POP:
			pop();
			break;

		case Program::OP_LOGIC_AND:
			{
				OS_ASSERT(stack_values.count >= 1);
				ValueData value = stack_values.lastElement();
				int offs = opcodes.readInt32();
				if(!valueToBool(value)){
					opcodes.movePos(offs);
				}else{
					pop();
				}
				break;
			}

		case Program::OP_LOGIC_OR:
			{
				OS_ASSERT(stack_values.count >= 1);
				ValueData value = stack_values.lastElement();
				int offs = opcodes.readInt32();
				if(valueToBool(value)){
					opcodes.movePos(offs);
				}else{
					pop();
				}
				break;
			}

		case Program::OP_TYPE_OF:
			{
				OS_ASSERT(stack_values.count >= 1);
				ValueData value = stack_values[stack_values.count-1];
				pushTypeOf(value);
				removeStackValue(-2);
				break;
			}

		case Program::OP_VALUE_OF:
			{
				OS_ASSERT(stack_values.count >= 1);
				ValueData value = stack_values[stack_values.count-1];
				pushValueOf(value);
				removeStackValue(-2);
				break;
			}

		case Program::OP_NUMBER_OF:
			{
				OS_ASSERT(stack_values.count >= 1);
				ValueData value = stack_values[stack_values.count-1];
				pushNumberOf(value);
				removeStackValue(-2);
				break;
			}

		case Program::OP_STRING_OF:
			{
				OS_ASSERT(stack_values.count >= 1);
				ValueData value = stack_values[stack_values.count-1];
				pushStringOf(value);
				removeStackValue(-2);
				break;
			}

		case Program::OP_ARRAY_OF:
			{
				OS_ASSERT(stack_values.count >= 1);
				ValueData value = stack_values[stack_values.count-1];
				pushArrayOf(value);
				removeStackValue(-2);
				break;
			}

		case Program::OP_OBJECT_OF:
			{
				OS_ASSERT(stack_values.count >= 1);
				ValueData value = stack_values[stack_values.count-1];
				pushObjectOf(value);
				removeStackValue(-2);
				break;
			}

		case Program::OP_USERDATA_OF:
			{
				OS_ASSERT(stack_values.count >= 1);
				ValueData value = stack_values[stack_values.count-1];
				pushUserDataOf(value);
				removeStackValue(-2);
				break;
			}

		case Program::OP_FUNCTION_OF:
			{
				OS_ASSERT(stack_values.count >= 1);
				ValueData value = stack_values[stack_values.count-1];
				pushFunctionOf(value);
				removeStackValue(-2);
				break;
			}

		case Program::OP_LOGIC_BOOL:
			{
				OS_ASSERT(stack_values.count >= 1);
				ValueData value = stack_values[stack_values.count-1];
				stack_values[stack_values.count-1] = ValueData(valueToBool(value));
				break;
			}

		case Program::OP_LOGIC_NOT:
			{
				OS_ASSERT(stack_values.count >= 1);
				ValueData value = stack_values[stack_values.count-1];
				stack_values[stack_values.count-1] = ValueData(!valueToBool(value));
				break;
			}

		case Program::OP_LENGTH:
			{
				OS_ASSERT(stack_values.count >= 1);
				ValueData value = stack_values[stack_values.count-1];
				bool prototype_enabled = true;
				ValueData func;
				if(getPropertyValue(func, value, 
					PropertyIndex(strings->__len, PropertyIndex::KeepStringIndex()), prototype_enabled)
					&& func.isFunction())
				{
					pushValueData(func);
					pushValueData(value);
					call(0, 1);
				}else{
					pushNull();
				}
				removeStackValue(-2);
				break;
			}

		case Program::OP_BIT_NOT:
		case Program::OP_PLUS:
		case Program::OP_NEG:
			{
				OS_ASSERT(stack_values.count >= 1);
				ValueData value = stack_values.lastElement();
				pushOpResultValue(opcode, value);
				removeStackValue(-2);
				break;
			}

		case Program::OP_CONCAT:
		case Program::OP_LOGIC_PTR_EQ:
		case Program::OP_LOGIC_PTR_NE:
		case Program::OP_LOGIC_EQ:
		case Program::OP_LOGIC_NE:
		case Program::OP_LOGIC_GE:
		case Program::OP_LOGIC_LE:
		case Program::OP_LOGIC_GREATER:
		case Program::OP_LOGIC_LESS:
		case Program::OP_BIT_AND:
		case Program::OP_BIT_OR:
		case Program::OP_BIT_XOR:
		case Program::OP_ADD: // +
		case Program::OP_SUB: // -
		case Program::OP_MUL: // *
		case Program::OP_DIV: // /
		case Program::OP_MOD: // %
		case Program::OP_LSHIFT: // <<
		case Program::OP_RSHIFT: // >>
		case Program::OP_POW: // **
			{
				OS_ASSERT(stack_values.count >= 2);
				ValueData left_value = stack_values[stack_values.count-2];
				ValueData right_value = stack_values[stack_values.count-1];
				pushOpResultValue(opcode, left_value, right_value);
				removeStackValues(-3, 2);
				break;
			}
		}
	}

	return 0;
}

void OS::runOp(OS_EOpcode opcode)
{
	struct Lib
	{
		Core * core;

		void runBinaryOpcode(int opcode)
		{
			int count = core->stack_values.count;
			if(count < 2){
				core->pushNull();
				return;
			}
			Core::ValueData left_value = core->stack_values[count-2];
			Core::ValueData right_value = core->stack_values[count-1];
			core->pushOpResultValue(opcode, left_value, right_value);
			core->removeStackValues(-3, 2);
		}
	
		void runUnaryOpcode(int opcode)
		{
			int count = core->stack_values.count;
			if(count < 1){
				core->pushNull();
				return;
			}
			Core::ValueData value = core->stack_values[count-1];
			core->pushOpResultValue(opcode, value);
			core->removeStackValue(-2);
		}
	
	} lib = {core};
	switch(opcode){
	case OP_LOGIC_PTR_EQ:	// ===
		return lib.runBinaryOpcode(Core::Program::OP_LOGIC_PTR_EQ);

	case OP_LOGIC_PTR_NE:	// !==
		return lib.runBinaryOpcode(Core::Program::OP_LOGIC_PTR_NE);

	case OP_LOGIC_EQ:		// ==
		return lib.runBinaryOpcode(Core::Program::OP_LOGIC_EQ);

	case OP_LOGIC_NE:		// !=
		return lib.runBinaryOpcode(Core::Program::OP_LOGIC_NE);

	case OP_LOGIC_GE:		// >=
		return lib.runBinaryOpcode(Core::Program::OP_LOGIC_GE);

	case OP_LOGIC_LE:		// <=
		return lib.runBinaryOpcode(Core::Program::OP_LOGIC_LE);

	case OP_LOGIC_GREATER:	// >
		return lib.runBinaryOpcode(Core::Program::OP_LOGIC_GREATER);

	case OP_LOGIC_LESS:		// <
		return lib.runBinaryOpcode(Core::Program::OP_LOGIC_LESS);

	case OP_BIT_AND:	// &
		return lib.runBinaryOpcode(Core::Program::OP_BIT_AND);

	case OP_BIT_OR:	// |
		return lib.runBinaryOpcode(Core::Program::OP_BIT_OR);

	case OP_BIT_XOR:	// ^
		return lib.runBinaryOpcode(Core::Program::OP_BIT_XOR);

	case OP_ADD: // +
		return lib.runBinaryOpcode(Core::Program::OP_ADD);

	case OP_SUB: // -
		return lib.runBinaryOpcode(Core::Program::OP_SUB);

	case OP_MUL: // *
		return lib.runBinaryOpcode(Core::Program::OP_MUL);

	case OP_DIV: // /
		return lib.runBinaryOpcode(Core::Program::OP_DIV);

	case OP_MOD: // %
		return lib.runBinaryOpcode(Core::Program::OP_MOD);

	case OP_LSHIFT: // <<
		return lib.runBinaryOpcode(Core::Program::OP_LSHIFT);

	case OP_RSHIFT: // >>
		return lib.runBinaryOpcode(Core::Program::OP_RSHIFT);

	case OP_POW: // **
		return lib.runBinaryOpcode(Core::Program::OP_POW);

	case OP_CONCAT: // ..
		return lib.runBinaryOpcode(Core::Program::OP_CONCAT);

	case OP_BIT_NOT:		// ~
		return lib.runUnaryOpcode(Core::Program::OP_BIT_NOT);

	case OP_PLUS:		// +
		return lib.runUnaryOpcode(Core::Program::OP_PLUS);

	case OP_NEG:			// -
		return lib.runUnaryOpcode(Core::Program::OP_NEG);

	case OP_LENGTH:		// #
		return lib.runUnaryOpcode(Core::Program::OP_LENGTH);

	/*
	case OP_LOGIC_BOOL:
		return lib.runUnaryOpcode(Core::Program::OP_LOGIC_BOOL);

	case OP_LOGIC_NOT:
		return lib.runUnaryOpcode(Core::Program::OP_LOGIC_NOT);

	case OP_VALUE_OF:
		return lib.runUnaryOpcode(Core::Program::OP_VALUE_OF);

	case OP_NUMBER_OF:
		return lib.runUnaryOpcode(Core::Program::OP_NUMBER_OF);

	case OP_STRING_OF:
		return lib.runUnaryOpcode(Core::Program::OP_STRING_OF);

	case OP_ARRAY_OF:
		return lib.runUnaryOpcode(Core::Program::OP_ARRAY_OF);

	case OP_OBJECT_OF:
		return lib.runUnaryOpcode(Core::Program::OP_OBJECT_OF);

	case OP_USERDATA_OF:
		return lib.runUnaryOpcode(Core::Program::OP_USERDATA_OF);

	case OP_FUNCTION_OF:
		return lib.runUnaryOpcode(Core::Program::OP_FUNCTION_OF);

	case OP_CLONE:
		return lib.runUnaryOpcode(Core::Program::OP_CLONE);
	*/
	}
	pushNull();
}

int OS::getLen(int offs)
{
	pushStackValue(offs);
	runOp(OP_LENGTH);
	int len = toInt();
	pop();
	return len;
}

void OS::setFuncs(const Func * list, int upvalues, void * user_param)
{
	for(; list->func; list++){
		pushStackValue(-1);
		pushString(list->name);
		// push upvalues for cfunction
		for(int i = 0; i < upvalues; i++){
			pushStackValue(-1-upvalues);
		}
		pushCFunction(list->func, upvalues, user_param);
		setProperty(false, false);
	}
}

void OS::getObject(const OS_CHAR * name, bool prototype_enabled, bool setter_enabled)
{
	pushStackValue(-1); // 2: copy parent object
	pushString(name);	// 3: index
	getProperty(prototype_enabled, setter_enabled); // 2: value
	if(isObject()){
		remove(-2);		// 1: remove parent object
		return;
	}
	pop();				// 1: parent object
	newObject();		// 2: result object
	pushStackValue(-2);	// 3: copy parent object
	pushString(name);	// 4: index
	pushStackValue(-3);	// 5: copy result object
	setProperty(prototype_enabled, setter_enabled); // 2: parent + result
	remove(-2);			// 1: remove parent object
}

void OS::getGlobalObject(const OS_CHAR * name, bool prototype_enabled, bool setter_enabled)
{
	pushGlobals();
	getObject(name, prototype_enabled, setter_enabled);
}

void OS::getGlobal(const OS_CHAR * name, bool prototype_enabled, bool getter_enabled)
{
	getGlobal(Core::String(this, name), prototype_enabled, getter_enabled);
}

void OS::getGlobal(const Core::String& name, bool prototype_enabled, bool getter_enabled)
{
	pushGlobals();
	pushString(name);
	getProperty(prototype_enabled, getter_enabled);
}

void OS::setGlobal(const OS_CHAR * name, bool prototype_enabled, bool setter_enabled)
{
	setGlobal(Core::String(this, name), prototype_enabled, setter_enabled);
}

void OS::setGlobal(const Core::String& name, bool prototype_enabled, bool setter_enabled)
{
	if(core->stack_values.count >= 1){
		Core::ValueData object = core->global_vars;
		Core::ValueData value = core->stack_values[core->stack_values.count - 1];
		Core::ValueData index = core->pushStringValue(name);
		core->setPropertyValue(object, Core::PropertyIndex(index), value, prototype_enabled, setter_enabled);
		pop(2);
	}
}

void OS::initGlobalFunctions()
{
	struct Lib
	{
		static int print(OS * os, int params, int upvalues, int, void*)
		{
			int params_offs = os->getAbsoluteOffs(-params-upvalues);
			for(int i = 0; i < params; i++){
				String str = os->toString(params_offs + i);
				printf("%s", str.toChar());
				if(i+1 < params){
					printf("\t");
				}
			}
			if(params > 0){
				printf("\n");
			}
			return 0;
		}

		static int echo(OS * os, int params, int upvalues, int, void*)
		{
			int params_offs = os->getAbsoluteOffs(-params-upvalues);
			for(int i = 0; i < params; i++){
				String str = os->toString(params_offs + i);
				printf("%s", str.toChar());
			}
			return 0;
		}

		static int concat(OS * os, int params, int upvalues, int, void*)
		{
			if(params < 1){
				return 0;
			}
			int params_offs = os->getAbsoluteOffs(-params-upvalues);
			String str = os->toString(params_offs);
			for(int i = 1; i < params; i++){
				str += os->toString(params_offs + i);
			}
			os->pushString(str);
			return 1;
		}

		static int minmax(OS * os, int params, int upvalues, OS_EOpcode opcode)
		{
			OS_ASSERT(params >= 0);
			if(params <= 1){
				return params;
			}
			int params_offs = os->getAbsoluteOffs(-params-upvalues);
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

		static int min(OS * os, int params, int upvalues, int, void*)
		{
			return minmax(os, params, upvalues, OP_LOGIC_LE);
		}

		static int max(OS * os, int params, int upvalues, int, void*)
		{
			return minmax(os, params, upvalues, OP_LOGIC_GE);
		}
	};
	Func list[] = {
		{OS_TEXT("print"), Lib::print},
		{OS_TEXT("echo"), Lib::echo},
		{OS_TEXT("concat"), Lib::concat},
		{OS_TEXT("min"), Lib::min},
		{OS_TEXT("max"), Lib::max},
		{}
	};
	pushGlobals();
	setFuncs(list);
	pop();
}

void OS::initObjectClass()
{
	static int iterator_crc = (int)&iterator_crc;

	struct Object
	{
		static int rawget(OS * os, int params, int upvalues, int, void*)
		{
			if(params == 1 && !upvalues){
				os->getProperty(false, false);
				return 1;
			}
			return 0;
		}

		static int rawset(OS * os, int params, int upvalues, int, void*)
		{
			if(params == 2 && !upvalues){
				os->setProperty(false, false);
				return 0;
			}
			return 0;
		}

		static int iteratorStep(OS * os, int params, int upvalues, int, void*)
		{
			OS_ASSERT(upvalues == 2);
			Core::ValueData self_var = os->core->getStackValue(-upvalues + 0);
			void * p = os->toUserData(-upvalues + 1, iterator_crc);
			Core::Table::IteratorState * iter = (Core::Table::IteratorState*)p;
			if(iter->table){
				Core::GCValue * self = self_var.getGCValue();
				OS_ASSERT(self && iter->table == self->table);
				if(iter->prop){
					os->pushBool(true);
					os->core->pushValueData(iter->prop->index);
					os->core->pushValueData(iter->prop->value);
					iter->prop = iter->prop->next;
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

		static int iterator(OS * os, int params, int upvalues, int, void*)
		{
			Core::ValueData self_var = os->core->getStackValue(-params-upvalues-1);
			Core::GCValue * self = self_var.getGCValue();
			if(self && self->table && self->table->count > 0){
				typedef Core::Table::IteratorState IteratorState;
				
				os->core->pushValue(self);

				IteratorState * iter = (IteratorState*)os->pushUserData(iterator_crc, sizeof(IteratorState), iteratorStateDestructor);
				iter->table = NULL;
				iter->next = NULL;
				iter->prop = NULL;
				self->table->addIterator(iter);

				os->pushCFunction(iteratorStep, 2);
				return 1;
			}
			return 0;
		}

		static int length(OS * os, int params, int upvalues, int, void*)
		{
			Core::ValueData self_var = os->core->getStackValue(-params-upvalues-1);
			Core::GCValue * self = self_var.getGCValue();
			if(self){
				os->pushNumber(self->table ? self->table->count : 0);
				return 1;
			}
			return 0;
		}

		static int valueof(OS * os, int params, int upvalues, int, void*)
		{
			Core::ValueData self_var = os->core->getStackValue(-params-upvalues-1);
			switch(self_var.type){
			case OS_VALUE_TYPE_NULL:
				os->pushString(os->core->strings->typeof_null);
				return 1;

			case OS_VALUE_TYPE_BOOL:
				os->pushString(self_var.v.boolean ? os->core->strings->syntax_true : os->core->strings->syntax_false);
				return 1;

			case OS_VALUE_TYPE_NUMBER:
			case OS_VALUE_TYPE_STRING:
				os->core->pushValueData(self_var);
				return 1;
			}
			Core::GCValue * self = self_var.getGCValue();
			if(!self){
				return 0;
			}
			switch(self->type){
			case OS_VALUE_TYPE_USERDATA:
			case OS_VALUE_TYPE_USERPTR:
				{
					Core::StringBuffer str(os);
					str.append(OS_TEXT("["));
					str += os->core->strings->typeof_userdata;
					str.append(OS_TEXT(":"));
					str += Core::String(os, (OS_INT)self->value_id);
					str.append(OS_TEXT("]"));
					os->pushString(str);
					return 1;
				}

			case OS_VALUE_TYPE_FUNCTION:
			case OS_VALUE_TYPE_CFUNCTION:
				{
					Core::StringBuffer str(os);
					str.append(OS_TEXT("["));
					str += os->core->strings->typeof_function;
					str.append(OS_TEXT(":"));
					str += Core::String(os, (OS_INT)self->value_id);
					str.append(OS_TEXT("]"));
					os->pushString(str);
					return 1;
				}
			case OS_VALUE_TYPE_ARRAY:
				if(!self->table || !self->table->count){
					os->pushString(OS_TEXT("[]"));
					return 1;
				}
				{
					Core::StringBuffer buf(os);
					buf.append(OS_TEXT("["));
					bool is_generic_array = true;
					int need_index = 0;
					Core::Property * prop = self->table->first;
					for(int i = 0; prop; prop = prop->next, i++, need_index++){
						if(prop->index.type == OS_VALUE_TYPE_NUMBER && prop->index.v.number == (OS_FLOAT)need_index){
							if(i > 100){
								buf.append(OS_TEXT("..."));
								break;
							}
							if(need_index > 0){
								buf.append(OS_TEXT(","));
							}
							buf += os->core->valueToString(prop->value, true);
						}else{
							is_generic_array = false;
							break;
						}
					}
					if(is_generic_array){
						os->pushString(buf.append(OS_TEXT("]")));
						return 1;
					}
					// no break
				}

			case OS_VALUE_TYPE_OBJECT:
				if(!self->table || !self->table->count){
					os->pushString(OS_TEXT("{}"));
					return 1;
				}
				{
					Core::StringBuffer buf(os);
					buf.append(OS_TEXT("{"));
					int need_index = 0;
					Core::Property * prop = self->table->first;
					for(int i = 0; prop; prop = prop->next, i++){
						if(i > 100){
							buf.append(OS_TEXT("..."));
							break;
						}
						if(i > 0){
							buf.append(OS_TEXT(","));
						}
						if(prop->index.type == OS_VALUE_TYPE_NUMBER){
							if(prop->index.v.number != (OS_FLOAT)need_index){
								buf += String(os, prop->index.v.number, OS_AUTO_PRECISION);
								buf.append(OS_TEXT(":"));
							}
							need_index = (int)(prop->index.v.number + 1);
						}else{ // if(prop->is_string_index){
							buf += os->core->valueToString(prop->index);
							buf.append(OS_TEXT(":"));
						}
						buf += os->core->valueToString(prop->value, true);
					}
					os->pushString(buf.append(OS_TEXT("}")));
					return 1;
				}
			}
			return 0;
		}
	};
	Func list[] = {
		{OS_TEXT("rawget"), Object::rawget},
		{OS_TEXT("rawset"), Object::rawset},
		{core->strings->__len, Object::length},
		// {OS_TEXT("__get@length"), Object::length},
		{core->strings->__iter, Object::iterator},
		{core->strings->__valueof, Object::valueof},
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
		static int length(OS * os, int params, int upvalues, int, void*)
		{
			Core::ValueData self_var = os->core->getStackValue(-params-upvalues-1);
			Core::GCValue * self = self_var.getGCValue();
			if(self){
				os->pushNumber(self->table ? (OS_FLOAT)self->table->next_id : 0);
				return 1;
			}
			return 0;
		}
	};
	Func list[] = {
		{core->strings->__len, Array::length},
		// {OS_TEXT("__get@length"), Array::length},
		{}
	};
	core->pushValue(core->prototypes[Core::PROTOTYPE_ARRAY]);
	setFuncs(list);
	pop();
}

void OS::initStringClass()
{
	struct String
	{
		static int length(OS * os, int params, int upvalues, int, void*)
		{
			Core::ValueData self_var = os->core->getStackValue(-params-upvalues-1);
			Core::GCValue * self = self_var.getGCValue();
			if(self){
				if(self->type == OS_VALUE_TYPE_STRING){
					Core::GCStringValue * string = (Core::GCStringValue*)self;
					os->pushNumber(string->getLen());
					// os->pushNumber(os->core->valueToString(self).getDataSize() / sizeof(OS_CHAR));
					return 1;
				}
				os->core->pushOpResultValue(Core::Program::OP_LENGTH, self_var);
				return 1;
			}
			return 0;
		}
	};
	Func list[] = {
		{core->strings->__len, String::length},
		// {OS_TEXT("__get@length"), String::length},
		{}
	};
	core->pushValue(core->prototypes[Core::PROTOTYPE_STRING]);
	setFuncs(list);
	pop();
}

void OS::initFunctionClass()
{
	struct Function
	{
		static int apply(OS * os, int params, int upvalues, int need_ret_values, void*)
		{
			int offs = os->getAbsoluteOffs(-params-upvalues);
			os->pushStackValue(offs-1); // self as func
			if(params < 1){
				os->pushNull();
				return os->call(0, need_ret_values);
			}
			os->pushStackValue(offs); // first param - new this
			
			Core::ValueData array_var = os->core->getStackValue(offs+1);
			Core::GCValue * array_value = array_var.getGCValue();
			if(array_value && array_value->table){
				Core::Property * prop = array_value->table->first;
				for(; prop; prop = prop->next){
					os->core->pushValueData(prop->value);	
				}
				return os->call(array_value->table->count, need_ret_values);
			}
			return os->call(0, need_ret_values);
		}
		
		static int call(OS * os, int params, int upvalues, int need_ret_values, void*)
		{
			int offs = os->getAbsoluteOffs(-params-upvalues);
			os->pushStackValue(offs-1); // self as func
			if(params < 1){
				os->pushNull(); // this
				return os->call(0, need_ret_values);
			}
			os->pushStackValue(offs); // first param - new this
			for(int i = 1; i < params; i++){
				os->pushStackValue(offs + i);
			}
			return os->call(params-1, need_ret_values);
		}

		static int iterator(OS * os, int params, int upvalues, int need_ret_values, void*)
		{
			os->pushStackValue(-params-upvalues-1); // self as func
			return 1;
		}
	};
	Func list[] = {
		{OS_TEXT("apply"), Function::apply},
		{OS_TEXT("call"), Function::call},
		{core->strings->__iter, Function::iterator},
		{}
	};
	core->pushValue(core->prototypes[Core::PROTOTYPE_FUNCTION]);
	setFuncs(list);
	pop();
}

void OS::initMathLibrary()
{
	struct Lib {
	};
	Func list[] = {
		{}
	};
	getGlobalObject(OS_TEXT("Math"));
	setFuncs(list);
	pop();
}

void OS::initScript()
{
	const OS_CHAR * code = 
		OS_TEXT("Object.__get@length = function(){ return #this }")
		;
	eval(code);
}

void OS::Core::syncStackRetValues(int need_ret_values, int cur_ret_values)
{
	if(cur_ret_values > need_ret_values){
		pop(cur_ret_values - need_ret_values);
	}else{ 
		for(; cur_ret_values < need_ret_values; cur_ret_values++){
			pushNull();
		}
	}
}

int OS::Core::call(int params, int ret_values)
{
	if(stack_values.count >= 2+params){
		int end_stack_size = stack_values.count-2-params;
		ValueData func_value = stack_values[stack_values.count-2-params];
		switch(func_value.type){
		case OS_VALUE_TYPE_FUNCTION:
			{
				ValueData self = stack_values[stack_values.count-1-params];
				enterFunction(func_value.v.func, self.getGCValue(), params, 2, ret_values);
				ret_values = execute();
				OS_ASSERT(stack_values.count == end_stack_size + ret_values);
				return ret_values;
			}

		case OS_VALUE_TYPE_CFUNCTION:
			{
				int stack_size_without_params = getStackOffs(-2-params);
				GCCFunctionValue * cfunc_upvalues = func_value.v.cfunc;
				ValueData * upvalues = (ValueData*)(cfunc_upvalues + 1);
				for(int i = 0; i < cfunc_upvalues->num_upvalues; i++){
					pushValueData(upvalues[i]);
				}
				int func_ret_values = cfunc_upvalues->func(allocator, params, cfunc_upvalues->num_upvalues, ret_values, cfunc_upvalues->user_param);
				int remove_values = getStackOffs(-func_ret_values) - stack_size_without_params;
				OS_ASSERT(remove_values >= 0);
				removeStackValues(stack_size_without_params, remove_values);
				syncStackRetValues(ret_values, func_ret_values);
				OS_ASSERT(stack_values.count == end_stack_size + ret_values);
				return ret_values;
			}

		case OS_VALUE_TYPE_OBJECT:
			{
				ValueData self_var = stack_values[stack_values.count-1-params];
				OS_ASSERT(self_var.type == OS_VALUE_TYPE_NULL || self_var.getGCValue() == func_value.getGCValue());
				bool prototype_enabled = true;
				ValueData func;
				if(getPropertyValue(func, func_value, PropertyIndex(strings->__constructor, PropertyIndex::KeepStringIndex()), prototype_enabled)
					&& func.isFunction())
				{
					// TODO: not tested
					GCValue * self = self_var.getGCValue();
					GCValue * object = func_value.v.value != self ? newObjectValue(func_value.v.value) : self;
					pushValue(object);
					pushValueData(func);
					pushValue(object);
					moveStackValues(-3, 3, -3-params);
					ret_values = call(params, 0); // ignore result
					syncStackRetValues(ret_values, 1); // object is already located inside of stack, use it as returned value
					// removeStackValues(offs);
					return true;
				}
				break;
			}
		}
	}
	// OS_ASSERT(false);
	pop(params + 2);
	syncStackRetValues(ret_values, 0);
	return false;
}

bool OS::compileFilename(const Core::String& filename, bool required)
{
	{
		Core::String compiled_filename = changeFilenameExt(filename, OS_COMPILED_EXT);
		Core::FileStreamReader file(this, compiled_filename);
		if(file.f){
			// TODO: check file time with original and load compiled file or the original
		}
	}

	Core::FileStreamReader file(this, filename);
	if(!file.f){
		if(required){
			core->error(OS_ERROR, String::format(this, OS_TEXT("required filename %s is not exist"), filename.toChar()));
			return false;
		}
	}


	Core::MemStreamWriter file_data(this);
	file_data.readFromStream(&file);

	Core::Tokenizer tokenizer(this);
	tokenizer.parseText((OS_CHAR*)file_data.buffer.buf, file_data.buffer.count, filename);

	Core::Compiler compiler(&tokenizer);
	return compiler.compile();
}

bool OS::compile(const Core::String& str)
{
	Core::Tokenizer tokenizer(this);
	tokenizer.parseText(str.toChar(), str.getLen(), String(this));

	Core::Compiler compiler(&tokenizer);
	return compiler.compile();
}

bool OS::compile()
{
	Core::String str(this);
	Core::ValueData val = core->getStackValue(-1);
	if(core->isValueString(val, &str)){
		pop(1);
		return compile(str);
	}
	pop(1);
	pushNull();
	return false;
}

int OS::call(int params, int ret_values)
{
	return core->call(params, ret_values);
}

int OS::eval(const OS_CHAR * str, int params, int ret_values)
{
	return eval(Core::String(this, str), params, ret_values);
}

int OS::eval(const Core::String& str, int params, int ret_values)
{
	pushString(str);
	compile();
	pushNull();
	move(-2, 2, -2-params);
	return core->call(params, ret_values);
}

int OS::run(const OS_CHAR * filename, bool required, int params, int ret_values)
{
	return run(Core::String(this, filename), required, params, ret_values);
}

int OS::run(const Core::String& filename, bool required, int params, int ret_values)
{
	compileFilename(filename, required);
	pushNull();
	move(-2, 2, -2-params);
	int ret = core->call(params, ret_values);

	// core->sortOpcodeUsage();

	return ret;
}

/*
void OS::Core::sortOpcodeUsage()
{
	struct Lib {
		static int __cdecl compare(const void * a, const void * b)
		{
			OpcodeTime * t1 = (OpcodeTime*)a;
			OpcodeTime * t2 = (OpcodeTime*)b;
			if(t1->time < t2->time){
				return 1;
			}
			if(t1->time > t2->time){
				return -1;
			}
			return 0;
		}
	};
	qsort(opcode_usage, Program::OPCODE_COUNT, sizeof(opcode_usage[0]), Lib::compare); 
}
*/

int OS::getSetting(OS_ESettings setting)
{
	switch(setting){
	case OS_SETTING_CREATE_DEBUG_INFO:
		return core->settings.create_debug_info;

	case OS_SETTING_CREATE_DEBUG_OPCODES:
		return core->settings.create_debug_opcodes;

	case OS_SETTING_RECOMPILE_SOURCECODE:
		return core->settings.recompile_sourcecode;
	}
	return -1;
}

int OS::setSetting(OS_ESettings setting, int value)
{
	int old_val;
	switch(setting){
	case OS_SETTING_CREATE_DEBUG_INFO:
		old_val = core->settings.create_debug_info;
		core->settings.create_debug_info = value ? true : false;
		return old_val;

	case OS_SETTING_CREATE_DEBUG_OPCODES:
		old_val = core->settings.create_debug_opcodes;
		core->settings.create_debug_opcodes = value ? true : false;
		return old_val;

	case OS_SETTING_RECOMPILE_SOURCECODE:
		old_val = core->settings.recompile_sourcecode;
		core->settings.recompile_sourcecode = value ? true : false;
		return old_val;
	}
	return -1;
}

int OS::gc()
{
	return core->gcStep();
}

void OS::gcFull()
{
	return;
	core->gcFull();
}

// =====================================================================
// =====================================================================
// =====================================================================

/*
OS::Value::Value(OS * p_allocator, Core::Value * p_value)
{
	allocator = p_allocator->retain();
	value = p_value ? p_value->retain() : NULL;
}

OS::Value::Value(const Value& p_value)
{
	allocator = p_value.allocator->retain();
	value = p_value.value ? p_value.value->retain() : NULL;
}

OS::Value::~Value()
{
	if(value){
		allocator->core->releaseValue(value);
	}
	allocator->release();
}

OS::Value& OS::Value::operator=(const Value& p_value)
{
	if(value != p_value.value){
		if(value){
			allocator->core->releaseValue(value);
		}
		value = p_value.value ? p_value.value->retain() : NULL;
		if(allocator != p_value.allocator){
			allocator->release();
			allocator = p_value.allocator->retain();
		}
	}
	return *this;
}

int OS::Value::getId() const
{
	return value ? value->value_id : 0;
}
*/