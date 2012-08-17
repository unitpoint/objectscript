#include "objectscript.h"

using namespace ObjectScript;

// =====================================================================
// =====================================================================
// =====================================================================

static int __snprintf__(char * buf, size_t num, const char * format, ...)
{
	va_list va;
	va_start(va, format);
	int ret = OS_VSNPRINTF(buf, num, format, va);
	va_end(va);
	return ret;
}

static inline void parseSpaces(const OS_CHAR *& str)
{
	while(*str && OS_ISSPACE(*str))
		str++;
}

static bool parseHexSimple(const OS_CHAR *& str, OS_INT& val)
{
	OS_INT int_val = 0;
	const OS_CHAR * start = str;
	for(;; str++)
	{
		if(*str >= OS_TEXT('0') && *str <= OS_TEXT('9'))
		{
			int_val = (int_val << 4) + (OS_INT)(*str - OS_TEXT('0'));
			continue;
		}
		if(*str >= OS_TEXT('a') && *str <= OS_TEXT('f'))
		{
			int_val = (int_val << 4) + 10 + (OS_INT)(*str - OS_TEXT('a'));
			continue;
		}
		if(*str >= OS_TEXT('A') && *str <= OS_TEXT('F'))
		{
			int_val = (int_val << 4) + 10 + (OS_INT)(*str - OS_TEXT('A'));
			continue;
		}
		// end parse
		break;
	}
	val = int_val;
	return str > start;
}

static bool parseBinSimple(const OS_CHAR *& str, OS_INT& val)
{
	OS_INT int_val = 0;
	const OS_CHAR * start = str;
	for(; *str >= OS_TEXT('0') && *str <= OS_TEXT('1'); str++)
	{
		int_val = (int_val << 1) + (OS_INT)(*str - OS_TEXT('0'));
	}
	val = int_val;
	return str > start;
}

static bool parseOctalSimple(const OS_CHAR *& str, OS_INT& val)
{
	OS_INT int_val = 0;
	const OS_CHAR * start = str;
	for(; *str >= OS_TEXT('0') && *str <= OS_TEXT('7'); str++)
	{
		int_val = (int_val << 3) + (OS_INT)(*str - OS_TEXT('0'));
	}
	val = int_val;
	return str > start;
}

static bool parseDecSimple(const OS_CHAR *& str, OS_INT& val)
{
	OS_INT int_val = 0;
	const OS_CHAR * start = str;
	for(; *str >= OS_TEXT('0') && *str <= OS_TEXT('9'); str++)
	{
		int_val = int_val * 10 + (OS_INT)(*str - OS_TEXT('0'));
	}
	val = int_val;
	return str > start;
}

OS::EParseNumType OS::Utils::parseNum(const OS_CHAR *& str, OS_FLOAT& fval, OS_INT& ival, int flags)
{
	const OS_CHAR * saveSrc = str;
	int sign = 1;
	if(*str == OS_TEXT('-')){
		str++;
		saveSrc++;
		sign = -1;
	}else if(*str == OS_TEXT('+')){
		str++;
		saveSrc++;
	}

	// bool escapeMode = false;
	OS_INT int_val = 0;
	if((flags & PARSE_NUM_FLAG_INT_EXPANDED) && str[0] == OS_TEXT('0') && str[1] != OS_TEXT('.'))
	{
		bool octalNum = false;
		if(str[1] == OS_TEXT('x') || str[1] == OS_TEXT('X')) // parse hex
		{
			str += 2;
			parseHexSimple(str, int_val);
		}
		else if(str[1] == OS_TEXT('b') || str[1] == OS_TEXT('B')) // parse hex
		{
			str += 2;
			parseBinSimple(str, int_val);
		}
		else // parse octal
		{
			octalNum = true;
			parseOctalSimple(str, int_val);
		}
		if(saveSrc+1 == str && !octalNum)
		{
			fval = 0;
			ival = 0;
			return PARSE_NUM_TYPE_ERROR;
		}
	}
	else
	{
		// parse int or float
		parseDecSimple(str, int_val);

		if((flags & PARSE_NUM_FLAG_FLOAT) && *str == OS_TEXT('.')) // parse float
		{
			// parse 1.#INF ...
			if(sign == 1 && saveSrc+1 == str && *saveSrc == OS_TEXT('1') && str[1] == OS_TEXT('#'))
			{
				const OS_CHAR * spec[] = {OS_TEXT("INF"), OS_TEXT("IND"), OS_TEXT("QNAN"), NULL};
				int i = 0;
				for(; spec[i]; i++)
				{
					if(OS_STRCMP(str, spec[i]) != 0)
						continue;

					size_t specLen = OS_STRLEN(spec[i]);
					str += specLen;
					if(!*str || OS_ISSPACE(*str) || OS_STRCHR(OS_TEXT("!@#$%^&*()-+={}[]\\|;:'\",<.>/?`~"), *str))
					{
						OS_INT32 spec_val;
						switch(i)
						{
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
						fval = (OS_FLOAT)*(float*)&spec_val;
						ival = 0;
						/* if(parseEndSpaces)
						{
						parseSpaces(str);
						} */
						return PARSE_NUM_TYPE_FLOAT;
					}            
				}
				fval = 0;
				ival = 0;
				return PARSE_NUM_TYPE_ERROR;
			}

			OS_FLOAT float_val = (OS_FLOAT)int_val, m = 0.1;
			OS_ASSERT((OS_INT)float_val == int_val);
			for(str++; *str >= OS_TEXT('0') && *str <= OS_TEXT('9'); str++, m *= 0.1)
			{
				float_val += (OS_FLOAT)(*str - OS_TEXT('0')) * m;
			}
			if(saveSrc == str)
			{
				fval = 0;
				ival = 0;
				return PARSE_NUM_TYPE_ERROR;
			}
			if(*str == OS_TEXT('e') || *str == OS_TEXT('E'))
			{
				str++;
				bool div = false; // + for default
				if(*str == OS_TEXT('-'))
				{
					div = true;
					str++;
				}
				else if(*str == OS_TEXT('+'))
				{
					// div = false;
					str++;
				}
				OS_INT pow;
				parseDecSimple(str, pow);
				m = 1.0;
				for(int i = 0; i < pow; i++)
				{
					m *= 10.0;
				}
				if(div)
				{
					float_val /= m;
				}
				else
				{
					float_val *= m;
				}
			}
			fval = sign > 0 ? float_val : -float_val;
			ival = 0;
			/* if(parseEndSpaces)
			{
			parseSpaces(str);
			} */
			return PARSE_NUM_TYPE_FLOAT;
		}
		else if(saveSrc == str)
		{
			fval = 0;
			ival = 0;
			return PARSE_NUM_TYPE_ERROR;
		}
	}
	// int_val *= sign;
	fval = 0;
	ival = sign > 0 ? int_val : -int_val;
	// OS_ASSERT((int)val == int_val);
	/* if(parseEndSpaces)
	{
	parseSpaces(str);
	} */
	return PARSE_NUM_TYPE_INT;
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
			OS_FLOAT p = OS_MATH_POW((OS_FLOAT)10.0, (OS_FLOAT)-precision);
			a = OS_MATH_FLOOR(a / p + (OS_FLOAT)0.5) * p;
		}
		OS_SNPRINTF(dst, sizeof(fmt), OS_TEXT("%.f"), a);
		return dst;
	}
	OS_SNPRINTF(fmt, sizeof(fmt), OS_TEXT("%%.%df"), precision);
	int n = OS_SNPRINTF(dst, sizeof(fmt), fmt, a);

	while(n > 0 && dst[n-1] == '0') dst[--n] = (OS_CHAR)0;
	if(n > 0 && dst[n-1] == '.') dst[--n] = (OS_CHAR)0;

	return dst;
}

OS_INT OS::Utils::strToInt(const OS_CHAR * str)
{
	OS_FLOAT fval;
	OS_INT ival;
	switch(parseNum(str, fval, ival, PARSE_NUM_FLAG_INT | PARSE_NUM_FLAG_FLOAT)){
	case PARSE_NUM_TYPE_ERROR:
		return 0;

	case PARSE_NUM_TYPE_FLOAT:
		return (OS_INT)fval;

		// case PARSE_NUM_TYPE_INT:
	}
	return ival;
}

OS_FLOAT OS::Utils::strToFloat(const OS_CHAR* str)
{
	OS_FLOAT fval;
	OS_INT ival;
	switch(parseNum(str, fval, ival, PARSE_NUM_FLAG_INT | PARSE_NUM_FLAG_FLOAT)){
	case PARSE_NUM_TYPE_ERROR:
		return 0;

		// case PARSE_NUM_TYPE_FLOAT:
		//  return (OS_INT)fval;

	case PARSE_NUM_TYPE_INT:
		return (OS_FLOAT)ival;
	}
	return fval;
}

int OS::Utils::keyToHash(const OS_CHAR * str, int size)
{
#define KeyHashAddNum KeyNum = ((KeyNum << 5) + KeyNum) + *str++

	unsigned KeyNum = 5381;
	for(; size >= 8; size -= 8) {
		KeyHashAddNum;
		KeyHashAddNum;
		KeyHashAddNum;
		KeyHashAddNum;
		KeyHashAddNum;
		KeyHashAddNum;
		KeyHashAddNum;
		KeyHashAddNum;
	}
	switch(size) {
	case 7: KeyHashAddNum;
	case 6: KeyHashAddNum;
	case 5: KeyHashAddNum;
	case 4: KeyHashAddNum;
	case 3: KeyHashAddNum;
	case 2: KeyHashAddNum;
	case 1: KeyHashAddNum;
	case 0: break;
	}
	return (int)KeyNum;

#undef KeyHashAddNum
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

// =====================================================================
// =====================================================================
// =====================================================================

OS::Core::StringData * OS::Core::StringData::alloc(OS * allocator, int size, const void * buf, int data_size)
{
	if(size <= 0){
		return allocator->core->empty_string_data->retain();
	}
	if(data_size > size){
		data_size = size;
	}
	int alloc_size = size + sizeof(StringData) + sizeof(wchar_t) + sizeof(wchar_t)/2;
	StringData * data = (StringData*)allocator->malloc(alloc_size);
	OS_ASSERT(data);
	if(!data){
		return allocator->core->empty_string_data->retain();
	}
	data->allocator = allocator;
	data->allocated_bytes = size;
	data->data_size = data_size;
	data->ref_count = 1;
	if(data_size > 0){
		OS_BYTE * dataBuf = (OS_BYTE*)data->toMemory();
		OS_MEMCPY(dataBuf, buf, data_size);
		OS_MEMSET(dataBuf + data_size, 0, size - data_size + sizeof(wchar_t) + sizeof(wchar_t)/2);
	}else{
		OS_MEMSET(data->toMemory(), 0, size + sizeof(wchar_t) + sizeof(wchar_t)/2);
	}
	return data;
}

OS::Core::StringData * OS::Core::StringData::alloc(OS * allocator, int size, const void * buf1, int len1, const void * buf2, int len2)
{
	if(size <= 0){
		return allocator->core->empty_string_data->retain();
	}
	if(len1 > size){
		len1 = size;
	}
	if(len2 > size - len1){
		len2 = size - len1;
	}
	int alloc_size = size + sizeof(StringData) + sizeof(wchar_t) + sizeof(wchar_t)/2;
	StringData * data = (StringData*)allocator->malloc(alloc_size);
	OS_ASSERT(data);
	if(!data){
		return allocator->core->empty_string_data->retain();
	}
	data->allocator = allocator;
	data->allocated_bytes = size;
	data->data_size = len1 + len2;
	data->ref_count = 1;

	OS_BYTE * curBuf = (OS_BYTE*)data->toMemory();
	OS_MEMCPY(curBuf, buf1, len1); curBuf += len1;
	OS_MEMCPY(curBuf, buf2, len2); curBuf += len2;
	OS_MEMSET(curBuf, 0, sizeof(wchar_t) + sizeof(wchar_t)/2);

	return data;
}

OS::Core::StringData * OS::Core::StringData::alloc(OS * allocator, int size, const void * buf1, int len1, const void * buf2, int len2, const void * buf3, int len3)
{
	if(size <= 0){
		return allocator->core->empty_string_data->retain();
	}
	if(len1 > size){
		len1 = size;
	}
	if(len2 > size - len1){
		len2 = size - len1;
	}
	if(len3 > size - len1 - len2){
		len3 = size - len1 - len2;
	}
	int alloc_size = size + sizeof(StringData) + sizeof(wchar_t) + sizeof(wchar_t)/2;
	StringData * data = (StringData*)allocator->malloc(alloc_size);
	OS_ASSERT(data);
	if(!data){
		return allocator->core->empty_string_data->retain();
	}
	data->allocator = allocator;
	data->allocated_bytes = size;
	data->data_size = len1 + len2 + len3;
	data->ref_count = 1;

	OS_BYTE * curBuf = (OS_BYTE*)data->toMemory();
	OS_MEMCPY(curBuf, buf1, len1); curBuf += len1;
	OS_MEMCPY(curBuf, buf2, len2); curBuf += len2;
	OS_MEMCPY(curBuf, buf3, len3); curBuf += len3;
	OS_MEMSET(curBuf, 0, sizeof(wchar_t) + sizeof(wchar_t)/2);

	return data;
}

void OS::Core::StringData::free(StringData * data)
{
	OS_ASSERT(data && data->ref_count == 0);
	OS_ASSERT(data->allocator);
	// delete [] ((OS_BYTE*)data);
	data->allocator->free(data);
}

OS::Core::StringData * OS::Core::StringData::alloc(OS * allocator, const void * buf, int data_size)
{
	return alloc(allocator, data_size, buf, data_size);
}

OS::Core::StringData * OS::Core::StringData::alloc(StringData * b)
{
	return alloc(b->allocator, b->data_size, b->toMemory(), b->data_size);
	// return b->retain();
}

OS::Core::StringData * OS::Core::StringData::alloc(int size, StringData * b)
{
	return alloc(b->allocator, size, b->toMemory(), b->data_size);
}

OS::Core::StringData * OS::Core::StringData::alloc(int size, StringData * b, int data_size)
{
	return alloc(b->allocator, size, b->toMemory(), b->data_size < data_size ? b->data_size : data_size);
}

OS::Core::StringData * OS::Core::StringData::alloc(StringData * b, int data_size)
{
	int size = b->data_size < data_size ? b->data_size : data_size;
	return alloc(b->allocator, size, b->toMemory(), size);
}

OS::Core::StringData * OS::Core::StringData::append(StringData * self, StringData * b)
{
	if(!b->data_size){
		return self;
	}
	if(!self->data_size){
		self->release();
		return b->retain();
	}
	if(self->ref_count > 1){
		self->release();
		return alloc(self->allocator, self->data_size + b->data_size, self->toMemory(), self->data_size, b->toMemory(), b->data_size); 
	}
	int new_data_size = self->data_size + b->data_size;
	if(new_data_size <= self->allocated_bytes){
		OS_MEMCPY((OS_BYTE*)self->toMemory() + self->data_size, b->toMemory(), b->data_size);

		self->data_size = new_data_size;
		OS_MEMSET((OS_BYTE*)self->toMemory() + self->data_size, 0, sizeof(wchar_t) + sizeof(wchar_t)/2);

		return self;
	}
	int new_size = (self->allocated_bytes * 2 + 15) & ~15;
	if(new_size < new_data_size){
		new_size = new_data_size;
	}
	StringData * newData = alloc(self->allocator, new_size, self->toMemory(), self->data_size, b->toMemory(), b->data_size);
	self->release();
	return newData;
}

OS::Core::StringData * OS::Core::StringData::append(StringData * self, const void * buf, int data_size)
{
	if(!data_size){
		return self;
	}
	if(!self->data_size){
		OS * allocator = self->allocator;
		self->release();
		return alloc(allocator, buf, data_size);
	}
	if(self->ref_count > 1){
		self->release();
		return alloc(self->allocator, self->data_size + data_size, self->toMemory(), self->data_size, buf, data_size); 
	}
	int new_data_size = self->data_size + data_size;
	if(new_data_size <= self->allocated_bytes){
		OS_MEMCPY((OS_BYTE*)self->toMemory() + self->data_size, buf, data_size);

		self->data_size = new_data_size;
		OS_MEMSET((OS_BYTE*)self->toMemory() + self->data_size, 0, sizeof(wchar_t) + sizeof(wchar_t)/2);

		return self;
	}
	int new_size = (self->allocated_bytes * 2 + 15) & ~15;
	if(new_size < new_data_size){
		new_size = new_data_size;
	}
	StringData * newData = alloc(self->allocator, new_size, self->toMemory(), self->data_size, buf, data_size);
	self->release();
	return newData;
}

OS::Core::StringData * OS::Core::StringData::retain()
{
	ref_count++;
	return this;
}

void OS::Core::StringData::release()
{
	if(--ref_count <= 0){
		OS_ASSERT(ref_count == 0);
		free(this);
	}
}

/*
OS::Core::StringData * OS::Core::StringData::retainSafely()
{
	if(this){
		retain();
	}
	return this;
}

void OS::Core::StringData::releaseSafely()
{
	if(this){
		release();
	}
}
*/

int OS::Core::StringData::cmp(const StringData * b) const
{
	return Utils::cmp(toMemory(), data_size, b->toMemory(), b->data_size);
}

int OS::Core::StringData::cmp(const void * buf, int bufSize) const
{
	return Utils::cmp(toMemory(), data_size, buf, bufSize);
}

int OS::Core::StringData::hash() const
{
	return Utils::keyToHash(toChar(), data_size);
}

OS_INT OS::Core::StringData::toInt() const
{
	return Utils::strToInt(toChar());
}

OS_FLOAT OS::Core::StringData::toFloat() const
{
	return Utils::strToFloat(toChar());
}

// =====================================================================

OS::Core::StringInternal::StringInternal(OS * allocator)
{
	// allocator->retain();
	str = allocator->core->empty_string_data->retain()->toChar();
}

OS::Core::StringInternal::StringInternal(OS * allocator, const OS_CHAR * s)
{
	// allocator->retain();
	str = StringData::alloc(allocator, s, OS_STRLEN(s))->toChar();
}

OS::Core::StringInternal::StringInternal(OS * allocator, OS_CHAR c, int count)
{
	StringData * data = StringData::alloc(allocator, sizeof(c)*count, NULL, 0);
	str = data->toChar();
	data->data_size = sizeof(c)*count;
	if(c){
		for(int i = 0; i < count; i++){
			((OS_CHAR*)str)[i] = c;
		}
		((OS_CHAR*)str)[count] = 0;
	}
}

OS::Core::StringInternal::StringInternal(OS * allocator, const void * buf, int size)
{
	// allocator->retain();
	str = StringData::alloc(allocator, buf, size)->toChar();
}

OS::Core::StringInternal::StringInternal(OS * allocator, const void * buf1, int len1, const void * buf2, int len2)
{
	// allocator->retain();
	int size = len1 + len2;
	str = StringData::alloc(allocator, size, buf1, len1, buf2, len2)->toChar();
}

OS::Core::StringInternal::StringInternal(const StringInternal& b)
{
	StringData * data = b.toData();
	// data->allocator->retain();
	str = data->retain()->toChar();
}

OS::Core::StringInternal::StringInternal(StringData * b)
{
	// b->allocator->retain();
	str = b->retain()->toChar();
}

OS::Core::StringInternal::StringInternal(OS * allocator, OS_INT value)
{
	// allocator->retain();
	OS_CHAR buf[64];
	OS::Utils::numToStr(buf, value);
	str = StringData::alloc(allocator, buf, OS_STRLEN(buf))->toChar();
}

OS::Core::StringInternal::StringInternal(OS * allocator, OS_FLOAT value, int precision)
{
	// allocator->retain();
	OS_CHAR buf[128];
	OS::Utils::numToStr(buf, value, precision);
	str = StringData::alloc(allocator, buf, OS_STRLEN(buf))->toChar();
}

OS::Core::StringInternal::~StringInternal()
{
	if(str){ // str is cleared by String due to OS release properly
		StringData * data = toData();
		OS * allocator = data->allocator;
		data->release();
		// allocator->release();
	}
}

struct OS_VaListDtor
{
	va_list * va;

	OS_VaListDtor(va_list * p_va)
	{
		va = p_va;
	}
	~OS_VaListDtor()
	{
		va_end(*va);
	}
};

OS::Core::StringInternal OS::Core::StringInternal::format(OS * allocator, int temp_buf_size, const OS_CHAR * fmt, ...)
{
	va_list va;
	va_start(va, fmt);
	OS_VaListDtor va_dtor(&va);
	return StringInternal(allocator).setFormat(temp_buf_size, fmt, va);
}

OS::Core::StringInternal OS::Core::StringInternal::format(OS * allocator, int temp_buf_size, const OS_CHAR * fmt, va_list va)
{
	return StringInternal(allocator).setFormat(temp_buf_size, fmt, va);
}

OS::Core::StringInternal OS::Core::StringInternal::format(OS * allocator, const OS_CHAR * fmt, ...)
{
	va_list va;
	va_start(va, fmt);
	OS_VaListDtor va_dtor(&va);
	return StringInternal(allocator).setFormat(fmt, va);
}

OS::Core::StringInternal OS::Core::StringInternal::format(OS * allocator, const OS_CHAR * fmt, va_list va)
{
	return StringInternal(allocator).setFormat(fmt, va);
}

OS::Core::StringInternal& OS::Core::StringInternal::setFormat(int temp_buf_size, const OS_CHAR * fmt, ...)
{
	va_list va;
	va_start(va, fmt);
	OS_VaListDtor va_dtor(&va);
	return setFormat(temp_buf_size, fmt, va);
}

OS::Core::StringInternal& OS::Core::StringInternal::setFormat(int temp_buf_size, const OS_CHAR * fmt, va_list va)
{
	StringInternal buf(getAllocator(), OS_CHAR(0), temp_buf_size);
	OS_VSNPRINTF((OS_CHAR*)buf.toChar(), sizeof(OS_CHAR)*temp_buf_size, fmt, va);
	return *this = buf.toChar();
}

OS::Core::StringInternal& OS::Core::StringInternal::setFormat(const OS_CHAR * fmt, ...)
{
	va_list va;
	va_start(va, fmt);
	OS_VaListDtor va_dtor(&va);
	return setFormat(OS_DEF_FMT_BUF_SIZE, fmt, va);
}

OS::Core::StringInternal& OS::Core::StringInternal::setFormat(const OS_CHAR * fmt, va_list va)
{
	return setFormat(OS_DEF_FMT_BUF_SIZE, fmt, va);
}

void OS::Core::StringInternal::clear()
{
	StringData * data = toData();
	str = data->allocator->core->empty_string_data->retain()->toChar();
	data->release();
}

OS::Core::StringInternal OS::Core::StringInternal::trim(bool left_trim, bool right_trim) const
{
	const OS_CHAR * start = toChar();
	const OS_CHAR * end = start + getDataSize();
	bool real_sub = false;
	if(left_trim){
		while(OS_ISSPACE(*start)){
			start++;
			real_sub = true;
		}
	}

	if(right_trim){
		while(end > start && OS_ISSPACE(end[-1])){
			end--;
			real_sub = true;
		}
	}

	return real_sub ? StringInternal(getAllocator(), (void*)start, (int)end - (int)start) : *this;
}


OS::Core::StringInternal& OS::Core::StringInternal::operator=(const StringInternal& b)
{
	StringData * old_data = toData();
	str = b.toData()->retain()->toChar();
	old_data->release();
	return *this;
}

OS::Core::StringInternal& OS::Core::StringInternal::operator=(const OS_CHAR * b)
{
	StringData * data = toData();
	OS * allocator = data->allocator;
	str = StringData::alloc(allocator, b, OS_STRLEN(b))->toChar();
	data->release();
	return *this;
}

OS::Core::StringInternal& OS::Core::StringInternal::operator+=(const StringInternal& b)
{
	str = StringData::append(toData(), b.toData())->toChar();
	return *this;
}

OS::Core::StringInternal& OS::Core::StringInternal::operator+=(const OS_CHAR * b)
{
	int len = OS_STRLEN(b);
	str = StringData::append(toData(), b, len)->toChar();
	return *this;
}

OS::Core::StringInternal& OS::Core::StringInternal::append(const void * buf, int size)
{
	str = StringData::append(toData(), buf, size)->toChar();
	return *this;
}

OS::Core::StringInternal& OS::Core::StringInternal::append(const OS_CHAR * b)
{
	return append(b, OS_STRLEN(b));
}

OS::Core::StringInternal OS::Core::StringInternal::operator+(const StringInternal& b) const
{
	StringData * data = toData(), * b_data = b.toData();
	if(data->data_size && b_data->data_size){
		return StringInternal(data->allocator, data->toMemory(), data->data_size, b_data->toMemory(), b_data->data_size);
	}
	if(data->data_size){
		return *this;
	}
	return b;
}

OS::Core::StringInternal OS::Core::StringInternal::operator+(const OS_CHAR * b) const
{
	StringData * data = toData();
	int len = OS_STRLEN(b);
	if(data->data_size && len > 0){
		return StringInternal(data->allocator, data->toMemory(), data->data_size, b, len);
	}
	if(data->data_size){
		return *this;
	}
	return StringInternal(data->allocator, b, len);
}

bool OS::Core::StringInternal::operator==(const StringInternal& b) const
{
	return cmp(b) == 0;
}

bool OS::Core::StringInternal::operator==(const OS_CHAR * b) const
{
	return cmp(b) == 0;
}

bool OS::Core::StringInternal::operator!=(const StringInternal& b) const
{
	return cmp(b) != 0;
}

bool OS::Core::StringInternal::operator!=(const OS_CHAR * b) const
{
	return cmp(b) != 0;
}

bool OS::Core::StringInternal::operator<=(const StringInternal& b) const
{
	return cmp(b) <= 0;
}

bool OS::Core::StringInternal::operator<=(const OS_CHAR * b) const
{
	return cmp(b) <= 0;
}

bool OS::Core::StringInternal::operator<(const StringInternal& b) const
{
	return cmp(b) < 0;
}

bool OS::Core::StringInternal::operator<(const OS_CHAR * b) const
{
	return cmp(b) < 0;
}

bool OS::Core::StringInternal::operator>=(const StringInternal& b) const
{
	return cmp(b) >= 0;
}

bool OS::Core::StringInternal::operator>=(const OS_CHAR * b) const
{
	return cmp(b) >= 0;
}

bool OS::Core::StringInternal::operator>(const StringInternal& b) const
{
	return cmp(b) > 0;
}

bool OS::Core::StringInternal::operator>(const OS_CHAR * b) const
{
	return cmp(b) > 0;
}

int OS::Core::StringInternal::cmp(const StringInternal& b) const
{
	// StringData * data = toData(), * b_data = b.toData();
	// return Utils::cmp(data->toChar(), data->data_size, b_data->toChar(), b_data->data_size);
	return toData()->cmp(b.toData());
}

int OS::Core::StringInternal::cmp(const OS_CHAR * b) const
{
	return toData()->cmp(b, OS_STRLEN(b));
}

int OS::Core::StringInternal::hash() const
{
	return toData()->hash();
}

OS_INT OS::Core::StringInternal::toInt() const
{
	return toData()->toInt();
}

OS_FLOAT OS::Core::StringInternal::toFloat() const
{
	return toData()->toFloat();
}

// =====================================================================
// =====================================================================
// =====================================================================

OS::String::String(OS * os): super(os)
{
	os->retain();
}

OS::String::String(OS * os, const OS_CHAR * str): super(os, str)
{
	os->retain();
}

OS::String::String(OS * os, OS_CHAR c, int count): super(os, c, count)
{
	os->retain();
}

OS::String::String(OS * os, const void * buf, int size): super(os, buf, size)
{
	os->retain();
}

OS::String::String(OS * os, const void * buf1, int len1, const void * buf2, int len2): super(os, buf1, len1, buf2, len2)
{
	os->retain();
}

OS::String::String(const StringInternal& str): super(str)
{
	str.getAllocator()->retain();
}

OS::String::String(const String& str): super(str)
{
	str.getAllocator()->retain();
}

OS::String::String(Core::StringData * data): super(data)
{
	data->allocator->retain();
}

OS::String::String(OS * os, OS_INT value): super(os, value)
{
	os->retain();
}

OS::String::String(OS * os, OS_FLOAT value, int precision): super(os, value, precision)
{
	os->retain();
}

OS::String::~String()
{
	Core::StringData * data = toData();
	OS * allocator = data->allocator;
	data->release();
	allocator->release();
	str = NULL;
}


// operator const StringInternal&() const { return *this; }

OS::String& OS::String::operator=(const StringInternal& str)
{
	super::operator=(str);
	return *this;
}

OS::String& OS::String::operator=(const OS_CHAR * str)
{
	super::operator=(str);
	return *this;
}


OS::String& OS::String::operator+=(const StringInternal& str)
{
	super::operator+=(str);
	return *this;
}

OS::String& OS::String::operator+=(const OS_CHAR * str)
{
	super::operator+=(str);
	return *this;
}

OS::String& OS::String::append(const void * buf, int size)
{
	super::append(buf, size);
	return *this;
}

OS::String& OS::String::append(const OS_CHAR * str)
{
	super::append(str);
	return *this;
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

		/*
		case BEGIN_PREPROCESSOR_IF_BLOCK:     return OS_TEXT("BEGIN_PREPROCESSOR_IF_BLOCK");
		case BEGIN_PREPROCESSOR_IFDEF_BLOCK:  return OS_TEXT("BEGIN_PREPROCESSOR_IFDEF_BLOCK");
		case BEGIN_PREPROCESSOR_IFNDEF_BLOCK: return OS_TEXT("BEGIN_PREPROCESSOR_IFNDEF_BLOCK");
		case PREPROCESSOR_ELSE_BLOCK:         return OS_TEXT("PREPROCESSOR_ELSE_BLOCK");
		case PREPROCESSOR_ELSE_IF_BLOCK:      return OS_TEXT("PREPROCESSOR_ELSE_IF_BLOCK");
		case END_PREPROCESSOR_IF_BLOCK:       return OS_TEXT("END_PREPROCESSOR_IF_BLOCK");
		*/

	case CODE_SEPARATOR:      return OS_TEXT("CODE_SEPARATOR");
	case PARAM_SEPARATOR:     return OS_TEXT("PARAM_SEPARATOR");

	case COMMENT_LINE:        return OS_TEXT("COMMENT_LINE");
	case COMMENT_MULTI_LINE:  return OS_TEXT("COMMENT_MULTI_LINE");

	case NAME:      return OS_TEXT("NAME");
		// case DOT_NAME:  return OS_TEXT("DOT_NAME");
	case IDENTIFER:  return OS_TEXT("IDENTIFER");
	case DOT_IDENTIFER:  return OS_TEXT("DOT_IDENTIFER");
	case STRING:    return OS_TEXT("STRING");

	case NUM_INT:     return OS_TEXT("NUM_INT");
	case NUM_FLOAT:   return OS_TEXT("NUM_FLOAT");
	case NUM_VECTOR_3:  return OS_TEXT("NUM_VECTOR_3");
	case NUM_VECTOR_4:  return OS_TEXT("NUM_VECTOR_4");

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

	case OPERATOR_BIN_AND:  return OS_TEXT("OPERATOR_BIN_AND");
	case OPERATOR_BIN_OR:   return OS_TEXT("OPERATOR_BIN_OR");
	case OPERATOR_BIN_XOR:  return OS_TEXT("OPERATOR_BIN_XOR");
	case OPERATOR_BIN_NOT:  return OS_TEXT("OPERATOR_BIN_NOT");
	case OPERATOR_ADD:      return OS_TEXT("OPERATOR_ADD");
	case OPERATOR_SUB:      return OS_TEXT("OPERATOR_SUB");
	case OPERATOR_MUL:      return OS_TEXT("OPERATOR_MUL");
	case OPERATOR_DIV:      return OS_TEXT("OPERATOR_DIV");
	case OPERATOR_MOD:      return OS_TEXT("OPERATOR_MOD");
	case OPERATOR_MUL_SHIFT:  return OS_TEXT("OPERATOR_MUL_SHIFT");
	case OPERATOR_DIV_SHIFT:  return OS_TEXT("OPERATOR_DIV_SHIFT");
	case OPERATOR_POW:      return OS_TEXT("OPERATOR_POW");

	case OPERATOR_DOT:    return OS_TEXT("OPERATOR_DOT");
	case OPERATOR_CROSS:  return OS_TEXT("OPERATOR_CROSS");
	case OPERATOR_SWAP:  return OS_TEXT("OPERATOR_SWAP");
	case OPERATOR_IS:     return OS_TEXT("OPERATOR_IS");
	case OPERATOR_AS:     return OS_TEXT("OPERATOR_AS");

	case OPERATOR_BIN_AND_ASSIGN: return OS_TEXT("OPERATOR_BIN_AND_ASSIGN");
	case OPERATOR_BIN_OR_ASSIGN:  return OS_TEXT("OPERATOR_BIN_OR_ASSIGN");
	case OPERATOR_BIN_XOR_ASSIGN: return OS_TEXT("OPERATOR_BIN_XOR_ASSIGN");
	case OPERATOR_BIN_NOT_ASSIGN: return OS_TEXT("OPERATOR_BIN_NOT_ASSIGN");
	case OPERATOR_ADD_ASSIGN:     return OS_TEXT("OPERATOR_ADD_ASSIGN");
	case OPERATOR_SUB_ASSIGN:     return OS_TEXT("OPERATOR_SUB_ASSIGN");
	case OPERATOR_MUL_ASSIGN:     return OS_TEXT("OPERATOR_MUL_ASSIGN");
	case OPERATOR_DIV_ASSIGN:     return OS_TEXT("OPERATOR_DIV_ASSIGN");
	case OPERATOR_MOD_ASSIGN:     return OS_TEXT("OPERATOR_MOD_ASSIGN");
	case OPERATOR_MUL_SHIFT_ASSIGN: return OS_TEXT("OPERATOR_MUL_SHIFT_ASSIGN");
	case OPERATOR_DIV_SHIFT_ASSIGN: return OS_TEXT("OPERATOR_DIV_SHIFT_ASSIGN");
	case OPERATOR_POW_ASSIGN:     return OS_TEXT("OPERATOR_POW_ASSIGN");

	case OPERATOR_ASSIGN: return OS_TEXT("OPERATOR_ASSIGN");

	case OPERATOR_RESERVED: return OS_TEXT("OPERATOR_RESERVED");

	case OPERATOR_END:  return OS_TEXT("OPERATOR_END");

		// case PRE_PROCESSOR: return OS_TEXT("PRE_PROCESSOR");

	case ERROR_TOKEN:   return OS_TEXT("ERROR_TOKEN");
	}
	return OS_TEXT("TOKENTYPE !!!");
}

