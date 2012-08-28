#ifndef __OBJECT_SCRIPT_H__
#define __OBJECT_SCRIPT_H__

/******************************************************************************
* Copyright (C) 2012 Evgeniy Golovin (evgeniy.golovin@unitpoint.ru)
* Copyright (C) 2012 unitpoint.com (support@unitpoint.ru)
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

// #include <vector>
#include <stdarg.h>
#include <stdio.h>
#include <assert.h>
#include <ctype.h>
#include <math.h>
#include <string.h>
#include <malloc.h>
#include <new.h>
#include <vadefs.h>
#include <stdlib.h>

#define OS_ASSERT assert

#define OS_MEMCMP memcmp
#define OS_MEMMOVE memmove
#define OS_MEMSET memset
#define OS_MEMCPY memcpy
#define OS_STRLEN strlen
#define OS_STRCMP strcmp
#define OS_STRNCMP strncmp
#define OS_STRCHR strchr
#define OS_STRSTR strstr
#define OS_ISSPACE isspace
#define OS_VSNPRINTF vsnprintf_s
#define OS_SNPRINTF __snprintf__

#define OS_MATH_POW pow
#define OS_MATH_FLOOR floor
#define OS_MATH_FMOD fmod

#define OS_CHAR char
#define OS_INT __int64
#define OS_FLOAT double
#define OS_INT32 __int32
#define OS_INT64 __int64
#define OS_BYTE unsigned char
#define OS_U16 unsigned short
#define OS_INT16 short

#define OS_TEXT(s) s

#define OS_DEF_VAR_HASH_SIZE 4
#define OS_DEF_VALUES_HASH_SIZE 16

#define OS_AUTO_PRECISION 20
#define OS_DEF_PRECISION OS_AUTO_PRECISION

#define OS_DEF_FMT_BUF_SIZE 1024*10

#define OS_INFINITE_LOOP_OPCODES 10000000000
#define OS_CALL_STACK_MAX_SIZE 200

#define OS_VERSION OS_TEXT("0.9-vm2")
#define OS_COMPILED_HEADER OS_TEXT("OBJECTSCRIPT")
#define OS_DEBUGINFO_HEADER OS_TEXT("OBJECTSCRIPT.DEBUGINFO")
#define OS_SOURCECODE_EXT OS_TEXT(".os")
#define OS_COMPILED_EXT OS_TEXT(".osc")
#define OS_DEBUG_INFO_EXT OS_TEXT(".osd")
#define OS_DEBUG_OPCODES_FILENAME OS_TEXT("os-opcodes.txt")

#define OS_MEMORY_MANAGER_PAGE_BLOCKS 32

#define OS_DEBUGGER_SAVE_NUM_LINES 11

#define FUNC_VAL_ONE_PARENT

namespace ObjectScript
{

	class OS;

	typedef void (*OS_UserDataDtor)(OS*, void * data, void * user_param);
	typedef int (*OS_CFunction)(OS*, int params, int upvalues, int need_ret_values, void * user_param);

	enum OS_ESettings
	{
		OS_SETTING_CREATE_DEBUG_OPCODES,
		OS_SETTING_CREATE_DEBUG_INFO,
		OS_SETTING_RECOMPILE_SOURCECODE,
	};

	enum OS_EValueType
	{
		OS_VALUE_TYPE_UNKNOWN,
		OS_VALUE_TYPE_NULL,
		OS_VALUE_TYPE_BOOL,
		OS_VALUE_TYPE_NUMBER,
		OS_VALUE_TYPE_STRING,
		OS_VALUE_TYPE_ARRAY,
		OS_VALUE_TYPE_OBJECT,
		OS_VALUE_TYPE_USERDATA,
		OS_VALUE_TYPE_USERPTR,
		OS_VALUE_TYPE_FUNCTION,
		OS_VALUE_TYPE_CFUNCTION,
		OS_VALUE_TYPE_WEAKREF,
	};

	enum // OS_ValueRegister
	{
		OS_REGISTER_GLOBALS = 0x10000000,
		OS_REGISTER_USERPOOL
	};

	enum
	{
		OS_GC_PHASE_MARK,
		OS_GC_PHASE_SWEEP,
	};

	enum OS_EFatalError
	{
		OS_FATAL_ERROR_OPCODE,
	};

	enum
	{
		OS_WARNING	= 1<<0,
		OS_ERROR	= 1<<1
	};

	enum OS_EOpcode {
		// binary operators

		OP_LOGIC_PTR_EQ,	// ===
		OP_LOGIC_PTR_NE,	// !==
		OP_LOGIC_EQ,		// ==
		OP_LOGIC_NE,		// !=
		OP_LOGIC_GE,		// >=
		OP_LOGIC_LE,		// <=
		OP_LOGIC_GREATER,	// >
		OP_LOGIC_LESS,		// <

		OP_BIT_AND,	// &
		OP_BIT_OR,	// |
		OP_BIT_XOR,	// ^

		OP_ADD, // +
		OP_SUB, // -
		OP_MUL, // *
		OP_DIV, // /
		OP_MOD, // %
		OP_LSHIFT, // <<
		OP_RSHIFT, // >>
		OP_POW, // **

		OP_CONCAT, // ..

		// unary operators

		OP_BIT_NOT,		// ~
		OP_PLUS,		// +
		OP_NEG,			// -
		OP_LENGTH,		// #

		/*
		OP_LOGIC_BOOL,
		OP_LOGIC_NOT,

		OP_VALUE_OF,
		OP_NUMBER_OF,
		OP_STRING_OF,
		OP_ARRAY_OF,
		OP_OBJECT_OF,
		OP_USERDATA_OF,
		OP_FUNCTION_OF,
		OP_CLONE,
		*/
	};

	class OS
#ifdef OBJECT_SCRIPT_EXTENDS_CLASS
		: public OBJECT_SCRIPT_EXTENDS_CLASS