OS::Core::Tokenizer::TokenData::TokenData(TextData * p_text_data, const StringInternal& p_str, TokenType p_type, int p_line, int p_pos): str(p_str)
{
	text_data = p_text_data->retain();
	ref_count = 1;
	type = p_type;
	line = p_line;
	pos = p_pos;
	vec3 = NULL;
}

OS * OS::Core::Tokenizer::TokenData::getAllocator() const
{
	return str.getAllocator();
}

OS::Core::Tokenizer::TokenData::~TokenData()
{
	OS_ASSERT(ref_count == 0);
	switch(type){
	case NUM_FLOAT:
		// getAllocator()->free(float_value);
		break;

	case NUM_VECTOR_3:
		getAllocator()->free(vec3);
		break;

	case NUM_VECTOR_4:
		getAllocator()->free(vec4);
		break;
	}
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

OS_INT OS::Core::Tokenizer::TokenData::getInt() const
{
	switch(type){
	case NUM_INT: 
		return int_value;
	case NUM_FLOAT: 
		return (OS_INT)float_value;
	case NUM_VECTOR_3:
	case NUM_VECTOR_4:
		return (OS_INT)vec3[0];
	}
	return 0;
}

OS_FLOAT OS::Core::Tokenizer::TokenData::getFloat() const
{
	switch(type){
	case NUM_INT: 
		return (OS_FLOAT)int_value;
	case NUM_FLOAT:
		// OS_ASSERT(float_value);
		return float_value;
	case NUM_VECTOR_3:
	case NUM_VECTOR_4:
		OS_ASSERT(vec3);
		return vec3[0];
	}
	return 0;
}

static const OS_FLOAT zeroVec4[] = { 0, 0, 0, 0 };

const OS_FLOAT * OS::Core::Tokenizer::TokenData::getVec3() const
{
	switch(type){
	case NUM_VECTOR_3:
		OS_ASSERT(vec3);
		return vec3;
	}
	return zeroVec4;
}

const OS_FLOAT * OS::Core::Tokenizer::TokenData::getVec4() const
{
	switch(type){
	case NUM_VECTOR_4:
		OS_ASSERT(vec4);
		return vec4;
	}
	return zeroVec4;
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

		case OS::Core::Tokenizer::OPERATOR_BIN_AND: // &
		case OS::Core::Tokenizer::OPERATOR_BIN_OR:  // |
		case OS::Core::Tokenizer::OPERATOR_BIN_XOR: // ^
		case OS::Core::Tokenizer::OPERATOR_BIN_NOT: // ~
		case OS::Core::Tokenizer::OPERATOR_ADD: // +
		case OS::Core::Tokenizer::OPERATOR_SUB: // -
		case OS::Core::Tokenizer::OPERATOR_MUL: // *
		case OS::Core::Tokenizer::OPERATOR_DIV: // /
		case OS::Core::Tokenizer::OPERATOR_MOD: // %
		case OS::Core::Tokenizer::OPERATOR_MUL_SHIFT: // <<
		case OS::Core::Tokenizer::OPERATOR_DIV_SHIFT: // >>
		case OS::Core::Tokenizer::OPERATOR_POW: // **

		case OS::Core::Tokenizer::OPERATOR_DOT:
		case OS::Core::Tokenizer::OPERATOR_CROSS:
		case OS::Core::Tokenizer::OPERATOR_SWAP:
		case OS::Core::Tokenizer::OPERATOR_IS:
		case OS::Core::Tokenizer::OPERATOR_AS:

		case OS::Core::Tokenizer::OPERATOR_BIN_AND_ASSIGN: // &=
		case OS::Core::Tokenizer::OPERATOR_BIN_OR_ASSIGN:  // |=
		case OS::Core::Tokenizer::OPERATOR_BIN_XOR_ASSIGN: // ^=
		case OS::Core::Tokenizer::OPERATOR_BIN_NOT_ASSIGN: // ~=
		case OS::Core::Tokenizer::OPERATOR_ADD_ASSIGN: // +=
		case OS::Core::Tokenizer::OPERATOR_SUB_ASSIGN: // -=
		case OS::Core::Tokenizer::OPERATOR_MUL_ASSIGN: // *=
		case OS::Core::Tokenizer::OPERATOR_DIV_ASSIGN: // /=
		case OS::Core::Tokenizer::OPERATOR_MOD_ASSIGN: // %=
		case OS::Core::Tokenizer::OPERATOR_MUL_SHIFT_ASSIGN: // <<=
		case OS::Core::Tokenizer::OPERATOR_DIV_SHIFT_ASSIGN: // >>=
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
	{ OPERATOR_INDIRECT, OS_TEXT(".") }, // , OP_LEVEL_10},
	{ OPERATOR_CONCAT, OS_TEXT("..") },
	{ REST_ARGUMENTS, OS_TEXT("...") }, // , OP_LEVEL_10},
	// { OPERATOR_PRECOMP, OS_TEXT("#") }, // ,  OP_LEVEL_0},  
	// { OPERATOR_DOLLAR, OS_TEXT("$") }, // , OP_LEVEL_0},  

	{ OPERATOR_RESERVED, OS_TEXT("->") }, // , OP_LEVEL_10},  
	{ OPERATOR_RESERVED, OS_TEXT("::") }, // , OP_LEVEL_0},  

	{ OPERATOR_LOGIC_AND, OS_TEXT("&&") }, // , OP_LEVEL_1},
	{ OPERATOR_LOGIC_OR,  OS_TEXT("||") }, // , OP_LEVEL_0},
	{ OPERATOR_LOGIC_PTR_EQ, OS_TEXT("===") }, // , OP_LEVEL_2},
	{ OPERATOR_LOGIC_PTR_NE, OS_TEXT("!==") }, // , OP_LEVEL_2},
	{ OPERATOR_LOGIC_EQ,  OS_TEXT("==") }, // , OP_LEVEL_2},
	{ OPERATOR_LOGIC_NE,  OS_TEXT("!=") }, // , OP_LEVEL_2},
	{ OPERATOR_LOGIC_GE,  OS_TEXT(">=") }, // , OP_LEVEL_2},
	{ OPERATOR_LOGIC_LE,  OS_TEXT("<=") }, // , OP_LEVEL_2},
	{ OPERATOR_LOGIC_GREATER, OS_TEXT(">") }, // , OP_LEVEL_2},
	{ OPERATOR_LOGIC_LESS,    OS_TEXT("<") }, // , OP_LEVEL_2},
	{ OPERATOR_LOGIC_NOT,     OS_TEXT("!") }, // , OP_LEVEL_9},

	{ OPERATOR_INC,     OS_TEXT("++") }, // , OP_LEVEL_9},
	{ OPERATOR_DEC,     OS_TEXT("--") }, // , OP_LEVEL_9},

	{ OPERATOR_QUESTION,  OS_TEXT("?") }, // , OP_LEVEL_5},
	{ OPERATOR_COLON,     OS_TEXT(":") }, // , OP_LEVEL_5},

	{ OPERATOR_BIN_AND, OS_TEXT("&") }, // , OP_LEVEL_5},
	{ OPERATOR_BIN_OR,  OS_TEXT("|") }, // , OP_LEVEL_5},
	{ OPERATOR_BIN_XOR, OS_TEXT("^") }, // , OP_LEVEL_5},
	{ OPERATOR_BIN_NOT, OS_TEXT("~") }, // , OP_LEVEL_9},
	{ OPERATOR_CONCAT, OS_TEXT("..") }, // , OP_LEVEL_6},
	{ OPERATOR_ADD, OS_TEXT("+") }, // , OP_LEVEL_6},
	{ OPERATOR_SUB, OS_TEXT("-") }, // , OP_LEVEL_6},
	{ OPERATOR_MUL, OS_TEXT("*") }, // , OP_LEVEL_7},
	{ OPERATOR_DIV, OS_TEXT("/") }, // , OP_LEVEL_7},
	{ OPERATOR_MOD, OS_TEXT("%") }, // , OP_LEVEL_7},
	{ OPERATOR_MUL_SHIFT, OS_TEXT("<<") }, // , OP_LEVEL_7},
	{ OPERATOR_DIV_SHIFT, OS_TEXT(">>") }, // , OP_LEVEL_7},
	{ OPERATOR_POW, OS_TEXT("**") }, // , OP_LEVEL_8},

	{ OPERATOR_BIN_AND_ASSIGN, OS_TEXT("&=") }, // , OP_LEVEL_3},
	{ OPERATOR_BIN_OR_ASSIGN,  OS_TEXT("|=") }, // , OP_LEVEL_3},
	{ OPERATOR_BIN_XOR_ASSIGN, OS_TEXT("^=") }, // , OP_LEVEL_3},
	{ OPERATOR_BIN_NOT_ASSIGN, OS_TEXT("~=") }, // , OP_LEVEL_3},
	{ OPERATOR_ADD_ASSIGN, OS_TEXT("+=") }, // , OP_LEVEL_3},
	{ OPERATOR_SUB_ASSIGN, OS_TEXT("-=") }, // , OP_LEVEL_3},
	{ OPERATOR_MUL_ASSIGN, OS_TEXT("*=") }, // , OP_LEVEL_3},
	{ OPERATOR_DIV_ASSIGN, OS_TEXT("/=") }, // , OP_LEVEL_3},
	{ OPERATOR_MOD_ASSIGN, OS_TEXT("%=") }, // , OP_LEVEL_3},
	{ OPERATOR_MUL_SHIFT_ASSIGN, OS_TEXT("<<=") }, // , OP_LEVEL_3},
	{ OPERATOR_DIV_SHIFT_ASSIGN, OS_TEXT(">>=") }, // , OP_LEVEL_3},
	{ OPERATOR_POW_ASSIGN, OS_TEXT("**=") }, // , OP_LEVEL_3},

	{ OPERATOR_ASSIGN, OS_TEXT("=") }, // , OP_LEVEL_3},

	{ BEGIN_CODE_BLOCK, OS_TEXT("{") }, // , OP_LEVEL_0 },
	{ END_CODE_BLOCK, OS_TEXT("}") }, // , OP_LEVEL_0 },

	{ BEGIN_BRACKET_BLOCK, OS_TEXT("(") }, // , OP_LEVEL_0 },
	{ END_BRACKET_BLOCK, OS_TEXT(")") }, // , OP_LEVEL_0 },

	{ BEGIN_ARRAY_BLOCK, OS_TEXT("[") }, // , OP_LEVEL_0 },
	{ END_ARRAY_BLOCK, OS_TEXT("]") }, // , OP_LEVEL_0 },

	{ CODE_SEPARATOR, OS_TEXT(";") }, // , OP_LEVEL_0 },
	{ PARAM_SEPARATOR, OS_TEXT(",") } // , OP_LEVEL_0 }
};

const int OS::Core::Tokenizer::operator_count = sizeof(operator_desc) / sizeof(operator_desc[0]);

int __cdecl OS::Core::Tokenizer::CompareOperatorDesc(const void * a, const void * b) 
{
	const OperatorDesc * op0 = (const OperatorDesc*)a;
	const OperatorDesc * op1 = (const OperatorDesc*)b;
	return (int)OS_STRLEN(op1->name) - (int)OS_STRLEN(op0->name);
}

void OS::Core::Tokenizer::initOperatorTable()
{
	if(!operator_initialized){
		qsort(operator_desc, operator_count, sizeof(operator_desc[0]), CompareOperatorDesc);
		operator_initialized = true;
	}
}

OS::Core::Tokenizer::TextData::TextData(OS * allocator): filename(allocator)
{
	ref_count = 1;
}

OS::Core::Tokenizer::TextData::~TextData()
{
	OS_ASSERT(!ref_count);
}

OS * OS::Core::Tokenizer::TextData::getAllocator()
{
	return filename.getAllocator();
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

OS::Core::Tokenizer::Tokenizer(OS * allocator)
{
	initOperatorTable();
	settings.parseVector = false;
	settings.parseStringOperator = false;
	// settings.parsePreprocessor = true;
	settings.saveComment = false;
	error = ERROR_NOTHING;
	cur_line = 0;
	cur_pos = 0;
	loaded = false;
	compiled = false;

	text_data = new (allocator->malloc(sizeof(TextData))) TextData(allocator);
}

OS * OS::Core::Tokenizer::getAllocator()
{
	return text_data->filename.getAllocator();
}

OS::Core::Tokenizer::~Tokenizer()
{
	OS * allocator = getAllocator();
	for(int i = 0; i < tokens.count; i++){
		TokenData * token = tokens[i];
		token->release();
		// token->~TokenData();
		// allocator->free(token);
	}
	allocator->vectorClear(tokens);
	// allocator->vectorClear(lines);
	text_data->release();
}

void OS::Core::Tokenizer::reset()
{
	OS * allocator = getAllocator();
	text_data->release();
	text_data = new (allocator->malloc(sizeof(TextData))) TextData(allocator);
	allocator->vectorClear(tokens);
	cur_line = cur_pos = 0;
	loaded = false;
	compiled = false;
}

void OS::Core::Tokenizer::insertToken(int i, TokenData * token)
{
	getAllocator()->vectorInsertAtIndex(tokens, i, token);
}

/*
void OS::Core::Tokenizer::PrintLines()
{
	_tprintf(OS_TEXT("[OS::Core::Tokenizer::PrintLines] lines: %d\n\n"), lines.Count());
	for(int i = 0; i < lines.Count(); i++)
	{
		_tprintf(OS_TEXT("%s\n"), lines[i]);
	}
}

void OS::Core::Tokenizer::PrintTokens()
{
	_tprintf(OS_TEXT("[OS::Core::Tokenizer::PrintTokens] tokens: %d\n\n"), tokens.Count());
	for(int i = 0; i < tokens.Count(); i++)
	{
		TokenData * token = tokens[i];
		_tprintf(OS_TEXT("token: %s, type: %d, line: %d, %d\n"), token->str, token_type, token->line, token->pos); //, lines[token.line].SubString(token.pos, 20));
	}
}
*/

bool OS::Core::Tokenizer::parseText(const StringInternal& text)
{
	OS * allocator = getAllocator();

	text_data->release();
	text_data = new (allocator->malloc(sizeof(TextData))) TextData(allocator);

	int len = text.getDataSize();
	const OS_CHAR * str = text; // text.toChar();
	const OS_CHAR * str_end = str + len;
	while(str < str_end)
	{
		const OS_CHAR * line_end = OS_STRCHR(str, OS_TEXT('\n'));
		if(line_end){
			allocator->vectorAddItem(text_data->lines, StringInternal(allocator, str, line_end - str).trim(false, true));
			str = line_end+1;
		}else{
			allocator->vectorAddItem(text_data->lines, StringInternal(allocator, str).trim(false, true));
			break;
		}
	}
	loaded = true;

	// PrintLines();
	// lines loaded

	return parseLines();
}

void OS::Core::Tokenizer::TokenData::setVec4(OS_FLOAT values[4])
{
	OS_ASSERT(type == NUM_VECTOR_4 && !vec4);
	vec4 = (OS_FLOAT*)getAllocator()->malloc(sizeof(OS_FLOAT)*4);
	vec4[0] = values[0];
	vec4[1] = values[1];
	vec4[2] = values[2];
	vec4[3] = values[3];
}

void OS::Core::Tokenizer::TokenData::setVec3(OS_FLOAT values[3])
{
	OS_ASSERT(type == NUM_VECTOR_3 && !vec3);
	vec3 = (OS_FLOAT*)getAllocator()->malloc(sizeof(OS_FLOAT)*3);
	vec3[0] = values[0];
	vec3[1] = values[1];
	vec3[2] = values[2];
}

void OS::Core::Tokenizer::TokenData::setFloat(OS_FLOAT value)
{
	// OS_ASSERT(type == NUM_FLOAT); // && !float_value);
	float_value = value;
}

void OS::Core::Tokenizer::TokenData::setInt(OS_INT value)
{
	// OS_ASSERT(type == NUM_INT); // && !vec3);
	int_value = value;
}

OS::Core::Tokenizer::TokenData * OS::Core::Tokenizer::addToken(const StringInternal& str, TokenType type, int line, int pos)
{
	OS * allocator = getAllocator();
	TokenData * token = new (allocator->malloc(sizeof(TokenData))) TokenData(text_data, str, type, line, pos);
	allocator->vectorAddItem(tokens, token);
	return token;
}

OS::Core::TokenType OS::Core::Tokenizer::parseNum(const OS_CHAR *& str, OS_FLOAT& fval, OS_INT& ival, bool parse_end_spaces)
{
	switch(Utils::parseNum(str, fval, ival, PARSE_NUM_FLAG_INT | PARSE_NUM_FLAG_FLOAT)){
	// case PARSE_NUM_TYPE_ERROR:
	// 	return ERROR_TOKEN;

	case PARSE_NUM_TYPE_FLOAT:
		if(parse_end_spaces){
			parseSpaces(str);
		}
		return NUM_FLOAT;

	case PARSE_NUM_TYPE_INT:
		if(parse_end_spaces){
			parseSpaces(str);
		}
		// fval = (OS_FLOAT)ival;
		return NUM_INT;
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

			if(*str == OS_TEXT('"') || (!settings.parseVector && *str == OS_TEXT('\''))){ // begin string
				StringInternal s(allocator);
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
									if(!parseHexSimple(str, val)){
										cur_pos = str - line_start;
										error = ERROR_CONST_STRING_ESCAPE_CHAR;
										return false;
									}
								}else if(*str == OS_TEXT('0')){ // octal
									if(!parseOctalSimple(str, val)){
										cur_pos = str - line_start;
										error = ERROR_CONST_STRING_ESCAPE_CHAR;
										return false;
									}
								}else if(*str >= OS_TEXT('1') && *str <= OS_TEXT('9')){
									if(!parseDecSimple(str, val)){
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
					s.append(&c, sizeof(c));
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
			if(*str == OS_TEXT('\'') && settings.parseVector){ // begin const vec3
				const OS_CHAR * tokenStart = str++;
				parseSpaces(str);
				OS_FLOAT values[4];
				int i;
				for(i = 0; i < 4; i++){
					if(i == 3 && *str == OS_TEXT('\'')){
						break;
					}
					OS_INT ival;
					switch(parseNum(str, values[i], ival, true)){
					case ERROR_TOKEN:
						cur_pos = str - line_start;
						error = i < 3 ? ERROR_CONST_VECTOR_3 : ERROR_CONST_VECTOR_4;
						return false;

					case NUM_INT:
						values[i] = (OS_FLOAT)ival;
					}
				}
				if(*str != OS_TEXT('\'')){
					cur_pos = str - line_start;
					error = i < 4 ? ERROR_CONST_VECTOR_3 : ERROR_CONST_VECTOR_4;
					return false;
				}
				str++;
				TokenData * vecToken = addToken(StringInternal(allocator, tokenStart, str - tokenStart), i < 4 ? NUM_VECTOR_3 : NUM_VECTOR_4, cur_line, tokenStart - line_start);
				if(i < 4){
					vecToken->setVec3(values);
				}else{
					vecToken->setVec4(values);
				}
				continue;
			}

			if(*str == OS_TEXT('/')){
				if(str[1] == OS_TEXT('/')){ // begin line comment
					if(settings.saveComment){
						addToken(StringInternal(allocator, str), COMMENT_LINE, cur_line, str - line_start);
					}
					break;
				}
				if(str[1] == OS_TEXT('*')){ // begin multi line comment
					StringInternal comment(allocator, str, sizeof(OS_CHAR)*2);
					int startLine = cur_line;
					int startPos = str - line_start;
					for(str += 2;;){
						const OS_CHAR * end = OS_STRSTR(str, OS_TEXT("*/"));
						if(end){
							comment.append(str, (int)(end+2) - (int)str);
							if(settings.saveComment){
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
						comment.append(str);
						comment.append(OS_TEXT("\r\n"));
						str = line_start = text_data->lines[++cur_line].toChar();
					}
					continue;
				}
			}

			if(*str == OS_TEXT('_') || *str == OS_TEXT('$') 
				|| (*str >= OS_TEXT('a') && *str <= OS_TEXT('z'))
				|| (*str >= OS_TEXT('A') && *str <= OS_TEXT('Z')) )
			{ // parse name
				const OS_CHAR * nameStart = str;
				for(str++; *str; str++){
					if(*str == OS_TEXT('_') || *str == OS_TEXT('$')
						|| (*str >= OS_TEXT('a') && *str <= OS_TEXT('z'))
						|| (*str >= OS_TEXT('A') && *str <= OS_TEXT('Z'))
						|| (*str >= OS_TEXT('0') && *str <= OS_TEXT('9')) )
					{
						continue;
					}
					break;
				}
				StringInternal name = StringInternal(allocator, nameStart, str - nameStart);
				TokenType type = NAME;
				if(settings.parseStringOperator){
					if(name == OS_TEXT("dot")){
						type = OPERATOR_DOT;
					}else if(name == OS_TEXT("cross")){
						type = OPERATOR_CROSS;
					}else if(name == OS_TEXT("swap")){
						type = OPERATOR_SWAP;
					}else if(name == OS_TEXT("as")){
						type = OPERATOR_AS;
					}else if(name == OS_TEXT("is")){
						type = OPERATOR_IS;
					}else if(name == OS_TEXT("in")){
						type = OPERATOR_IN;
					}else if(name == OS_TEXT("eq")){
						type = OPERATOR_LOGIC_PTR_EQ;
					}else if(name == OS_TEXT("ne")){
						type = OPERATOR_LOGIC_PTR_NE;
					}
				}
				addToken(name, type, cur_line, nameStart - line_start);
				continue;
			}
			// parse operator
			int i;
			for(i = 0; i < operator_count; i++){
				size_t len = OS_STRLEN(operator_desc[i].name);
				if(OS_STRNCMP(str, operator_desc[i].name, len) == 0)
				{
					addToken(StringInternal(allocator, str, (int)len), operator_desc[i].type, cur_line, str - line_start);
					str += len;
					break;
				}
			}
			if(i < operator_count){
				continue;
			}

			{
				OS_INT ival;
				OS_FLOAT val;
				const OS_CHAR * tokenStart = str;
				TokenType type = parseNum(str, val, ival, true);
				if(type != ERROR_TOKEN){
					TokenData * token = addToken(StringInternal(allocator, tokenStart, str - tokenStart).trim(false, true), type, cur_line, tokenStart - line_start);
					if(type == NUM_INT){
						token->setInt(ival);
					}else{
						token->setFloat(val);
					}
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
	is_param = false;
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
	case EXP_TYPE_CALL_PROPERTY:
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
	case EXP_TYPE_LOGIC_NOT:     // !
	case EXP_TYPE_PLUS:    // +
	case EXP_TYPE_NEG:     // -
		// case EXP_TYPE_INC:     // ++
		//case EXP_TYPE_DEC:     // --
	case EXP_TYPE_PRE_INC:     // ++
	case EXP_TYPE_PRE_DEC:     // --
	case EXP_TYPE_POST_INC:    // ++
	case EXP_TYPE_POST_DEC:    // --
	case EXP_TYPE_BIN_NOT:      // ~
		return true;
	}
	return false;
}

bool OS::Core::Compiler::Expression::isLogicOperator() const
{
	switch(type){
	case EXP_TYPE_LOGIC_NOT: // !

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

	case EXP_TYPE_BIN_AND: // &
	case EXP_TYPE_BIN_OR:  // |
	case EXP_TYPE_BIN_XOR: // ^

	case EXP_TYPE_BIN_AND_ASSIGN: // &=
	case EXP_TYPE_BIN_OR_ASSIGN:  // |=
	case EXP_TYPE_BIN_XOR_ASSIGN: // ^=
	case EXP_TYPE_BIN_NOT_ASSIGN: // ~=

	case EXP_TYPE_ADD: // +
	case EXP_TYPE_SUB: // -
	case EXP_TYPE_MUL: // *
	case EXP_TYPE_DIV: // /
	case EXP_TYPE_MOD: // %
	case EXP_TYPE_MUL_SHIFT: // <<
	case EXP_TYPE_DIV_SHIFT: // >>
	case EXP_TYPE_POW: // **

	case EXP_TYPE_DOT: // dot
	case EXP_TYPE_CROSS: // cross

	case EXP_TYPE_SWAP: // swap

	case EXP_TYPE_AS:
	case EXP_TYPE_IS:
	case EXP_TYPE_IN:

	case EXP_TYPE_ADD_ASSIGN: // +=
	case EXP_TYPE_SUB_ASSIGN: // -=
	case EXP_TYPE_MUL_ASSIGN: // *=
	case EXP_TYPE_DIV_ASSIGN: // /=
	case EXP_TYPE_MOD_ASSIGN: // %=
	case EXP_TYPE_MUL_SHIFT_ASSIGN: // <<=
	case EXP_TYPE_DIV_SHIFT_ASSIGN: // >>=
	case EXP_TYPE_POW_ASSIGN: // **=
		return true;
	}
	return isAssignOperator();
}

bool OS::Core::Compiler::Expression::isAssignOperator() const
{
	switch(type){
	case EXP_TYPE_ASSIGN: // =

	case EXP_TYPE_BIN_AND_ASSIGN: // &=
	case EXP_TYPE_BIN_OR_ASSIGN:  // |=
	case EXP_TYPE_BIN_XOR_ASSIGN: // ^=
	case EXP_TYPE_BIN_NOT_ASSIGN: // ~=

	case EXP_TYPE_ADD_ASSIGN: // +=
	case EXP_TYPE_SUB_ASSIGN: // -=
	case EXP_TYPE_MUL_ASSIGN: // *=
	case EXP_TYPE_DIV_ASSIGN: // /=
	case EXP_TYPE_MOD_ASSIGN: // %=
	case EXP_TYPE_MUL_SHIFT_ASSIGN: // <<=
	case EXP_TYPE_DIV_SHIFT_ASSIGN: // >>=
	case EXP_TYPE_POW_ASSIGN: // **=
		return true;
	}
	return false;
}

/*
static void fillSpaces(OS_CHAR * s, int len)
{
  for(int i = 0; i < len; i++){
    s[i] = OS_TEXT(' ');
  }
  s[len] = 0;
}
*/

OS::Core::StringInternal OS::Core::Compiler::Expression::debugPrint(OS::Core::Compiler * compiler, int depth)
{
	OS * allocator = getAllocator();
	StringInternal out = compiler->debugPrintSourceLine(token);

	// OS_CHAR * spaces = (OS_CHAR*)alloca(sizeof(OS_CHAR)*(depth+1));
	// fillSpaces(spaces, depth);
	StringInternal spaces_buf(allocator, OS_TEXT(' '), depth*2);
	const OS_CHAR * spaces = spaces_buf;

	int i;
	const OS_CHAR * type_name;
	switch(type){
	case EXP_TYPE_NOP:
		out += StringInternal::format(allocator, OS_TEXT("%snop\n"), spaces);
		break;

	case EXP_TYPE_CODE_LIST:
		type_name = OS::Core::Compiler::getExpName(type);
		out += StringInternal::format(allocator, OS_TEXT("%sbegin %s\n"), spaces, type_name);
		for(i = 0; i < list.count; i++){
			if(i > 0){
				out += OS_TEXT("\n");
			}
			// OS_ASSERT(i+1 == list.count ? list[i]->ret_values == ret_values : list[i]->ret_values == 0);
			out += list[i]->debugPrint(compiler, depth+1);
		}
		out += StringInternal::format(allocator, OS_TEXT("%send %s ret values %d\n"), spaces, type_name, ret_values);
		break;

	case EXP_TYPE_CONST_NUMBER:
	case EXP_TYPE_CONST_STRING:
		{
			// const OS_CHAR * a = OS_TEXT(""), * b = OS_TEXT("");
			switch(token->getType()){
			case Tokenizer::NUM_INT: type_name = OS_TEXT("int "); break;
			case Tokenizer::NUM_FLOAT: type_name = OS_TEXT("float "); break;
			case Tokenizer::NUM_VECTOR_3: type_name = OS_TEXT("vec3 "); break;
			case Tokenizer::NUM_VECTOR_4: type_name = OS_TEXT("vec4 "); break;
			case Tokenizer::STRING: type_name = OS_TEXT("string "); break;
			case Tokenizer::NAME: type_name = OS_TEXT("string "); break;
			default: type_name = OS_TEXT("???"); break;
			}
			out += StringInternal::format(allocator, OS_TEXT("%spush const %s%s\n"), spaces, type_name, token->str.toChar());
		}
		break;

	case EXP_TYPE_CONST_NULL:
	case EXP_TYPE_CONST_TRUE:
	case EXP_TYPE_CONST_FALSE:
		out += StringInternal::format(allocator, OS_TEXT("%s%s\n"), spaces, getExpName(type));
		break;

	case EXP_TYPE_NAME:
		out += StringInternal::format(allocator, OS_TEXT("%sname %s\n"), spaces, token->str.toChar());
		break;

	case EXP_TYPE_PARAMS:
		out += StringInternal::format(allocator, OS_TEXT("%sbegin params %d\n"), spaces, list.count);
		for(i = 0; i < list.count; i++){
			if(i > 0){
				out += StringInternal::format(allocator, OS_TEXT("%s  ,\n"), spaces);
			}
			out += list[i]->debugPrint(compiler, depth+1);
		}
		out += StringInternal::format(allocator, OS_TEXT("%send params ret values %d\n"), spaces, ret_values);
		break;

	case EXP_TYPE_ARRAY:
		out += StringInternal::format(allocator, OS_TEXT("%sbegin array %d\n"), spaces, list.count);
		for(i = 0; i < list.count; i++){
			if(i > 0){
				out += StringInternal::format(allocator, OS_TEXT("%s  ,\n"), spaces);
			}
			out += list[i]->debugPrint(compiler, depth+1);
		}
		out += StringInternal::format(allocator, OS_TEXT("%send array\n"), spaces);
		break;

	case EXP_TYPE_OBJECT:
		out += StringInternal::format(allocator, OS_TEXT("%sbegin object %d\n"), spaces, list.count);
		for(i = 0; i < list.count; i++){
			if(i > 0){
				out += StringInternal::format(allocator, OS_TEXT("%s ,\n"), spaces);
			}
			out += list[i]->debugPrint(compiler, depth+1);
		}
		out += StringInternal::format(allocator, OS_TEXT("%send object\n"), spaces);
		break;

	case EXP_TYPE_OBJECT_SET_BY_NAME:
		OS_ASSERT(list.count == 1);
		out += list[0]->debugPrint(compiler, depth+1);
		out += StringInternal::format(allocator, OS_TEXT("%sset by name: [%s]\n"), spaces, token->str.toChar());
		break;

	case EXP_TYPE_OBJECT_SET_BY_INDEX:
		OS_ASSERT(list.count == 1);
		out += list[0]->debugPrint(compiler, depth+1);
		out += StringInternal::format(allocator, OS_TEXT("%sset by index: [%d]\n"), spaces, token->getInt());
		break;

	case EXP_TYPE_OBJECT_SET_BY_EXP:
		OS_ASSERT(list.count == 2);
		out += list[0]->debugPrint(compiler, depth+1);
		out += list[1]->debugPrint(compiler, depth+1);
		out += StringInternal::format(allocator, OS_TEXT("%sset by exp\n"), spaces);
		break;

	case EXP_TYPE_OBJECT_SET_BY_AUTO_INDEX:
		OS_ASSERT(list.count == 1);
		out += list[0]->debugPrint(compiler, depth+1);
		out += StringInternal::format(allocator, OS_TEXT("%sset like array\n"), spaces);
		break;

	case EXP_TYPE_FUNCTION:
		{
			// OS_ASSERT(list.count >= 1);
			Scope * scope = dynamic_cast<Scope*>(this);
			OS_ASSERT(scope);
			out += StringInternal::format(allocator, OS_TEXT("%sbegin function\n"), spaces);
			if(scope->num_locals > 0){
				out += StringInternal::format(allocator, OS_TEXT("%s  begin locals, total %d\n"), spaces, scope->num_locals);
				for(i = 0; i < scope->locals.count; i++){
					out += StringInternal::format(allocator, OS_TEXT("%s    %d %s%s\n"), spaces, 
						scope->locals[i].index,
						scope->locals[i].name.toChar(),
						i < scope->num_params ? OS_TEXT(" (param)") : OS_TEXT("")
						);
				}
				out += StringInternal::format(allocator, OS_TEXT("%s  end locals\n"), spaces);
			}
			for(i = 0; i < list.count; i++){
				if(i > 0){
					out += OS_TEXT("\n");
				}
				out += list[i]->debugPrint(compiler, depth+1);
			}
			out += StringInternal::format(allocator, OS_TEXT("%send function\n"), spaces);
			break;
		}

	case EXP_TYPE_SCOPE:
		{
			// OS_ASSERT(list.count >= 1);
			Scope * scope = dynamic_cast<Scope*>(this);
			OS_ASSERT(scope);
			out += StringInternal::format(allocator, OS_TEXT("%sbegin scope\n"), spaces);
			if(scope->locals.count > 0){
				out += StringInternal::format(allocator, OS_TEXT("%s  begin locals %d\n"), spaces, scope->locals.count);
				for(i = 0; i < scope->locals.count; i++){
					out += StringInternal::format(allocator, OS_TEXT("%s    %d %s%s\n"), spaces, 
						scope->locals[i].index,
						scope->locals[i].name.toChar(),
						i < scope->num_params ? OS_TEXT(" (param)") : OS_TEXT("")
						);
				}
				out += StringInternal::format(allocator, OS_TEXT("%s  end locals\n"), spaces);
			}
			for(i = 0; i < list.count; i++){
				if(i > 0){
					out += OS_TEXT("\n");
				}
				out += list[i]->debugPrint(compiler, depth+1);
			}
			out += StringInternal::format(allocator, OS_TEXT("%send scope ret values %d\n"), spaces, ret_values);
			break;
		}

	case EXP_TYPE_RETURN:
		if(list.count > 0){
			out += StringInternal::format(allocator, OS_TEXT("%sbegin return\n"), spaces);
			for(i = 0; i < list.count; i++){
				if(i > 0){
					out += StringInternal::format(allocator, OS_TEXT("%s ,\n"), spaces);
				}
				out += list[i]->debugPrint(compiler, depth+1);
			}
			out += StringInternal::format(allocator, OS_TEXT("%send return values %d\n"), spaces, ret_values);
		}else{
			out += OS_TEXT("return\n");
		}
		break;

	case EXP_TYPE_TAIL_CALL:
		OS_ASSERT(list.count == 2);
		out += StringInternal::format(allocator, OS_TEXT("%sbegin tail call\n"), spaces);
		out += list[0]->debugPrint(compiler, depth+1);
		out += list[1]->debugPrint(compiler, depth+1);
		out += StringInternal::format(allocator, OS_TEXT("%send tail call\n"), spaces);
		break;

	case EXP_TYPE_CALL:
	case EXP_TYPE_CALL_DIM:
	case EXP_TYPE_GET_DIM:
	case EXP_TYPE_CALL_PROPERTY:
	case EXP_TYPE_GET_PROPERTY:
	case EXP_TYPE_GET_PROPERTY_DIM:
	case EXP_TYPE_SET_AUTO_VAR_DIM:
		OS_ASSERT(list.count == 2);
		out += StringInternal::format(allocator, OS_TEXT("%sbegin %s\n"), spaces, OS::Core::Compiler::getExpName(type));
		out += list[0]->debugPrint(compiler, depth+1);
		out += list[1]->debugPrint(compiler, depth+1);
		out += StringInternal::format(allocator, OS_TEXT("%send %s ret values %d\n"), spaces, OS::Core::Compiler::getExpName(type), ret_values);
		break;

	case EXP_TYPE_SET_LOCAL_VAR_DIM:
		OS_ASSERT(false);
		break;

	case EXP_TYPE_GET_AUTO_VAR_DIM:
		OS_ASSERT(list.count == 1);
		out += StringInternal::format(allocator, OS_TEXT("%sbegin %s\n"), spaces, OS::Core::Compiler::getExpName(type));
		out += list[0]->debugPrint(compiler, depth+1);
		out += StringInternal::format(allocator, OS_TEXT("%send %s ret values %d\n"), spaces, OS::Core::Compiler::getExpName(type), ret_values);
		break;

	case EXP_TYPE_VALUE:
		OS_ASSERT(list.count == 1);
		out += StringInternal::format(allocator, OS_TEXT("%sbegin %s\n"), spaces, OS::Core::Compiler::getExpName(type));
		out += list[0]->debugPrint(compiler, depth+1);
		out += StringInternal::format(allocator, OS_TEXT("%send %s\n"), spaces, OS::Core::Compiler::getExpName(type));
		break;

	case EXP_TYPE_POP_VALUE:
		OS_ASSERT(list.count == 1);
		out += StringInternal::format(allocator, OS_TEXT("%sbegin %s\n"), spaces, OS::Core::Compiler::getExpName(type));
		out += list[0]->debugPrint(compiler, depth+1);
		out += StringInternal::format(allocator, OS_TEXT("%send %s ret values %d\n"), spaces, OS::Core::Compiler::getExpName(type), ret_values);
		break;

	/*
	case EXP_TYPE_THREAD:
		OS_ASSERT(list.count == 2);
		out += StringInternal::format(allocator, OS_TEXT("%sbegin thread call\n"), spaces);
		out += list[0]->debugPrint(compiler, depth+1);
		out += list[1]->debugPrint(compiler, depth+1);
		out += StringInternal::format(allocator, OS_TEXT("%send thread call\n"), spaces);
		break;

	case EXP_TYPE_FOR:
		OS_ASSERT(list.count == 4);
		out += StringInternal::format(allocator, OS_TEXT("%sbegin for\n"), spaces);

		out += StringInternal::format(allocator, OS_TEXT("%s begin init\n"), spaces);
		out += list[0]->debugPrint(compiler, depth+2);
		out += StringInternal::format(allocator, OS_TEXT("%s end init\n"), spaces);

		out += StringInternal::format(allocator, OS_TEXT("%s begin bool\n"), spaces);
		out += list[1]->debugPrint(compiler, depth+2);
		out += StringInternal::format(allocator, OS_TEXT("%s end bool\n"), spaces);

		out += StringInternal::format(allocator, OS_TEXT("%s begin step\n"), spaces);
		out += list[2]->debugPrint(compiler, depth+2);
		out += StringInternal::format(allocator, OS_TEXT("%s end step\n"), spaces);

		out += StringInternal::format(allocator, OS_TEXT("%s begin code\n"), spaces);
		out += list[3]->debugPrint(compiler, depth+2);
		out += StringInternal::format(allocator, OS_TEXT("%s end code\n"), spaces);
		out += StringInternal::format(allocator, OS_TEXT("%send for\n"), spaces);
		break;

	case EXP_TYPE_DO_WHILE:
		OS_ASSERT(list.count == 2);
		out += StringInternal::format(allocator, OS_TEXT("%sbegin do while\n"), spaces);

		out += StringInternal::format(allocator, OS_TEXT("%s begin code\n"), spaces);
		out += list[0]->debugPrint(compiler, depth+2);
		out += StringInternal::format(allocator, OS_TEXT("%s end code\n"), spaces);

		out += StringInternal::format(allocator, OS_TEXT("%s begin bool\n"), spaces);
		out += list[1]->debugPrint(compiler, depth+2);
		out += StringInternal::format(allocator, OS_TEXT("%s end bool\n"), spaces);

		out += StringInternal::format(allocator, OS_TEXT("%send do while\n"), spaces);
		break;

	case EXP_TYPE_WHILE_DO:
		OS_ASSERT(list.count == 2);
		out += StringInternal::format(allocator, OS_TEXT("%sbegin while do\n"), spaces);

		out += StringInternal::format(allocator, OS_TEXT("%s begin bool\n"), spaces);
		out += list[0]->debugPrint(compiler, depth+2);
		out += StringInternal::format(allocator, OS_TEXT("%s end bool\n"), spaces);

		out += StringInternal::format(allocator, OS_TEXT("%s begin code\n"), spaces);
		out += list[1]->debugPrint(compiler, depth+2);
		out += StringInternal::format(allocator, OS_TEXT("%s end code\n"), spaces);

		out += StringInternal::format(allocator, OS_TEXT("%send while do\n"), spaces);
		break;

	case EXP_TYPE_SWITCH:
		OS_ASSERT(list.count == 2);
		out += StringInternal::format(allocator, OS_TEXT("%sbegin switch\n"), spaces);
		out += StringInternal::format(allocator, OS_TEXT("%s begin value\n"), spaces);
		out += list[0]->debugPrint(compiler, depth+2);
		out += StringInternal::format(allocator, OS_TEXT("%s end value\n"), spaces);

		out += StringInternal::format(allocator, OS_TEXT("%s begin cases\n"), spaces);
		out += list[1]->debugPrint(compiler, depth+2);
		out += StringInternal::format(allocator, OS_TEXT("%s end cases\n"), spaces);
		out += StringInternal::format(allocator, OS_TEXT("%send switch\n"), spaces);
		break;

	case EXP_TYPE_SWITCH_CASE:
		OS_ASSERT(list.count == 2);
		out += StringInternal::format(allocator, OS_TEXT("%sbegin case\n"), spaces);

		out += StringInternal::format(allocator, OS_TEXT("%s begin value\n"), spaces);
		out += list[0]->debugPrint(compiler, depth+2);
		out += StringInternal::format(allocator, OS_TEXT("%s end value\n"), spaces);

		out += StringInternal::format(allocator, OS_TEXT("%s begin code\n"), spaces);
		out += list[1]->debugPrint(compiler, depth+2);
		out += StringInternal::format(allocator, OS_TEXT("%s end code\n"), spaces);

		out += StringInternal::format(allocator, OS_TEXT("%send case\n"), spaces);
		break;

	case EXP_TYPE_SWITCH_CASE_INTERVAL:
		OS_ASSERT(list.count == 3);
		out += StringInternal::format(allocator, OS_TEXT("%sbegin case interval\n"), spaces);

		out += StringInternal::format(allocator, OS_TEXT("%s begin value A\n"), spaces);
		out += list[0]->debugPrint(compiler, depth+2);
		out += StringInternal::format(allocator, OS_TEXT("%s end value A\n"), spaces);

		out += StringInternal::format(allocator, OS_TEXT("%s begin value B\n"), spaces);
		out += list[1]->debugPrint(compiler, depth+2);
		out += StringInternal::format(allocator, OS_TEXT("%s end value B\n"), spaces);

		out += StringInternal::format(allocator, OS_TEXT("%s begin code\n"), spaces);
		out += list[2]->debugPrint(compiler, depth+2);
		out += StringInternal::format(allocator, OS_TEXT("%s end code\n"), spaces);

		out += StringInternal::format(allocator, OS_TEXT("%send case interval\n"), spaces);
		break;

	case EXP_TYPE_SWITCH_DEFAULT:
		OS_ASSERT(list.count == 1);
		out += StringInternal::format(allocator, OS_TEXT("%sbegin default code\n"), spaces);

		// out += StringInternal::format(allocator, OS_TEXT("%s begin code\n"), spaces);
		out += list[0]->debugPrint(compiler, depth+1);
		// out += StringInternal::format(allocator, OS_TEXT("%s end code\n"), spaces);

		out += StringInternal::format(allocator, OS_TEXT("%send default code\n"), spaces);
		break;

	case EXP_TYPE_BREAK:
	case EXP_TYPE_CONTINUE:
		{
			OS_ASSERT(!list.count || (list.count == 1 && list[0]->macro == EXP_TYPE_LABEL));
			out += StringInternal::format(allocator, OS_TEXT("%s%s%s\n"), spaces, OS::Core::Compiler::getExpName(type), list.count ? OS_TEXT(" ") + list[0]->token->str : OS_TEXT(""));
			break;
		}

	case EXP_TYPE_IN:
		{
			OS_ASSERT(list.count >= 2);
			out += StringInternal::format(allocator, OS_TEXT("%sbegin operator in\n"), spaces);
			out += StringInternal::format(allocator, OS_TEXT("%s begin value\n"), spaces);
			out += list[0]->debugPrint(compiler, depth+2);
			out += StringInternal::format(allocator, OS_TEXT("%s end value\n"), spaces);

			out += StringInternal::format(allocator, OS_TEXT("%s begin cases\n"), spaces);
			for(int i = 1; i < list.count; i++)
			{
				OS_ASSERT(list[i]->macro == EXP_TYPE_IN_CASE || list[i]->macro == EXP_TYPE_IN_CASE_INTERVAL);
				out += list[i]->debugPrint(compiler, depth+2);
			}
			out += StringInternal::format(allocator, OS_TEXT("%s end cases\n"), spaces);
			out += StringInternal::format(allocator, OS_TEXT("%send operator in\n"), spaces);
		}
		break;

	case EXP_TYPE_IN_CASE:
		OS_ASSERT(list.count == 1);
		out += StringInternal::format(allocator, OS_TEXT("%sbegin case\n"), spaces);
		out += list[0]->debugPrint(compiler, depth+1);
		out += StringInternal::format(allocator, OS_TEXT("%send case\n"), spaces);
		break;

	case EXP_TYPE_IN_CASE_INTERVAL:
		OS_ASSERT(list.count == 2);
		out += StringInternal::format(allocator, OS_TEXT("%sbegin case interval\n"), spaces);

		out += StringInternal::format(allocator, OS_TEXT("%s begin value A\n"), spaces);
		out += list[0]->debugPrint(compiler, depth+2);
		out += StringInternal::format(allocator, OS_TEXT("%s end value A\n"), spaces);

		out += StringInternal::format(allocator, OS_TEXT("%s begin value B\n"), spaces);
		out += list[1]->debugPrint(compiler, depth+2);
		out += StringInternal::format(allocator, OS_TEXT("%s end value B\n"), spaces);

		out += StringInternal::format(allocator, OS_TEXT("%send case interval\n"), spaces);
		break;

		//   case EXP_TYPE_SET_LOCAL:
		//   case EXP_TYPE_SET_OBJECT_FIELD:

	case EXP_TYPE_GOTO:
		break;

	case EXP_TYPE_IF:
		// case EXP_TYPE_IF_NOT:
	case EXP_TYPE_POST_IF:
		{
			OS_ASSERT(list.count == 2 || list.count == 3);
			out += StringInternal::format(allocator, OS_TEXT("%sbegin %s\n"), spaces, OS::Core::Compiler::getExpName(type));

			out += StringInternal::format(allocator, OS_TEXT("%s begin bool\n"), spaces);
			out += list[0]->debugPrint(compiler, depth+2);
			out += StringInternal::format(allocator, OS_TEXT("%s end bool\n"), spaces);

			out += StringInternal::format(allocator, OS_TEXT("%s begin then\n"), spaces);
			out += list[1]->debugPrint(compiler, depth+2);
			out += StringInternal::format(allocator, OS_TEXT("%s end then\n"), spaces);

			if(list.count == 3)
			{
				out += StringInternal::format(allocator, OS_TEXT("%s begin else\n"), spaces);
				out += list[2]->debugPrint(compiler, depth+2);
				out += StringInternal::format(allocator, OS_TEXT("%s end else\n"), spaces);
			}
			out += StringInternal::format(allocator, OS_TEXT("%send %s\n"), spaces, OS::Core::Compiler::getExpName(type));
			break;
		}

	case EXP_TYPE_WAIT_FRAME:
		out += StringInternal::format(allocator, OS_TEXT("%s%s\n"), spaces, OS::Core::Compiler::getExpName(type));
		break;

	case EXP_TYPE_WAIT_APP_TIME_MS:
	case EXP_TYPE_WAIT_OBJECT:
	case EXP_TYPE_KILL:
	case EXP_TYPE_CLONE:
	case EXP_TYPE_VALID:
		OS_ASSERT(list.count == 1);
		out += StringInternal::format(allocator, OS_TEXT("%sbegin %s\n"), spaces, OS::Core::Compiler::getExpName(type));
		out += list[0]->debugPrint(compiler, depth+1);
		out += StringInternal::format(allocator, OS_TEXT("%send %s\n"), spaces, OS::Core::Compiler::getExpName(type));
		break;
	*/

	case EXP_TYPE_PLUS:     // +
	case EXP_TYPE_NEG:     // -
	case EXP_TYPE_LOGIC_NOT:     // !
	case EXP_TYPE_BIN_NOT: // ~
	case EXP_TYPE_PRE_INC:     // ++
	case EXP_TYPE_PRE_DEC:     // --
	case EXP_TYPE_POST_INC:    // ++
	case EXP_TYPE_POST_DEC:    // --
		{
			OS_ASSERT(list.count == 1);
			const OS_CHAR * exp_name = OS::Core::Compiler::getExpName(type);
			out += StringInternal::format(allocator, OS_TEXT("%sbegin %s\n"), spaces, exp_name);
			out += list[0]->debugPrint(compiler, depth+1);
			out += StringInternal::format(allocator, OS_TEXT("%send %s\n"), spaces, exp_name);
			break;
		}

	/*
	case EXP_TYPE_QUESTION:
		{
			OS_ASSERT(list.count == 3);
			const OS_CHAR * exp_name = OS::Core::Compiler::getExpName(type);
			out += StringInternal::format(allocator, OS_TEXT("%sbegin %s\n"), spaces, exp_name);

			out += StringInternal::format(allocator, OS_TEXT("%s begin bool\n"), spaces);
			out += list[0]->debugPrint(compiler, depth+2);
			out += StringInternal::format(allocator, OS_TEXT("%s end bool\n"), spaces);

			out += StringInternal::format(allocator, OS_TEXT("%s begin then\n"), spaces);
			out += list[1]->debugPrint(compiler, depth+2);
			out += StringInternal::format(allocator, OS_TEXT("%s end then\n"), spaces);

			out += StringInternal::format(allocator, OS_TEXT("%s begin else\n"), spaces);
			out += list[2]->debugPrint(compiler, depth+2);
			out += StringInternal::format(allocator, OS_TEXT("%s end else\n"), spaces);

			out += StringInternal::format(allocator, OS_TEXT("%send %s\n"), spaces, exp_name);
			break;
		}
	*/

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
	case EXP_TYPE_BIN_AND: // &
	case EXP_TYPE_BIN_OR:  // |
	case EXP_TYPE_BIN_XOR: // ^
		// case EXP_TYPE_BIN_NOT: // ~
	case EXP_TYPE_BIN_AND_ASSIGN: // &=
	case EXP_TYPE_BIN_OR_ASSIGN:  // |=
	case EXP_TYPE_BIN_XOR_ASSIGN: // ^=
	case EXP_TYPE_BIN_NOT_ASSIGN: // ~=
	case EXP_TYPE_ADD: // +
	case EXP_TYPE_SUB: // -
	case EXP_TYPE_MUL: // *
	case EXP_TYPE_DIV: // /
	case EXP_TYPE_MOD: // %
	case EXP_TYPE_MUL_SHIFT: // <<
	case EXP_TYPE_DIV_SHIFT: // >>
	case EXP_TYPE_POW: // **

	case EXP_TYPE_DOT:
	case EXP_TYPE_CROSS:
	case EXP_TYPE_SWAP:
	case EXP_TYPE_AS:
	case EXP_TYPE_IS:
		// case EXP_TYPE_IN:

	case EXP_TYPE_ADD_ASSIGN: // +=
	case EXP_TYPE_SUB_ASSIGN: // -=
	case EXP_TYPE_MUL_ASSIGN: // *=
	case EXP_TYPE_DIV_ASSIGN: // /=
	case EXP_TYPE_MOD_ASSIGN: // %=
	case EXP_TYPE_MUL_SHIFT_ASSIGN: // <<=
	case EXP_TYPE_DIV_SHIFT_ASSIGN: // >>=
	case EXP_TYPE_POW_ASSIGN: // **=
		{
			OS_ASSERT(list.count == 2);
			const OS_CHAR * exp_name = OS::Core::Compiler::getExpName(type);
			out += StringInternal::format(allocator, OS_TEXT("%sbegin %s\n"), spaces, exp_name);
			out += list[0]->debugPrint(compiler, depth+1);
			out += list[1]->debugPrint(compiler, depth+1);
			out += StringInternal::format(allocator, OS_TEXT("%send %s\n"), spaces, exp_name);
			break;
		}

	case EXP_TYPE_NEW_LOCAL_VAR:
		{
			OS_ASSERT(list.count == 0);
			StringInternal info = StringInternal::format(allocator, OS_TEXT("(%d %d%s)"),
				local_var.index, local_var.up_count, 
				local_var.is_param ? OS_TEXT(" param") : OS_TEXT(""));
			out += StringInternal::format(allocator, OS_TEXT("%snew local var %s %s\n"), spaces, token->str.toChar(), info.toChar());
			break;
		}

	case EXP_TYPE_GET_THIS:
	case EXP_TYPE_GET_ARGUMENTS:
	case EXP_TYPE_GET_REST_ARGUMENTS:
		{
			OS_ASSERT(list.count == 0);
			const OS_CHAR * exp_name = OS::Core::Compiler::getExpName(type);
			out += StringInternal::format(allocator, OS_TEXT("%s%s\n"), spaces, exp_name);
			break;
		}

	case EXP_TYPE_GET_LOCAL_VAR:
		{
			OS_ASSERT(list.count == 0);
			const OS_CHAR * exp_name = OS::Core::Compiler::getExpName(type);
			StringInternal info = StringInternal::format(allocator, OS_TEXT("(%d %d%s)"),
				local_var.index, local_var.up_count, 
				local_var.is_param ? OS_TEXT(" param") : OS_TEXT(""));
			out += StringInternal::format(allocator, OS_TEXT("%s%s %s %s\n"), spaces, exp_name, token->str.toChar(), info.toChar());
			break;
		}

	case EXP_TYPE_GET_AUTO_VAR:
		{
			OS_ASSERT(list.count == 0);
			const OS_CHAR * exp_name = OS::Core::Compiler::getExpName(type);
			out += StringInternal::format(allocator, OS_TEXT("%s%s %s\n"), spaces, exp_name, token->str.toChar());
			break;
		}

	case EXP_TYPE_SET_LOCAL_VAR:
		{
			OS_ASSERT(list.count == 1);
			const OS_CHAR * exp_name = OS::Core::Compiler::getExpName(type);
			StringInternal info = StringInternal::format(allocator, OS_TEXT("(%d %d%s)"),
				local_var.index, local_var.up_count, 
				local_var.is_param ? OS_TEXT(" param") : OS_TEXT(""));
			out += StringInternal::format(allocator, OS_TEXT("%sbegin %s\n"), spaces, exp_name);
			out += list[0]->debugPrint(compiler, depth+1);
			out += StringInternal::format(allocator, OS_TEXT("%send %s %s %s\n"), spaces, exp_name, token->str.toChar(), info.toChar());
			break;
		}

	case EXP_TYPE_SET_AUTO_VAR:
		{
			OS_ASSERT(list.count == 1);
			const OS_CHAR * exp_name = OS::Core::Compiler::getExpName(type);
			out += StringInternal::format(allocator, OS_TEXT("%sbegin %s\n"), spaces, exp_name);
			out += list[0]->debugPrint(compiler, depth+1);
			out += StringInternal::format(allocator, OS_TEXT("%send %s %s\n"), spaces, exp_name, token->str.toChar());
			break;
		}

	case EXP_TYPE_SET_PROPERTY:
	case EXP_TYPE_SET_PROPERTY_DIM:
	case EXP_TYPE_SET_DIM:
		{
			OS_ASSERT(list.count == 3);
			const OS_CHAR * exp_name = OS::Core::Compiler::getExpName(type);
			out += StringInternal::format(allocator, OS_TEXT("%sbegin %s\n"), spaces, exp_name);
			out += list[0]->debugPrint(compiler, depth+1);
			out += list[1]->debugPrint(compiler, depth+1);
			out += list[2]->debugPrint(compiler, depth+1);
			out += StringInternal::format(allocator, OS_TEXT("%send %s ret values %d\n"), spaces, exp_name, ret_values);
			break;
		}

	}
	return out;
}

// =====================================================================

int OS::Core::Compiler::cacheString(const StringInternal& str)
{
	VariableIndex index(str, VariableIndex::NoFixStringIndex());
	Value::Variable * var = prog_strings_table->get(index);
	if(var){
		OS_ASSERT((OS_INT)(int)var->int_index == var->int_index);
		OS_ASSERT(var->int_index >= 0 && var->int_index <= 0xffff);
		return (int)var->int_index;
	}
	Value * value = allocator->core->pushNumberValue(prog_strings_table->count);
	var = new (malloc(sizeof(Value::Variable))) Value::Variable(index);
	var->value_id = value->value_id;
	value->ref_count++;
	allocator->core->addTableVariable(prog_strings_table, var);
	allocator->vectorAddItem(prog_strings, str);
	allocator->pop();
	OS_ASSERT(prog_strings_table->count == prog_strings.count);
	return prog_strings_table->count-1;
}

int OS::Core::Compiler::cacheNumber(OS_FLOAT num)
{
	VariableIndex index(allocator, num);
	Value::Variable * var = prog_numbers_table->get(index);
	if(var){
		OS_ASSERT((OS_INT)(int)var->int_index == var->int_index);
		OS_ASSERT(var->int_index >= 0 && var->int_index <= 0xffff);
		return (int)var->int_index;
	}
	Value * value = allocator->core->pushNumberValue(prog_numbers_table->count);
	var = new (malloc(sizeof(Value::Variable))) Value::Variable(index);
	var->value_id = value->value_id;
	value->ref_count++;
	allocator->core->addTableVariable(prog_numbers_table, var);
	allocator->vectorAddItem(prog_numbers, num);
	allocator->pop();
	OS_ASSERT(prog_numbers_table->count == prog_numbers.count);
	return prog_numbers_table->count-1;
}

bool OS::Core::Compiler::writeOpcodes(ExpressionList& list)
{
	for(int i = 0; i < list.count; i++){
		if(!writeOpcodes(list[i])){
			return false;
		}
	}
	return true;
}

bool OS::Core::Compiler::writeOpcodes(Expression * exp)
{
	int i;
	switch(exp->type){
	default:
		{
			ExpressionType exp_type = exp->type;
			OS_ASSERT(false);
			break;
		}

	case EXP_TYPE_NOP:
		break;

	case EXP_TYPE_CODE_LIST:
		if(!writeOpcodes(exp->list)){
			return false;
		}
		break;

	case EXP_TYPE_CONST_NUMBER:
		OS_ASSERT(exp->list.count == 0);
		prog_opcodes->writeByte(Program::OP_PUSH_NUMBER);
		prog_opcodes->writeU16(cacheNumber(exp->token->getFloat()));
		break;

	case EXP_TYPE_CONST_STRING:
		OS_ASSERT(exp->list.count == 0);
		prog_opcodes->writeByte(Program::OP_PUSH_STRING);
		prog_opcodes->writeU16(cacheString(exp->token->str));
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
#if 1
			int func_index = prog_functions.indexOf(scope);
			OS_ASSERT(func_index >= 0);
			prog_opcodes->writeU16(func_index);

			allocator->vectorReserveCapacity(scope->locals_compiled, scope->num_locals);
			scope->locals_compiled.count = scope->num_locals;

			scope->opcodes_pos = prog_opcodes->buffer.count;
			if(!writeOpcodes(exp->list)){
				return false;
			}
			scope->opcodes_size = prog_opcodes->buffer.count - scope->opcodes_pos;

			for(i = 0; i < scope->locals.count; i++){
				Scope::LocalVar& var = scope->locals[i];
				Scope::LocalVarCompiled& var_scope = scope->locals_compiled[var.index];
				var_scope.cached_name_index = cacheString(var.name);
				var_scope.start_code_pos = scope->opcodes_pos;
				var_scope.end_code_pos = prog_opcodes->buffer.count;
			}
#else
			int func_size_pos = prog_opcodes->buffer.count;
			prog_opcodes->writeInt32(0); // func size
			prog_opcodes->writeInt32(0); // func opcodes size
			
			prog_opcodes->writeByte(scope->num_locals);
			prog_opcodes->writeByte(scope->num_params);

			allocator->vectorReserveCapacity(scope->locals_compiled, scope->num_locals);
			scope->locals_compiled.count = scope->num_locals;

			int func_opcodes_pos = prog_opcodes->buffer.count;
			if(!writeOpcodes(exp->list)){
				return false;
			}
			int opcodes_size = prog_opcodes->buffer.count - func_opcodes_pos;
			for(i = 0; i < scope->locals.count; i++){
				Scope::LocalVar& var = scope->locals[i];
				Scope::LocalVarCompiled& var_scope = scope->locals_compiled[var.index];
				var_scope.cached_name_index = cacheString(var.name);
				var_scope.start_code_pos = func_size_pos;
				var_scope.end_code_pos = prog_opcodes->buffer.count;
			}
			for(i = 0; i < scope->locals_compiled.count; i++){
				Scope::LocalVarCompiled& var_scope = scope->locals_compiled[i];
				OS_ASSERT(var_scope.start_code_pos >= 0);
				OS_ASSERT(var_scope.end_code_pos >= 0);
				prog_opcodes->writeU16(var_scope.cached_name_index);
				prog_opcodes->writeInt32(var_scope.start_code_pos);
				prog_opcodes->writeInt32(var_scope.end_code_pos);
			}

			prog_opcodes->writeInt32AtPos(prog_opcodes->buffer.count - func_size_pos, func_size_pos);
			prog_opcodes->writeInt32AtPos(opcodes_size, func_size_pos + sizeof(OS_INT32));
#endif
			break;
		}

	case EXP_TYPE_SCOPE:
		{
			Scope * scope = dynamic_cast<Scope*>(exp);
			OS_ASSERT(scope);
			int start_code_pos = prog_opcodes->buffer.count;
			if(!writeOpcodes(exp->list)){
				return false;
			}
			for(i = 0; i < scope->locals.count; i++){
				Scope::LocalVar& var = scope->locals[i];
				Scope::LocalVarCompiled& var_scope = scope->function->locals_compiled[var.index];
				var_scope.cached_name_index = cacheString(var.name);
				var_scope.start_code_pos = start_code_pos;
				var_scope.end_code_pos = prog_opcodes->buffer.count;
			}
			break;
		}

	case EXP_TYPE_OBJECT:
		// OS_ASSERT(exp->list.count >= 0);
		prog_opcodes->writeByte(Program::OP_PUSH_NEW_OBJECT);
		if(!writeOpcodes(exp->list)){
			return false;
		}
		break;

	case EXP_TYPE_OBJECT_SET_BY_AUTO_INDEX:
		OS_ASSERT(exp->list.count == 1);
		if(!writeOpcodes(exp->list)){
			return false;
		}
		prog_opcodes->writeByte(Program::OP_OBJECT_SET_BY_AUTO_INDEX);
		break;

	case EXP_TYPE_OBJECT_SET_BY_EXP:
		OS_ASSERT(exp->list.count == 2);
		if(!writeOpcodes(exp->list)){
			return false;
		}
		prog_opcodes->writeByte(Program::OP_OBJECT_SET_BY_EXP);
		break;

	case EXP_TYPE_OBJECT_SET_BY_INDEX:
		OS_ASSERT(exp->list.count == 1);
		if(!writeOpcodes(exp->list)){
			return false;
		}
		prog_opcodes->writeByte(Program::OP_OBJECT_SET_BY_INDEX);
		// prog_opcodes->writeInt64(exp->token->getInt());
		prog_opcodes->writeU16(cacheNumber(exp->token->getFloat()));
		break;

	case EXP_TYPE_OBJECT_SET_BY_NAME:
		OS_ASSERT(exp->list.count == 1);
		if(!writeOpcodes(exp->list)){
			return false;
		}
		prog_opcodes->writeByte(Program::OP_OBJECT_SET_BY_NAME);
		prog_opcodes->writeU16(cacheString(exp->token->str));
		break;

	case EXP_TYPE_GET_AUTO_VAR:
		OS_ASSERT(exp->list.count == 0);
		prog_opcodes->writeByte(Program::OP_PUSH_AUTO_VAR);
		prog_opcodes->writeU16(cacheString(exp->token->str));
		break;

	case EXP_TYPE_SET_AUTO_VAR:
		OS_ASSERT(exp->list.count > 0);
		if(!writeOpcodes(exp->list)){
			return false;
		}
		prog_opcodes->writeByte(Program::OP_SET_AUTO_VAR);
		prog_opcodes->writeU16(cacheString(exp->token->str));
		break;

	case EXP_TYPE_GET_THIS:
	case EXP_TYPE_GET_ARGUMENTS:
	case EXP_TYPE_GET_REST_ARGUMENTS:
		OS_ASSERT(exp->list.count == 0);
		prog_opcodes->writeByte(Program::toOpcodeType(exp->type));

	case EXP_TYPE_GET_LOCAL_VAR:
		OS_ASSERT(exp->list.count == 0);
		prog_opcodes->writeByte(Program::OP_PUSH_LOCAL_VAR);
		prog_opcodes->writeByte(exp->local_var.index);
		prog_opcodes->writeByte(exp->local_var.up_count);
		break;

	case EXP_TYPE_SET_LOCAL_VAR:
		OS_ASSERT(exp->list.count > 0);
		if(!writeOpcodes(exp->list)){
			return false;
		}
		prog_opcodes->writeByte(Program::OP_SET_LOCAL_VAR);
		prog_opcodes->writeByte(exp->local_var.index);
		prog_opcodes->writeByte(exp->local_var.up_count);
		break;

	case EXP_TYPE_CALL:
	case EXP_TYPE_GET_DIM:
	case EXP_TYPE_CALL_PROPERTY:
	case EXP_TYPE_TAIL_CALL:
		OS_ASSERT(exp->list.count == 2);
		OS_ASSERT(exp->list[1]->type == EXP_TYPE_PARAMS);
		if(!writeOpcodes(exp->list)){
			return false;
		}
		prog_opcodes->writeByte(Program::toOpcodeType(exp->type));
		prog_opcodes->writeByte(exp->list[1]->ret_values); // params number
		prog_opcodes->writeByte(exp->ret_values);
		break;

	case EXP_TYPE_GET_PROPERTY:
		OS_ASSERT(exp->list.count == 2);
		if(!writeOpcodes(exp->list)){
			return false;
		}
		prog_opcodes->writeByte(Program::OP_GET_PROPERTY);
		break;

	case EXP_TYPE_SET_PROPERTY:
		OS_ASSERT(exp->list.count == 3);
		if(!writeOpcodes(exp->list)){
			return false;
		}
		prog_opcodes->writeByte(Program::OP_SET_PROPERTY);
		prog_opcodes->writeByte(exp->ret_values);
		break;

	case EXP_TYPE_PARAMS:
		if(!writeOpcodes(exp->list)){
			return false;
		}
		break;

	case EXP_TYPE_RETURN:
		if(!writeOpcodes(exp->list)){
			return false;
		}
		prog_opcodes->writeByte(Program::OP_RETURN);
		prog_opcodes->writeByte(exp->ret_values);
		break;

	case EXP_TYPE_POP_VALUE:
		OS_ASSERT(exp->list.count == 1);
		if(!writeOpcodes(exp->list)){
			return false;
		}
		prog_opcodes->writeByte(Program::OP_POP);
		break;

	case EXP_TYPE_CONCAT:

	case EXP_TYPE_LOGIC_AND:
	case EXP_TYPE_LOGIC_OR:
	case EXP_TYPE_LOGIC_PTR_EQ:
	case EXP_TYPE_LOGIC_PTR_NE:
	case EXP_TYPE_LOGIC_EQ:
	case EXP_TYPE_LOGIC_NE:
	case EXP_TYPE_LOGIC_GE:
	case EXP_TYPE_LOGIC_LE:
	case EXP_TYPE_LOGIC_GREATER:
	case EXP_TYPE_LOGIC_LESS:
	case EXP_TYPE_LOGIC_NOT:

	case EXP_TYPE_BIN_AND:
	case EXP_TYPE_BIN_OR:
	case EXP_TYPE_BIN_XOR:
	case EXP_TYPE_BIN_NOT:

	case EXP_TYPE_ADD:
	case EXP_TYPE_SUB:
	case EXP_TYPE_MUL:
	case EXP_TYPE_DIV:
	case EXP_TYPE_MOD:
	case EXP_TYPE_MUL_SHIFT:
	case EXP_TYPE_DIV_SHIFT:
	case EXP_TYPE_POW:
		// OS_ASSERT(exp->list.count == 1);
		if(!writeOpcodes(exp->list)){
			return false;
		}
		prog_opcodes->writeByte(Program::toOpcodeType(exp->type));
		break;
	}
	return true;
}

// =====================================================================
/*
OS::Core::Compiler::VarAssingExpression::VarAssingExpression(const StringInternal& p_var_name, Expression * p_value_exp)
	: Expression(EXP_VAR_ASSING), var_name(p_var_name)
{
	value_exp = p_value_exp;
}

OS::Core::Compiler::VarAssingExpression::~VarAssingExpression()
{
	var_name.getAllocator()->deleteObj(value_exp);
}
*/
// =====================================================================

OS::Core::Compiler::Scope::Scope(Scope * p_parent, ExpressionType type, TokenData * token): Expression(type, token)
{
	OS_ASSERT(type == EXP_TYPE_FUNCTION || type == EXP_TYPE_SCOPE);
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
}

OS::Core::Compiler::Scope::LocalVar::LocalVar(const StringInternal& p_name, int p_index): name(p_name)
{
	index = p_index;
}

OS::Core::Compiler::Scope::LocalVarCompiled::LocalVarCompiled()
{
	cached_name_index = -1;
	start_code_pos = -1;
	end_code_pos = -1;
}

void OS::Core::Compiler::Scope::addLocalVar(const StringInternal& name)
{
	OS * allocator = getAllocator();
	LocalVar local_var(name, function->num_locals);
	allocator->vectorAddItem(locals, local_var);
	function->num_locals++;
	/* if(function != this){
		allocator->vectorAddItem(locals, local_var);
	} */
}

void OS::Core::Compiler::Scope::addLocalVar(const StringInternal& name, LocalVarDesc& local_var)
{
	local_var.index = function->num_locals;
	local_var.up_count = 0;
	local_var.is_param = false;
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
	prog_numbers_table = NULL;
	prog_opcodes = NULL;
	prog_max_up_count = 0;
}

OS::Core::Compiler::~Compiler()
{
	if(recent_printed_text_data){
		recent_printed_text_data->release();
	}
	/* if(prog){
		prog->release();
		prog = NULL;
	} */
	if(prog_numbers_table){
		allocator->core->deleteTable(prog_numbers_table);
		prog_numbers_table = NULL;
	}
	if(prog_strings_table){
		allocator->core->deleteTable(prog_strings_table);
		prog_strings_table = NULL;
	}
	allocator->vectorClear(prog_numbers);
	allocator->vectorClear(prog_strings);
	allocator->vectorClear(prog_functions);
	allocator->deleteObj(prog_opcodes);
}

bool OS::Core::Compiler::compile()
{
	OS_ASSERT(!prog_opcodes && !prog_strings_table && !prog_numbers_table);
	OS_ASSERT(!prog_functions.count && !prog_numbers.count && !prog_strings.count);
	
	Scope * scope = NULL;
	if(!readToken()){
		setError(ERROR_EXPECT_TOKEN, recent_token);
	}else{
		scope = expectTextExpression();
	}
	if(scope){
		Expression * exp = processExpressionSecondPass(scope, scope);
		OS_ASSERT(exp->type == EXP_TYPE_FUNCTION);

		OS::Core::StringInternal dump = exp->debugPrint(this, 0);
		FileStreamWriter(allocator, "test-data/debug-exp-dump.txt").writeBytes(dump.toChar(), dump.getDataSize());

		prog_strings_table = allocator->core->newTable();
		prog_numbers_table = allocator->core->newTable();
		prog_opcodes = new (malloc(sizeof(MemStreamWriter))) MemStreamWriter(allocator);

		if(!writeOpcodes(exp)){
		
		}

		MemStreamWriter mem_writer(allocator);
		saveToStream(mem_writer);

		// TODO: save binary formal of compiled program if needed
		FileStreamWriter(allocator, "test-data/test.osb").writeBytes(mem_writer.buffer.buf, mem_writer.buffer.count);
		
		Program * prog = new (malloc(sizeof(Program))) Program(allocator);
		prog->filename = tokenizer->getTextData()->filename;
#if 1
		prog->loadFromStream(MemStreamReader(NULL, mem_writer.buffer.buf, mem_writer.buffer.count));
#else
		prog->loadFromStream(FileStreamReader(allocator, "test-data/test.osb"));
#endif
		prog->pushFunction();
		prog->release();
		
		allocator->deleteObj(exp);

		return true;
	}else{
		OS::Core::StringInternal dump = OS::Core::StringInternal(allocator, "Error");
		switch(error){
		default:
			dump += OS::Core::StringInternal(allocator, " unknown");
			break;

		case ERROR_SYNTAX:
			dump += OS::Core::StringInternal(allocator, " SYNTAX");
			break;

		case ERROR_VAR_ALREADY_EXIST:
			dump += OS::Core::StringInternal(allocator, " VAR_ALREADY_EXIST");
			break;

		case ERROR_EXPECT_TOKEN_TYPE:
			dump += OS::Core::StringInternal(allocator, " EXPECT_TOKEN_TYPE ");
			dump += OS::Core::StringInternal(allocator, Tokenizer::getTokenTypeName(expect_token_type));
			break;

		case ERROR_EXPECT_TOKEN_STR:
			dump += OS::Core::StringInternal(allocator, " EXPECT_TOKEN_STR ");
			dump += expect_token;
			break;

		case ERROR_EXPECT_TOKEN:
			dump += OS::Core::StringInternal(allocator, " EXPECT_TOKEN");
			break;

		case ERROR_EXPECT_VALUE:
			dump += OS::Core::StringInternal(allocator, " EXPECT_VALUE");
			break;

		case ERROR_EXPECT_WRITEABLE:
			dump += OS::Core::StringInternal(allocator, " EXPECT_WRITEABLE");
			break;

		case ERROR_EXPECT_EXPRESSION:
			dump += OS::Core::StringInternal(allocator, " EXPECT_EXPRESSION");
			break;

		case ERROR_EXPECT_FUNCTION_SCOPE:
			dump += OS::Core::StringInternal(allocator, " EXPECT_FUNCTION_SCOPE");
			break;

		case ERROR_EXPECT_SWITCH_SCOPE:
			dump += OS::Core::StringInternal(allocator, " EXPECT_SWITCH_SCOPE");
			break;

		case ERROR_FINISH_BIN_OP:
			dump += OS::Core::StringInternal(allocator, " FINISH_BIN_OP");
			break;

		case ERROR_FINISH_UNARY_OP:
			dump += OS::Core::StringInternal(allocator, " FINISH_UNARY_OP");
			break;
		}
		dump += OS::String(allocator, "\n");
		if(error_token){
			if(error_token->text_data->filename.getDataSize() > 0){
				dump += OS::Core::StringInternal::format(allocator, "filename %s\n", error_token->text_data->filename.toChar());
			}
			dump += OS::Core::StringInternal::format(allocator, "[%d] %s\n", error_token->line+1, error_token->text_data->lines[error_token->line].toChar());
			dump += OS::Core::StringInternal::format(allocator, "pos %d, token: %s\n", error_token->pos+1, error_token->str.toChar());
		}
		FileStreamWriter(allocator, "test-data/debug-exp-dump.txt").writeBytes(dump.toChar(), dump.getDataSize());
		
		allocator->core->pushNullValue();
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

void OS::Core::Compiler::setError(const StringInternal& str, TokenData * error_token)
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

OS::Core::Compiler::ExpressionType OS::Core::Compiler::toExpressionType(TokenType token_type)
{
	switch(token_type){
	case Tokenizer::OPERATOR_INDIRECT: return EXP_TYPE_INDIRECT;

	case Tokenizer::OPERATOR_CONCAT: return EXP_TYPE_CONCAT;

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

	case Tokenizer::OPERATOR_BIN_AND: return EXP_TYPE_BIN_AND;
	case Tokenizer::OPERATOR_BIN_OR: return EXP_TYPE_BIN_OR;
	case Tokenizer::OPERATOR_BIN_XOR: return EXP_TYPE_BIN_XOR;
	case Tokenizer::OPERATOR_BIN_NOT: return EXP_TYPE_BIN_NOT;
	case Tokenizer::OPERATOR_ADD: return EXP_TYPE_ADD;
	case Tokenizer::OPERATOR_SUB: return EXP_TYPE_SUB;
	case Tokenizer::OPERATOR_MUL: return EXP_TYPE_MUL;
	case Tokenizer::OPERATOR_DIV: return EXP_TYPE_DIV;
	case Tokenizer::OPERATOR_MOD: return EXP_TYPE_MOD;
	case Tokenizer::OPERATOR_MUL_SHIFT: return EXP_TYPE_MUL_SHIFT;
	case Tokenizer::OPERATOR_DIV_SHIFT: return EXP_TYPE_DIV_SHIFT;
	case Tokenizer::OPERATOR_POW: return EXP_TYPE_POW;

	case Tokenizer::OPERATOR_DOT: return EXP_TYPE_DOT;
	case Tokenizer::OPERATOR_CROSS: return EXP_TYPE_CROSS;
	case Tokenizer::OPERATOR_SWAP: return EXP_TYPE_SWAP;
	case Tokenizer::OPERATOR_IS: return EXP_TYPE_IS;
	case Tokenizer::OPERATOR_AS: return EXP_TYPE_AS;
	case Tokenizer::OPERATOR_IN: return EXP_TYPE_IN;

	case Tokenizer::OPERATOR_BIN_AND_ASSIGN: return EXP_TYPE_BIN_AND_ASSIGN;
	case Tokenizer::OPERATOR_BIN_OR_ASSIGN: return EXP_TYPE_BIN_OR_ASSIGN;
	case Tokenizer::OPERATOR_BIN_XOR_ASSIGN: return EXP_TYPE_BIN_XOR_ASSIGN;
	case Tokenizer::OPERATOR_BIN_NOT_ASSIGN: return EXP_TYPE_BIN_NOT_ASSIGN;
	case Tokenizer::OPERATOR_ADD_ASSIGN: return EXP_TYPE_ADD_ASSIGN;
	case Tokenizer::OPERATOR_SUB_ASSIGN: return EXP_TYPE_SUB_ASSIGN;
	case Tokenizer::OPERATOR_MUL_ASSIGN: return EXP_TYPE_MUL_ASSIGN;
	case Tokenizer::OPERATOR_DIV_ASSIGN: return EXP_TYPE_DIV_ASSIGN;
	case Tokenizer::OPERATOR_MOD_ASSIGN: return EXP_TYPE_MOD_ASSIGN;
	case Tokenizer::OPERATOR_MUL_SHIFT_ASSIGN: return EXP_TYPE_MUL_SHIFT_ASSIGN;
	case Tokenizer::OPERATOR_DIV_SHIFT_ASSIGN: return EXP_TYPE_DIV_SHIFT_ASSIGN;
	case Tokenizer::OPERATOR_POW_ASSIGN: return EXP_TYPE_POW_ASSIGN;

	case Tokenizer::OPERATOR_ASSIGN: return EXP_TYPE_ASSIGN;
	}
	return EXP_TYPE_UNKNOWN;
}

OS::Core::Compiler::OpcodeLevel OS::Core::Compiler::toOpcodeLevel(ExpressionType exp_type)
{
	switch(exp_type){
	// case EXP_TYPE_NOP:

	/*
	case EXP_TYPE_CODE_LIST:
		return OP_LEVEL_10;

	// case EXP_TYPE_CODE_SCOPE:
	// 	return OP_LEVEL_10;

	case EXP_TYPE_CONST_NUMBER:
	case EXP_TYPE_CONST_STRING:
	case EXP_TYPE_CONST_NULL:
	case EXP_TYPE_CONST_TRUE:
	case EXP_TYPE_CONST_FALSE:
		return OP_LEVEL_10;
	
	case EXP_TYPE_NAME:
		return OP_LEVEL_10;

	case EXP_TYPE_PARAMS:
		return OP_LEVEL_10;

	case EXP_TYPE_ARRAY:
		return OP_LEVEL_10;

	case EXP_TYPE_CALL:
		return OP_LEVEL_10;

	case EXP_TYPE_CALL_DIM:
		return OP_LEVEL_10;

		// case EXP_TYPE_RETURN:

	case EXP_TYPE_THREAD:
		return OP_LEVEL_10;
	*/

	case EXP_TYPE_INDIRECT:
		return OP_LEVEL_10;

	case EXP_TYPE_ASSIGN:
		return OP_LEVEL_3;

	case EXP_TYPE_LOGIC_AND: // &&
		return OP_LEVEL_1;

	case EXP_TYPE_LOGIC_OR:  // ||
		return OP_LEVEL_0;

	case EXP_TYPE_LOGIC_PTR_EQ:  // ===
		return OP_LEVEL_2;

	case EXP_TYPE_LOGIC_PTR_NE:  // !==
		return OP_LEVEL_2;

	case EXP_TYPE_LOGIC_EQ:  // ==
		return OP_LEVEL_2;

	case EXP_TYPE_LOGIC_NE:  // !=
		return OP_LEVEL_2;

	case EXP_TYPE_LOGIC_GE:  // >=
		return OP_LEVEL_2;

	case EXP_TYPE_LOGIC_LE:  // <=
		return OP_LEVEL_2;

	case EXP_TYPE_LOGIC_GREATER: // >
		return OP_LEVEL_2;

	case EXP_TYPE_LOGIC_LESS:    // <
		return OP_LEVEL_2;

	case EXP_TYPE_LOGIC_NOT:     // !
		return OP_LEVEL_9;

	case EXP_TYPE_PLUS:
		return OP_LEVEL_9;

	case EXP_TYPE_NEG:
		return OP_LEVEL_9;

	/*
	case EXP_TYPE_INC:     // ++
	return OP_LEVEL_9;

	case EXP_TYPE_DEC:     // --
	return OP_LEVEL_9;
	*/

	case EXP_TYPE_PRE_INC:     // ++
		return OP_LEVEL_9;

	case EXP_TYPE_PRE_DEC:     // --
		return OP_LEVEL_9;

	case EXP_TYPE_POST_INC:    // ++
		return OP_LEVEL_9;

	case EXP_TYPE_POST_DEC:    // --
		return OP_LEVEL_9;

	// case EXP_TYPE_QUESTION:    // ?
	// 	return OP_LEVEL_9;

	case EXP_TYPE_BIN_AND: // &
		return OP_LEVEL_5;

	case EXP_TYPE_BIN_OR:  // |
		return OP_LEVEL_4;

	case EXP_TYPE_BIN_XOR: // ^
		return OP_LEVEL_5;

	case EXP_TYPE_BIN_NOT: // ~
		return OP_LEVEL_9;

	case EXP_TYPE_BIN_AND_ASSIGN: // &=
		return OP_LEVEL_3;

	case EXP_TYPE_BIN_OR_ASSIGN:  // |=
		return OP_LEVEL_3;

	case EXP_TYPE_BIN_XOR_ASSIGN: // ^=
		return OP_LEVEL_3;

	case EXP_TYPE_BIN_NOT_ASSIGN: // ~=
		return OP_LEVEL_3;

	case EXP_TYPE_CONCAT: // ..
		return OP_LEVEL_5;

	case EXP_TYPE_ADD: // +
		return OP_LEVEL_6;

	case EXP_TYPE_SUB: // -
		return OP_LEVEL_6;

	case EXP_TYPE_MUL: // *
		return OP_LEVEL_7;

	case EXP_TYPE_DIV: // /
		return OP_LEVEL_7;

	case EXP_TYPE_MOD: // %
		return OP_LEVEL_7;

	case EXP_TYPE_MUL_SHIFT: // <<
		return OP_LEVEL_7;

	case EXP_TYPE_DIV_SHIFT: // >>
		return OP_LEVEL_7;

	case EXP_TYPE_POW: // **
		return OP_LEVEL_8;

	case EXP_TYPE_DOT:
	case EXP_TYPE_CROSS:
		return OP_LEVEL_7;

	case EXP_TYPE_SWAP:
	case EXP_TYPE_AS:
	case EXP_TYPE_IS:
	case EXP_TYPE_IN:
		return OP_LEVEL_9;

	case EXP_TYPE_ADD_ASSIGN: // +=
		return OP_LEVEL_3;

	case EXP_TYPE_SUB_ASSIGN: // -=
		return OP_LEVEL_3;

	case EXP_TYPE_MUL_ASSIGN: // *=
		return OP_LEVEL_3;

	case EXP_TYPE_DIV_ASSIGN: // /=
		return OP_LEVEL_3;

	case EXP_TYPE_MOD_ASSIGN: // %=
		return OP_LEVEL_3;

	case EXP_TYPE_MUL_SHIFT_ASSIGN: // <<=
		return OP_LEVEL_3;

	case EXP_TYPE_DIV_SHIFT_ASSIGN: // >>=
		return OP_LEVEL_3;

	case EXP_TYPE_POW_ASSIGN: // **=
		return OP_LEVEL_3;
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

	TokenData * token = new (malloc(sizeof(TokenData))) TokenData(recent_token->text_data, StringInternal(allocator), token_type, recent_token->line, recent_token->pos);
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
	case EXP_TYPE_CALL_DIM:
	case EXP_TYPE_GET_DIM:
	case EXP_TYPE_CALL_PROPERTY:
	case EXP_TYPE_GET_PROPERTY:
	case EXP_TYPE_GET_PROPERTY_DIM:
	case EXP_TYPE_INDIRECT:
	case EXP_TYPE_GET_AUTO_VAR_DIM:
	case EXP_TYPE_TAIL_CALL: // ret values are not used for tail call
		exp->ret_values = ret_values;
		return exp;

	case EXP_TYPE_CODE_LIST:
		if(exp->list.count > 0){
			Expression * last_exp = exp->list[exp->list.count-1];
			switch(last_exp->type){
			case EXP_TYPE_CALL:
			case EXP_TYPE_CALL_DIM:
			case EXP_TYPE_GET_DIM:
			case EXP_TYPE_CALL_PROPERTY:
			case EXP_TYPE_GET_PROPERTY:
			case EXP_TYPE_GET_PROPERTY_DIM:
			case EXP_TYPE_INDIRECT:
			case EXP_TYPE_GET_AUTO_VAR_DIM:
			case EXP_TYPE_TAIL_CALL: // ret values are not used for tail call
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
				case EXP_TYPE_CALL_DIM:
				case EXP_TYPE_GET_DIM:
				case EXP_TYPE_CALL_PROPERTY:
				case EXP_TYPE_GET_PROPERTY:
				case EXP_TYPE_GET_PROPERTY_DIM:
				case EXP_TYPE_GET_AUTO_VAR_DIM:
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
	}
	// int active_locals = exp->active_locals;
	while(exp->ret_values > ret_values){
		int new_ret_values = exp->ret_values-1;
		exp = new (malloc(sizeof(Expression))) Expression(EXP_TYPE_POP_VALUE, exp->token, exp);
		exp->ret_values = new_ret_values;
		// exp->active_locals = active_locals;
	}
	if(exp->ret_values < ret_values){
		if(exp->type != EXP_TYPE_PARAMS){
			exp = new (malloc(sizeof(Expression))) Expression(EXP_TYPE_PARAMS, exp->token, exp);
			exp->ret_values = exp->list[0]->ret_values;
			// exp->active_locals = active_locals;
		}
		while(exp->ret_values < ret_values){
			Expression * null_exp = new (malloc(sizeof(Expression))) Expression(EXP_TYPE_CONST_NULL, exp->token);
			null_exp->ret_values = 1;
			// null_exp->active_locals = active_locals;
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
	case EXP_TYPE_CALL_DIM:
	case EXP_TYPE_GET_DIM:
	case EXP_TYPE_CALL_PROPERTY:
	case EXP_TYPE_GET_PROPERTY:
	case EXP_TYPE_GET_PROPERTY_DIM:
	case EXP_TYPE_GET_AUTO_VAR_DIM:
	case EXP_TYPE_INDIRECT:
		{
			// int active_locals = exp->active_locals;
			exp = new (malloc(sizeof(Expression))) Expression(EXP_TYPE_VALUE, exp->token, exp);
			exp->ret_values = 1;
			// exp->active_locals = active_locals;
			break;
		}
	}
	return exp;
}

OS::Core::Compiler::Expression * OS::Core::Compiler::newExpressionFromList(ExpressionList& list, int ret_values)
{
	Expression * exp;
	// DeleteSeparators(list);
	/* if(!list.count){
		if(allow_auto_nop){
			return new (malloc(sizeof(Expression))) Expression(EXP_TYPE_NOP, recent_token);
		}
		setError(ERROR_EXPECT_EXPRESSION, recent_token);
		return NULL;
	} */
	// deleteNops(list);
	if(list.count == 1){
		exp = list[0];
		list.removeIndex(0);
	}else if(list.count == 0){
		TokenData * cur_token = ungetToken();
		readToken();
		exp = new (malloc(sizeof(Expression))) Expression(EXP_TYPE_CODE_LIST, cur_token);
	}else{
		/* deleteNops(list);
		if(!list.count){
			return new (malloc(sizeof(Expression))) Expression(EXP_TYPE_NOP, recent_token);
		} */
		int i;
		for(i = 0; i < list.count-1; i++){
			OS_ASSERT(list[i]->type != EXP_TYPE_CODE_LIST);
			list[i] = expectExpressionValues(list[i], 0);
		}
		exp = new (malloc(sizeof(Expression))) Expression(EXP_TYPE_CODE_LIST, list[0]->token);
		exp->list.swap(list);
		// exp->ret_values = 0;
		// exp->active_locals = list[list.count-1]->active_locals;
		/* for(i = 0; i < exp->list.count; i++){
			exp->ret_values += exp->list[i]->ret_values;
		} */
		exp->ret_values = exp->list[exp->list.count-1]->ret_values;
	}
	return expectExpressionValues(exp, ret_values);
}

OS::Core::Compiler::Expression * OS::Core::Compiler::processExpressionSecondPass(Scope * scope, Expression * exp)
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
		{
			Scope * new_scope = dynamic_cast<Scope*>(exp);
			OS_ASSERT(new_scope && (new_scope->parent == scope || (!new_scope->parent && new_scope->type == EXP_TYPE_FUNCTION)));
			scope = new_scope;
			break;
		}

	case EXP_TYPE_PARAMS:
		{
			for(int i = exp->list.count-1; i >= 0; i--){
				Expression * sub_exp = processExpressionSecondPass(scope, exp->list[i]);
				if(sub_exp->type == EXP_TYPE_PARAMS){
					OS_ASSERT(false);
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

	case EXP_TYPE_NAME:
		if(findLocalVar(exp->local_var, scope, exp->token->str, exp->active_locals)){
			exp->type = EXP_TYPE_GET_LOCAL_VAR;
			if(scope->function->max_up_count < exp->local_var.up_count){
				scope->function->max_up_count = exp->local_var.up_count;
			}
		}else{
			exp->type = EXP_TYPE_GET_AUTO_VAR;
		}
		break;

	case EXP_TYPE_RETURN:
		if(exp->list.count == 1){
			Expression * sub_exp = exp->list[0] = processExpressionSecondPass(scope, exp->list[0]);
			switch(sub_exp->type){
			case EXP_TYPE_CALL:
				sub_exp->type = EXP_TYPE_TAIL_CALL;
				allocator->vectorClear(exp->list);
				allocator->deleteObj(exp);
				return sub_exp;
			}
			return exp;
		}

	case EXP_TYPE_CALL:
		{
			OS_ASSERT(exp->list.count == 2);
			exp->list[0] = processExpressionSecondPass(scope, exp->list[0]);
			exp->list[1] = processExpressionSecondPass(scope, exp->list[1]);
			Expression * left_exp = exp->list[0];
			Expression * right_exp = exp->list[1];
			if(left_exp->type == EXP_TYPE_GET_PROPERTY){
				OS_ASSERT(left_exp->list.count == 2);
				OS_ASSERT(right_exp->type == EXP_TYPE_PARAMS);
				allocator->vectorInsertAtIndex(right_exp->list, 0, left_exp->list[1]);
				right_exp->ret_values += left_exp->list[1]->ret_values;
				left_exp->list[1] = right_exp;
				left_exp->type = EXP_TYPE_CALL_PROPERTY;
				left_exp->ret_values = exp->ret_values;
				allocator->vectorClear(exp->list);
				allocator->deleteObj(exp);
				return left_exp;
			}
			return exp;
		}

	case EXP_TYPE_CALL_DIM:
		{
			OS_ASSERT(exp->list.count == 2);
			exp->list[0] = processExpressionSecondPass(scope, exp->list[0]);
			exp->list[1] = processExpressionSecondPass(scope, exp->list[1]);
			Expression * name_exp = exp->list[0];
			Expression * params = exp->list[1];
			OS_ASSERT(params->type == EXP_TYPE_PARAMS);
			switch(name_exp->type){
			case EXP_TYPE_GET_PROPERTY:
				OS_ASSERT(name_exp->list.count == 2);
				allocator->vectorInsertAtIndex(params->list, 0, name_exp->list[1]);
				params->ret_values += name_exp->list[1]->ret_values;
				name_exp->list[1] = params;
				name_exp->type = EXP_TYPE_GET_PROPERTY_DIM;
				allocator->vectorClear(exp->list);
				allocator->deleteObj(exp);
				return name_exp;

			case EXP_TYPE_GET_AUTO_VAR:
				OS_ASSERT(name_exp->list.count == 0);
				name_exp->type = EXP_TYPE_CONST_STRING;
				allocator->vectorInsertAtIndex(params->list, 0, name_exp);
				params->ret_values += name_exp->ret_values;
				allocator->vectorRemoveAtIndex(exp->list, 1);
				exp->list[0] = params;
				exp->type = EXP_TYPE_GET_AUTO_VAR_DIM;
				return exp;

			case EXP_TYPE_GET_LOCAL_VAR:
				// OS_ASSERT(false);
				// break;

			default:
				exp->type = EXP_TYPE_GET_DIM;
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
			/* switch(left_exp->type){
			case EXP_TYPE_NAME:
				{
					int local_var_index = findLocalVarIndex(NULL, left_exp->token->str);
					left_exp->type = local_var_index < 0 ? EXP_TYPE_GET_AUTO_VAR : EXP_TYPE_GET_LOCAL_VAR;
					break;
				}
			} */
			ExpressionType exp_type = EXP_TYPE_GET_PROPERTY;
			switch(right_exp->type){
			case EXP_TYPE_NAME:
				right_exp->type = EXP_TYPE_CONST_STRING;
				break;

			case EXP_TYPE_CALL:
				right_exp->type = EXP_TYPE_PARAMS;
				exp_type = EXP_TYPE_CALL_PROPERTY;
				OS_ASSERT(right_exp->list.count == 2);
				if(right_exp->list[0]->type == EXP_TYPE_NAME){
					OS_ASSERT(right_exp->list[0]->ret_values == 1);
					right_exp->list[0]->type = EXP_TYPE_CONST_STRING;
				}
				break;

			case EXP_TYPE_CALL_DIM:
				right_exp->type = EXP_TYPE_PARAMS;
				exp_type = EXP_TYPE_GET_PROPERTY_DIM;
				OS_ASSERT(right_exp->list.count == 2);
				if(right_exp->list[0]->type == EXP_TYPE_NAME){
					OS_ASSERT(right_exp->list[0]->ret_values == 1);
					right_exp->list[0]->type = EXP_TYPE_CONST_STRING;
				}
				break;
			}
			exp->type = exp_type;
			// exp->ret_values = 1;
			// return exp;
			break;
		}
	}
	for(int i = 0; i < exp->list.count; i++){
		exp->list[i] = processExpressionSecondPass(scope, exp->list[i]);
	}
	return exp;
}

OS::Core::Compiler::Scope * OS::Core::Compiler::expectTextExpression()
{
	OS_ASSERT(recent_token);

	Scope * scope = new (malloc(sizeof(Scope))) Scope(NULL, EXP_TYPE_FUNCTION, recent_token);
	// scope->function = scope;
	scope->parser_started = true;
	scope->ret_values = 1;

	Expression * exp;
	ExpressionList list(allocator);

	while(!isError()){
		if(recent_token->getType() == Tokenizer::BEGIN_CODE_BLOCK){
			exp = expectCodeExpression(scope, 0);
			if(!exp){
				break;
			}
			list.add(exp);
			if(!recent_token){
				break;
			}
		}else{
			exp = expectSingleExpression(scope, true, true, true);
			if(!exp){
				break;
			}
			list.add(exp);
			if(!recent_token){
				break;
			}
			switch(recent_token->getType()){
			case Tokenizer::END_ARRAY_BLOCK:
			case Tokenizer::END_BRACKET_BLOCK:
			case Tokenizer::END_CODE_BLOCK:
				break;
			}
			if(recent_token->getType() == Tokenizer::CODE_SEPARATOR){
				if(!readToken()){
					break;
				}
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
	int ret_values = list.count == 1 && list[0]->ret_values > 0 ? 1 : 0;
	exp = newExpressionFromList(list, ret_values);

	/* struct Lib {
		static Expression * returnFunction(Scope * scope)
		{
			OS_ASSERT(scope->type == EXP_TYPE_FUNCTION);
			Expression * exp = new (malloc(sizeof(Expression))) Expression(EXP_TYPE_RETURN
		}
	}; */

	switch(exp->type){
	case EXP_TYPE_CODE_LIST:
		if(exp->list.count == 1 && exp->list[0]->type == EXP_TYPE_FUNCTION){
			allocator->deleteObj(scope);
			scope = dynamic_cast<Scope*>(exp->list[0]);
			allocator->vectorClear(exp->list);
			allocator->deleteObj(exp);
			return scope;
		}
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

	Expression * exp;
	ExpressionList list(allocator);
	while(!isError()){
		if(recent_token->getType() == Tokenizer::BEGIN_CODE_BLOCK){
			exp = expectCodeExpression(scope, 0);
			if(!exp){
				break;
			}
			list.add(exp);
		}else{
			exp = expectSingleExpression(scope, true, true, true);
			if(!exp){
				break;
			}
			// exp = expectExpressionValues(exp, 1);
			list.add(exp);
			if(!recent_token || recent_token->getType() == Tokenizer::END_CODE_BLOCK){
				break;
			}
			if(!expectToken()){
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
	/* for(int i = 0; i < list.count-1; i++){
		list[i] = expectExpressionValues(list[i], 0);
	} */
	exp = newExpressionFromList(list, ret_values);
	switch(exp->type){
	case EXP_TYPE_CODE_LIST:
		{ // if(is_new_func || scope->type == EXP_TYPE_SCOPE){
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
			// obj_exp->active_locals = active_locals;
		}

		Expression * error()
		{
			compiler->allocator->deleteObj(obj_exp);
			return NULL;
		}

		/* Expression * error(ErrorType err)
		{
			return error(err, compiler->recent_token);
		} */
		
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
			exp = expectSingleExpression(scope, true, false, false);
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
			if(!readToken() || recent_token->getType() != Tokenizer::OPERATOR_COLON){
				allocator->deleteObj(exp);
				return lib.error(Tokenizer::OPERATOR_COLON, recent_token);
			}
			save_token = readToken();
			Expression * exp2 = expectSingleExpression(scope, true, false, false);
			if(!exp2){
				return isError() ? lib.error() : lib.error(ERROR_EXPECT_EXPRESSION, save_token);
			}
			exp2 = expectExpressionValues(exp2, 1);
			exp = new (malloc(sizeof(Expression))) Expression(EXP_TYPE_OBJECT_SET_BY_EXP, name_token, exp, exp2);
			// exp->active_locals = scope->function->num_locals;
		}else if(isNextToken(Tokenizer::OPERATOR_COLON)){
			ExpressionType exp_type = EXP_TYPE_OBJECT_SET_BY_NAME;
			switch(name_token->getType()){
			case Tokenizer::STRING:
			case Tokenizer::NAME:
				break;

			case Tokenizer::NUM_FLOAT:
				if(name_token->getFloat() != (OS_FLOAT)name_token->getInt()){
					// use it as EXP_TYPE_OBJECT_SET_BY_NAME
					break;
				}
				// use it as EXP_TYPE_OBJECT_SET_BY_INDEX
				// no break

			case Tokenizer::NUM_INT:
				exp_type = EXP_TYPE_OBJECT_SET_BY_INDEX;
				break;

			default:
				return lib.error(ERROR_SYNTAX, name_token);
			}
			readToken(); // skip OPERATOR_COLON
			TokenData * save_token = readToken();
			exp = expectSingleExpression(scope, true, false, false);
			if(!exp){
				return isError() ? lib.error() : lib.error(ERROR_EXPECT_EXPRESSION, save_token);
			}
			exp = expectExpressionValues(exp, 1);
			exp = new (malloc(sizeof(Expression))) Expression(exp_type, name_token, exp);
			// exp->active_locals = scope->function->num_locals;
		}else{
			exp = expectSingleExpression(scope, true, false, false);
			if(!exp){
				return isError() ? lib.error() : lib.error(ERROR_EXPECT_EXPRESSION, name_token);
			}
			exp = expectExpressionValues(exp, 1);
			exp = new (malloc(sizeof(Expression))) Expression(EXP_TYPE_OBJECT_SET_BY_AUTO_INDEX, name_token, exp);
			// exp->active_locals = scope->function->num_locals;
		}
		OS_ASSERT(exp);
		lib.obj_exp->list.add(exp);
		if(recent_token && recent_token->getType() == Tokenizer::END_CODE_BLOCK){
			readToken();
			return lib.obj_exp;
		}
		if(!recent_token || (recent_token->getType() != Tokenizer::PARAM_SEPARATOR
				&& recent_token->getType() != Tokenizer::CODE_SEPARATOR)){
			return lib.error(Tokenizer::PARAM_SEPARATOR, recent_token);
		}
		readToken();
	}
	return NULL; // shut up compiler
}

OS::Core::Compiler::Expression * OS::Core::Compiler::expectArrayExpression(Scope * scope)
{
	Expression * params = new (malloc(sizeof(Expression))) Expression(EXP_TYPE_ARRAY, recent_token);
	// params->active_locals = scope->function->num_locals;
	readToken();
	for(;;){
		Expression * exp = expectSingleExpression(scope, true, false, false);
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
			return params;
		}
		exp = expectExpressionValues(exp, 1);
		params->list.add(exp);
		if(recent_token && recent_token->getType() == Tokenizer::END_ARRAY_BLOCK){
			readToken();
			return params;
		}
		if(!recent_token || (recent_token->getType() != Tokenizer::PARAM_SEPARATOR
				&& recent_token->getType() != Tokenizer::CODE_SEPARATOR)){
			setError(Tokenizer::PARAM_SEPARATOR, recent_token);
			allocator->deleteObj(params);
			return NULL;
		}
		readToken();
	}
	return NULL; // shut up compiler
}

OS::Core::Compiler::Expression * OS::Core::Compiler::finishParamsExpression(Scope * scope, Expression * params)
{
	// params->active_locals = scope->function->num_locals;
	if(params->list.count > 1){
		for(int i = 0; i < params->list.count; i++){
			params->list[i] = expectExpressionValues(params->list[i], 1);
		}
		params->ret_values = params->list.count;
	}else if(params->list.count == 1){
		params->ret_values = params->list[0]->ret_values;
	}
	return params;
}

OS::Core::Compiler::Expression * OS::Core::Compiler::expectParamsExpression(Scope * scope)
{
	// OS_ASSERT(recent_token->getType() == Tokenizer::PARAM_SEPARATOR);
	Expression * params = new (malloc(sizeof(Expression))) Expression(EXP_TYPE_PARAMS, recent_token);
	// params->active_locals = scope->function->num_locals;
	bool is_dim = recent_token->getType() == Tokenizer::BEGIN_ARRAY_BLOCK;
	TokenType end_exp_type = is_dim ? Tokenizer::END_ARRAY_BLOCK : Tokenizer::END_BRACKET_BLOCK;
	readToken();
	for(;;){
		Expression * exp = expectSingleExpression(scope, true, false, false);
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
			return params;
		}
		// exp = expectExpressionValues(exp, 1);
		params->list.add(exp);
		// params->ret_values += exp->ret_values;
		if(recent_token && recent_token->getType() == end_exp_type){
			readToken();
			return finishParamsExpression(scope, params);
		}
		if(!recent_token || recent_token->getType() != Tokenizer::PARAM_SEPARATOR){
			setError(Tokenizer::PARAM_SEPARATOR, recent_token);
			allocator->deleteObj(params);
			return NULL;
		}
		readToken();
	}
	return NULL; // shut up compiler
}

OS::Core::Compiler::Expression * OS::Core::Compiler::expectParamsExpression(Scope * scope, Expression * first_param)
{
	OS_ASSERT(recent_token && recent_token->getType() == Tokenizer::PARAM_SEPARATOR);
	Expression * params = new (malloc(sizeof(Expression))) Expression(EXP_TYPE_PARAMS, first_param->token, first_param);
	params->ret_values = 1;
	readToken();
	for(;;){
		Expression * exp = expectSingleExpression(scope, false, false, false);
		if(!exp){
			if(isError()){
				allocator->deleteObj(params);
				return NULL;
			}
			/* if(!recent_token){ // || recent_token->getType() != Tokenizer::PARAM_SEPARATOR){
				setError(ERROR_SYNTAX, recent_token);
				allocator->deleteObj(params);
				return NULL;
			} */
			// readToken();
			return finishParamsExpression(scope, params);
		}
		// exp = expectExpressionValues(exp, 1);
		params->list.add(exp);
		// params->ret_values++;
		if(!recent_token || recent_token->getType() != Tokenizer::PARAM_SEPARATOR){
			return finishParamsExpression(scope, params);
		}
		readToken();
	}
	return NULL; // shut up compiler
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
			setError(ERROR_SYNTAX, recent_token);
			allocator->deleteObj(scope);
			return NULL;

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
	
	return expectCodeExpression(scope, 0);
}

OS::Core::Compiler::Expression * OS::Core::Compiler::expectVarExpression(Scope * scope)
{
	OS_ASSERT(recent_token && recent_token->str == allocator->core->strings->syntax_var);
	if(!expectToken(Tokenizer::NAME)){
		return NULL;
	}
	// ungetToken();
	Expression * exp = expectSingleExpression(scope, true, true, false);
	Expression * ret_exp = exp;
	while(exp){
		switch(exp->type){
		case EXP_TYPE_SET_LOCAL_VAR:
			for(;;){
				if(exp->local_var.up_scope_count == 0){
					setError(ERROR_VAR_ALREADY_EXIST, exp->token);
					allocator->deleteObj(ret_exp);
					return NULL;
				}
				// OS_ASSERT(!findLocalVar(exp->local_var, scope, exp->token->str, 0));
				scope->addLocalVar(exp->token->str, exp->local_var);
				OS_ASSERT(exp->list.count == 1);
				exp = exp->list[0];
				switch(exp->type){
				case EXP_TYPE_SET_AUTO_VAR:
				case EXP_TYPE_SET_LOCAL_VAR:
					break;

				default:
					return ret_exp;
				}
				break;
			}
			break;

		case EXP_TYPE_SET_AUTO_VAR:
			for(;;){
				OS_ASSERT(!findLocalVar(exp->local_var, scope, exp->token->str, exp->active_locals));
				scope->addLocalVar(exp->token->str, exp->local_var);
				exp->type = EXP_TYPE_SET_LOCAL_VAR;
				OS_ASSERT(exp->list.count == 1);
				exp = exp->list[0];
				switch(exp->type){
				case EXP_TYPE_SET_AUTO_VAR:
				case EXP_TYPE_SET_LOCAL_VAR:
					break;

				default:
					return ret_exp;
				}
				break;
			}
			break;

		case EXP_TYPE_NAME:
			if(findLocalVar(exp->local_var, scope, exp->token->str, exp->active_locals)){
				setError(ERROR_VAR_ALREADY_EXIST, exp->token);
				allocator->deleteObj(ret_exp);
				return NULL;
			}
			scope->addLocalVar(exp->token->str, exp->local_var);
			exp->type = EXP_TYPE_NEW_LOCAL_VAR;
			exp->ret_values = 0;
			// exp->active_locals = scope->function->num_locals;
			return ret_exp;

		default:
			return ret_exp;
		}
	}
	return ret_exp;
}

OS::Core::Compiler::Expression * OS::Core::Compiler::expectReturnExpression(Scope * scope)
{
	Expression * ret_exp = new (malloc(sizeof(Expression))) Expression(EXP_TYPE_RETURN, recent_token);
	// ret_exp->active_locals = scope->function->num_locals;
	if(!readToken()){
		setError(ERROR_SYNTAX, recent_token);
		allocator->deleteObj(ret_exp);
		return NULL;
	}
	switch(recent_token->getType()){
	case Tokenizer::END_ARRAY_BLOCK:
	case Tokenizer::END_BRACKET_BLOCK:
		return ret_exp;

	case Tokenizer::END_CODE_BLOCK:
		/* if(!readToken()){
			setError(ERROR_SYNTAX, recent_token);
			allocator->deleteObj(ret_exp);
			return NULL;
		} */
		return ret_exp;
	}
	for(;;){
		Expression * exp = expectSingleExpression(scope, true, false, false);
		if(!exp){
			allocator->deleteObj(ret_exp);
			return NULL;
		}
		exp = expectExpressionValues(exp, 1);
		ret_exp->list.add(exp);
		ret_exp->ret_values++;
		if(!recent_token){
			// setError(ERROR_SYNTAX, recent_token);
			// allocator->deleteObj(ret_exp);
			// return NULL;
			return ret_exp;
		}
		switch(recent_token->getType()){
		/*
		case Tokenizer::END_ARRAY_BLOCK:
		case Tokenizer::END_BRACKET_BLOCK:
			return ret_exp;
		*/
		case Tokenizer::END_CODE_BLOCK:
			return ret_exp;

		case Tokenizer::CODE_SEPARATOR:
			// if(!readToken()){
				// setError(ERROR_SYNTAX, recent_token);
				// allocator->deleteObj(ret_exp);
				// return NULL;
			// }
			return ret_exp;

		case Tokenizer::PARAM_SEPARATOR:
			if(!readToken()){
				setError(ERROR_SYNTAX, recent_token);
				allocator->deleteObj(ret_exp);
				return NULL;
			}
			continue;
		}
		setError(ERROR_SYNTAX, recent_token);
		allocator->deleteObj(ret_exp);
		return NULL;
	}
	return NULL; // shut up compiler
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

			Expression * newExpression(const StringInternal& str, Expression * left_exp, Expression * right_exp)
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
				token = new (malloc(sizeof(TokenData))) TokenData(token->text_data, StringInternal(compiler->allocator, val), Tokenizer::NUM_FLOAT, token->line, token->pos);
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
			return lib.newExpression(left_exp->toString() + right_exp->toString(), left_exp, right_exp);

		// case EXP_TYPE_ANY_PARAMS:  // ...

		case EXP_TYPE_LOGIC_AND: // &&
			return lib.switchExpression(left_exp->toInt() == 0, left_exp, right_exp);

		case EXP_TYPE_LOGIC_OR:  // ||
			return lib.switchExpression(left_exp->toInt() != 0, left_exp, right_exp);

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

		case EXP_TYPE_BIN_AND: // &
			return lib.newExpression(left_exp->toInt() & right_exp->toInt(), left_exp, right_exp);

		case EXP_TYPE_BIN_OR:  // |
			return lib.newExpression(left_exp->toInt() | right_exp->toInt(), left_exp, right_exp);

		case EXP_TYPE_BIN_XOR: // ^
			return lib.newExpression(left_exp->toInt() ^ right_exp->toInt(), left_exp, right_exp);

		// case EXP_TYPE_BIN_NOT: // ~
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

		case EXP_TYPE_MUL_SHIFT: // <<
			return lib.newExpression(left_exp->toInt() << right_exp->toInt(), left_exp, right_exp);

		case EXP_TYPE_DIV_SHIFT: // >>
			return lib.newExpression(left_exp->toInt() >> right_exp->toInt(), left_exp, right_exp);

		case EXP_TYPE_POW: // **
			return lib.newExpression(OS_MATH_POW(left_exp->toNumber(), right_exp->toNumber()), left_exp, right_exp);


		/*
		case EXP_TYPE_DOT:   // dot
		case EXP_TYPE_CROSS: // cross
		// case EXP_TYPE_SWAP:  // swap
		case EXP_TYPE_IS:    // is
		case EXP_TYPE_AS:    // as
		// case EXP_TYPE_IN:    // in
		*/
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
	/*
	case EXP_TYPE_INDIRECT:
		{
			left_exp = expectExpressionValues(left_exp, 1);
			right_exp = expectExpressionValues(right_exp, 1);
			switch(left_exp->type){
			case EXP_TYPE_NAME:
				{
					int local_var_index = findLocalVarIndex(scope, left_exp->token->str);
					left_exp->type = local_var_index < 0 ? EXP_TYPE_GET_AUTO_VAR : EXP_TYPE_GET_LOCAL_VAR;
					break;
				}
			}
			switch(right_exp->type){
			case EXP_TYPE_NAME:
				right_exp->type = EXP_TYPE_CONST_STRING;
				break;
			}
			Expression * exp = new (malloc(sizeof(Expression))) Expression(EXP_TYPE_GET_PROPERTY, token, left_exp, right_exp);
			exp->ret_values = 1;
			return exp;
		}
	*/
	}
	left_exp = expectExpressionValues(left_exp, 1);
	right_exp = expectExpressionValues(right_exp, 1);
	Expression * exp = new (malloc(sizeof(Expression))) Expression(exp_type, token, left_exp, right_exp);
	exp->ret_values = 1;
	// exp->active_locals = scope->function->num_locals;
	return exp;
}

bool OS::Core::Compiler::findLocalVar(LocalVarDesc& desc, Scope * scope, const StringInternal& name, int active_locals, int max_up_count)
{
	OS_ASSERT(scope);
	for(int up_count = 0, up_scope_count = 0;;){
		for(int i = scope->locals.count-1; i >= 0; i--){
			const Scope::LocalVar& local_var = scope->locals[i];
			if(local_var.index < active_locals && local_var.name == name){
				desc.index = local_var.index;
				desc.up_count = up_count;
				desc.up_scope_count = up_scope_count;
				desc.is_param = i < scope->num_params;
				return true;
			}
		}
		if(scope->parent){
			if(scope->type == EXP_TYPE_FUNCTION){
				if(up_count >= max_up_count){
					return false;
				}
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
	// int local_var_index = -1;
	// ExpressionType type = EXP_TYPE_ASSIGN;
	switch(var_exp->type){
	case EXP_TYPE_CALL_DIM:
		{
			Expression * name_exp = var_exp->list[0];
			Expression * params = var_exp->list[1];
			OS_ASSERT(params->type == EXP_TYPE_PARAMS);
			switch(name_exp->type){
			// case EXP_TYPE_CALL_DIM:
			default:
				allocator->vectorInsertAtIndex(var_exp->list, 0, value_exp);
				var_exp->type = EXP_TYPE_SET_DIM;
				var_exp->ret_values = value_exp->ret_values-1;
				return var_exp;

			case EXP_TYPE_NAME:
				{
					OS_ASSERT(name_exp->ret_values == 1);
					LocalVarDesc local_var;
					if(!findLocalVar(local_var, scope, name_exp->token->str, name_exp->active_locals)){
						OS_ASSERT(name_exp->list.count == 0);
						Expression * field_exp = name_exp;
						field_exp->type = EXP_TYPE_CONST_STRING;
						allocator->vectorInsertAtIndex(params->list, 0, field_exp);
						params->ret_values += field_exp->ret_values;

						OS_ASSERT(var_exp->list.count == 2);
						var_exp->list[0] = value_exp;
						var_exp->list[1] = params;
						var_exp->type = EXP_TYPE_SET_AUTO_VAR_DIM;
						var_exp->ret_values = value_exp->ret_values-1;

						return var_exp;
					}
					OS_ASSERT(false);
				}

			case EXP_TYPE_INDIRECT:
				{
					OS_ASSERT(name_exp->list.count == 2);
					Expression * field_exp = name_exp->list[1];
					if(field_exp->type == EXP_TYPE_NAME){
						field_exp->type = EXP_TYPE_CONST_STRING;
					}
					allocator->vectorInsertAtIndex(params->list, 0, field_exp);
					params->ret_values += field_exp->ret_values;

					OS_ASSERT(var_exp->list.count == 2);
					var_exp->list[0] = value_exp;
					var_exp->list[1] = name_exp->list[0];
					var_exp->list.add(params);
					var_exp->type = EXP_TYPE_SET_PROPERTY_DIM;
					var_exp->ret_values = value_exp->ret_values-1;

					allocator->vectorClear(name_exp->list);
					allocator->deleteObj(name_exp);
					return var_exp;
				}
			}
			break;
		}

	case EXP_TYPE_INDIRECT:
		{
			OS_ASSERT(var_exp->list.count == 2);
			Expression * var_exp_left = var_exp->list[0];
			switch(var_exp_left->type){
			case EXP_TYPE_NAME:
				{
					OS_ASSERT(var_exp_left->ret_values == 1);
					if(findLocalVar(var_exp_left->local_var, scope, var_exp_left->token->str, var_exp_left->active_locals)){
						var_exp_left->type = EXP_TYPE_GET_LOCAL_VAR;
					}else{
						var_exp_left->type = EXP_TYPE_GET_AUTO_VAR;
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
				OS_ASSERT(false);
				/* setError(ERROR_EXPECT_WRITEABLE, var_exp_right->token);
				allocator->deleteObj(var_exp);
				allocator->deleteObj(value_exp); */
				return NULL;

			case EXP_TYPE_CALL_DIM:
				OS_ASSERT(false);
				/* var_exp_right->type = EXP_TYPE_PARAMS;
				exp_type = EXP_TYPE_SET_PROPERTY_DIM;
				OS_ASSERT(var_exp_right->list.count == 2);
				if(var_exp_right->list[0]->type == EXP_TYPE_NAME){
					OS_ASSERT(var_exp_right->list[0]->ret_values == 1);
					var_exp_right->list[0]->type = EXP_TYPE_CONST_STRING;
				}
				break; */
				return NULL;
			}
			Expression * exp = new (malloc(sizeof(Expression))) Expression(exp_type, var_exp->token, value_exp, var_exp_left, var_exp_right);
			exp->ret_values = value_exp->ret_values-1;
			// exp->active_locals = scope->function->num_locals;
			allocator->vectorClear(var_exp->list);
			allocator->deleteObj(var_exp);
			return exp;
		}
		break;

	case EXP_TYPE_NAME:
		if(findLocalVar(var_exp->local_var, scope, var_exp->token->str, var_exp->active_locals)){
			var_exp->type = EXP_TYPE_SET_LOCAL_VAR;
			if(scope->function->max_up_count < var_exp->local_var.up_count){
				scope->function->max_up_count = var_exp->local_var.up_count;
			}		}else{
			var_exp->type = EXP_TYPE_SET_AUTO_VAR;
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

OS::Core::Compiler::Expression * OS::Core::Compiler::finishBinaryOperator(Scope * scope, OpcodeLevel prev_level, Expression * exp, bool allow_param)
{
	TokenData * binary_operator = recent_token;
	OS_ASSERT(binary_operator->isTypeOf(Tokenizer::BINARY_OPERATOR));
	readToken();
	Expression * exp2 = expectSingleExpression(scope, false, allow_param, false);
	if(!exp2){
		allocator->deleteObj(exp);
		return NULL;
	}
	// exp2 = expectExpressionValues(exp2, 1);
	if(!recent_token || !recent_token->isTypeOf(Tokenizer::BINARY_OPERATOR)){
		// return new (malloc(sizeof(Expression))) Expression(toExpressionType(binary_operator->getType()), binary_operator, exp, exp2);
		return newBinaryExpression(scope, toExpressionType(binary_operator->getType()), binary_operator, exp, exp2);
	}
	ExpressionType left_exp_type = toExpressionType(binary_operator->getType());
	ExpressionType right_exp_type = toExpressionType(recent_token->getType());
	OpcodeLevel left_level = toOpcodeLevel(left_exp_type);
	OpcodeLevel right_level = toOpcodeLevel(right_exp_type);
	if(left_level == right_level){
		// exp = new (malloc(sizeof(Expression))) Expression(left_exp_type, binary_operator, exp, exp2);
		exp = newBinaryExpression(scope, left_exp_type, binary_operator, exp, exp2);
		return finishBinaryOperator(scope, prev_level, exp, allow_param);
	}
	if(left_level > right_level){
		// exp = new (malloc(sizeof(Expression))) Expression(left_exp_type, binary_operator, exp, exp2);
		exp = newBinaryExpression(scope, left_exp_type, binary_operator, exp, exp2);
		if(prev_level >= right_level){
			return exp;
		}
		return finishBinaryOperator(scope, prev_level, exp, allow_param);
	}
	exp2 = finishBinaryOperator(scope, left_level, exp2, allow_param);
	if(!exp2){
		allocator->deleteObj(exp);
		return NULL;
	}
	return newBinaryExpression(scope, left_exp_type, binary_operator, exp, exp2);
}

OS::Core::Compiler::Expression * OS::Core::Compiler::finishValueExpression(Scope * scope, Expression * exp, bool allow_binary_operator, bool allow_param)
{
	/* switch(exp->type){
	case EXP_TYPE_CALL:
	case EXP_TYPE_CALL_DIM:
		exp->ret_values = 1;
		break;

	default:
		OS_ASSERT(exp->ret_values == 1);
		break;
	} */
	for(;;){
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
			exp2->active_locals = scope->function->num_locals;
			exp = new (malloc(sizeof(Expression))) Expression(EXP_TYPE_INDIRECT, exp2->token, exp, exp2);
			exp->ret_values = 1;
			// exp->active_locals = scope->function->num_locals;
			readToken();
			continue;

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

		case Tokenizer::OPERATOR_BIN_AND: // &
		case Tokenizer::OPERATOR_BIN_OR:  // |
		case Tokenizer::OPERATOR_BIN_XOR: // ^
		case Tokenizer::OPERATOR_BIN_NOT: // ~
		case Tokenizer::OPERATOR_ADD: // +
		case Tokenizer::OPERATOR_SUB: // -
		case Tokenizer::OPERATOR_MUL: // *
		case Tokenizer::OPERATOR_DIV: // /
		case Tokenizer::OPERATOR_MOD: // %
		case Tokenizer::OPERATOR_MUL_SHIFT: // <<
		case Tokenizer::OPERATOR_DIV_SHIFT: // >>
		case Tokenizer::OPERATOR_POW: // **

		case Tokenizer::OPERATOR_DOT:   // dot
		case Tokenizer::OPERATOR_CROSS: // cross
		// case Tokenizer::OPERATOR_SWAP:  // swap
		case Tokenizer::OPERATOR_IS:    // is
		case Tokenizer::OPERATOR_AS:    // as
		// case Tokenizer::OPERATOR_IN:    // in
			if(!allow_binary_operator && token_type != Tokenizer::OPERATOR_INDIRECT){
				return exp;
			}
			exp = finishBinaryOperator(scope, OP_LEVEL_NOTHING, exp, allow_param);
			if(!exp){
				return NULL;
			}
			continue;

		case Tokenizer::OPERATOR_BIN_AND_ASSIGN: // &=
		case Tokenizer::OPERATOR_BIN_OR_ASSIGN:  // |=
		case Tokenizer::OPERATOR_BIN_XOR_ASSIGN: // ^=
		case Tokenizer::OPERATOR_BIN_NOT_ASSIGN: // ~=
		case Tokenizer::OPERATOR_ADD_ASSIGN: // +=
		case Tokenizer::OPERATOR_SUB_ASSIGN: // -=
		case Tokenizer::OPERATOR_MUL_ASSIGN: // *=
		case Tokenizer::OPERATOR_DIV_ASSIGN: // /=
		case Tokenizer::OPERATOR_MOD_ASSIGN: // %=
		case Tokenizer::OPERATOR_MUL_SHIFT_ASSIGN: // <<=
		case Tokenizer::OPERATOR_DIV_SHIFT_ASSIGN: // >>=
		case Tokenizer::OPERATOR_POW_ASSIGN: // **=
			setError(ERROR_SYNTAX, token);
			return NULL;

		case Tokenizer::OPERATOR_ASSIGN: // =
			if(!allow_binary_operator){
				return exp;
			}
			exp = finishBinaryOperator(scope, OP_LEVEL_NOTHING, exp, allow_param);
			if(!exp){
				return NULL;
			}
			continue;

		/*
		case Tokenizer::END_ARRAY_BLOCK:
		case Tokenizer::END_BRACKET_BLOCK:
		case Tokenizer::END_CODE_BLOCK:
		case Tokenizer::CODE_SEPARATOR:
			return NULL;
		*/

		case Tokenizer::PARAM_SEPARATOR:
			if(!allow_param){
				return exp;
			}			
			exp = expectParamsExpression(scope, exp);
			if(!exp){
				// allocator->deleteObj(exp);
				return NULL;
			}
			continue;

		case Tokenizer::BEGIN_CODE_BLOCK: // {
			exp2 = expectObjectExpression(scope);
			if(!exp2){
				allocator->deleteObj(exp);
				return NULL;
			}
			OS_ASSERT(exp2->ret_values == 1);
			exp2 = new (malloc(sizeof(Expression))) Expression(EXP_TYPE_PARAMS, exp2->token, exp2);
			exp2->ret_values = 1;
			// exp2->active_locals = scope->function->num_locals;
			exp = new (malloc(sizeof(Expression))) Expression(EXP_TYPE_CALL, token, exp, exp2);
			exp->ret_values = 1;
			// exp->active_locals = scope->function->num_locals;
			continue;

		case Tokenizer::BEGIN_BRACKET_BLOCK: // (
			exp2 = expectParamsExpression(scope);
			if(!exp2){
				allocator->deleteObj(exp);
				return NULL;
			}
			exp = new (malloc(sizeof(Expression))) Expression(EXP_TYPE_CALL, token, exp, exp2);
			exp->ret_values = 1;
			// exp->active_locals = scope->function->num_locals;
			continue;

		case Tokenizer::BEGIN_ARRAY_BLOCK: // [
			exp2 = expectParamsExpression(scope);
			if(!exp2){
				allocator->deleteObj(exp);
				return NULL;
			}
			exp = new (malloc(sizeof(Expression))) Expression(EXP_TYPE_CALL_DIM, token, exp, exp2);
			exp->ret_values = 1;
			// exp->active_locals = scope->function->num_locals;
			if(0 && !allow_binary_operator){
				return exp;
			}
			continue;

		default:
			return exp;
		}
	}
	return NULL; // shut up compiler
}

OS::Core::Compiler::Expression * OS::Core::Compiler::expectSingleExpression(Scope * scope, bool allow_binary_operator, bool allow_param, bool allow_var)
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
		exp = expectSingleExpression(scope, false, false, false);
		if(!exp){
			return NULL;
		}
		OS_ASSERT(exp->ret_values == 1);
		exp = new (malloc(sizeof(Expression))) Expression(EXP_TYPE_PLUS, exp->token, exp);
		exp->ret_values = 1;
		// exp->active_locals = scope->function->num_locals;
		return finishValueExpression(scope, exp, allow_binary_operator, allow_param);

	case Tokenizer::OPERATOR_SUB:
		exp = expectSingleExpression(scope, false, false, false);
		if(!exp){
			return NULL;
		}
		OS_ASSERT(exp->ret_values == 1);
		exp = new (malloc(sizeof(Expression))) Expression(EXP_TYPE_NEG, exp->token, exp);
		exp->ret_values = 1;
		// exp->active_locals = scope->function->num_locals;
		return finishValueExpression(scope, exp, allow_binary_operator, allow_param);

	case Tokenizer::OPERATOR_BIN_NOT:
		exp = expectSingleExpression(scope, false, false, false);
		if(!exp){
			return NULL;
		}
		OS_ASSERT(exp->ret_values == 1);
		exp = new (malloc(sizeof(Expression))) Expression(EXP_TYPE_BIN_NOT, exp->token, exp);
		exp->ret_values = 1;
		// exp->active_locals = scope->function->num_locals;
		return finishValueExpression(scope, exp, allow_binary_operator, allow_param);

	case Tokenizer::OPERATOR_LOGIC_NOT:
		exp = expectSingleExpression(scope, false, false, false);
		if(!exp){
			return NULL;
		}
		OS_ASSERT(exp->ret_values == 1);
		exp = new (malloc(sizeof(Expression))) Expression(EXP_TYPE_LOGIC_NOT, exp->token, exp);
		exp->ret_values = 1;
		// exp->active_locals = scope->function->num_locals;
		return finishValueExpression(scope, exp, allow_binary_operator, allow_param);

	case Tokenizer::OPERATOR_INC:
	case Tokenizer::OPERATOR_DEC:
		setError(ERROR_SYNTAX, token);
		return NULL;
	// end unary operators

	case Tokenizer::BEGIN_CODE_BLOCK:
		exp = expectObjectExpression(scope);
		if(!exp){
			return NULL;
		}
		OS_ASSERT(exp->ret_values == 1);
		return finishValueExpression(scope, exp, allow_binary_operator, allow_param);

	case Tokenizer::BEGIN_ARRAY_BLOCK:
		exp = expectArrayExpression(scope);
		if(!exp){
			return NULL;
		}
		OS_ASSERT(exp->ret_values == 1);
		return finishValueExpression(scope, exp, allow_binary_operator, allow_param);

	case Tokenizer::BEGIN_BRACKET_BLOCK:
		readToken();
		exp = expectSingleExpression(scope, true, false, false);
		if(!exp){
			return NULL;
		}
		exp = newSingleValueExpression(exp);
		OS_ASSERT(exp->ret_values == 1);
		if(recent_token && recent_token->getType() == Tokenizer::PARAM_SEPARATOR){
			exp = expectParamsExpression(scope, exp);
			if(!exp){
				return NULL;
			}
			OS_ASSERT(exp->type == EXP_TYPE_PARAMS);
			exp->type = EXP_TYPE_CODE_LIST;
			int i = 0;
			for(; i < exp->list.count-1; i++){
				exp->list[i] = expectExpressionValues(exp->list[i], 0);
			}
			exp->list[i] = newSingleValueExpression(exp->list[i]);
			exp->ret_values = 1; // exp->list[i]->ret_values;
		}
		if(!recent_token){
			setError(Tokenizer::END_BRACKET_BLOCK, recent_token);
			allocator->deleteObj(exp);
			return NULL;
		}
		switch(recent_token->getType()){
		case Tokenizer::END_BRACKET_BLOCK:
			readToken();
			return finishValueExpression(scope, exp, allow_binary_operator, allow_param);

		default:
			setError(Tokenizer::END_BRACKET_BLOCK, recent_token);
			allocator->deleteObj(exp);
			return NULL;
		}

	case Tokenizer::STRING:
		exp = new (malloc(sizeof(Expression))) Expression(EXP_TYPE_CONST_STRING, token);
		exp->ret_values = 1;
		// exp->active_locals = scope->function->num_locals;
		readToken();
		return finishValueExpression(scope, exp, allow_binary_operator, allow_param);

	case Tokenizer::NUM_INT:
	case Tokenizer::NUM_FLOAT:
		exp = new (malloc(sizeof(Expression))) Expression(EXP_TYPE_CONST_NUMBER, token);
		exp->ret_values = 1;
		// exp->active_locals = scope->function->num_locals;
		readToken();
		return finishValueExpression(scope, exp, allow_binary_operator, allow_param);

	case Tokenizer::REST_ARGUMENTS:
		exp = new (malloc(sizeof(Expression))) Expression(EXP_TYPE_GET_REST_ARGUMENTS, token);
		exp->ret_values = 1;
		readToken();
		return finishValueExpression(scope, exp, allow_binary_operator, allow_param);

	case Tokenizer::NAME:
		if(token->str == allocator->core->strings->syntax_var){
			if(!allow_var){
				setError(ERROR_SYNTAX, token);
				return NULL;
			}
			return expectVarExpression(scope);
		}
		if(token->str == allocator->core->strings->syntax_function){
			exp = expectFunctionExpression(scope);
			if(!exp){
				return NULL;
			}
			return finishValueExpression(scope, exp, allow_binary_operator, allow_param);
		}
		if(token->str == allocator->core->strings->syntax_return){
			return expectReturnExpression(scope);
		}
		if(token->str == allocator->core->strings->syntax_this){
			exp = new (malloc(sizeof(Expression))) Expression(EXP_TYPE_GET_THIS, token);
			exp->ret_values = 1;
			readToken();
			return finishValueExpression(scope, exp, allow_binary_operator, allow_param);
		}
		if(token->str == allocator->core->strings->syntax_arguments){
			exp = new (malloc(sizeof(Expression))) Expression(EXP_TYPE_GET_ARGUMENTS, token);
			exp->ret_values = 1;
			readToken();
			return finishValueExpression(scope, exp, allow_binary_operator, allow_param);
		}
		if(token->str == allocator->core->strings->syntax_null){
			exp = new (malloc(sizeof(Expression))) Expression(EXP_TYPE_CONST_NULL, token);
			exp->ret_values = 1;
			// exp->active_locals = scope->function->num_locals;
			readToken();
			return finishValueExpression(scope, exp, allow_binary_operator, allow_param);
		}
		if(token->str == allocator->core->strings->syntax_true){
			token->setInt(1);
			exp = new (malloc(sizeof(Expression))) Expression(EXP_TYPE_CONST_TRUE, token);
			exp->ret_values = 1;
			// exp->active_locals = scope->function->num_locals;
			readToken();
			return finishValueExpression(scope, exp, allow_binary_operator, allow_param);
		}
		if(token->str == allocator->core->strings->syntax_false){
			exp = new (malloc(sizeof(Expression))) Expression(EXP_TYPE_CONST_FALSE, token);
			exp->ret_values = 1;
			// exp->active_locals = scope->function->num_locals;
			readToken();
			return finishValueExpression(scope, exp, allow_binary_operator, allow_param);
		}
		if(token->str == allocator->core->strings->syntax_break || token->str == allocator->core->strings->syntax_continue){
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
		if(token->str == allocator->core->strings->syntax_if){
			setError(ERROR_SYNTAX, token);
			return NULL;
		}
		if(token->str == allocator->core->strings->syntax_for){
			setError(ERROR_SYNTAX, token);
			return NULL;
		}
		if(token->str == allocator->core->strings->syntax_do){
			setError(ERROR_SYNTAX, token);
			return NULL;
		}
		if(token->str == allocator->core->strings->syntax_while){
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
		exp->active_locals = scope->function->num_locals;
		readToken();
		return finishValueExpression(scope, exp, allow_binary_operator, allow_param);
	}
	return NULL;
}

OS::Core::StringInternal OS::Core::Compiler::debugPrintSourceLine(TokenData * token)
{
	StringInternal out(allocator);
	if(!token){
		return out;
	}

	bool filePrinted = false;
	if(recent_printed_text_data != token->text_data){
		if(recent_printed_text_data){
			recent_printed_text_data->release();
		}
		filePrinted = true;
		recent_printed_line = -1;
		recent_printed_text_data = token->text_data->retain();
		out += StringInternal::format(allocator, OS_TEXT("\n[FILE] %s"), token->text_data->filename.toChar());
	}
	if(recent_printed_line != token->line && token->line >= 0){
		recent_printed_line = token->line;
		StringInternal line = token->text_data->lines[token->line].trim();
		out += StringInternal::format(allocator, OS_TEXT("\n[%d] %s\n\n"), token->line+1, line.toChar());
	}
	else if(filePrinted){
		out += StringInternal::format(allocator, OS_TEXT("\n"));
	}
	return out;
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

	case EXP_TYPE_SET_PROPERTY_DIM:
		return OS_TEXT("set property dim");

	case EXP_TYPE_GET_PROPERTY_DIM:
		return OS_TEXT("get property dim");

	case EXP_TYPE_GET_AUTO_VAR_DIM:
		return OS_TEXT("get auto var dim");

	case EXP_TYPE_SET_LOCAL_VAR_DIM:
		return OS_TEXT("set local var dim");

	case EXP_TYPE_SET_AUTO_VAR_DIM:
		return OS_TEXT("set auto var dim");

	case EXP_TYPE_SET_DIM:
		return OS_TEXT("set dim");

	case EXP_TYPE_POP_VALUE:
		return OS_TEXT("pop");

	case EXP_TYPE_CALL:
		return OS_TEXT("call");

	case EXP_TYPE_CALL_DIM:
		return OS_TEXT("dim");

	case EXP_TYPE_GET_DIM:
		return OS_TEXT("get dim");

	case EXP_TYPE_CALL_PROPERTY:
		return OS_TEXT("call property");

	case EXP_TYPE_VALUE:
		return OS_TEXT("single value");

	case EXP_TYPE_RETURN:
		return OS_TEXT("return");

	case EXP_TYPE_GET_THIS:
		return OS_TEXT("push this");

	case EXP_TYPE_GET_ARGUMENTS:
		return OS_TEXT("push arguments");

	case EXP_TYPE_GET_REST_ARGUMENTS:
		return OS_TEXT("push rest arguments");

	case EXP_TYPE_GET_LOCAL_VAR:
		return OS_TEXT("get local var");

	case EXP_TYPE_GET_AUTO_VAR:
		return OS_TEXT("get auto var");

	case EXP_TYPE_SET_LOCAL_VAR:
		return OS_TEXT("set local var");

	case EXP_TYPE_SET_AUTO_VAR:
		return OS_TEXT("set auto var");

	/*
	case EXP_TYPE_THREAD:
		return OS_TEXT("thread");

	case EXP_TYPE_FOR:
		return OS_TEXT("for");

	case EXP_TYPE_DO_WHILE:
		return OS_TEXT("do while");

	case EXP_TYPE_WHILE_DO:
		return OS_TEXT("while do");

	case EXP_TYPE_SWITCH:
		return OS_TEXT("switch");

	case EXP_TYPE_SWITCH_CASE:
		return OS_TEXT("case");

	case EXP_TYPE_SWITCH_CASE_INTERVAL:
		return OS_TEXT("case interval");

	case EXP_TYPE_SWITCH_DEFAULT:
		return OS_TEXT("default");

	case EXP_TYPE_BREAK:
		return OS_TEXT("break");

	case EXP_TYPE_CONTINUE:
		return OS_TEXT("continue");
	*/

	case EXP_TYPE_ASSIGN:
		return OS_TEXT("operator =");

	/*
	case EXP_TYPE_IF:
		return OS_TEXT("if");

		// case EXP_TYPE_IF_NOT:
		//   return OS_TEXT("if not");

	case EXP_TYPE_POST_IF:
		return OS_TEXT("post_if");

	case EXP_TYPE_KILL:
		return OS_TEXT("kill");

	case EXP_TYPE_CLONE:
		return OS_TEXT("clone");

	case EXP_TYPE_VALID:
		return OS_TEXT("valid");

	case EXP_TYPE_WAIT_FRAME:
		return OS_TEXT("waitFrame");

	case EXP_TYPE_WAIT_APP_TIME_MS:
		return OS_TEXT("waitAppTimeMS");

	case EXP_TYPE_WAIT_OBJECT:
		return OS_TEXT("waitObject");
	*/

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

	case EXP_TYPE_LOGIC_NOT:     // !
		return OS_TEXT("logic not");

	case EXP_TYPE_PLUS:
		return OS_TEXT("plus");

	case EXP_TYPE_NEG:
		return OS_TEXT("neg");

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

	case EXP_TYPE_BIN_AND: // &
		return OS_TEXT("bin &");

	case EXP_TYPE_BIN_OR:  // |
		return OS_TEXT("bin |");

	case EXP_TYPE_BIN_XOR: // ^
		return OS_TEXT("bin ^");

	case EXP_TYPE_BIN_NOT: // ~
		return OS_TEXT("bin ~");

	case EXP_TYPE_BIN_AND_ASSIGN: // &=
		return OS_TEXT("bin &=");

	case EXP_TYPE_BIN_OR_ASSIGN:  // |=
		return OS_TEXT("bin |=");

	case EXP_TYPE_BIN_XOR_ASSIGN: // ^=
		return OS_TEXT("bin ^=");

	case EXP_TYPE_BIN_NOT_ASSIGN: // ~=
		return OS_TEXT("bin ~=");

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

	case EXP_TYPE_MUL_SHIFT: // <<
		return OS_TEXT("operator <<");

	case EXP_TYPE_DIV_SHIFT: // >>
		return OS_TEXT("operator >>");

	case EXP_TYPE_POW: // **
		return OS_TEXT("operator **");

	case EXP_TYPE_DOT:
		return OS_TEXT("operator dot");

	case EXP_TYPE_CROSS:
		return OS_TEXT("operator cross");

	case EXP_TYPE_SWAP:
		return OS_TEXT("operator swap");

	case EXP_TYPE_AS:
		return OS_TEXT("operator as");

	case EXP_TYPE_IS:
		return OS_TEXT("operator is");

	case EXP_TYPE_IN:
		return OS_TEXT("operator in");

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

	case EXP_TYPE_MUL_SHIFT_ASSIGN: // <<=
		return OS_TEXT("operator <<=");

	case EXP_TYPE_DIV_SHIFT_ASSIGN: // >>=
		return OS_TEXT("operator >>=");

	case EXP_TYPE_POW_ASSIGN: // **=
		return OS_TEXT("operator **=");
	}
	return OS_TEXT("unknown exp");
}

// =====================================================================
// =====================================================================
// =====================================================================
/*
OS::FunctionData::LocalVar::LocalVar(OS * allocator): name(allocator)
{
	start_code_pos = -1;
	end_code_pos = -1;
}

// =====================================================================

OS::FunctionData::FunctionData()
{
	prog = NULL;
	num_locals = 0;
	num_params = 0;
	opcodes_pos = 0;
	opcodes_size = 0;
	up_funcs = NULL;
	num_up_funcs = 0;
}

OS::FunctionData::~FunctionData()
{
	OS_ASSERT(!prog && !up_funcs);
}
*/
// =====================================================================
// =====================================================================
// =====================================================================

OS::Core::ProgramFunctionDecl::LocalVar::LocalVar(const StringInternal& p_name): name(p_name)
{
	start_code_pos = -1;
	end_code_pos = -1;
}

OS::Core::ProgramFunctionDecl::LocalVar::~LocalVar()
{
}

OS::Core::ProgramFunctionDecl::ProgramFunctionDecl(Program * p_prog)
{
	prog = p_prog;
	parent_func_index = -1;
	locals = NULL;
	num_locals = 0;
	num_params = 0;
	max_up_count = 0;
	opcodes_pos = 0;
	opcodes_size = 0;
}

OS::Core::ProgramFunctionDecl::~ProgramFunctionDecl()
{
	OS_ASSERT(!locals && !prog);
}

// =====================================================================

OS::Core::Program::Program(OS * allocator): filename(allocator)
{
	this->allocator = allocator;
	ref_count = 1;
	opcodes = NULL;
	const_values = NULL;
	num_numbers = 0;
	num_strings = 0;
}

OS::Core::Program::~Program()
{
	OS_ASSERT(ref_count == 0);
	int i;
	for(i = num_numbers+num_strings-1; i >= 0; i--){
		allocator->core->releaseValue(const_values[i]);
	}
	allocator->free(const_values);
	const_values = NULL;

	for(i = 0; i < num_functions; i++){
		ProgramFunctionDecl * func = functions + i;
		for(int j = 0; j < func->num_locals; j++){
			func->locals[j].~LocalVar();
		}
		allocator->free(func->locals);
		func->locals = NULL;
		func->prog = NULL;
		func->~ProgramFunctionDecl();
	}
	allocator->free(functions);
	functions = NULL;

	allocator->deleteObj(opcodes);
}

bool OS::Core::Compiler::saveToStream(StreamWriter& writer)
{
	writer.writeBytes(OS_COMPILED_HEADER, OS_STRLEN(OS_COMPILED_HEADER));

	int i, len = OS_STRLEN(OS_COMPILED_VERSION);
	writer.writeByte(len);
	writer.writeBytes(OS_COMPILED_VERSION, len);

	writer.writeU16(prog_numbers.count);
	writer.writeU16(prog_strings.count);
	writer.writeU16(prog_functions.count);
	writer.writeInt32(prog_opcodes->buffer.count);

	for(i = 0; i < prog_numbers.count; i++){
		writer.writeFloat(prog_numbers[i]);	
	}
	for(i = 0; i < prog_strings.count; i++){
		const StringInternal& str = prog_strings[i];
		int data_size = str.getDataSize();
		writer.writeU16(data_size);
		writer.writeBytes(str.toChar(), data_size);
	}
	for(i = 0; i < prog_functions.count; i++){
		Compiler::Scope * func_scope = prog_functions[i];
		writer.writeInt16(func_scope->parent ? func_scope->parent->func_index : -1); // prog_functions.indexOf(func_scope->parent));
		writer.writeByte(func_scope->num_locals);
		writer.writeByte(func_scope->num_params);
		writer.writeByte(func_scope->max_up_count);
		writer.writeInt32(func_scope->opcodes_pos);
		writer.writeInt32(func_scope->opcodes_size);

		OS_ASSERT(func_scope->locals_compiled.count == func_scope->num_locals);
		for(int j = 0; j < func_scope->locals_compiled.count; j++){
			Compiler::Scope::LocalVarCompiled& var_scope = func_scope->locals_compiled[j];
			OS_ASSERT(var_scope.start_code_pos >= func_scope->opcodes_pos && var_scope.start_code_pos < func_scope->opcodes_pos+func_scope->opcodes_size);
			OS_ASSERT(var_scope.end_code_pos > func_scope->opcodes_pos && var_scope.end_code_pos <= func_scope->opcodes_pos+func_scope->opcodes_size);
			writer.writeU16(var_scope.cached_name_index);
			writer.writeInt32(var_scope.start_code_pos);
			writer.writeInt32(var_scope.end_code_pos);
		}
	}

	writer.writeBytes(prog_opcodes->buffer.buf, prog_opcodes->buffer.count);

	return true;
}

bool OS::Core::Program::loadFromStream(StreamReader& reader)
{
	OS_ASSERT(!opcodes && !const_values && !num_numbers && !num_strings);

	int i, len = OS_STRLEN(OS_COMPILED_HEADER);
	if(!reader.checkBytes(OS_COMPILED_HEADER, len)){
		return false;
	}

	len = OS_STRLEN(OS_COMPILED_VERSION);
	reader.skipBytes(1);
	if(!reader.checkBytes(OS_COMPILED_VERSION, len)){
		return false;
	}

	num_numbers = reader.readU16();
	num_strings = reader.readU16();
	num_functions = reader.readU16();
	int opcodes_size = reader.readInt32();

	const_values = (Value**)allocator->malloc(sizeof(Value*) * (num_numbers + num_strings));
	for(i = 0; i < num_numbers; i++){
		OS_FLOAT number = reader.readFloat();

		Value * value = allocator->core->pushNumberValue(number);
		value->ref_count++;
		const_values[i] = value;
		allocator->pop();
	}
	for(i = 0; i < num_strings; i++){
		int data_size = reader.readU16();
		StringInternal str(allocator, OS_TEXT('\0'), data_size/sizeof(OS_CHAR));
		reader.readBytes((void*)str.toChar(), data_size);

		Value * value = allocator->core->pushStringValue(str);
		value->ref_count++;
		const_values[num_numbers+i] = value;
		allocator->pop();
	}

	functions = (ProgramFunctionDecl*)allocator->malloc(sizeof(ProgramFunctionDecl) * num_functions);
	for(i = 0; i < num_functions; i++){
		ProgramFunctionDecl * func = functions + i;
		new (func) ProgramFunctionDecl(this);
		func->parent_func_index = reader.readInt16();
		func->num_locals = reader.readByte();
		func->num_params = reader.readByte();
		func->max_up_count = reader.readByte();
		func->opcodes_pos = reader.readInt32();
		func->opcodes_size = reader.readInt32();

		func->locals = (ProgramFunctionDecl::LocalVar*)allocator->malloc(sizeof(ProgramFunctionDecl::LocalVar) * func->num_locals);
		for(int j = 0; j < func->num_locals; j++){
			int cached_name_index = reader.readU16();
			OS_ASSERT(cached_name_index >= 0 && cached_name_index < num_strings);
			ProgramFunctionDecl::LocalVar * local_var = func->locals + j;
			StringInternal var_name = allocator->core->valueToString(const_values[num_numbers + cached_name_index]);
			new (local_var) ProgramFunctionDecl::LocalVar(var_name);
			local_var->start_code_pos = reader.readInt32();
			local_var->end_code_pos = reader.readInt32();
		}
	}

	opcodes = new (allocator->malloc(sizeof(MemStreamReader))) MemStreamReader(allocator, opcodes_size);
	reader.readBytes(opcodes->buffer, opcodes_size);

	return true;
}

void OS::Core::Program::pushFunction()
{
	int i, opcode = opcodes->readByte();
	if(opcode != OP_PUSH_FUNCTION){
		OS_ASSERT(false);
		allocator->core->pushNullValue();
		return;
	}

	int func_index = opcodes->readU16();
	OS_ASSERT(func_index >= 0 && func_index < num_functions);
	ProgramFunctionDecl * func_decl = functions + func_index;

	Value * func_value = allocator->core->pushNullValue();
	func_value->prototype = allocator->core->prototypes[PROTOTYPE_FUNCTION]->retain();
	func_value->value.func.decl = func_decl;
	func_value->value.func.parent_func = NULL;
	func_value->type = OS_VALUE_TYPE_FUNCTION;
	
	OS_ASSERT(func_decl->prog == this);
	retain(); // program is used by value

	OS_ASSERT(functions[func_index].opcodes_pos == opcodes->pos);
	opcodes->skipBytes(functions[func_index].opcodes_size);
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

OS::Core::Program::OpcodeType OS::Core::Program::toOpcodeType(Compiler::ExpressionType exp_type)
{
	switch(exp_type){
	case Compiler::EXP_TYPE_CALL: return OP_CALL;
	case Compiler::EXP_TYPE_GET_DIM: return OP_GET_DIM;
	case Compiler::EXP_TYPE_CALL_PROPERTY: return OP_CALL_PROPERTY;
	case Compiler::EXP_TYPE_TAIL_CALL: return OP_TAIL_CALL;

	case Compiler::EXP_TYPE_GET_THIS: return OP_PUSH_THIS;
	case Compiler::EXP_TYPE_GET_ARGUMENTS: return OP_PUSH_ARGUMENTS;
	case Compiler::EXP_TYPE_GET_REST_ARGUMENTS: return OP_PUSH_REST_ARGUMENTS;

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
	case Compiler::EXP_TYPE_LOGIC_NOT: return OP_LOGIC_NOT;

	case Compiler::EXP_TYPE_BIN_AND: return OP_BIN_AND;
	case Compiler::EXP_TYPE_BIN_OR: return OP_BIN_OR;
	case Compiler::EXP_TYPE_BIN_XOR: return OP_BIN_XOR;
	case Compiler::EXP_TYPE_BIN_NOT: return OP_BIN_NOT;

	case Compiler::EXP_TYPE_ADD: return OP_ADD;
	case Compiler::EXP_TYPE_SUB: return OP_SUB;
	case Compiler::EXP_TYPE_MUL: return OP_MUL;
	case Compiler::EXP_TYPE_DIV: return OP_DIV;
	case Compiler::EXP_TYPE_MOD: return OP_MOD;
	case Compiler::EXP_TYPE_MUL_SHIFT: return OP_MUL_SHIFT;
	case Compiler::EXP_TYPE_DIV_SHIFT: return OP_DIV_SHIFT;
	case Compiler::EXP_TYPE_POW: return OP_POW;

	/*
	case Compiler::EXP_TYPE_DOT: return OP_CONCAT;
	case Compiler::EXP_TYPE_CROSS: return OP_CONCAT;
	case Compiler::EXP_TYPE_SWAP: return OP_CONCAT;
	case Compiler::EXP_TYPE_IS: return OP_CONCAT;
	case Compiler::EXP_TYPE_AS: return OP_CONCAT;
	case Compiler::EXP_TYPE_IN: return OP_CONCAT;
	*/
	}
	OS_ASSERT(false);
	return OP_UNKNOWN;
}

// =====================================================================
// =====================================================================
// =====================================================================

OS::StreamWriter::StreamWriter(OS * p_allocator)
{
	allocator = p_allocator;
}

OS::StreamWriter::~StreamWriter()
{
}

void OS::StreamWriter::writeByte(int value)
{
	OS_ASSERT(value >= 0 && value <= 0xff);
	OS_BYTE le_value = toLittleEndianByteOrder((OS_BYTE)value);
	writeBytes(&le_value, sizeof(le_value));
}

void OS::StreamWriter::writeByteAtPos(int value, int pos)
{
	OS_ASSERT(value >= 0 && value <= 0xff);
	OS_BYTE le_value = toLittleEndianByteOrder((OS_BYTE)value);
	writeBytesAtPos(&le_value, sizeof(le_value), pos);
}

void OS::StreamWriter::writeU16(int value)
{
	OS_ASSERT(value >= 0 && value <= 0xffff);
	OS_U16 le_value = toLittleEndianByteOrder((OS_U16)value);
	writeBytes(&le_value, sizeof(le_value));
}

void OS::StreamWriter::writeU16AtPos(int value, int pos)
{
	OS_ASSERT(value >= 0 && value <= 0xffff);
	OS_U16 le_value = toLittleEndianByteOrder((OS_U16)value);
	writeBytesAtPos(&le_value, sizeof(le_value), pos);
}

void OS::StreamWriter::writeInt16(int value)
{
	OS_ASSERT((int)(OS_INT16)value == value);
	OS_INT16 le_value = toLittleEndianByteOrder((OS_INT16)value);
	writeBytes(&le_value, sizeof(le_value));
}

void OS::StreamWriter::writeInt16AtPos(int value, int pos)
{
	OS_ASSERT((int)(OS_INT16)value == value);
	OS_INT16 le_value = toLittleEndianByteOrder((OS_INT16)value);
	writeBytesAtPos(&le_value, sizeof(le_value), pos);
}

void OS::StreamWriter::writeInt32(int value)
{
	OS_INT32 le_value = toLittleEndianByteOrder((OS_INT32)value);
	writeBytes(&le_value, sizeof(le_value));
}

void OS::StreamWriter::writeInt32AtPos(int value, int pos)
{
	OS_ASSERT((int)(OS_INT32)value == value);
	OS_INT32 le_value = toLittleEndianByteOrder((OS_INT32)value);
	writeBytesAtPos(&le_value, sizeof(le_value), pos);
}

void OS::StreamWriter::writeInt64(OS_INT64 value)
{
	OS_INT64 le_value = toLittleEndianByteOrder((OS_INT64)value);
	writeBytes(&le_value, sizeof(le_value));
}

void OS::StreamWriter::writeInt64AtPos(OS_INT64 value, int pos)
{
	OS_INT64 le_value = toLittleEndianByteOrder((OS_INT64)value);
	writeBytesAtPos(&le_value, sizeof(le_value), pos);
}

void OS::StreamWriter::writeFloat(OS_FLOAT value)
{
	OS_FLOAT le_value = toLittleEndianByteOrder((OS_FLOAT)value);
	writeBytes(&le_value, sizeof(le_value));
}

void OS::StreamWriter::writeFloatAtPos(OS_FLOAT value, int pos)
{
	OS_FLOAT le_value = toLittleEndianByteOrder((OS_FLOAT)value);
	writeBytesAtPos(&le_value, sizeof(le_value), pos);
}

// =====================================================================

OS::MemStreamWriter::MemStreamWriter(OS * allocator): StreamWriter(allocator)
{
}

OS::MemStreamWriter::~MemStreamWriter()
{
	allocator->vectorClear(buffer);
}

int OS::MemStreamWriter::getPos() const
{
	return buffer.count;
}

void OS::MemStreamWriter::writeBytes(const void * buf, int len)
{
	int pos = buffer.count;
	allocator->vectorReserveCapacity(buffer, pos + len);
	buffer.count += len;
	writeBytesAtPos(buf, len, pos);
}

void OS::MemStreamWriter::writeBytesAtPos(const void * buf, int len, int pos)
{
	OS_ASSERT(pos >= 0 && pos <= buffer.count-len);
	OS_MEMCPY(buffer.buf+pos, buf, len);
}

void OS::MemStreamWriter::writeByte(int value)
{
	OS_ASSERT(value >= 0 && value <= 0xff);
	allocator->vectorAddItem(buffer, (OS_BYTE)value);
}

void OS::MemStreamWriter::writeByteAtPos(int value, int pos)
{
	OS_ASSERT(value >= 0 && value <= 0xff);
	OS_ASSERT(pos >= 0 && pos <= buffer.count-1);
	buffer[pos] = (OS_BYTE)value;
}

// =====================================================================

OS::FileStreamWriter::FileStreamWriter(OS * allocator, const char * filename): StreamWriter(allocator)
{
	f = fopen(filename, "wb");
	OS_ASSERT(f);
}

OS::FileStreamWriter::FileStreamWriter(OS * allocator, FILE * f): StreamWriter(allocator)
{
	this->f = f;
	OS_ASSERT(f);
}

OS::FileStreamWriter::~FileStreamWriter()
{
	if(f){
		fclose(f);
	}
}

int OS::FileStreamWriter::getPos() const
{
	return f ? ftell(f) : 0;
}

void OS::FileStreamWriter::writeBytes(const void * buf, int len)
{
	if(f){
		fwrite(buf, len, 1, f);
	}
}

void OS::FileStreamWriter::writeBytesAtPos(const void * buf, int len, int pos)
{
	int save_pos = getPos();
	fseek(f, pos, SEEK_SET);
	writeBytes(buf, len);
	fseek(f, save_pos, SEEK_SET);
}

// =====================================================================
// =====================================================================
// =====================================================================

OS::StreamReader::StreamReader(OS * p_allocator)
{
	allocator = p_allocator;
}

OS::StreamReader::~StreamReader()
{
}

OS_BYTE OS::StreamReader::readByte()
{
	OS_BYTE le_value;
	readBytes(&le_value, sizeof(le_value));
	return fromLittleEndianByteOrder(le_value);
}

OS_BYTE OS::StreamReader::readByteAtPos(int pos)
{
	OS_BYTE le_value;
	readBytesAtPos(&le_value, sizeof(le_value), pos);
	return fromLittleEndianByteOrder(le_value);
}

OS_U16 OS::StreamReader::readU16()
{
	OS_U16 le_value;
	readBytes(&le_value, sizeof(le_value));
	return fromLittleEndianByteOrder(le_value);
}

OS_U16 OS::StreamReader::readU16AtPos(int pos)
{
	OS_U16 le_value;
	readBytesAtPos(&le_value, sizeof(le_value), pos);
	return fromLittleEndianByteOrder(le_value);
}

OS_INT16 OS::StreamReader::readInt16()
{
	OS_INT16 le_value;
	readBytes(&le_value, sizeof(le_value));
	return fromLittleEndianByteOrder(le_value);
}

OS_INT16 OS::StreamReader::readInt16AtPos(int pos)
{
	OS_INT16 le_value;
	readBytesAtPos(&le_value, sizeof(le_value), pos);
	return fromLittleEndianByteOrder(le_value);
}

OS_INT32 OS::StreamReader::readInt32()
{
	OS_INT32 le_value;
	readBytes(&le_value, sizeof(le_value));
	return fromLittleEndianByteOrder(le_value);
}

OS_INT32 OS::StreamReader::readInt32AtPos(int pos)
{
	OS_INT32 le_value;
	readBytesAtPos(&le_value, sizeof(le_value), pos);
	return fromLittleEndianByteOrder(le_value);
}

OS_INT64 OS::StreamReader::readInt64()
{
	OS_INT64 le_value;
	readBytes(&le_value, sizeof(le_value));
	return fromLittleEndianByteOrder(le_value);
}

OS_INT64 OS::StreamReader::readInt64AtPos(int pos)
{
	OS_INT64 le_value;
	readBytesAtPos(&le_value, sizeof(le_value), pos);
	return fromLittleEndianByteOrder(le_value);
}

OS_FLOAT OS::StreamReader::readFloat()
{
	OS_FLOAT le_value;
	readBytes(&le_value, sizeof(le_value));
	return fromLittleEndianByteOrder(le_value);
}

OS_FLOAT OS::StreamReader::readFloatAtPos(int pos)
{
	OS_FLOAT le_value;
	readBytesAtPos(&le_value, sizeof(le_value), pos);
	return fromLittleEndianByteOrder(le_value);
}

// =====================================================================

OS::MemStreamReader::MemStreamReader(OS * allocator, int buf_size): StreamReader(allocator)
{
	buffer = (OS_BYTE*)allocator->malloc(buf_size);
	size = buf_size;
	pos = 0;
}

OS::MemStreamReader::MemStreamReader(OS * allocator, OS_BYTE * buf, int buf_size): StreamReader(allocator)
{
	buffer = buf;
	size = buf_size;
	pos = 0;
}

OS::MemStreamReader::~MemStreamReader()
{
	if(allocator){
		allocator->free(buffer);
	}
}

int OS::MemStreamReader::getPos() const
{
	return pos;
}

void OS::MemStreamReader::skipBytes(int len)
{
	OS_ASSERT(pos >= 0 && pos+len <= size);
	pos += len;
}

bool OS::MemStreamReader::checkBytes(void * src, int len)
{
	OS_ASSERT(pos >= 0 && pos+len <= size);
	bool r = OS_MEMCMP(buffer+pos, src, len) == 0;
	pos += len;
	return r;
}

void * OS::MemStreamReader::readBytes(void * dst, int len)
{
	OS_ASSERT(pos >= 0 && pos+len <= size);
	OS_MEMCPY(dst, buffer+pos, len);
	pos += len;
	return dst;
}

void * OS::MemStreamReader::readBytesAtPos(void * dst, int len, int pos)
{
	OS_ASSERT(pos >= 0 && pos+len <= size);
	OS_MEMCPY(dst, buffer+pos, len);
	return dst;
}

OS_BYTE OS::MemStreamReader::readByte()
{
	OS_ASSERT(pos >= 0 && pos+sizeof(OS_BYTE) <= size);
	return buffer[pos++];
}

OS_BYTE OS::MemStreamReader::readByteAtPos(int pos)
{
	OS_ASSERT(pos >= 0 && pos+sizeof(OS_BYTE) <= size);
	return buffer[pos];
}

// =====================================================================

OS::FileStreamReader::FileStreamReader(OS * allocator, const char * filename): StreamReader(allocator)
{
	f = fopen(filename, "rb");
	OS_ASSERT(f);
}

OS::FileStreamReader::FileStreamReader(OS * allocator, FILE * f): StreamReader(allocator)
{
	this->f = f;
	OS_ASSERT(f);
}

OS::FileStreamReader::~FileStreamReader()
{
	if(f){
		fclose(f);
	}
}

int OS::FileStreamReader::getPos() const
{
	return f ? ftell(f) : 0;
}

void OS::FileStreamReader::skipBytes(int len)
{
	if(f){
		fseek(f, len, SEEK_CUR);
	}
}

bool OS::FileStreamReader::checkBytes(void * src, int len)
{
	void * buf = alloca(len);
	readBytes(buf, len);
	return OS_MEMCMP(buf, src, len) == 0;
}

void * OS::FileStreamReader::readBytes(void * buf, int len)
{
	if(f){
		fread(buf, len, 1, f);
	}else{
		OS_MEMSET(buf, 0, len);
	}
	return buf;
}

void * OS::FileStreamReader::readBytesAtPos(void * buf, int len, int pos)
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

OS::Core::VariableIndex::VariableIndex(const VariableIndex& index): string_index(index.string_index)
{
	int_index = index.int_index;
	hash_value = index.hash_value;
	is_string_index = index.is_string_index;
	int_valid = index.int_valid;
}

OS::Core::VariableIndex::VariableIndex(const StringInternal& index): string_index(index)
{
	int_index = 0;
	hash_value = 0; // set by fix
	is_string_index = true;
	int_valid = false;
	fixStringIndex();
}

OS::Core::VariableIndex::VariableIndex(const StringInternal& index, const NoFixStringIndex&): string_index(index)
{
	int_index = 0;
	hash_value = 0; // set by fix
	is_string_index = true;
	int_valid = false;
	hash_value = string_index.hash();
}

OS::Core::VariableIndex::VariableIndex(StringData * index): string_index(index)
{
	int_index = 0;
	hash_value = 0; // set by fix
	is_string_index = true;
	int_valid = false;
	fixStringIndex();
}

OS::Core::VariableIndex::VariableIndex(StringData * index, const NoFixStringIndex&): string_index(index)
{
	int_index = 0;
	hash_value = 0; // set by fix
	is_string_index = true;
	int_valid = false;
	hash_value = string_index.hash();
}

OS::Core::VariableIndex::VariableIndex(OS * allocator, const OS_CHAR * index): string_index(allocator, index)
{
	int_index = 0;
	hash_value = 0; // set by fix
	is_string_index = true;
	int_valid = false;
	fixStringIndex();
}

OS::Core::VariableIndex::VariableIndex(OS * allocator, OS_INT64 index): string_index(allocator)
{
	int_index = (OS_INT)index;
	hash_value = (int)int_index;
	is_string_index = false;
	int_valid = true;
}

OS::Core::VariableIndex::VariableIndex(OS * allocator, OS_INT32 index): string_index(allocator)
{
	int_index = (OS_INT)index;
	hash_value = (int)int_index;
	is_string_index = false;
	int_valid = true;
}

OS::Core::VariableIndex::VariableIndex(OS * allocator, OS_FLOAT index, int precision): string_index(allocator)
{
	int_index = (OS_INT)index;
	if((OS_FLOAT)int_index == index){
		is_string_index = false;
		int_valid = true;
		hash_value = (int)int_index;
		// string_index = allocator->core->empty_string_data->retain();
	}else{
		// string_index = StringData(allocator, index, precision);
		string_index = StringInternal(allocator, index, precision);
		is_string_index = true;
		int_valid = false;
		hash_value = string_index.hash();
		// fixStringIndex();
	}
}

OS::Core::VariableIndex::~VariableIndex()
{
	// string_index->release();
}

bool OS::Core::VariableIndex::checkIntIndex() const
{
	OS_ASSERT(is_string_index);
	const OS_CHAR * str = string_index.toChar();
	int len = string_index.getDataSize();
	if(*str == OS_CHAR('-')){ // || *str == OS_CHAR('+')){
		str++;
		if(--len <= 0){
			return false;
		}
	}
	bool point_found = false;
	for(; len > 0; len--, str++){
		if(0 && *str == OS_CHAR('.')){
			if(point_found){
				return false;
			}
			point_found = true;
			continue;
		}
		if(*str < OS_CHAR('0') || *str > OS_CHAR('9')){
			return false;
		}
	}
	return true;
}

int OS::Core::VariableIndex::cmp(const VariableIndex& b) const
{
	if(int_valid && b.int_valid){
		return (int)(int_index - b.int_index);
	}
	if(is_string_index != b.is_string_index){
		OS_CHAR buf[128];
		if(!is_string_index){
			Utils::numToStr(buf, int_index);
			return Utils::cmp(buf, OS_STRLEN(buf), b.string_index.toChar(), b.string_index.getDataSize());
		}else{
			Utils::numToStr(buf, b.int_index);
			return Utils::cmp(string_index.toChar(), string_index.getDataSize(), buf, OS_STRLEN(buf));
		}
	}
	return string_index.cmp(b.string_index);
}

int OS::Core::VariableIndex::hash() const
{
	return hash_value; // int_valid ? (int)int_index : string_index.hash();
}

OS::Core::StringInternal OS::Core::VariableIndex::toString() const
{
	return is_string_index ? string_index : StringInternal(getAllocator(), int_index);
}

void OS::Core::VariableIndex::fixStringIndex()
{
	if(!int_valid && checkIntIndex()){
		OS_FLOAT fval;
		OS_INT ival;
		const OS_CHAR * str = string_index.toChar();
		const OS_CHAR * end = str + string_index.getDataSize();
		OS::EParseNumType num_type = Utils::parseNum(str, fval, ival, PARSE_NUM_FLAG_INT | PARSE_NUM_FLAG_FLOAT);
		switch(num_type){
		case PARSE_NUM_TYPE_FLOAT:
			ival = (OS_INT)fval;
			if((OS_FLOAT)ival != fval){
				hash_value = string_index.hash();
				return;
			}
			// no break

		case PARSE_NUM_TYPE_INT:
			if(str == end){
				int_valid = true;
				int_index = ival;
				// string_index.empty();
				// StringData * old_string_index = string_index;
				// string_index = string_index->allocator->core->empty_string_data->retain();
				// old_string_index->release();
				hash_value = (int)int_index;
				return;
			}
		}
	}
	hash_value = string_index.hash();
}

// =====================================================================

OS::Core::Value::Variable::Variable(const VariableIndex& index): VariableIndex(index)
{
	value_id = 0;
	hash_next = NULL;
	prev = NULL;
	next = NULL;
}

OS::Core::Value::Variable::Variable(const StringInternal& index): VariableIndex(index)
{
	value_id = 0;
	hash_next = NULL;
	prev = NULL;
	next = NULL;
}

OS::Core::Value::Variable::Variable(OS * allocator, const OS_CHAR * index): VariableIndex(allocator, index)
{
	value_id = 0;
	hash_next = NULL;
	prev = NULL;
	next = NULL;
}

OS::Core::Value::Variable::Variable(OS * allocator, OS_INT index): VariableIndex(allocator, index)
{
	value_id = 0;
	hash_next = NULL;
	prev = NULL;
	next = NULL;
}

OS::Core::Value::Variable::Variable(OS * allocator, int index): VariableIndex(allocator, index)
{
	value_id = 0;
	hash_next = NULL;
	prev = NULL;
	next = NULL;
}

OS::Core::Value::Variable::Variable(OS * allocator, OS_FLOAT index, int precision): VariableIndex(allocator, index, precision)
{
	value_id = 0;
	hash_next = NULL;
	prev = NULL;
	next = NULL;
}

OS::Core::Value::Variable::~Variable()
{
	OS_ASSERT(!value_id);
	OS_ASSERT(!hash_next);
	OS_ASSERT(!prev);
	OS_ASSERT(!next);
}

// =====================================================================

OS::Core::Value::Table::Table()
{
	head_mask = 0;
	heads = NULL;
	next_id = 0;
	count = 0;
	first = last = NULL;
}

OS::Core::Value::Table::~Table()
{
	OS_ASSERT(count == 0 && !first && !last);
	OS_ASSERT(!heads);
}

OS::Core::Value::Table * OS::Core::newTable()
{
	return new (malloc(sizeof(Value::Table))) Value::Table();
}

void OS::Core::deleteTable(Value::Table * table)
{
	OS_ASSERT(table);
	Value::Variable * var = table->last, * prev;

	table->count = 0;
	table->first = NULL;
	table->last = NULL;

	for(; var; var = prev){
		prev = var->prev;

		releaseValue(var->value_id);

		var->hash_next = NULL;
		var->prev = NULL;
		var->next = NULL;
		var->value_id = 0;
		var->~Variable();
		free(var);
	}
	
	// OS_ASSERT(table->count == 0 && !table->first && !table->last);
	free(table->heads);
	table->heads = NULL;
	table->~Table();
	free(table);
}

void OS::Core::addTableVariable(Value::Table * table, Value::Variable * var)
{
	OS_ASSERT(var->next == NULL);
	OS_ASSERT(!table->get(*var));

	if((table->count >> 1) >= table->head_mask){
		int new_size = table->heads ? (table->head_mask+1) * 2 : OS_DEF_VAR_HASH_SIZE;
		int alloc_size = sizeof(Value::Variable*)*new_size;
		Value::Variable ** new_heads = (Value::Variable**)malloc(alloc_size);
		OS_ASSERT(new_heads);
		OS_MEMSET(new_heads, 0, alloc_size);

		if(new_heads){
			Value::Variable ** old_heads = table->heads;
			table->heads = new_heads;
			table->head_mask = new_size-1;

			for(Value::Variable * cur = table->first; cur; cur = cur->next){
				int slot = cur->hash() & table->head_mask;
				cur->hash_next = table->heads[slot];
				table->heads[slot] = cur;
			}

			// delete [] old_heads;
			free(old_heads);
		}
	}

	int slot = var->hash() & table->head_mask;
	var->hash_next = table->heads[slot];
	table->heads[slot] = var;

	if(!table->first){
		table->first = var;    
	}else{
		OS_ASSERT(table->last);
		table->last->next = var;
		var->prev = table->last;
	}
	table->last = var;

	if(var->int_valid && table->next_id <= var->int_index){
		table->next_id = var->int_index + 1;
	}

	table->count++;
}

bool OS::Core::deleteTableVariable(Value::Table * table, const VariableIndex& index)
{
	int slot = index.hash() & table->head_mask;
	Value::Variable * cur = table->heads[slot], * chain_prev = NULL;
	for(; cur; chain_prev = cur, cur = cur->hash_next){
		if(cur->cmp(index) == 0){
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

			int value_id = cur->value_id;

			cur->next = NULL;
			cur->prev = NULL;
			cur->hash_next = NULL;
			cur->value_id = 0;

			table->count--;

			cur->~Variable();
			free(cur);

			releaseValue(value_id);
			return true;
		}
	}  
	return false;
}

OS::Core::Value::Variable * OS::Core::Value::Table::get(const VariableIndex& index)
{
	if(heads){
		Variable * cur = heads[index.hash() & head_mask];
		for(; cur; cur = cur->hash_next){
			if(cur->cmp(index) == 0){
				return cur;
			}
		}
	}
	return NULL;
}

// =====================================================================

OS::Core::Value::Array::Array()
{
}

OS::Core::Value::Array::~Array()
{
}

OS::Core::Value::Array * OS::Core::newArray()
{
	return new (malloc(sizeof(Value::Array))) Value::Array();
}

void OS::Core::deleteArray(Value::Array * arr)
{
	for(int i = arr->values.count-1; i >= 0; i--){
		releaseValue(arr->values[i]);
	}
	allocator->vectorClear(arr->values);
	deleteTable(arr);
}

// =====================================================================

OS::Core::Value::Value(int p_value_id)
{
	value_id = p_value_id;
	ref_count = 1;
	type = OS_VALUE_TYPE_NULL;
	value.number = 0;
	// properties = NULL;
	hash_next = NULL;
	prototype = NULL;
	// gc_time = 0;
}

OS::Core::Value::~Value()
{
	OS_ASSERT(type == OS_VALUE_TYPE_NULL);
	OS_ASSERT(!value.number);
	// OS_ASSERT(!properties);
	OS_ASSERT(!hash_next);
	OS_ASSERT(!prototype);
}

int OS::Core::valueToInt(Value * val)
{
	return (int)valueToNumber(val);
}

/*
OS_FLOAT OS::valueToNumber(Value * val)
{
	OS_FLOAT out;
	isValueNumber(val, &out);
	return out;
}

OS::Core::StringInternal OS::valueToString(Value * val)
{
	StringInternal out(this);
	isValueString(val, &out);
	return out;
}
*/

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

OS_FLOAT OS::Core::valueToNumber(Value * val)
{
	switch(val->type){
	case OS_VALUE_TYPE_NULL:
		return 0;

	case OS_VALUE_TYPE_BOOL:
	case OS_VALUE_TYPE_NUMBER:
		return val->value.number;

	case OS_VALUE_TYPE_STRING:
		return val->value.string_data->toFloat();

	// case OS_VALUE_TYPE_OBJECT:
	// case OS_VALUE_TYPE_ARRAY:
	// 	return val->value.table ? val->value.table->count : 0;
	}
	return 0;
}

bool OS::Core::isValueNumber(Value * val, OS_FLOAT * out)
{
	switch(val->type){
	case OS_VALUE_TYPE_NULL:
		if(out){
			*out = 0;
		}
		return true;

	case OS_VALUE_TYPE_BOOL:
	case OS_VALUE_TYPE_NUMBER:
		if(out){
			*out = val->value.number;
		}
		return true;

	case OS_VALUE_TYPE_STRING:
		if(out){
			*out = val->value.string_data->toFloat();
		}
		return true;

	// case OS_VALUE_TYPE_OBJECT:
	// case OS_VALUE_TYPE_ARRAY:
	// 	return val->value.table ? val->value.table->count : 0;
	}
	if(out){
		*out = 0;
	}
	return false;
}

OS::Core::StringInternal OS::Core::Compiler::Expression::toString()
{
	switch(type){
	case EXP_TYPE_CONST_NULL:
		return StringInternal(getAllocator());

	case EXP_TYPE_CONST_STRING:
		return token->str;

	case EXP_TYPE_CONST_NUMBER:
		// OS_ASSERT(token->str.toFloat() == token->getFloat());
		// return token->str;
		return StringInternal(getAllocator(), token->getFloat());

	case EXP_TYPE_CONST_TRUE:
		return StringInternal(getAllocator(), OS_TEXT("1"));

	case EXP_TYPE_CONST_FALSE:
		return StringInternal(getAllocator());
	}
	OS_ASSERT(false);
	return StringInternal(getAllocator());
}

OS::Core::StringInternal OS::Core::valueToString(Value * val)
{
	switch(val->type){
	case OS_VALUE_TYPE_NULL:
		return StringInternal(allocator);

	case OS_VALUE_TYPE_BOOL:
		return val->value.number ? StringInternal(allocator, OS_TEXT("1")) : StringInternal(allocator);

	case OS_VALUE_TYPE_NUMBER:
		return StringInternal(allocator, val->value.number, OS_DEF_PRECISION);

	case OS_VALUE_TYPE_STRING:
		return StringInternal(val->value.string_data);

	// case OS_VALUE_TYPE_OBJECT:
	// case OS_VALUE_TYPE_ARRAY:
	// 	return StringInternal(this, (OS_INT)(val->value.table ? val->value.table->count : 0));
	}
	return StringInternal(allocator);
}

bool OS::Core::isValueString(Value * val, StringInternal * out)
{
	switch(val->type){
	case OS_VALUE_TYPE_NULL:
		if(out){
			*out = StringInternal(allocator);
		}
		return true;

	case OS_VALUE_TYPE_BOOL:
		if(out){
			*out = StringInternal(allocator, val->value.number ? OS_TEXT("1") : OS_TEXT(""));
		}
		return true;

	case OS_VALUE_TYPE_NUMBER:
		if(out){
			*out = StringInternal(allocator, val->value.number, OS_DEF_PRECISION);
		}
		return true;

	case OS_VALUE_TYPE_STRING:
		if(out){
			*out = StringInternal(val->value.string_data);
		}
		return true;

	// case OS_VALUE_TYPE_OBJECT:
	// case OS_VALUE_TYPE_ARRAY:
	// 	return StringInternal(this, (OS_INT)(val->value.table ? val->value.table->count : 0));
	}
	if(out){
		*out = StringInternal(allocator);
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

OS::Core::Value * OS::Core::registerValue(Value * value)
{
	OS_ASSERT(value->hash_next == NULL);
	if((values.count>>1) >= values.head_mask){
		int new_size = values.heads ? (values.head_mask+1) * 2 : OS_DEF_VALUES_HASH_SIZE;
		int alloc_size = sizeof(Value*) * new_size;
		Value ** new_heads = (Value**)malloc(alloc_size); // new Value*[new_size];
		OS_ASSERT(new_heads);
		OS_MEMSET(new_heads, 0, alloc_size);

		if(new_heads){
			Value ** old_heads = values.heads;
			int old_mask = values.head_mask;

			values.heads = new_heads;
			values.head_mask = new_size-1;

			if(old_heads){
				for(int i = 0; i <= old_mask; i++){
					for(Value * value = old_heads[i], * next; value; value = next){
						next = value->hash_next;
						int slot = value->value_id & values.head_mask;
						value->hash_next = values.heads[slot];
						values.heads[slot] = value;
					}
				}
				// delete [] old_heads;
				free(old_heads);
			}
		}
	}

	int slot = value->value_id & values.head_mask;
	value->hash_next = values.heads[slot];
	values.heads[slot] = value;
	values.count++;

	return value;
}

OS::Core::Value * OS::Core::unregisterValue(int value_id)
{
	int slot = value_id & values.head_mask;
	Value * value = values.heads[slot], * prev = NULL;
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

void OS::Core::deleteValues()
{
	if(values.heads){
		while(values.count > 0){
			for(int i = 0; i <= values.head_mask; i++){
		#if 1
				for(Value * value; value = values.heads[i]; ){
					deleteValue(value);
				}
		#else
				for(Value * value; value = values_heads[i]; ){
					values_heads[i] = value->hash_next;
					// values.count--;

					value->hash_next = NULL;
					deleteValue(value);
				}
		#endif
			}
		}
		OS_ASSERT(values.count == 0);
		free(values.heads);
		values.heads = NULL;
		values.head_mask = 0;
		values.next_id = 1;
	}
}

OS::Core::Value * OS::Core::Values::get(int value_id)
{
	int slot = value_id & head_mask;
	for(Value * value = heads[slot]; value; value = value->hash_next){
		if(value->value_id == value_id){
			return value;
		}
	}
	return NULL;
}

// =====================================================================
// =====================================================================
// =====================================================================

OS::Core::StackFunction::StackFunction()
{
	func = NULL;
	cur_opcode_num = 0;
	local_vars = NULL;
}

OS::Core::StackFunction::~StackFunction()
{
	OS_ASSERT(!func);
	OS_ASSERT(!local_vars);
}

// =====================================================================
// =====================================================================
// =====================================================================

OS::Core::Strings::Strings(OS * allocator)
	:
	__get(allocator, OS_TEXT("__get")),
	__set(allocator, OS_TEXT("__set")),
	__construct(allocator, OS_TEXT("__construct")),
	__destruct(allocator, OS_TEXT("__destruct")),
	__cmp(allocator, OS_TEXT("__cmp")),
	__tostring(allocator, OS_TEXT("__tostring")),
	__tobool(allocator, OS_TEXT("__tobool")),
	__add(allocator, OS_TEXT("__add")),
	__sub(allocator, OS_TEXT("__sub")),
	__mul(allocator, OS_TEXT("__mul")),
	__div(allocator, OS_TEXT("__div")),
	__mod(allocator, OS_TEXT("__mod")),

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
	syntax_do(allocator, OS_TEXT("do")),
	syntax_while(allocator, OS_TEXT("while")),
	syntax_break(allocator, OS_TEXT("break")),
	syntax_continue(allocator, OS_TEXT("continue")),

	__dummy__(0)
{
}

// =====================================================================
// =====================================================================
// =====================================================================

OS::MemoryManager::~MemoryManager()
{
}

OS::GenericMemoryManager::GenericMemoryManager()
{
	allocated_bytes = 0;
	max_allocated_bytes = 0;
	cached_bytes = 0;
	OS_MEMSET(cached_memory_blocks, 0, sizeof(cached_memory_blocks));
	OS_MEMSET(cached_memory_count, 0, sizeof(cached_memory_count));
	real_malloc_count = 0;
}

OS::GenericMemoryManager::~GenericMemoryManager()
{
	freeCachedMemory();
	// OS_ASSERT(!allocated_bytes);
	OS_ASSERT(!cached_bytes);
}

void OS::GenericMemoryManager::freeCachedMemory()
{
	if(cached_bytes > 0){
		for(int i = 0; i < CACHED_COUNT; i++){
			int size = getCachedMemorySize(i);
			while(cached_memory_blocks[i]){
				int * p = (int*)getCachedMemory(size, i);
				OS_ASSERT(p);
				stdFree(p-1, size);
			}
		}
		OS_ASSERT(!cached_bytes);
	}
}

void * OS::GenericMemoryManager::stdAlloc(int size)
{
	real_malloc_count++;
	int * p = (int*)::malloc(size + sizeof(int));
	if(!p && cached_bytes > 0){
		freeCachedMemory();
		p = (int*)::malloc(size + sizeof(int));
		if(!p){
			return NULL;
		}
	}
	p[0] = size;
	allocated_bytes += size + sizeof(int);
	if(max_allocated_bytes < allocated_bytes){
		max_allocated_bytes = allocated_bytes;
	}
	OS_MEMSET(p+1, 0, size);
	return p+1;
}

void OS::GenericMemoryManager::stdFree(void * ptr, int size)
{
	allocated_bytes -= size + sizeof(int);
	::free(ptr);
}

void * OS::GenericMemoryManager::getCachedMemory(int size, int i)
{
	OS_ASSERT(i >= 0 && i < CACHED_COUNT);
	cached_memory_count[i]++;
	CachedMemoryBlock * block = cached_memory_blocks[i];
	if(block){
		cached_memory_blocks[i] = block->next;
		cached_bytes -= size + sizeof(int);
		int * p = (int*)block;
		p[0] = size;
		OS_MEMSET(p+1, 0, size);
		return p+1;
	}
	return stdAlloc(size);
}

void OS::GenericMemoryManager::putCachedMemory(void * p, int size, int i)
{
	OS_ASSERT(i >= 0 && i < CACHED_COUNT);
	CachedMemoryBlock * block = (CachedMemoryBlock*)p;
	block->next = cached_memory_blocks[i];
	cached_memory_blocks[i] = block;
	cached_bytes += size + sizeof(int);
}

int OS::GenericMemoryManager::getCachedMemorySize(int i)
{
	switch(i){
	case VALUE:
		return sizeof(Core::Value);

	case VARIABLE:
		return sizeof(Core::Value::Variable);

	case TABLE:
		return sizeof(Core::Value::Table);

	case ARRAY:
		return sizeof(Core::Value::Array);

	case EXPRESSION:
		return Core::Compiler::EXPRESSION_SIZE;

	case TOKENDATA:
		return sizeof(Core::TokenData);
	}
	OS_ASSERT(false);
	return 0;
}

void * OS::GenericMemoryManager::malloc(int size)
{
	if(size == sizeof(Core::Value)){ // 32 bytes
		return getCachedMemory(size, VALUE);
	}
	if(size == sizeof(Core::Value::Variable)){ // 40 bytes
		return getCachedMemory(size, VARIABLE);
	}
	if(size == sizeof(Core::Value::Table)){
		return getCachedMemory(size, TABLE);
	}
	if(size == sizeof(Core::Value::Array)){ // 56 bytes
		return getCachedMemory(size, ARRAY);
	}
	if(size == Core::Compiler::EXPRESSION_SIZE){ // 36 bytes
		return getCachedMemory(size, EXPRESSION);
	}
	if(size == sizeof(Core::TokenData)){
		return getCachedMemory(size, TOKENDATA);
	}
	return stdAlloc(size);
}

void OS::GenericMemoryManager::free(void * ptr)
{
	if(!ptr){
		return;
	}
	int * p = (int*)ptr - 1;
	int size = p[0];
	if(size == sizeof(Core::Value)){
		return putCachedMemory(p, size, VALUE);
	}
	if(size == sizeof(Core::Value::Variable)){
		return putCachedMemory(p, size, VARIABLE);
	}
	if(size == sizeof(Core::Value::Table)){
		return putCachedMemory(p, size, TABLE);
	}
	if(size == sizeof(Core::Value::Array)){
		return putCachedMemory(p, size, ARRAY);
	}
	if(size == Core::Compiler::EXPRESSION_SIZE){
		return putCachedMemory(p, size, EXPRESSION);
	}
	if(size == sizeof(Core::TokenData)){
		return putCachedMemory(p, size, TOKENDATA);
	}
	stdFree(p, size);
}

int OS::GenericMemoryManager::getPointerSize(void * ptr)
{
	if(ptr){
		return ((int*)ptr)[-1];
	}
	return 0;
}

int OS::GenericMemoryManager::getAllocatedBytes()
{
	return allocated_bytes;
}

int OS::GenericMemoryManager::getMaxAllocatedBytes()
{
	return max_allocated_bytes;
}

int OS::GenericMemoryManager::getCachedBytes()
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
		memory_manager = new GenericMemoryManager();
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

OS::Core::Core(OS * p_allocator)
{
	allocator = p_allocator;
	empty_string_data = NULL;
	strings = NULL;
	OS_MEMSET(prototypes, 0, sizeof(prototypes));
	global_vars = NULL;
}

OS::Core::~Core()
{
	OS_ASSERT(!strings);
	int i;
	for(i = 0; i < PROTOTYPES_NUMBER; i++){
		OS_ASSERT(!prototypes[i]);
	}
}

OS * OS::create(MemoryManager * manager)
{
	OS * os = new OS(manager);
	os->init();
	return os;
}

bool OS::init()
{
	return core->init();
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
	empty_string_data = StringData::alloc(allocator, "", 1);
	empty_string_data->data_size = 0;
	empty_string_data->ref_count = 2;

	strings = new (malloc(sizeof(Strings))) Strings(allocator);

	// global_vars

	for(int i = 0; i < PROTOTYPES_NUMBER; i++){
		// prototypes[i] = pushTableValue()->retain();		
		// prototypes[i] = newTable(
		prototypes[i] = new (malloc(sizeof(Value))) Value(values.next_id++); // pushNullValue()->retain();
		prototypes[i]->type = OS_VALUE_TYPE_OBJECT;
	}
	// removeStackValues(PROTOTYPES_NUMBER);

	string_values_table = newTable();
	global_vars = pushObjectValue()->retain(); pop();

	return true;
}

void OS::Core::shutdown()
{
	// freeAutoreleaseValues();
	// vectorClear(autorelease_values);
	pop(stack_values.count);
	allocator->vectorReleaseValues(temp_values);

	allocator->vectorClear(stack_values);
	allocator->vectorClear(temp_values);
	allocator->vectorClear(call_stack_funcs);
	// vectorClear(cache_values);
	
	releaseValue(global_vars); global_vars = NULL;
	deleteTable(string_values_table); string_values_table = NULL;

	for(int i = 0; i < PROTOTYPES_NUMBER; i++){
		// releaseValue(prototypes[i]);
		prototypes[i] = NULL;
	}
	deleteValues();
	// vectorClear(stack_values); // !!!

	allocator->deleteObj(strings);

	OS_ASSERT(empty_string_data->ref_count == 2);
	empty_string_data->ref_count = 1;
	empty_string_data->release();
	empty_string_data = NULL;
}

void OS::Core::resetValue(Value * val)
{
	OS_ASSERT(val);
	switch(val->type){
	case OS_VALUE_TYPE_NULL:
		// OS_ASSERT(!val->table);
		break;

	case OS_VALUE_TYPE_BOOL:
	case OS_VALUE_TYPE_NUMBER:
		val->value.number = 0;
		break;

	case OS_VALUE_TYPE_STRING:
		val->value.string_data->release();
		val->value.string_data = NULL;
		break;

	case OS_VALUE_TYPE_USERDATA:
		if(val->value.userdata.dtor){
			val->value.userdata.dtor(allocator, val->value.userdata.ptr);
		}
		free(val->value.userdata.ptr);
		val->value.userdata.ptr = NULL;
		val->value.userdata.dtor = NULL;
		break;

	case OS_VALUE_TYPE_USERPTR:
		if(val->value.userdata.dtor){
			val->value.userdata.dtor(allocator, val->value.userdata.ptr);
		}
		val->value.userdata.ptr = NULL;
		val->value.userdata.dtor = NULL;
		break;

	case OS_VALUE_TYPE_FUNCTION:
		OS_ASSERT(val->value.func.decl && val->value.func.decl->prog);
		val->value.func.decl->prog->release();
		val->value.func.decl = NULL;
		if(val->value.func.parent_func){
			OS_ASSERT(val->value.func.parent_func->type == OS_VALUE_TYPE_FUNCTION);
			releaseValue(val->value.func.parent_func);
			val->value.func.parent_func = NULL;
		}
		break;

	case OS_VALUE_TYPE_OBJECT:
		if(val->value.table){
			Value::Table * table = val->value.table;
			val->value.table = NULL;
			deleteTable(table);
		}
		break;

	case OS_VALUE_TYPE_ARRAY:
		if(val->value.arr){
			Value::Array * arr = val->value.arr;
			val->value.arr = NULL;
			deleteArray(arr);
		}
		break;
	}
	if(val->prototype){
		releaseValue(val->prototype);
		val->prototype = NULL;
	}
	val->type = OS_VALUE_TYPE_NULL;
}

void OS::Core::deleteValue(Value * val)
{
	OS_ASSERT(val);
	resetValue(val);
	unregisterValue(val->value_id);
	val->~Value();
	free(val);
}

void OS::Core::releaseValue(Value * val)
{
	OS_ASSERT(val);
	if(--val->ref_count <= 0){
		OS_ASSERT(val->ref_count == 0);
		deleteValue(val);
	}else if(val->type == OS_VALUE_TYPE_STRING && val->ref_count == 1){
		deleteTableVariable(string_values_table, VariableIndex(val->value.string_data, VariableIndex::NoFixStringIndex()));
	}
}

void OS::Core::releaseValue(int value_id)
{
	Value * val = values.get(value_id);
	if(val){		
		releaseValue(val);
	}
}

OS::Core::Value::Variable * OS::Core::setTableValue(Value::Table * table, VariableIndex& index, Value * value, bool prototype_enabled, bool setter_enabled)
{
	OS_ASSERT(table);
	OS_ASSERT(value);
	Value::Variable * var = table->get(index);
	if(var){
		OS_ASSERT(var->value_id);
		if(var->value_id != value->value_id){
			int old_value_id = var->value_id;			
			var->value_id = value->value_id;
			value->ref_count++;
			releaseValue(old_value_id);
		}
		return var;
	}
	var = new (malloc(sizeof(Value::Variable))) Value::Variable(index);
	var->value_id = value->value_id;
	value->ref_count++;
	addTableVariable(table, var);
	return var;
}

OS::Core::Value * OS::Core::getOffsValue(int offs)
{
	if(offs < 0){
		offs += stack_values.count;
	}
	OS_ASSERT(offs >= 0);
	if(offs < stack_values.count){
		return stack_values.buf[offs];
	}
	if(offs == OS_GLOBALS){
		return global_vars;
	}
	return NULL;
}

OS::Core::Value * OS::Core::pushValue(Value * val)
{
	OS_ASSERT(val);
	allocator->vectorAddItem(stack_values, val);
	val->ref_count++;
	return val;
}

OS::Core::Value * OS::Core::pushValueAutoNull(Value * val)
{
	if(val){
		return pushValue(val);
	}
	return pushNullValue();
}

OS::Core::Value * OS::Core::pushNullValue()
{
	Value * val = new (malloc(sizeof(Value))) Value(values.next_id++);
	allocator->vectorAddItem(stack_values, registerValue(val));
	return val;
}

OS::Core::Value * OS::Core::pushBoolValue(bool val)
{
	Value * res = pushNullValue();
	res->prototype = prototypes[PROTOTYPE_BOOL]->retain();
	res->value.number = val;
	res->type = OS_VALUE_TYPE_BOOL;
	return res;
}

OS::Core::Value * OS::Core::pushNumberValue(OS_FLOAT val)
{
	Value * res = pushNullValue();
	res->prototype = prototypes[PROTOTYPE_NUMBER]->retain();
	res->value.number = val;
	res->type = OS_VALUE_TYPE_NUMBER;
	return res;
}

OS::Core::Value * OS::Core::pushStringValue(const StringInternal& str)
{
#if 1
	VariableIndex index(str, VariableIndex::NoFixStringIndex());
	Value::Variable * var = string_values_table->get(index);
	if(var){
		Value * value = values.get(var->value_id);
		return pushValue(value);
	}
	Value * value = pushNullValue();
	value->prototype = prototypes[PROTOTYPE_STRING]->retain();
	value->value.string_data = str.toData()->retain();
	value->type = OS_VALUE_TYPE_STRING;

	var = new (malloc(sizeof(Value::Variable))) Value::Variable(index);
	var->value_id = value->value_id;
	value->ref_count++;
	addTableVariable(string_values_table, var);
	return value;
#else
	Value * res = pushNullValue();
	res->prototype = prototypes[PROTOTYPE_STRING]->retain();
	res->value.string_data = val.toData()->retain();
	res->type = OS_VALUE_TYPE_STRING;
	return res;
#endif
}

OS::Core::Value * OS::Core::pushStringValue(const OS_CHAR * val)
{
#if 1
	return pushStringValue(StringInternal(allocator, val));
#else
	Value * res = pushNullValue();
	res->prototype = prototypes[PROTOTYPE_STRING]->retain();
	res->value.string_data = StringData::alloc(allocator, val, OS_STRLEN(val));
	res->type = OS_VALUE_TYPE_STRING;
	return res;
#endif
}

OS::Core::Value * OS::Core::pushCFunctionValue(OS_CFunction func, void * user_param)
{
	Value * res = pushNullValue();
	res->value.cfunc.func = func;
	res->value.cfunc.user_param = user_param;
	res->type = OS_VALUE_TYPE_CFUNCTION;
	return res;
}

OS::Core::Value * OS::Core::pushUserdataValue(int data_size, OS_UserDataDtor dtor)
{
	Value * res = pushNullValue();
	res->value.userdata.ptr = malloc(data_size);
	res->value.userdata.dtor = dtor;
	res->type = OS_VALUE_TYPE_USERDATA;
	return res;
}

OS::Core::Value * OS::Core::pushUserPointerValue(void * data, OS_UserDataDtor dtor)
{
	Value * res = pushNullValue();
	res->value.userdata.ptr = data;
	res->value.userdata.dtor = dtor;
	res->type = OS_VALUE_TYPE_USERPTR;
	return res;
}

OS::Core::Value * OS::Core::pushObjectValue()
{
	Value * res = pushNullValue();
	res->prototype = prototypes[PROTOTYPE_OBJECT]->retain();
	res->type = OS_VALUE_TYPE_OBJECT;
	return res;
}

OS::Core::Value * OS::Core::pushArrayValue()
{
	Value * res = pushNullValue();
	res->prototype = prototypes[PROTOTYPE_ARRAY]->retain();
	res->type = OS_VALUE_TYPE_ARRAY;
	return res;
}

void OS::Core::pop(int offs, int count)
{
	int start = stack_values.count + offs;
	if(start < 0){
		count += start;
		start = 0;
	}
	int i, end = start + count;
	if(end > stack_values.count){
		end = stack_values.count;
	}
	for(i = start; i < end; i++){
		Value * val = stack_values.buf[i];
		allocator->vectorAddItem(temp_values, val);
		// releaseValue(val);
	}
	count = stack_values.count - end;
	if(count > 0){
		for(i = 0; i < count; i++){
			stack_values.buf[start+i] = stack_values.buf[end+i];
		}
	}
	stack_values.count -= end - start;

	if(1 || temp_values.count > 16){
		allocator->vectorReleaseValues(temp_values);
	}
}

void OS::Core::pop(int count)
{
	pop(-count, count);
}

void OS::pushNull()
{
	core->pushNullValue();
}

void OS::pushNumber(OS_INT16 val)
{
	core->pushNumberValue((OS_FLOAT)val);
}

void OS::pushNumber(OS_INT32 val)
{
	core->pushNumberValue((OS_FLOAT)val);
}

void OS::pushNumber(OS_INT64 val)
{
	core->pushNumberValue((OS_FLOAT)val);
}

void OS::pushNumber(double val)
{
	core->pushNumberValue((OS_FLOAT)val);
}

void OS::pushBool(bool val)
{
	core->pushBoolValue(val);
}

void OS::pushString(const OS_CHAR * val)
{
	core->pushStringValue(val);
}

void OS::pushString(const Core::StringInternal& val)
{
	core->pushStringValue(val);
}

void OS::pushCFunction(OS_CFunction func, void * user_param)
{
	core->pushCFunctionValue(func, user_param);
}

void * OS::pushUserData(int data_size, OS_UserDataDtor dtor)
{
	Core::Value * val = core->pushUserdataValue(data_size, dtor);
	return val->value.userdata.ptr;
}

void * OS::pushUserPointer(void * data, OS_UserDataDtor dtor)
{
	Core::Value * val = core->pushUserPointerValue(data, dtor);
	return val->value.userdata.ptr;
}

void OS::newObject()
{
	core->pushObjectValue();
}

void OS::newArray()
{
	core->pushArrayValue();
}

void OS::pushStackValue(int offs)
{
	core->pushValueAutoNull(core->getOffsValue(offs));
}

void OS::pushValueById(int id)
{
	core->pushValueAutoNull(core->values.get(id));
}

void OS::pushValue(const Value& value)
{
	core->pushValueAutoNull(value.value);
}

OS::Value OS::getStackValue(int offs)
{
	return Value(this, core->getOffsValue(offs));
}

OS::Value OS::getStackValueById(int id)
{
	return Value(this, core->values.get(id));
}

void OS::pop(int start_offs, int count)
{
	core->pop(start_offs, count);
}

void OS::pop(int count)
{
	core->pop(count);
}

void OS::popAll()
{
	core->pop(core->stack_values.count);
}

OS_FLOAT OS::toNumber(int offs)
{
	Core::Value * val = core->getOffsValue(offs);
	if(val){
		return core->valueToNumber(val);
	}
	return 0;
}

bool OS::isNumber(int offs, OS_FLOAT * out)
{
	Core::Value * val = core->getOffsValue(offs);
	if(val){
		return core->isValueNumber(val, out);
	}
	if(out){
		*out = 0;
	}
	return false;
}

OS::String OS::toString(int offs)
{
	Core::Value * val = core->getOffsValue(offs);
	if(val){
		return core->valueToString(val);
	}
	return String(this);
}

bool OS::isString(int offs, String * out)
{
	Core::Value * val = core->getOffsValue(offs);
	if(val){
		if(out){
			Core::StringInternal str(this);
			if(core->isValueString(val, &str)){
				*out = str;
				return true;
			}
			return false;
		}else{
			return core->isValueString(val);
		}
	}
	if(out){
		*out = String(this);
	}
	return false;
}

OS_EValueType OS::getType(int offs)
{
	Core::Value * val = core->getOffsValue(offs);
	return val ? val->type : OS_VALUE_TYPE_UNKNOWN;
}

OS_EValueType OS::getTypeById(int id)
{
	Core::Value * val = core->values.get(id);
	return val ? val->type : OS_VALUE_TYPE_UNKNOWN;
}

bool OS::isType(OS_EValueType type, int offs)
{
	Core::Value * val = core->getOffsValue(offs);
	return val && val->type == type;
}

bool OS::isObject(int offs)
{
	Core::Value * val = core->getOffsValue(offs);
	if(val){
		switch(val->type){
		case OS_VALUE_TYPE_OBJECT:
		case OS_VALUE_TYPE_ARRAY:
			return true;
		}
	}
	return false;
}

bool OS::isArray(int offs)
{
	return isType(OS_VALUE_TYPE_ARRAY, offs);
}

bool OS::isFunction(int offs)
{
	Core::Value * val = core->getOffsValue(offs);
	if(val){
		switch(val->type){
		case OS_VALUE_TYPE_CFUNCTION:
		case OS_VALUE_TYPE_FUNCTION:
			return true;
		}
	}
	return false;
}

bool OS::Core::isValueInstanceOf(Value * val, Value * prototype_val)
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

bool OS::isInstanceOf(int value_offs, int prototype_offs)
{
	Core::Value * val = core->getOffsValue(value_offs);
	Core::Value * prototype_val = core->getOffsValue(prototype_offs);
	if(val && prototype_val){
		return core->isValueInstanceOf(val, prototype_val);
	}
	return false;
}

void OS::setProperty(int table_offs, int pop_count, bool prototype_enabled, bool setter_enabled)
{
	Core::Value * table_arg = core->getOffsValue(table_offs);
	if(table_arg && core->stack_values.count >= 2){
		Core::Value * index_arg = core->stack_values.buf[core->stack_values.count - 2];
		Core::Value * value_arg = core->stack_values.buf[core->stack_values.count - 1];
		if(table_arg->type == OS_VALUE_TYPE_ARRAY){
		}
		if(table_arg->type == OS_VALUE_TYPE_OBJECT || table_arg->type == OS_VALUE_TYPE_ARRAY){
			Core::Value::Table * table = table_arg->value.table;
			if(!table){
				table = table_arg->value.table = core->newTable();
			}
			switch(index_arg->type){
			case OS_VALUE_TYPE_BOOL:
				core->setTableValue(table, Core::VariableIndex(this, (OS_INT)index_arg->value.number), value_arg, prototype_enabled, setter_enabled);
				pop(pop_count);
				return;

			case OS_VALUE_TYPE_NUMBER:
				core->setTableValue(table, Core::VariableIndex(this, index_arg->value.number), value_arg, prototype_enabled, setter_enabled);
				pop(pop_count);
				return;

			case OS_VALUE_TYPE_STRING:
				core->setTableValue(table, Core::VariableIndex(index_arg->value.string_data), value_arg, prototype_enabled, setter_enabled);
				pop(pop_count);
				return;

			default:
				OS_ASSERT(false); // TODO: not yet implemented
			}
		}
	}
	// error
	pop(pop_count);
}

OS::Core::Value * OS::Core::pushPropertyValue(Value * table_value, VariableIndex& index, bool prototype_enabled, bool getter_enabled)
{
	OS_ASSERT(table_value->type == OS_VALUE_TYPE_OBJECT);
	if(table_value->type != OS_VALUE_TYPE_OBJECT){
		return pushNullValue();
	}
	Value::Variable * var = NULL;
	Value::Table * table = table_value->value.table;
	if(table && (var = table->get(index))){
		return pushValueAutoNull(values.get(var->value_id));		
	}
	if(prototype_enabled){
		// vectorClear(cache_values);
		// vectorAddItem(cache_values, table_value);
		Value * cur_value = table_value;
		while(cur_value->prototype){ // && !cache_values.contains(cur_value->prototype)){
			cur_value = cur_value->prototype;
			if(cur_value->type != OS_VALUE_TYPE_OBJECT){
				break;
			}
			Value::Table * cur_table = cur_value->value.table;
			if(cur_table && (var = cur_table->get(index))){
				return pushValueAutoNull(values.get(var->value_id));
			}
		}
	}
	if(getter_enabled){
		VariableIndex getter_index(strings->__get);
		if(table){
			var = table->get(getter_index);
		}
		if(!var && prototype_enabled){
			// vectorClear(cache_values);
			// vectorAddItem(cache_values, table_value);
			Value * cur_value = table_value;
			while(cur_value->prototype){ // && !cache_values.contains(cur_value->prototype)){
				cur_value = cur_value->prototype;
				if(cur_value->type != OS_VALUE_TYPE_OBJECT){
					break;
				}
				Value::Table * cur_table = cur_value->value.table;
				if(cur_table && (var = cur_table->get(getter_index))){
					break;
				}
			}
		}
		if(var){
			Value * value = values.get(var->value_id);
			if(value){
				switch(value->type){
				case OS_VALUE_TYPE_OBJECT:
					return pushPropertyValue(value, index, prototype_enabled, getter_enabled);

				case OS_VALUE_TYPE_CFUNCTION:
				case OS_VALUE_TYPE_FUNCTION:
					pushValue(table_value);
					pushValue(value); // func
					call(1, 1);
					return getOffsValue(-1);

				default:
					// error
					;
				}			
			}
		}
	}
	// error
	return pushNullValue();
}

void OS::getProperty(int table_offs, int pop_count, bool prototype_enabled, bool getter_enabled)
{
	Core::Value * table_arg = core->getOffsValue(table_offs);
	if(table_arg && core->stack_values.count >= 1){
		Core::Value * index_arg = core->stack_values.buf[core->stack_values.count - 1];
		if(table_arg->type == OS_VALUE_TYPE_OBJECT){
			table_arg->retain();
			index_arg->retain();
			pop(pop_count);

			switch(index_arg->type){
			case OS_VALUE_TYPE_BOOL:
				core->pushPropertyValue(table_arg, Core::VariableIndex(this, (OS_INT)index_arg->value.number), prototype_enabled, getter_enabled);
				break;

			case OS_VALUE_TYPE_NUMBER:
				core->pushPropertyValue(table_arg, Core::VariableIndex(this, index_arg->value.number), prototype_enabled, getter_enabled);
				break;

			case OS_VALUE_TYPE_STRING:
				core->pushPropertyValue(table_arg, Core::VariableIndex(index_arg->value.string_data), prototype_enabled, getter_enabled);
				break;

			default:
				// error				
				core->releaseValue(table_arg);
				core->releaseValue(index_arg);
				pushNull();
				return;
			}
			core->releaseValue(table_arg);
			core->releaseValue(index_arg);
			return;
		}
	}
	// error
	pop(pop_count);
	pushNull();
}

void OS::Core::enterFunction()
{
	Value * func = NULL; // pop();
	if(func->type == OS_VALUE_TYPE_CFUNCTION){
		func->value.cfunc.func(allocator, func->value.cfunc.user_param);
		return;
	}
	if(func->type != OS_VALUE_TYPE_FUNCTION){
		OS_ASSERT(false);
		fatal_error = OS_FATAL_ERROR_OPCODE;
		return;
	}
	int num_params = 0; // stack_count; // pop()->toInt();

	StackFunction * stack_func = new (malloc(sizeof(StackFunction))) StackFunction();
	allocator->vectorAddItem(call_stack_funcs, stack_func);

	stack_func->local_vars;
}

void OS::Core::leaveFunction()
{
	// Value * num_results = pop();
}

int OS::Core::call(int params, int ret_values)
{
	pop(params+1);
	while(ret_values-- > 0){
		pushNullValue();
	}
	return 0;
}

bool OS::compile(const Core::StringInternal& str)
{
	Core::Tokenizer tokenizer(this);
	tokenizer.parseText(str);

	Core::Compiler compiler(&tokenizer);
	return compiler.compile();
}

bool OS::compile()
{
	Core::StringInternal str(this);
	Core::Value * val = core->getOffsValue(-1);
	if(val && core->isValueString(val, &str)){
		pop(1);
		return compile(str);
	}
	pop(1);
	pushNull();
	return false;
}

int OS::call(int params)
{
	pop();
	return 0;
}

int OS::eval(OS_CHAR * str)
{
	return eval(Core::StringInternal(this, str));
}

int OS::eval(const Core::StringInternal& str)
{
	pushString(str);
	compile();
	return call(1);
}

// =====================================================================
// =====================================================================
// =====================================================================

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