#endif
	{
	public:

		class MemoryManager
		{
		public:

			// MemoryManager();
			virtual ~MemoryManager();

			virtual void * malloc(int size) = 0;
			virtual void free(void * p) = 0;

			virtual int getPointerSize(void * p) = 0;
			virtual int getAllocatedBytes() = 0;
			virtual int getMaxAllocatedBytes() = 0;
			virtual int getCachedBytes() = 0;
		};

		class SmartMemoryManager: public MemoryManager
		{
		protected:

			int allocated_bytes;
			int max_allocated_bytes;
			int cached_bytes;

			struct PageDesc
			{
				int block_size;
				int num_blocks;

				int allocated_bytes;
			};

			struct Page
			{
				int index;
				int num_cached_blocks;
				Page * next_page;
			};

			struct CachedBlock
			{
				Page * page;
				CachedBlock * next;
			};

			struct MemBlock
			{
				Page * page;
				int block_size;
			};

			enum {
				MAX_PAGE_COUNT = 17
			};

			PageDesc page_desc[MAX_PAGE_COUNT];
			int num_page_desc;

			Page * pages[MAX_PAGE_COUNT];

			CachedBlock * cached_blocks[MAX_PAGE_COUNT];

			int stat_malloc_count;
			int stat_free_count;

			void registerPageDesc(int block_size, int num_blocks);
			void sortPageDesc();

			void * allocFromCachedBlock(int i);
			void * allocFromPageType(int i);
			void freeMemBlock(MemBlock*);

			void freeCachedMemory(int new_cached_bytes);

			void * stdAlloc(int size);
			void stdFree(void * p);

		public:

			SmartMemoryManager();
			~SmartMemoryManager();
			
			void * malloc(int size);
			void free(void * p);

			int getPointerSize(void * p);
			int getAllocatedBytes();
			int getMaxAllocatedBytes();
			int getCachedBytes();
		};

		struct Utils
		{
			static bool parseNum(const OS_CHAR *& str, OS_FLOAT& fval);

			static OS_CHAR * numToStr(OS_CHAR*, OS_INT value);
			static OS_CHAR * numToStr(OS_CHAR*, OS_FLOAT value, int precision = OS_AUTO_PRECISION);

			static OS_INT strToInt(const OS_CHAR*);
			static OS_FLOAT strToFloat(const OS_CHAR*);

			static int keyToHash(const OS_CHAR*, int size);

			static int cmp(const void * buf1, int len1, const void * buf2, int len2);
			static int cmp(const void * buf1, int len1, const void * buf2, int len2, int maxLen);
		};

	protected:

		template<class T>
		struct Vector
		{
			T * buf;
			int capacity;
			int count;

			Vector()
			{
				buf = NULL;
				capacity = 0;
				count = 0;
			}
			~Vector()
			{
				OS_ASSERT(!buf && !capacity && !count);
			}
			T& operator [] (int i)
			{
				OS_ASSERT(i >= 0 && i < count);
				return buf[i];
			}
			const T& operator [] (int i) const
			{
				OS_ASSERT(i >= 0 && i < count);
				return buf[i];
			}

			const T& lastElement() const
			{
				OS_ASSERT(count > 0);
				return buf[count-1];
			}

			bool contains(const T& val) const 
			{
				for(int i = count-1; i >= 0; i--){
					if(buf[i] == val){
						return true;
					}
				}
				return false;
			}

			int indexOf(const T& val) const
			{
				for(int i = 0; i < count; i++){
					if(buf[i] == val){
						return i;
					}
				}
				return -1;
			}
		};

		template<class T> void vectorReserveCapacity(Vector<T>& vec, int new_capacity)
		{
			if(vec.capacity < new_capacity){
				vec.capacity = vec.capacity > 0 ? vec.capacity*2 : 4;
				if(vec.capacity < new_capacity){
					vec.capacity = new_capacity; // (capacity+3) & ~3;
				}
				T * new_buf = (T*)malloc(sizeof(T)*vec.capacity);
				OS_ASSERT(new_buf);
				for(int i = 0; i < vec.count; i++){
					new (new_buf+i) T(vec.buf[i]);
					vec.buf[i].~T();
				}
				free(vec.buf);
				vec.buf = new_buf;
			}
		}

		template<class T> void vectorReserveCapacityExact(Vector<T>& vec, int capacity)
		{
			if(vec.capacity < capacity){
				vec.capacity = capacity;
				T * new_buf = (T*)malloc(sizeof(T)*vec.capacity);
				OS_ASSERT(new_buf);
				for(int i = 0; i < vec.count; i++){
					new (new_buf+i) T(vec.buf[i]);
					vec.buf[i].~T();
				}
				free(vec.buf);
				vec.buf = new_buf;
			}
		}

		template<class T> void vectorAddItem(Vector<T>& vec, const T& val)
		{
			vectorReserveCapacity(vec, vec.count+1);
			new (vec.buf + vec.count++) T(val);
		}

		template<class T> void vectorClear(Vector<T>& vec)
		{
			for(int i = 0; i < vec.count; i++){
				vec.buf[i].~T();
			}
			free(vec.buf);
			vec.buf = NULL;
			vec.capacity = 0;
			vec.count = 0;
		}

		template<class T> void vectorReleaseItems(Vector<T>& vec)
		{
			for(int i = 0; i < vec.count; i++){
				vec.buf[i]->release();
			}
			// free(vec.buf);
			// vec.buf = NULL;
			// vec.capacity = 0;
			vec.count = 0;
		}

		template<class T> void vectorDeleteItems(Vector<T*>& vec)
		{
			for(int i = 0; i < vec.count; i++){
				T * item = vec.buf[i];
				item->~T();
				free(item);
			}
			// free(vec.buf);
			// vec.buf = NULL;
			// vec.capacity = 0;
			vec.count = 0;
		}

		template<class T> void vectorInsertAtIndex(Vector<T>& vec, int i, const T& val)
		{
			OS_ASSERT(i >= 0 && i <= vec.count);
			if(vec.count+1 >= vec.capacity){
				vectorReserveCapacity(vec, vec.capacity > 0 ? vec.capacity*2 : 4);
			}
			for(int j = vec.count-1; j >= i; j--){
				new (vec.buf+j+1) T(vec.buf[j]);
				vec.buf[j].~T();
			}
			new (vec.buf+i) T(val);
			vec.count++;
		}

		template<class T> void vectorRemoveAtIndex(Vector<T>& vec, int i)
		{
			OS_ASSERT(i >= 0 && i < vec.count);
			// T val = vec.buf[i];
			vec.buf[i].~T();
			for(i++; i < vec.count; i++){
				new (vec.buf+i-1) T(vec.buf[i]);
				vec.buf[i].~T();
			}
			vec.count--;
			// return val;
		}

		template<class T> void releaseObj(T *& obj)
		{
			if(--obj->ref_count <= 0){
				OS_ASSERT(obj->ref_count == 0);
				obj->~T();
				free(obj);
			}
		}

		template<class T> void deleteObj(T *& obj)
		{
			if(obj){
				obj->~T();
				free(obj);
				obj = NULL;
			}
		}

		class Core
		{
		public:

			class StreamReader;
			class StreamWriter
			{
			public:

				OS * allocator;

				StreamWriter(OS*);
				virtual ~StreamWriter();

				virtual int getPos() const = 0;
				virtual void setPos(int) = 0;
				
				virtual int getSize() const = 0;

				virtual void readFromStream(StreamReader*);

				virtual void writeBytes(const void*, int len) = 0;
				virtual void writeBytesAtPos(const void*, int len, int pos) = 0;

				virtual void writeByte(int);
				virtual void writeByteAtPos(int value, int pos);

				virtual void writeUVariable(int);

				virtual void writeU16(int);
				virtual void writeU16AtPos(int value, int pos);

				virtual void writeInt16(int);
				virtual void writeInt16AtPos(int value, int pos);

				virtual void writeInt32(int);
				virtual void writeInt32AtPos(int value, int pos);

				virtual void writeInt64(OS_INT64);
				virtual void writeInt64AtPos(OS_INT64 value, int pos);

				virtual void writeFloat(float);
				virtual void writeFloatAtPos(float value, int pos);

				virtual void writeDouble(double);
				virtual void writeDoubleAtPos(double value, int pos);
			};

			class MemStreamWriter: public StreamWriter
			{
			public:

				Vector<OS_BYTE> buffer;
				int pos;

				MemStreamWriter(OS*);
				~MemStreamWriter();

				int getPos() const;
				void setPos(int);
				
				int getSize() const;

				void writeBytes(const void*, int len);
				void writeBytesAtPos(const void*, int len, int pos);

				void writeByte(int);
				void writeByteAtPos(int value, int pos);
			};

			class FileStreamWriter: public StreamWriter
			{
			public:

				FILE * f;

				FileStreamWriter(OS*, const char * filename);
				FileStreamWriter(OS*, FILE * f);
				~FileStreamWriter();

				int getPos() const;
				void setPos(int);
				
				int getSize() const;

				void writeBytes(const void*, int len);
				void writeBytesAtPos(const void*, int len, int pos);
			};

			class StreamReader
			{
			public:

				OS * allocator; // if NULL then buffer will not be freed

				StreamReader(OS*);
				virtual ~StreamReader();

				virtual int getPos() const = 0;
				virtual void setPos(int) = 0;
				
				virtual int getSize() const = 0;

				virtual void movePos(int len) = 0;
				virtual bool checkBytes(void*, int len) = 0;

				virtual void * readBytes(void*, int len) = 0;
				virtual void * readBytesAtPos(void*, int len, int pos) = 0;

				virtual OS_BYTE readByte();
				virtual OS_BYTE readByteAtPos(int pos);

				virtual int readUVariable();

				virtual OS_U16 readU16();
				virtual OS_U16 readU16AtPos(int pos);

				virtual OS_INT16 readInt16();
				virtual OS_INT16 readInt16AtPos(int pos);

				virtual OS_INT32 readInt32();
				virtual OS_INT32 readInt32AtPos(int pos);

				virtual OS_INT64 readInt64();
				virtual OS_INT64 readInt64AtPos(int pos);

				virtual float readFloat();
				virtual float readFloatAtPos(int pos);

				virtual double readDouble();
				virtual double readDoubleAtPos(int pos);
			};

			class MemStreamReader: public StreamReader
			{
			public:

				OS_BYTE * buffer;
				int size;
				int pos;

				// if allocator is NULL then buffer will not be freed
				MemStreamReader(OS*, int buf_size);
				MemStreamReader(OS*, OS_BYTE * buf, int buf_size);
				~MemStreamReader();

				int getPos() const;
				void setPos(int);
				
				int getSize() const;

				void movePos(int len);
				bool checkBytes(void*, int len);

				void * readBytes(void*, int len);
				void * readBytesAtPos(void*, int len, int pos);

				OS_BYTE readByte();
				OS_BYTE readByteAtPos(int pos);
			};

			class FileStreamReader: public StreamReader
			{
			public:

				FILE * f;

				FileStreamReader(OS*, const char * filename);
				FileStreamReader(OS*, FILE * f);
				~FileStreamReader();

				int getPos() const;
				void setPos(int);
				
				int getSize() const;

				void movePos(int len);
				bool checkBytes(void*, int len);

				void * readBytes(void*, int len);
				void * readBytesAtPos(void*, int len, int pos);
			};

			class StringData
			{
			public:

				OS * allocator;
				int ref_count;
				int allocated_bytes;
				int data_size;

				const OS_CHAR * toChar() const { return (OS_CHAR*)(this + 1); }
				static StringData * toData(const OS_CHAR * s){ return (StringData*)s-1; }
				void * toMemory() const { return (void*)(this + 1); }

				int getAllocatedBytes() const { return allocated_bytes; }
				int getDataSize() const { return data_size; }
				int getLen() const { return data_size/sizeof(OS_CHAR); }

				const OS_CHAR * toCharSafely() const { return this ? (OS_CHAR*)(this + 1) : OS_TEXT(""); }

				static StringData * alloc(OS*, int size, const void *, int data_size);
				static StringData * alloc(OS*, int size, const void * buf1, int len1, const void * buf2, int len2);
				static StringData * alloc(OS*, int size, const void * buf1, int len1, const void * buf2, int len2, const void * buf3, int len3);
				static void free(StringData*);

				static StringData * alloc(OS*, const void *, int data_size);
				static StringData * alloc(int size, StringData*);
				static StringData * alloc(StringData*);
				static StringData * alloc(int size, StringData*, int data_size);
				static StringData * alloc(StringData*, int data_size);

				static StringData * append(StringData * self, StringData * b);
				static StringData * append(StringData * self, const void *, int data_size);

				StringData * retain();
				void release();

				// StringData * retainSafely();
				// void releaseSafely();

				OS_INT toInt() const;
				OS_FLOAT toFloat() const;

				int cmp(const StringData*) const;
				int cmp(const void *, int data_size) const;
				int hash() const;

			private:

				StringData();
				StringData(const StringData&);
				StringData& operator=(const StringData&);

				~StringData()
				{
					OS_ASSERT(allocated_bytes == 0);
					OS_ASSERT(data_size == 0);
				}
			};

			class String // doesn't retain OS, strings inside of OS must use OS::Core::String instead of OS::String
			{
			protected:

				// StringData * data;
				const OS_CHAR * str;

			public:

				String(OS*);
				String(OS*, const OS_CHAR*);
				String(OS*, OS_CHAR, int count);
				String(OS*, const void*, int size);
				String(OS*, const void * buf1, int len1, const void * buf2, int len2);
				String(OS*, const void * buf1, int len1, const void * buf2, int len2, const void * buf3, int len3);
				String(const String&);
				String(StringData*);
				String(OS*, OS_INT value);
				String(OS*, OS_FLOAT value, int precision = OS_DEF_PRECISION);
				~String();

				static String format(OS*, int temp_buf_size, const OS_CHAR * fmt, ...);
				static String format(OS*, int temp_buf_size, const OS_CHAR * fmt, va_list);
				static String format(OS*, const OS_CHAR * fmt, ...);
				static String format(OS*, const OS_CHAR * fmt, va_list);

				String& setFormat(int temp_buf_size, const OS_CHAR * fmt, ...);
				String& setFormat(int temp_buf_size, const OS_CHAR * fmt, va_list);
				String& setFormat(const OS_CHAR * fmt, ...);
				String& setFormat(const OS_CHAR * fmt, va_list);

				StringData * toData() const { return StringData::toData(str); }
				StringData * toData(){ return StringData::toData(str); }

				OS * getAllocator(){ return toData()->allocator; }
				OS * getAllocator() const { return toData()->allocator; }

				const OS_CHAR * toChar() const { return str; } // toData()->toChar(); }
				operator const OS_CHAR*() const { return str; } // toChar(); }
				// operator StringData*() { return data; }

				// bool isNull() const { return data == NULL; }
				int getAllocatedBytes() const { return toData()->allocated_bytes; }
				int getDataSize() const { return toData()->data_size; }
				int getLen() const { return toData()->getLen(); }

				void clear();

				OS_INT toInt() const;
				OS_FLOAT toFloat() const;

				String& operator=(const String&);
				String& operator=(const OS_CHAR*);

				String& operator+=(const String&);
				String& operator+=(const OS_CHAR*);
				String& append(const void*, int size);
				String& append(const OS_CHAR*);

				String operator+(const String&) const;
				String operator+(const OS_CHAR*) const;

				bool operator==(const String&) const;
				bool operator==(const OS_CHAR*) const;

				bool operator!=(const String&) const;
				bool operator!=(const OS_CHAR*) const;

				bool operator<=(const String&) const;
				bool operator<=(const OS_CHAR*) const;

				bool operator<(const String&) const;
				bool operator<(const OS_CHAR*) const;

				bool operator>=(const String&) const;
				bool operator>=(const OS_CHAR*) const;

				bool operator>(const String&) const;
				bool operator>(const OS_CHAR*) const;

				String trim(bool trim_left = true, bool trim_right = true) const;

				int cmp(const String&) const;
				int cmp(const OS_CHAR*) const;
				int hash() const;

				String clone() const;
			};

			class Tokenizer
			{
			public:

				enum Error
				{
					ERROR_NOTHING,
					ERROR_MULTI_LINE_COMMENT, // multi line comment not end
					// ERROR_CONST_VECTOR_3,       // cant parse const vec3
					// ERROR_CONST_VECTOR_4,     // cant parse const vec4
					ERROR_CONST_STRING,             // string not end
					ERROR_CONST_STRING_ESCAPE_CHAR, // string escape error
					// ERROR_NAME,               // error name, _ - this is error
					ERROR_SYNTAX
				};

				enum TokenType
				{
					NOTHING,

					BEGIN_CODE_BLOCK,  // {
					END_CODE_BLOCK,    // }

					BEGIN_BRACKET_BLOCK,  // (
					END_BRACKET_BLOCK,    // )

					BEGIN_ARRAY_BLOCK,  // [
					END_ARRAY_BLOCK,    // ]

					CODE_SEPARATOR,     // ;
					PARAM_SEPARATOR,    // ,

					COMMENT_LINE,
					COMMENT_MULTI_LINE,

					NAME,           // [a..z_$][a..z0..9_$]*
					DOT_NAME,       // used in compiler [NAME].[NAME]... - for types
					IDENTIFER,      // used in compiler [NAME]
					DOT_IDENTIFER,  // used in compiler [NAME].[NAME]... - for types

					STRING,         // ["].*?["]

					// NUM_INT,        // -?[0..9]+
					NUMBER,      // -?[0..9][.]?[0..9]+(e[+-]?[0..9]+)?
					// NUM_VECTOR_3,   // 'NUM_FLOAT NUM_FLOAT NUM_FLOAT'
					// NUM_VECTOR_4,   // 'NUM_FLOAT NUM_FLOAT NUM_FLOAT NUM_FLOAT'
					// NULL,           // used in compiler

					// [not real operators]
					OPERATOR,
					BINARY_OPERATOR,
					SEPARATOR,
					// [/not real operators]

					OPERATOR_INDIRECT,    // .
					OPERATOR_CONCAT,    // ..
					REST_ARGUMENTS,  // ...

					OPERATOR_LOGIC_AND, // &&
					OPERATOR_LOGIC_OR,  // ||

					OPERATOR_LOGIC_PTR_EQ,  // ===
					OPERATOR_LOGIC_PTR_NE,  // !==
					OPERATOR_LOGIC_EQ,  // ==
					OPERATOR_LOGIC_NE,  // !=
					OPERATOR_LOGIC_GE,  // >=
					OPERATOR_LOGIC_LE,  // <=
					OPERATOR_LOGIC_GREATER, // >
					OPERATOR_LOGIC_LESS,    // <
					OPERATOR_LOGIC_NOT,     // !

					OPERATOR_INC,     // ++
					OPERATOR_DEC,     // --

					OPERATOR_QUESTION,  // ?
					OPERATOR_COLON,     // :

					OPERATOR_LENGTH,	// #

					OPERATOR_BIT_AND, // &
					OPERATOR_BIT_OR,  // |
					OPERATOR_BIT_XOR, // ^
					OPERATOR_BIT_NOT, // ~
					OPERATOR_ADD, // +
					OPERATOR_SUB, // -
					OPERATOR_MUL, // *
					OPERATOR_DIV, // /
					OPERATOR_MOD, // %
					OPERATOR_LSHIFT, // <<
					OPERATOR_RSHIFT, // >>
					OPERATOR_POW, // **

					OPERATOR_BIT_AND_ASSIGN, // &=
					OPERATOR_BIT_OR_ASSIGN,  // |=
					OPERATOR_BIT_XOR_ASSIGN, // ^=
					OPERATOR_BIT_NOT_ASSIGN, // ~=
					OPERATOR_ADD_ASSIGN, // +=
					OPERATOR_SUB_ASSIGN, // -=
					OPERATOR_MUL_ASSIGN, // *=
					OPERATOR_DIV_ASSIGN, // /=
					OPERATOR_MOD_ASSIGN, // %=
					OPERATOR_LSHIFT_ASSIGN, // <<=
					OPERATOR_RSHIFT_ASSIGN, // >>=
					OPERATOR_POW_ASSIGN, // **=

					OPERATOR_ASSIGN, // =

					OPERATOR_RESERVED,

					OPERATOR_END,

					ERROR_TOKEN
				};

				class TextData
				{
				protected:

					~TextData();

					OS * getAllocator();

				public:

					String filename;
					Vector<String> lines;

					int ref_count;

					TextData(OS*);

					TextData * retain();
					void release();
				};

				class TokenData
				{
				protected:

					union
					{
						// OS_INT int_value;
						OS_FLOAT float_value;
						// OS_FLOAT * vec3;
						// OS_FLOAT * vec4;
					};
					TokenType type;

					~TokenData();

				public:

					TextData * text_data;

					String str;
					int line, pos;
					int ref_count;

					OS * getAllocator() const;
					TokenType getType() const { return type; }

					OS_FLOAT getFloat() const;

					TokenData(TextData * text_data, const String& p_str, TokenType p_type, int p_line, int p_pos);

					TokenData * retain();
					void release();

					void setFloat(OS_FLOAT value);

					operator const String& () const { return str; }

					bool isTypeOf(TokenType tokenType) const;
				};

			protected:

				struct Settings
				{
					bool save_comments;
				} settings;

				OS * allocator;
				TextData * text_data;

				int cur_line, cur_pos;
				// bool loaded;
				// bool compiled;

				Vector<TokenData*> tokens;
				Error error;

				struct OperatorDesc
				{
					TokenType type;
					const OS_CHAR * name;
				};

				static const int operator_count;
				static OperatorDesc operator_desc[];
				static bool operator_initialized;

				void printLines();
				void printTokens();

				// private:

				static int __cdecl CompareOperatorDesc(const void * a, const void * b);
				static void initOperatorsTable();

				TokenData * addToken(const String& token, TokenType type, int line, int pos);

				TokenType parseNum(const OS_CHAR *& str, OS_FLOAT& fval, bool parse_end_spaces);
				bool parseLines();

			public:

				Tokenizer(OS*);
				~Tokenizer();

				// void reset();

				OS * getAllocator();
				TextData * getTextData() const { return text_data; }

				// bool isLoaded() const { return loaded; }

				// bool isCompiled() const { return compiled; }
				// void setCompiled(bool value){ compiled = value; }

				bool isError() const { return error != ERROR_NOTHING; }
				Error getErrorCode() const { return error; }
				int getErrorLine() const { return cur_line; }
				int getErrorPos() const { return cur_pos; }

				static const OS_CHAR * getTokenTypeName(TokenType tokenType);

				String getFilename() const { return text_data->filename; }
				String getLineString(int i) const { return text_data->lines[i]; }
				int getNumLines() const { return text_data->lines.count; }

				bool getSettingSaveComment() const { return settings.save_comments; }
				void setSettingSaveComment(bool value){ settings.save_comments = value; }

				bool parseText(const OS_CHAR * text, int len, const String& filename);

				int getNumTokens() const { return tokens.count; }
				TokenData * getToken(int i) const { return tokens[i]; }
				void insertToken(int i, TokenData * token);
			};

			typedef Tokenizer::TokenType TokenType;
			typedef Tokenizer::TokenData TokenData;
			typedef Tokenizer::TextData TextData;

			struct Property;
			struct PropertyIndex;
			struct Table
			{
				struct IteratorState
				{
					Table * table;
					Property * prop;
					IteratorState * next;

					IteratorState();
					~IteratorState();
				};

				Property ** heads;
				int head_mask;
				int count;

				OS_INT next_id;

				Property * first, * last;
				IteratorState * iterators;

				Table();    
				~Table();

				Property * get(const PropertyIndex& index);

				bool containsIterator(IteratorState*);
				void addIterator(IteratorState*);
				void removeIterator(IteratorState*);
			};

			struct GC_Value
			{
				int value_id;
				// int ref_count;
				GC_Value * prototype;
				GC_Value * hash_next;

				// Value * gc_grey_prev;
				GC_Value * gc_grey_next;

				Table * table;

				OS_EValueType type;

				enum {
					GC_WHITE,
					GC_GREY,
					GC_BLACK
				} gc_color;

				GC_Value();
				virtual ~GC_Value();
			};

			struct GC_ObjectValue: public GC_Value
			{
			};

			struct GC_StringValue: public GC_Value
			{
				StringData * string_data;
			};

			struct GC_UserDataValue: public GC_Value
			{
				int crc;
				void * ptr;
				OS_UserDataDtor dtor;
				void * user_param;
			};

			struct GC_CFunctionValue: public GC_Value
			{
				OS_CFunction func;
				void * user_param;
				int num_upvalues;
			};

			struct GC_FunctionValue;

			struct WeakRef { WeakRef(){} };
			struct ValueData
			{
				union {
					bool boolean;
					OS_FLOAT number;
					// StringData * string;
					int value_id;
					GC_Value * value;
					GC_ObjectValue * object;
					GC_StringValue * string;
					GC_UserDataValue * userdata;
					GC_FunctionValue * func;
					GC_CFunctionValue * cfunc;
				} v;

				OS_EValueType type;

				ValueData();
				ValueData(bool);
				ValueData(OS_FLOAT);
				ValueData(int);
				ValueData(int, const WeakRef&);
				// ValueData(const ValueData&);
				ValueData(GC_Value*);

				// ValueData& operator=(const ValueData&);
				ValueData& operator=(GC_Value*);
				
				void clear();

				GC_Value * getGCValue() const;

				bool isFunction() const;
				bool isUserData() const;
			};

			struct Program;
			struct FunctionDecl;
			struct FunctionRunningInstance;
			struct GC_FunctionValue: public GC_Value
			{
				Program * prog; // retained
				FunctionDecl * func_decl;
				ValueData env;

				FunctionRunningInstance * parent_inctance; // retained

				GC_FunctionValue();
				~GC_FunctionValue();
			};

			struct PropertyIndex
			{
				struct KeepStringIndex
				{
					KeepStringIndex(){}
				};

				String string_index;
				OS_INT int_index;
				int hash_value;
				bool is_string_index;
				bool int_valid;

				PropertyIndex(const PropertyIndex& index);
				PropertyIndex(const String& index);
				PropertyIndex(const String& index, const KeepStringIndex&);
				PropertyIndex(StringData * index);
				PropertyIndex(StringData * index, const KeepStringIndex&);
				PropertyIndex(OS*, const OS_CHAR * index);
				PropertyIndex(OS*, OS_INT32 index);
				PropertyIndex(OS*, OS_INT64 index);
				PropertyIndex(OS*, OS_FLOAT index, int precision = OS_DEF_PRECISION);
				~PropertyIndex();

				OS * getAllocator() const { return string_index.getAllocator(); }

				int cmp(const PropertyIndex& b) const;
				int hash() const;

				String toString() const;

				bool checkIntIndex() const;
				void fixStringIndex();
			};

			struct Property: public PropertyIndex
			{
				ValueData value;

				Property * hash_next;
				Property * prev, * next;

				Property(const PropertyIndex& index);
				Property(const String& index);
				Property(OS*, const OS_CHAR * index);
				Property(OS*, OS_INT index);
				Property(OS*, int index);
				Property(OS*, OS_FLOAT index, int precision = OS_DEF_PRECISION);
				~Property();
			};


			class Compiler
			{
			public:

				enum ExpressionType
				{
					EXP_TYPE_UNKNOWN,
					EXP_TYPE_NOP,
					EXP_TYPE_NEW_LOCAL_VAR,
					EXP_TYPE_SCOPE,
					EXP_TYPE_LOOP_SCOPE,
					EXP_TYPE_CODE_LIST,
					EXP_TYPE_NAME, // temp
					// EXP_TYPE_POST_IF,
					EXP_TYPE_POP_VALUE,
					EXP_TYPE_CALL,
					EXP_TYPE_CALL_AUTO_PARAM,
					EXP_TYPE_CALL_DIM, // temp
					EXP_TYPE_VALUE,
					EXP_TYPE_PARAMS,
					EXP_TYPE_FUNCTION,
					EXP_TYPE_EXTENDS,
					EXP_TYPE_CLONE,
					EXP_TYPE_DELETE,
					EXP_TYPE_RETURN,
					EXP_TYPE_BREAK,
					EXP_TYPE_CONTINUE,
					EXP_TYPE_DEBUGGER,
					EXP_TYPE_DEBUGGER_LOCALS,
					
					EXP_TYPE_IF,

					EXP_TYPE_ARRAY,

					EXP_TYPE_OBJECT,
					EXP_TYPE_OBJECT_SET_BY_NAME,
					EXP_TYPE_OBJECT_SET_BY_INDEX,
					EXP_TYPE_OBJECT_SET_BY_EXP,
					EXP_TYPE_OBJECT_SET_BY_AUTO_INDEX,

					EXP_TYPE_TYPE_OF,
					EXP_TYPE_VALUE_OF,
					// EXP_TYPE_BOOLEAN_OF, == EXP_TYPE_LOGIC_BOOL
					EXP_TYPE_NUMBER_OF,
					EXP_TYPE_STRING_OF,
					EXP_TYPE_ARRAY_OF,
					EXP_TYPE_OBJECT_OF,
					EXP_TYPE_USERDATA_OF,
					EXP_TYPE_FUNCTION_OF,

					EXP_TYPE_GET_THIS,
					EXP_TYPE_GET_ARGUMENTS,
					EXP_TYPE_GET_REST_ARGUMENTS,

					EXP_TYPE_GET_LOCAL_VAR,
					EXP_TYPE_GET_LOCAL_VAR_AUTO_CREATE,
					EXP_TYPE_SET_LOCAL_VAR,

					EXP_TYPE_GET_ENV_VAR,
					EXP_TYPE_GET_ENV_VAR_AUTO_CREATE,
					EXP_TYPE_SET_ENV_VAR,

					EXP_TYPE_INDIRECT, // temp

					EXP_TYPE_GET_PROPERTY,
					EXP_TYPE_GET_PROPERTY_AUTO_CREATE,
					EXP_TYPE_SET_PROPERTY,

					// EXP_TYPE_GET_PROPERTY_DIM,
					// EXP_TYPE_SET_PROPERTY_DIM,

					// EXP_TYPE_GET_LOCAL_VAR_DIM,
					// EXP_TYPE_GET_ENV_VAR_DIM,

					// EXP_TYPE_GET_DIM,
					EXP_TYPE_SET_DIM,

					EXP_TYPE_CALL_METHOD,

					EXP_TYPE_TAIL_CALL,
					EXP_TYPE_TAIL_CALL_METHOD,

					// EXP_TYPE_SET_LOCAL_VAR_DIM,
					// EXP_TYPE_SET_ENV_VAR_DIM,

					// EXP_CONST_STRING,
					// EXP_CONST_FLOAT,
					// EXP_VAR_ASSING,

					EXP_TYPE_CONST_NULL,
					// EXP_TYPE_CONST_VALUE,
					EXP_TYPE_CONST_NUMBER,
					EXP_TYPE_CONST_STRING,
					EXP_TYPE_CONST_TRUE,
					EXP_TYPE_CONST_FALSE,

					EXP_TYPE_LOGIC_BOOL,    // !!
					EXP_TYPE_LOGIC_NOT,     // !
					EXP_TYPE_BIT_NOT, // ~
					EXP_TYPE_PLUS,    // +
					EXP_TYPE_NEG,     // -
					EXP_TYPE_LENGTH,  // #

					// EXP_TYPE_PARAM_SEPARTOR, // ,

					EXP_TYPE_CONCAT, // ..

					EXP_TYPE_LOGIC_AND, // &&
					EXP_TYPE_LOGIC_OR,  // ||

					EXP_TYPE_LOGIC_PTR_EQ,  // ===
					EXP_TYPE_LOGIC_PTR_NE,  // !==
					EXP_TYPE_LOGIC_EQ,  // ==
					EXP_TYPE_LOGIC_NE,  // !=
					EXP_TYPE_LOGIC_GE,  // >=
					EXP_TYPE_LOGIC_LE,  // <
					EXP_TYPE_LOGIC_GREATER, // >
					EXP_TYPE_LOGIC_LESS,    // <

					// EXP_TYPE_INC,     // ++
					// EXP_TYPE_DEC,     // --

					EXP_TYPE_PRE_INC,     // ++
					EXP_TYPE_PRE_DEC,     // --

					EXP_TYPE_POST_INC,    // ++
					EXP_TYPE_POST_DEC,    // --

					// EXP_TYPE_QUESTION,

					EXP_TYPE_BIT_AND, // &
					EXP_TYPE_BIT_OR,  // |
					EXP_TYPE_BIT_XOR, // ^

					EXP_TYPE_BIT_AND_ASSIGN, // &=
					EXP_TYPE_BIT_OR_ASSIGN,  // |=
					EXP_TYPE_BIT_XOR_ASSIGN, // ^=
					EXP_TYPE_BIT_NOT_ASSIGN, // ~=

					EXP_TYPE_ADD, // +
					EXP_TYPE_SUB, // -
					EXP_TYPE_MUL, // *
					EXP_TYPE_DIV, // /
					EXP_TYPE_MOD, // %
					EXP_TYPE_LSHIFT, // <<
					EXP_TYPE_RSHIFT, // >>
					EXP_TYPE_POW, // **

					EXP_TYPE_ADD_ASSIGN, // +=
					EXP_TYPE_SUB_ASSIGN, // -=
					EXP_TYPE_MUL_ASSIGN, // *=
					EXP_TYPE_DIV_ASSIGN, // /=
					EXP_TYPE_MOD_ASSIGN, // %=
					EXP_TYPE_LSHIFT_ASSIGN, // <<=
					EXP_TYPE_RSHIFT_ASSIGN, // >>=
					EXP_TYPE_POW_ASSIGN, // **=

					EXP_TYPE_ASSIGN,
				};

			protected:

				friend class Program;

				struct Expression;
				struct ExpressionList: public Vector<Expression*>
				{
					OS * allocator;

					ExpressionList(OS*);
					~ExpressionList();

					bool isValue() const;
					bool isClear() const;
					bool isWriteable() const;

					Expression * add(Expression*);
					Expression * removeIndex(int i);
					Expression * removeLast();

					void swap(ExpressionList&);
				};

				enum ELocalVarType
				{
					LOCAL_GENERIC,
					LOCAL_PARAM,
					LOCAL_TEMP
				};

				struct LocalVarDesc
				{
					OS_U16 up_count;
					OS_U16 up_scope_count;
					OS_U16 index;
					ELocalVarType type;
					// bool is_param;

					LocalVarDesc();
				};

				struct Expression
				{
					TokenData * token;
					ExpressionList list;
					LocalVarDesc local_var;
					OS_U16 active_locals;
					OS_U16 ret_values;
					ExpressionType type;
					
					Expression(ExpressionType type, TokenData*);
					Expression(ExpressionType type, TokenData*, Expression * e1);
					Expression(ExpressionType type, TokenData*, Expression * e1, Expression * e2);
					Expression(ExpressionType type, TokenData*, Expression * e1, Expression * e2, Expression * e3);
					virtual ~Expression();

					OS * getAllocator(){ return list.allocator; }

					OS_FLOAT toNumber();
					OS_INT toInt();
					String toString();

					bool isConstValue() const;
					bool isValue() const;
					bool isClear() const;
					bool isWriteable() const;
					bool isOperator() const;
					bool isUnaryOperator() const;
					bool isBinaryOperator() const;
					bool isAssignOperator() const;
					bool isLogicOperator() const;

					String debugPrint(Compiler * compiler, int depth);
				};

				struct Scope: public Expression
				{
					Scope * parent;
					Scope * function;

					struct LocalVar
					{
						String name;
						int index;

						LocalVar(const String& name, int index);
					};

					struct LocalVarCompiled
					{
						int cached_name_index;
						int start_code_pos;
						int end_code_pos;

						LocalVarCompiled();
					};

					enum ELoopBreakType
					{
						LOOP_CONTINUE,
						LOOP_BREAK
					};

					struct LoopBreak
					{
						int pos;
						ELoopBreakType type;
					};

					// used by function scope
					int func_index;
					Vector<LocalVar> locals;
					Vector<LocalVarCompiled> locals_compiled;
					int num_params;
					int num_locals;
					int opcodes_pos;
					int opcodes_size;
					int max_up_count;

					Vector<LoopBreak> loop_breaks;

					bool parser_started;

					Scope(Scope * parent, ExpressionType, TokenData*);
					virtual ~Scope();

					bool addLoopBreak(int pos, ELoopBreakType);
					void fixLoopBreaks(int scope_start_pos, int scope_end_pos, StreamWriter*);

					void addLocalVar(const String& name);
					void addLocalVar(const String& name, LocalVarDesc&);
				};

				enum ErrorType {
					ERROR_NOTHING,
					ERROR_SYNTAX,
					ERROR_NESTED_ROOT_BLOCK,
					ERROR_LOCAL_VAL_NOT_DECLARED,
					ERROR_VAR_ALREADY_EXIST,
					ERROR_EXPECT_TOKEN_TYPE,
					ERROR_EXPECT_TOKEN_STR,
					ERROR_EXPECT_TOKEN,
					ERROR_EXPECT_VALUE,
					ERROR_EXPECT_WRITEABLE,
					ERROR_EXPECT_EXPRESSION,
					ERROR_EXPECT_FUNCTION_SCOPE,
					ERROR_EXPECT_SWITCH_SCOPE,
					ERROR_FINISH_BINARY_OP,
					ERROR_FINISH_UNARY_OP,
				};

				enum OpcodeLevel {
					OP_LEVEL_NOTHING = -1,
					OP_LEVEL_0,
					OP_LEVEL_1, // = += -= *= /= %=
					OP_LEVEL_1_1, // ,
					OP_LEVEL_2, // ?:
					OP_LEVEL_3, // ||
					OP_LEVEL_4, // &&
					OP_LEVEL_5, // ..
					OP_LEVEL_6, // == !=
					OP_LEVEL_7, // < <= > >=
					OP_LEVEL_8, // |
					OP_LEVEL_9, // & ^
					OP_LEVEL_10, // << >> >>>
					OP_LEVEL_11, // + -
					OP_LEVEL_12, // * / %
					OP_LEVEL_13, // ** as is
					OP_LEVEL_14, // ++ --
					OP_LEVEL_15, // unary ! ~ + #
					OP_LEVEL_16, // .

					OP_LEVEL_COUNT
				};

				OS * allocator;
				Tokenizer * tokenizer;

				ErrorType error;
				TokenData * error_token;
				TokenType expect_token_type;
				String expect_token;

				TokenData * recent_token;
				int next_token_index;

				// String recent_printed_filename;
				TextData * recent_printed_text_data;
				int recent_printed_line;

				// code generation
				// Program * prog;
				Table * prog_numbers_table;
				Table * prog_strings_table;
				Table * prog_debug_strings_table;
				Vector<OS_FLOAT> prog_numbers;
				Vector<String> prog_strings;
				Vector<String> prog_debug_strings;
				Vector<Scope*> prog_functions;
				MemStreamWriter * prog_opcodes;
				MemStreamWriter * prog_debug_info;
				int prog_num_debug_infos;
				int prog_max_up_count;

				bool isError();
				void resetError();
				void setError();
				void setError(ErrorType value, TokenData * error_token);
				void setError(TokenType expect_token_type, TokenData * error_token);
				void setError(const String& str, TokenData * error_token);

				void * malloc(int size);

				TokenData * setNextTokenIndex(int i);
				TokenData * setNextToken(TokenData * token);
				TokenData * putNextTokenType(TokenType tokenType);
				TokenData * ungetToken();

				bool isNextTokens(TokenType * list, int count);
				bool isNextToken(TokenType t0);
				bool isNextTokens(TokenType t0, TokenType t1);
				bool isNextTokens(TokenType t0, TokenType t1, TokenType t2);
				bool isNextTokens(TokenType t0, TokenType t1, TokenType t2, TokenType t3);

				void deleteNops(ExpressionList& list);

				ExpressionType getUnaryExpressionType(TokenType);
				ExpressionType getExpressionType(TokenType);
				OpcodeLevel getOpcodeLevel(ExpressionType exp_type);

				TokenData * readToken();
				TokenData * expectToken(TokenType);
				TokenData * expectToken();

				struct Params
				{
					bool allow_root_blocks;
					bool allow_binary_operator;
					bool allow_assing;
					// bool allow_left_side_params;
					// bool allow_right_side_params;
					bool allow_params;
					// bool allow_var;
					bool allow_auto_call;

					Params();
					Params(const Params&);

					Params& setAllowRootBlocks(bool);
					Params& setAllowBinaryOperator(bool);
					Params& setAllowAssign(bool);
					Params& setAllowParams(bool);
					// Params& setAllowLeftSideParams(bool);
					// Params& setAllowRightSideParams(bool);
					Params& setAllowAutoCall(bool);
				};

				Expression * expectSingleExpression(Scope*, const Params& p);
				Expression * expectSingleExpression(Scope*);
					// bool allow_binary_operator, bool allow_param, bool allow_var, bool allow_assign, bool allow_auto_call);

				Expression * expectExpressionValues(Expression * exp, int ret_values);
				Expression * newExpressionFromList(ExpressionList& list, int ret_values);
				Expression * newAssingExpression(Scope * scope, Expression * var_exp, Expression * value_exp);
				// Expression * newIndirectExpression(Scope * scope, Expression * var_exp, Expression * value_exp);
				Expression * newSingleValueExpression(Expression * exp);
				Expression * processExpressionSecondPass(Scope * scope, Expression * exp);

				Scope * expectTextExpression();
				Scope * expectCodeExpression(Scope*, int ret_values = 0);
				Scope * expectFunctionExpression(Scope*);
				Expression * expectExtendsExpression(Scope*);
				Expression * expectCloneExpression(Scope*);
				Expression * expectDeleteExpression(Scope*);
				Expression * expectValueOfExpression(Scope*, ExpressionType exp_type);
				Expression * expectVarExpression(Scope*);
				Expression * expectObjectExpression(Scope*);
				Expression * expectArrayExpression(Scope*);
				// Expression * finishParamsExpression(Scope*, Expression * params);
				Expression * expectParamsExpression(Scope*);
				// Expression * expectParamsExpression(Scope*, Expression * first_param);
				Expression * expectReturnExpression(Scope*);
				Expression * expectIfExpression(Scope*);
				Expression * expectForExpression(Scope*);
				Expression * expectDebuggerLocalsExpression(Scope*);
				Expression * finishValueExpression(Scope*, Expression*, const Params& p); // bool allow_binary_operator, bool allow_param, bool allow_assign, bool allow_auto_call);
				Expression * finishBinaryOperator(Scope * scope, OpcodeLevel prev_level, Expression * exp, const Params& p, bool& is_finished); // bool allow_param, bool& is_finished);
				Expression * newBinaryExpression(Scope * scope, ExpressionType, TokenData*, Expression * left_exp, Expression * right_exp);

				bool findLocalVar(LocalVarDesc&, Scope * scope, const String& name, int active_locals, bool all_scopes);

				String debugPrintSourceLine(TokenData*);
				static const OS_CHAR * getExpName(ExpressionType);

				int cacheString(Table * strings_table, Vector<String>& strings, const String& str);
				int cacheString(const String& str);
				int cacheDebugString(const String& str);
				int cacheNumber(OS_FLOAT);

				bool writeOpcodes(Scope*, Expression*);
				bool writeOpcodes(Scope*, ExpressionList&);
				void writeDebugInfo(Expression*);
				bool saveToStream(StreamWriter * writer, StreamWriter * debug_info_writer);

			public:

				static const int EXPRESSION_SIZE = sizeof(Expression);

				Compiler(Tokenizer*);
				virtual ~Compiler();

				bool compile(); // compile text and push text root function
			};

			struct FunctionDecl
			{
				struct LocalVar
				{
					String name;
					int start_code_pos;
					int end_code_pos;
					// Value * value;

					LocalVar(const String&);
					~LocalVar();
				};

				int parent_func_index;
				LocalVar * locals;
				int num_locals;
				int num_params;
				int max_up_count;
				int opcodes_pos;
				int opcodes_size;

				// Program * prog; // retained for value of function type

				FunctionDecl(); // Program*);
				~FunctionDecl();
			};

			class Program
			{
			protected:

				int ref_count;

				virtual ~Program();

			public:

				enum OpcodeType
				{
					OP_UNKNOWN,
					OP_PUSH_NUMBER,
					OP_PUSH_STRING,
					OP_PUSH_NULL,
					OP_PUSH_TRUE,
					OP_PUSH_FALSE,

					OP_PUSH_FUNCTION,

					OP_PUSH_NEW_OBJECT,
					OP_OBJECT_SET_BY_AUTO_INDEX,
					OP_OBJECT_SET_BY_EXP,
					OP_OBJECT_SET_BY_INDEX,
					OP_OBJECT_SET_BY_NAME,

					OP_PUSH_ENV_VAR,
					OP_PUSH_ENV_VAR_AUTO_CREATE,
					OP_SET_ENV_VAR,

					OP_PUSH_THIS,
					OP_PUSH_ARGUMENTS,
					OP_PUSH_REST_ARGUMENTS,

					OP_PUSH_LOCAL_VAR,
					OP_PUSH_LOCAL_VAR_AUTO_CREATE,
					OP_SET_LOCAL_VAR,

					OP_PUSH_UP_LOCAL_VAR,
					OP_PUSH_UP_LOCAL_VAR_AUTO_CREATE,
					OP_SET_UP_LOCAL_VAR,

					/*
					OP_LOCAL_VAR_INC,
					OP_PUSH_LOCAL_VAR_PRE_INC,
					OP_PUSH_LOCAL_VAR_POST_INC,

					OP_LOCAL_VAR_DEC,
					OP_PUSH_LOCAL_VAR_PRE_DEC,
					OP_PUSH_LOCAL_VAR_POST_DEC,
					*/

					OP_CALL,
					OP_TAIL_CALL,
					OP_CALL_METHOD,
					OP_TAIL_CALL_METHOD,

					OP_GET_PROPERTY,
					OP_GET_PROPERTY_AUTO_CREATE,
					OP_SET_PROPERTY,

					OP_SET_DIM,

					OP_IF_NOT_JUMP,
					OP_JUMP,
					OP_DEBUGGER,

					OP_EXTENDS,
					OP_DELETE_PROP,
					OP_RETURN,
					OP_POP,

					OP_LOGIC_AND,
					OP_LOGIC_OR,

					OP_LOGIC_PTR_EQ,
					OP_LOGIC_PTR_NE,
					OP_LOGIC_EQ,
					OP_LOGIC_NE,
					OP_LOGIC_GE,
					OP_LOGIC_LE,
					OP_LOGIC_GREATER,
					OP_LOGIC_LESS,

					OP_BIT_AND,
					OP_BIT_OR,
					OP_BIT_XOR,

					OP_ADD, // +
					OP_SUB, // -
					OP_MUL, // *
					OP_DIV, // /
					OP_MOD, // %
					OP_LSHIFT, // <<
					OP_RSHIFT, // >>
					OP_POW, // **

					OP_CONCAT, // ..

					OP_BIT_NOT,
					OP_PLUS,
					OP_NEG,
					OP_LENGTH,

					OP_LOGIC_BOOL,
					OP_LOGIC_NOT,

					OP_TYPE_OF,
					OP_VALUE_OF,
					OP_NUMBER_OF,
					OP_STRING_OF,
					OP_ARRAY_OF,
					OP_OBJECT_OF,
					OP_USERDATA_OF,
					OP_FUNCTION_OF,
					OP_CLONE,

					OP_NOP,

					OPCODE_COUNT
				};

				OS * allocator;
				String filename;

				GC_StringValue ** const_strings;
				int num_strings;
				
				OS_FLOAT * const_numbers;
				int num_numbers;

				FunctionDecl * functions;
				int num_functions;

				MemStreamReader * opcodes;

				struct DebugInfoItem
				{
					int opcode_offs;
					int line;
					int pos;
					String token;

					DebugInfoItem(int opcode_offs, int line, int pos, const String&);
				};
				Vector<DebugInfoItem> debug_info;

				int gc_time;

				Program(OS * allocator);

				Program * retain();
				void release();

				static OpcodeType getOpcodeType(Compiler::ExpressionType);

				bool loadFromStream(StreamReader * reader, StreamReader * debuginfo_reader);

				void pushFunction();
			};

			struct FunctionRunningInstance
			{
				GC_FunctionValue * func;
				GC_Value * self;

				FunctionRunningInstance ** parent_inctances;

				ValueData * locals;
				int num_params;
				int num_extra_params;
				int initial_stack_size;
				int need_ret_values;

				GC_Value * arguments;
				GC_Value * rest_arguments;

				int opcodes_pos;
				int ref_count;

				int gc_time;

				FunctionRunningInstance();
				~FunctionRunningInstance();

				FunctionRunningInstance * retain();
			};

			struct Values
			{
				GC_Value ** heads;
				int head_mask;
				int count;

				int next_id;

				Values();
				~Values();

				// Value * add(Value * obj);
				// Value * remove(int value_id);
				GC_Value * get(int value_id);
			};

			OS * allocator;
			StringData * empty_string_data;

			struct Strings
			{
				String __constructor;
				// String __destructor;
				String __get;
				String __set;
				String __del;
				String __getempty;
				String __setempty;
				String __delempty;
				String __getdim;
				String __setdim;
				String __deldim;
				String __cmp;
				String __iter;
				// String __tostring;
				String __valueof;
				String __booleanof;
				String __numberof;
				String __stringof;
				String __arrayof;
				String __objectof;
				String __userdataof;
				String __functionof;
				String __clone;
				// String __tobool;
				String __concat;
				String __bitand;
				String __bitor;
				String __bitxor;
				String __bitnot;
				String __plus;
				String __neg;
				String __len;
				String __add;
				String __sub;
				String __mul;
				String __div;
				String __mod;
				String __lshift;
				String __rshift;
				String __pow;

				String typeof_null;
				String typeof_boolean;
				String typeof_number;
				String typeof_string;
				String typeof_object;
				String typeof_array;
				String typeof_userdata;
				String typeof_function;

				String syntax_typeof;
				String syntax_valueof;
				String syntax_booleanof;
				String syntax_numberof;
				String syntax_stringof;
				String syntax_arrayof;
				String syntax_objectof;
				String syntax_userdataof;
				String syntax_functionof;
				String syntax_extends;
				String syntax_clone;
				String syntax_delete;
				String syntax_prototype;
				String syntax_var;
				String syntax_this;
				String syntax_arguments;
				String syntax_function;
				String syntax_null;
				String syntax_true;
				String syntax_false;
				String syntax_return;
				String syntax_class;
				String syntax_enum;
				String syntax_switch;
				String syntax_case;
				String syntax_default;
				String syntax_if;
				String syntax_else;
				String syntax_elseif;
				String syntax_for;
				String syntax_in;
				String syntax_break;
				String syntax_continue;
				String syntax_debugger;
				String syntax_debugger_locals;

				int __dummy__;

				Strings(OS * allocator);
			} * strings;

			Values values;
			int num_created_values;
			int num_destroyed_values;

			Table * string_values_table;
			GC_ObjectValue * global_vars;
			GC_ObjectValue * user_pool;

			enum {
				PROTOTYPE_BOOL,
				PROTOTYPE_NUMBER,
				PROTOTYPE_STRING,
				PROTOTYPE_OBJECT,
				PROTOTYPE_ARRAY,
				PROTOTYPE_FUNCTION,
				// -----------------
				PROTOTYPE_COUNT
			};

			GC_ObjectValue * prototypes[PROTOTYPE_COUNT];

			// Vector<Value*> autorelease_values;
			Vector<ValueData> stack_values;
			Vector<FunctionRunningInstance*> call_stack_funcs;

			GC_Value * gc_grey_list_first;
			bool gc_grey_root_initialized;
			int gc_values_head_index;
			int gc_time;
			int gc_grey_added_count;
			
			float gc_start_values_mult;
			float gc_step_size_mult;
			int gc_start_next_values;
			int gc_step_size;

			struct {
				bool create_debug_opcodes;
				bool create_debug_info;
				bool recompile_sourcecode;
			} settings;

			void * malloc(int size);
			void free(void * p);

			void error(int code, const OS_CHAR * message);
			void error(int code, const String& message);

			void gcInitGreyList();
			void gcResetGreyList();
			void gcAddToGreyList(GC_Value*);
			void gcAddToGreyList(ValueData&);
			void gcRemoveFromGreyList(GC_Value*);
			void gcProcessGreyProgram(Program * prog);
			void gcProcessGreyTable(Table * table);
			void gcProcessGreyFunctionValue(GC_FunctionValue * func_value);
			void gcProcessGreyFunctionRunning(FunctionRunningInstance*);
			void gcProcessStringsCacheTable();
			void gcProcessGreyValue(GC_Value*);
			void gcProcessGreyList(int step_size);
			
			// return next gc phase
			int gcStep();
			void gcStepIfNeeded();
			void gcFinishSweepPhase();
			void gcFinishMarkPhase();
			void gcFull();

			void clearValue(ValueData&);
			void clearValue(GC_Value*);
			void deleteValue(GC_Value*);
			bool isValueUsed(GC_Value*);

			GC_FunctionValue * pushFunctionValue(FunctionRunningInstance * func_running, Program*, FunctionDecl*);
			void clearFunctionValue(GC_FunctionValue*);

			void releaseFunctionRunningInstance(FunctionRunningInstance*);

			// GC_Value * newValue();
			// ValueData newBoolValue(bool);
			// ValueData newNumberValue(OS_FLOAT);
			GC_StringValue * newStringValue(const String&);
			GC_StringValue * newStringValue(const OS_CHAR*);
			GC_CFunctionValue * newCFunctionValue(OS_CFunction func, void * user_param);
			GC_CFunctionValue * newCFunctionValue(OS_CFunction func, int upvalues, void * user_param);
			GC_UserDataValue * newUserDataValue(int crc, int data_size, OS_UserDataDtor dtor, void * user_param);
			GC_UserDataValue * newUserPointerValue(int crc, void * data, OS_UserDataDtor dtor, void * user_param);
			GC_ObjectValue * newObjectValue();
			GC_ObjectValue * newObjectValue(GC_Value * prototype);
			GC_ObjectValue * newArrayValue();

			template<class T> T * pushValue(T * val){ pushValueData(ValueData(val)); return val; }

			void pushValueData(const ValueData& val);
			void pushStackValue(int offs);
			void insertValue(ValueData& val, int offs);
			void pushNull();
			void pushTrue();
			void pushFalse();
			void pushBool(bool);
			void pushNumber(OS_INT);
			void pushNumber(OS_FLOAT);
			
			GC_StringValue * pushStringValue(const String&);
			GC_StringValue * pushStringValue(const OS_CHAR*);
			GC_CFunctionValue * pushCFunctionValue(OS_CFunction func, void * user_param);
			GC_CFunctionValue * pushCFunctionValue(OS_CFunction func, int upvalues, void * user_param);
			GC_UserDataValue * pushUserDataValue(int crc, int data_size, OS_UserDataDtor dtor, void * user_param);
			GC_UserDataValue * pushUserPointerValue(int crc, void * data, OS_UserDataDtor dtor, void * user_param);
			GC_ObjectValue * pushObjectValue();
			GC_ObjectValue * pushObjectValue(GC_Value * prototype);
			GC_ObjectValue * pushArrayValue();

			void pushTypeOf(const ValueData& val);
			bool pushNumberOf(const ValueData& val);
			bool pushStringOf(const ValueData& val);
			bool pushValueOf(const ValueData& val);
			GC_ObjectValue * pushArrayOf(ValueData& val);
			GC_ObjectValue * pushObjectOf(ValueData& val);
			GC_UserDataValue * pushUserDataOf(ValueData& val);
			bool pushFunctionOf(ValueData& val);

			void pushCloneValue(ValueData& val);

			// unary operator
			void pushOpResultValue(int opcode, ValueData& value);

			// binary operator
			void pushOpResultValue(int opcode, ValueData& left_value, ValueData& right_value);

			void setGlobalValue(const String& name, ValueData& value, bool prototype_enabled, bool setter_enabled);
			void setGlobalValue(const OS_CHAR * name, ValueData& value, bool prototype_enabled, bool setter_enabled);

			int getStackOffs(int offs);
			ValueData getStackValue(int offs);

			void removeStackValues(int offs, int count);
			void removeStackValue(int offs = -1);
			void removeAllStackValues();
			void pop(int count = 1);
			void moveStackValues(int offs, int count, int new_offs);
			void moveStackValue(int offs, int new_offs);

			void syncStackRetValues(int need_ret_values, int cur_ret_values);

			void registerValue(GC_Value * val);
			GC_Value * unregisterValue(int value_id);
			void deleteValues();

			bool valueToBool(const ValueData& val);
			OS_INT valueToInt(const ValueData& val, bool valueof_enabled = false);
			OS_FLOAT valueToNumber(const ValueData& val, bool valueof_enabled = false);
			String valueToString(const ValueData& val, bool valueof_enabled = false);

			bool isValueNumber(const ValueData& val, OS_FLOAT * out = NULL);
			bool isValueString(const ValueData& val, String * out = NULL);
			bool isValueInstanceOf(GC_Value * val, GC_Value * prototype_val);
			bool isValueInstanceOf(const ValueData& val, const ValueData& prototype_val);

			Table * newTable();
			void deleteTable(Table*);
			void addTableProperty(Table * table, Property * prop);
			bool deleteTableProperty(Table * table, const PropertyIndex& index);
			void deleteValueProperty(GC_Value * table_value, ValueData * index_value, const PropertyIndex& index, bool prototype_enabled, bool del_method_enabled);
			void deleteValueProperty(ValueData& table_value, ValueData * index_value, const PropertyIndex& index, bool prototype_enabled, bool del_method_enabled);
			void reorderTableNumericKeys(Table * table);
			void reorderTableKeys(Table * table);
			void initTableProperties(Table * dst, Table * src);

			Property * setTableValue(Table * table, PropertyIndex& index, ValueData& val);
			void setPropertyValue(GC_Value * table_value, ValueData * index_value, PropertyIndex& index, ValueData& val, bool prototype_enabled, bool setter_enabled);
			void setPropertyValue(ValueData& table_value, ValueData * index_value, PropertyIndex& index, ValueData& val, bool prototype_enabled, bool setter_enabled);
			void setPropertyValue(GC_Value * table_value, ValueData& index_value, ValueData& val, bool prototype_enabled, bool setter_enabled);
			void setPropertyValue(ValueData& table_value, ValueData& index_value, ValueData& val, bool prototype_enabled, bool setter_enabled);

			bool getPropertyValue(ValueData& result, Table * table, const PropertyIndex& index);
			bool getPropertyValue(ValueData& result, GC_Value * table_value, PropertyIndex& index, bool prototype_enabled);
			bool getPropertyValue(ValueData& result, ValueData& table_value, PropertyIndex& index, bool prototype_enabled);

			void pushPropertyValue(GC_Value * table_value, ValueData * index_value, PropertyIndex& index, bool prototype_enabled, bool getter_enabled, bool auto_create);
			void pushPropertyValue(ValueData& table_value, ValueData * index_value, PropertyIndex& index, bool prototype_enabled, bool getter_enabled, bool auto_create);
			void pushPropertyValue(GC_Value * table_value, ValueData& index_value, bool prototype_enabled, bool getter_enabled, bool auto_create);
			void pushPropertyValue(ValueData& table_value, ValueData& index_value, bool prototype_enabled, bool getter_enabled, bool auto_create);

			void setPrototype(const ValueData& val, const ValueData& proto);
			void pushPrototype(const ValueData& val);

			void enterFunction(GC_FunctionValue * func_value, GC_Value * self, int params, int extra_remove_from_stack, int need_ret_values);
			int leaveFunction();
			int execute();

			int call(int params, int ret_values);

			Core(OS*);
			~Core();

			bool init();
			void shutdown();
		};

		MemoryManager * memory_manager;
		Core * core;
		int ref_count;

		OS(MemoryManager*);
		virtual ~OS();

		bool init();
		void shutdown();

		void * malloc(int size);
		void free(void * p);

		Core::String changeFilenameExt(const OS_CHAR * filename, const OS_CHAR * ext);
		Core::String getFilenameExt(const OS_CHAR * filename);

		int getPointerSize(void * p);

		void initGlobalFunctions();
		void initObjectClass();
		void initArrayClass();
		void initFunctionClass();
		void initStringClass();
		void initMathLibrary();
		void initScript();

	public:

		class String: public Core::String // retains os, external strings must use OS::String instead of OS::Core::String
		{
			typedef Core::String super;

		public:

			String(OS*);
			String(OS*, const OS_CHAR*);
			String(OS*, OS_CHAR, int count);
			String(OS*, const void*, int size);
			String(OS*, const void * buf1, int len1, const void * buf2, int len2);
			String(const Core::String&);
			String(const String&);
			String(Core::StringData*);
			String(OS*, OS_INT value);
			String(OS*, OS_FLOAT value, int precision);
			~String();

			// operator const String&() const { return *this; }

			String& operator=(const Core::String&);
			String& operator=(const OS_CHAR*);

			String& operator+=(const Core::String&);
			String& operator+=(const OS_CHAR*);
			String& append(const void*, int size);
			String& append(const OS_CHAR*);
		};

		static OS * create(MemoryManager * = NULL);

		OS * retain();
		void release();

		int getAllocatedBytes();
		int getMaxAllocatedBytes();
		int getCachedBytes();

		void getProperty(bool prototype_enabled = true, bool getter_enabled = true);
		void getProperty(const OS_CHAR*, bool prototype_enabled = true, bool getter_enabled = true);
		void getProperty(const Core::String&, bool prototype_enabled = true, bool getter_enabled = true);
		
		void setProperty(bool prototype_enabled = true, bool setter_enabled = true);
		void setProperty(const OS_CHAR*, bool prototype_enabled = true, bool setter_enabled = true);
		void setProperty(const Core::String&, bool prototype_enabled = true, bool setter_enabled = true);
		void addProperty();

		void getGlobal(const OS_CHAR*, bool prototype_enabled = true, bool getter_enabled = true);
		void getGlobal(const Core::String&, bool prototype_enabled = true, bool getter_enabled = true);

		void setGlobal(const OS_CHAR*, bool prototype_enabled = true, bool setter_enabled = true);
		void setGlobal(const Core::String&, bool prototype_enabled = true, bool setter_enabled = true);

		void getPrototype();
		void setPrototype();

		int getId(int offs = -1);
		
		void pushNull();
		void pushNumber(OS_FLOAT);
		void pushBool(bool);
		void pushString(const OS_CHAR*);
		void pushString(const Core::String&);
		void pushCFunction(OS_CFunction func, void * user_param = NULL);
		void pushCFunction(OS_CFunction func, int upvalues, void * user_param = NULL);
		void * pushUserData(int crc, int data_size, OS_UserDataDtor dtor = NULL, void * user_param = NULL);
		void * pushUserPointer(int crc, void * data, OS_UserDataDtor dtor = NULL, void * user_param = NULL);
		void newObject();
		void newArray();

		void pushStackValue(int offs = -1);
		void pushGlobals();
		void pushUserPool();
		void pushValueById(int id);

		int getStackSize();
		int getAbsoluteOffs(int offs);
		void remove(int start_offs = -1, int count = 1);
		void removeAll();
		void pop(int count = 1);
		void move(int start_offs, int count, int new_offs);
		void move(int offs, int new_offs);

		void runOp(OS_EOpcode opcode);

		// returns length of object, array, string or result of __len method
		// keep stack not changed
		int getLen(int offs = -1);

		OS_EValueType getType(int offs = -1);
		OS_EValueType getTypeById(int id);
		bool isNumber(int offs = -1, OS_FLOAT * out = NULL);
		bool isString(int offs = -1, String * out = NULL);
		bool isType(OS_EValueType, int offs = -1);
		bool isNull(int offs = -1);
		bool isObject(int offs = -1);
		bool isArray(int offs = -1);
		bool isFunction(int offs = -1);
		bool isUserData(int offs = -1);
		bool isInstanceOf(int value_offs = -2, int prototype_offs = -1);

		bool toBool(int offs = -1);
		OS_FLOAT toNumber(int offs = -1, bool valueof_enabled = true);
		int toInt(int offs = -1, bool valueof_enabled = true);
		String toString(int offs = -1, bool valueof_enabled = true);
		void * toUserData(int offs, int crc);
		void * toUserData(int crc);

		int getSetting(OS_ESettings);
		int setSetting(OS_ESettings, int);

		bool compileFilename(const Core::String& filename, bool required = false);
		bool compile(const Core::String& str);
		bool compile();

		int call(int params = 0, int ret_values = 0);
		int eval(const OS_CHAR * str, int params = 0, int ret_values = 0);
		int eval(const Core::String& str, int params = 0, int ret_values = 0);

		int run(const OS_CHAR * filename, bool required = false, int params = 0, int ret_values = 0);
		int run(const Core::String& filename, bool required = false, int params = 0, int ret_values = 0);

		// return next gc phase
		int gc();
		void gcFull();

		struct Func
		{
			const OS_CHAR * name;
			OS_CFunction func;
		};
		void setFuncs(const Func * list, int upvalues = 0, void * user_param = NULL); // null terminated list
		void getObject(const OS_CHAR * name, bool prototype_enabled = true, bool getter_enabled = true);
		void getGlobalObject(const OS_CHAR * name, bool prototype_enabled = true, bool getter_enabled = true);
	};

} // namespace OS

#endif // __OBJECT_SCRIPT_H__