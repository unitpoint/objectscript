#ifndef __OBJECT_SCRIPT_H__
#define __OBJECT_SCRIPT_H__

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

// #include <vector>
#include <stdarg.h>
#include <stdio.h>
#include <assert.h>
#include <ctype.h>
#include <math.h>
#include <string.h>
#include <malloc.h>
#include <new.h>
#include <stdlib.h>

#if defined _MSC_VER && !defined IW_SDK
#include <vadefs.h>
#endif

#ifdef _DEBUG
#define OS_DEBUG
#endif

// select ObjectScript number type here
#define OS_NUMBER double
// #define OS_NUMBER float	// could be a bit faster
// #define OS_NUMBER int	// not recomended

#define OS_FLOAT double
#define OS_INT16 short
#define OS_BYTE unsigned char
#define OS_U16 unsigned short

#ifdef IW_SDK

#define OS_INT int64
#define OS_INT32 int32
#define OS_INT64 int64
#define OS_U32 uint32
#define OS_U64 uint64

#else

#define OS_INT __int64
#define OS_INT32 __int32
#define OS_INT64 __int64
#define OS_U32 unsigned __int32
#define OS_U64 unsigned __int64

#endif

#define OS_CHAR char
#define OS_TEXT(s) s

#ifdef OS_DEBUG

#define OS_ASSERT assert

#define OS_DBG_FILEPOS_DECL , const OS_CHAR * dbg_filename, int dbg_line
#define OS_DBG_FILEPOS_PARAM , dbg_filename, dbg_line
#define OS_DBG_FILEPOS , __FILE__, __LINE__

#define OS_DBG_FILEPOS_START_DECL const OS_CHAR * dbg_filename, int dbg_line
#define OS_DBG_FILEPOS_START_PARAM dbg_filename, dbg_line
#define OS_DBG_FILEPOS_START __FILE__, __LINE__

#else

#define OS_ASSERT(a)
#define OS_DBG_FILEPOS_DECL
#define OS_DBG_FILEPOS_PARAM
#define OS_DBG_FILEPOS
#define OS_DBG_FILEPOS_START_DECL
#define OS_DBG_FILEPOS_START_PARAM
#define OS_DBG_FILEPOS_START

#endif // OS_DEBUG

#define OS_MEMCMP memcmp
#define OS_MEMMOVE memmove
#define OS_MEMSET memset
#define OS_MEMCPY memcpy
#define OS_STRLEN strlen
#define OS_STRCMP strcmp
#define OS_STRNCMP strncmp
#define OS_STRCHR strchr
#define OS_STRSTR strstr

#ifdef IW_SDK
#define OS_VSNPRINTF vsnprintf
#define OS_SNPRINTF snprintf
#else
#define OS_VSNPRINTF OS_vsnprintf
#define OS_SNPRINTF OS_snprintf
#endif // IW_SDK

#define OS_VPRINTF vprintf
#define OS_PRINTF ::printf

#define OS_IS_SPACE isspace
#define OS_IS_ALPHA isalpha
#define OS_IS_ALNUM isalnum // ((c) >= OS_TEXT('0') && (c) <= OS_TEXT('9'))
#define OS_IS_SLASH(c) ((c) == OS_TEXT('/') || (c) == OS_TEXT('\\'))

#define OS_MATH_POW ::pow
#define OS_MATH_FLOOR ::floor
#define OS_MATH_FMOD ::fmod

#if defined _MSC_VER // && !defined IW_SDK
#define DEBUG_BREAK __debugbreak()
#else
#include <signal.h>
#define DEBUG_BREAK raise(SIGTRAP)
// #define DEBUG_BREAK __builtin_trap()
#endif

#define OS_GLOBALS_VAR_NAME OS_TEXT("_G")
#define OS_ENV_VAR_NAME OS_TEXT("_E")

#define OS_AUTO_PRECISION 20

#define OS_DEF_FMT_BUF_LEN (1024*10)

#define OS_PATH_SEPARATOR OS_TEXT("\\")

// uncomment it if need
// #define OS_INFINITE_LOOP_OPCODES 100000000

#define OS_CALL_STACK_MAX_SIZE 200

#define OS_VERSION OS_TEXT("0.91-vm2")
#define OS_COMPILED_HEADER OS_TEXT("OBJECTSCRIPT")
#define OS_DEBUGINFO_HEADER OS_TEXT("OBJECTSCRIPT.DEBUGINFO")
#define OS_SOURCECODE_EXT OS_TEXT(".os")
#define OS_COMPILED_EXT OS_TEXT(".osc")
#define OS_DEBUG_INFO_EXT OS_TEXT(".osd")
#define OS_DEBUG_OPCODES_FILENAME OS_TEXT("os-opcodes.txt")

#define OS_MEMORY_MANAGER_PAGE_BLOCKS 32

#define OS_DEBUGGER_SAVE_NUM_LINES 11

namespace ObjectScript
{

	class OS;

	typedef void (*OS_UserDataDtor)(OS*, void * data, void * user_param);
	typedef int (*OS_CFunction)(OS*, int params, int closure_values, int need_ret_values, void * user_param);

	enum OS_ESettings
	{
		OS_SETTING_CREATE_DEBUG_OPCODES,
		OS_SETTING_CREATE_DEBUG_INFO,
		OS_SETTING_RECOMPILE_SOURCECODE,
	};

	enum OS_EValueType
	{
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
		OS_VALUE_TYPE_UNKNOWN,
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

	enum
	{
		OS_WARNING	= 1<<0,
		OS_ERROR	= 1<<1
	};

	enum OS_EOpcode
	{
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
		protected:

			int ref_count;

			virtual ~MemoryManager();

		public:

			MemoryManager();

			MemoryManager * retain();
			void release();

			virtual void * malloc(int size OS_DBG_FILEPOS_DECL) = 0;
			virtual void free(void * p) = 0;
			virtual void setBreakpointId(int id) = 0;

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
#ifdef OS_DEBUG
				int mark;
#endif
				Page * page;
				CachedBlock * next;
			};

			struct MemBlock
			{
				Page * page;
#ifdef OS_DEBUG
				const OS_CHAR * dbg_filename;
				int dbg_line;
				int dbg_id;
				MemBlock * dbg_mem_prev;
				MemBlock * dbg_mem_next;
#endif
				int block_size;
#ifdef OS_DEBUG
				int mark;
#endif
			};

			struct StdMemBlock
			{
#ifdef OS_DEBUG
				const OS_CHAR * dbg_filename;
				int dbg_line;
				int dbg_id;
				StdMemBlock * dbg_mem_prev;
				StdMemBlock * dbg_mem_next;
#endif
				int block_size;
#ifdef OS_DEBUG
				int mark;
#endif
			};

			enum {
				MAX_PAGE_TYPE_COUNT = 17
			};

			PageDesc page_desc[MAX_PAGE_TYPE_COUNT];
			int num_page_desc;

			Page * pages[MAX_PAGE_TYPE_COUNT];

			CachedBlock * cached_blocks[MAX_PAGE_TYPE_COUNT];

#ifdef OS_DEBUG
			MemBlock * dbg_mem_list;
			StdMemBlock * dbg_std_mem_list;
			int dbg_breakpoint_id;
#endif

			int stat_malloc_count;
			int stat_free_count;

			void registerPageDesc(int block_size, int num_blocks);
			
			static int comparePageDesc(const void * pa, const void * pb);
			void sortPageDesc();

			void * allocFromCachedBlock(int i OS_DBG_FILEPOS_DECL);
			void * allocFromPageType(int i OS_DBG_FILEPOS_DECL);
			void freeMemBlock(MemBlock*);

			void freeCachedMemory(int new_cached_bytes);

			void * stdAlloc(int size OS_DBG_FILEPOS_DECL);
			void stdFree(void * p);

		public:

			SmartMemoryManager();
			~SmartMemoryManager();
			
			void * malloc(int size OS_DBG_FILEPOS_DECL);
			void free(void * p);
			void setBreakpointId(int id);

			int getAllocatedBytes();
			int getMaxAllocatedBytes();
			int getCachedBytes();
		};

		struct Utils
		{
			static bool parseFloat(const OS_CHAR *& str, OS_FLOAT& val);

			static OS_CHAR * numToStr(OS_CHAR*, OS_INT32 value);
			static OS_CHAR * numToStr(OS_CHAR*, OS_INT64 value);
			static OS_CHAR * numToStr(OS_CHAR*, float value, int precision = OS_AUTO_PRECISION);
			static OS_CHAR * numToStr(OS_CHAR*, double value, int precision = OS_AUTO_PRECISION);

			static OS_INT strToInt(const OS_CHAR*);
			static OS_FLOAT strToFloat(const OS_CHAR*);

			static int addKeyToHash(int hash, const void*, int size);
			static int keyToHash(const void*, int size);
			static int keyToHash(const void * buf1, int size1, const void * buf2, int size2);

			static int cmp(const void * buf1, int len1, const void * buf2, int len2);
		};

		class String;
		class ObjectScriptExtention;

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

			T& lastElement()
			{
				OS_ASSERT(count > 0);
				return buf[count-1];
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

		template<class T> void vectorReserveCapacity(Vector<T>& vec, int new_capacity OS_DBG_FILEPOS_DECL)
		{
			if(vec.capacity < new_capacity){
				vec.capacity = vec.capacity > 0 ? vec.capacity*2 : 4;
				if(vec.capacity < new_capacity){
					vec.capacity = new_capacity; // (capacity+3) & ~3;
				}
				T * new_buf = (T*)malloc(sizeof(T)*vec.capacity OS_DBG_FILEPOS_PARAM);
				OS_ASSERT(new_buf);
				for(int i = 0; i < vec.count; i++){
					new (new_buf+i) T(vec.buf[i]);
					vec.buf[i].~T();
				}
				free(vec.buf);
				vec.buf = new_buf;
			}
		}

		template<class T> void vectorReserveCapacityExact(Vector<T>& vec, int capacity OS_DBG_FILEPOS_DECL)
		{
			if(vec.capacity < capacity){
				vec.capacity = capacity;
				T * new_buf = (T*)malloc(sizeof(T)*vec.capacity OS_DBG_FILEPOS_PARAM);
				OS_ASSERT(new_buf);
				for(int i = 0; i < vec.count; i++){
					new (new_buf+i) T(vec.buf[i]);
					vec.buf[i].~T();
				}
				free(vec.buf);
				vec.buf = new_buf;
			}
		}

		template<class T> void vectorAddItem(Vector<T>& vec, const T& val OS_DBG_FILEPOS_DECL)
		{
			vectorReserveCapacity(vec, vec.count+1 OS_DBG_FILEPOS_PARAM);
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

		template<class T> void vectorInsertAtIndex(Vector<T>& vec, int i, const T& val OS_DBG_FILEPOS_DECL)
		{
			OS_ASSERT(i >= 0 && i <= vec.count);
			vectorReserveCapacity(vec, vec.count+1 OS_DBG_FILEPOS_PARAM);
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
				obj = NULL;
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

				void * f;

				FileStreamWriter(OS*, const OS_CHAR * filename);
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
				virtual bool checkBytes(const void*, int len) = 0;

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
				bool checkBytes(const void*, int len);

				void * readBytes(void*, int len);
				void * readBytesAtPos(void*, int len, int pos);

				OS_BYTE readByte();
				OS_BYTE readByteAtPos(int pos);
			};

			class FileStreamReader: public StreamReader
			{
			public:

				void * f;

				FileStreamReader(OS*, const OS_CHAR * filename);
				~FileStreamReader();

				int getPos() const;
				void setPos(int);
				
				int getSize() const;

				void movePos(int len);
				bool checkBytes(const void*, int len);

				void * readBytes(void*, int len);
				void * readBytesAtPos(void*, int len, int pos);
			};

			struct GCStringValue;
			class StringBuffer;
			class String
			{
			public:

#ifdef OS_DEBUG
				const OS_CHAR * str;
#endif
				GCStringValue * string;

				String(OS*);
				String(GCStringValue*);
				String(const String&);
				String(OS*, const OS_CHAR*);
				String(OS*, const OS_CHAR*, int len);
				String(OS*, const OS_CHAR*, int len, const OS_CHAR*, int len2);
				String(OS*, const OS_CHAR*, int len, bool trim_left, bool trim_right);
				String(OS*, const void*, int size);
				String(OS*, const void * buf1, int len1, const void * buf2, int len2);
				String(OS*, const void * buf1, int len1, const void * buf2, int len2, const void * buf3, int len3);
				String(OS*, OS_INT value);
				String(OS*, OS_FLOAT value, int precision = OS_AUTO_PRECISION);
				~String();

				static String format(OS*, int temp_buf_len, const OS_CHAR * fmt, ...);
				static String formatVa(OS*, int temp_buf_len, const OS_CHAR * fmt, va_list va);
				static String format(OS*, const OS_CHAR * fmt, ...);
				static String formatVa(OS*, const OS_CHAR * fmt, va_list va);

				const OS_CHAR * toChar() const { return string->toChar(); }
				operator const OS_CHAR*() const { return string->toChar(); }

				OS_CHAR operator[](int i)
				{
					if(i >= 0 && i < getLen()){
						return toChar()[i];
					}
					return OS_TEXT('\0');
				}

				int getDataSize() const { return string->data_size; }
				int getLen() const { return string->getLen(); }

				String& operator=(const String&);

				bool operator==(const String&) const;
				bool operator==(const OS_CHAR*) const;
				bool operator==(GCStringValue*) const;

				bool operator!=(const String&) const;
				bool operator!=(const OS_CHAR*) const;
				bool operator!=(GCStringValue*) const;

				bool operator<=(const String&) const;
				bool operator<=(const OS_CHAR*) const;

				bool operator<(const String&) const;
				bool operator<(const OS_CHAR*) const;

				bool operator>=(const String&) const;
				bool operator>=(const OS_CHAR*) const;

				bool operator>(const String&) const;
				bool operator>(const OS_CHAR*) const;

				int cmp(const String&) const;
				int cmp(const OS_CHAR*) const;
				int getHash() const;

				OS_FLOAT toFloat() const;
			};

			class StringBuffer: public Vector<OS_CHAR>
			{
			public:

				OS * allocator;

				StringBuffer(OS*);
				~StringBuffer();

				StringBuffer& append(OS_CHAR);
				StringBuffer& append(const OS_CHAR*);
				StringBuffer& append(const OS_CHAR*, int len);
				StringBuffer& append(const String&);
				StringBuffer& append(const StringBuffer&);

				StringBuffer& operator+=(const String&);
				StringBuffer& operator+=(const OS_CHAR*);

				operator String() const;
				String toString() const;

				GCStringValue * toGCStringValue() const;
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

				public:

					OS * allocator;
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

					~TokenData();

				public:

					TextData * text_data;

					String str;
					int line, pos;
					int ref_count;
					TokenType type;

					OS * getAllocator() const;

					OS_FLOAT getFloat() const;

					TokenData(TextData * text_data, const String& p_str, TokenType p_type, int p_line, int p_pos);

					TokenData * retain();
					void release();

					void setFloat(OS_FLOAT value);

					operator String () const { return str; }

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

				static int compareOperatorDesc(const void * a, const void * b) ;
				static void initOperatorsTable();

				TokenData * addToken(const String& token, TokenType type, int line, int pos OS_DBG_FILEPOS_DECL);

				bool parseFloat(const OS_CHAR *& str, OS_FLOAT& fval, bool parse_end_spaces);
				bool parseLines();

			public:

				Tokenizer(OS*);
				~Tokenizer();

				OS * getAllocator();
				TextData * getTextData() const { return text_data; }

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
				TokenData * removeToken(int i);
				void insertToken(int i, TokenData * token OS_DBG_FILEPOS_DECL);
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
				OS_INT next_index;

				Property * first, * last;
				IteratorState * iterators;

				Table();    
				~Table();

				Property * get(const PropertyIndex& index);

				bool containsIterator(IteratorState*);
				void addIterator(IteratorState*);
				void removeIterator(IteratorState*);
			};

			enum EGCColor
			{
				GC_WHITE,
				GC_GREY,
				GC_BLACK
			};

			struct GCValue
			{
				int value_id;
				int external_ref_count;
				GCValue * prototype;
				GCValue * hash_next;

				Table * table;

				// Value * gc_grey_prev;
				GCValue * gc_grey_next;
#ifdef OS_DEBUG
				int gc_time;
#endif

				OS_EValueType type;
				bool is_object_instance;

				EGCColor gc_color;

				GCValue();
				virtual ~GCValue();
			};

			template <class T>
			struct GCValueRetained
			{
				T * value;

				GCValueRetained(T * v)
				{
					value = v;
					if(value){
						value->external_ref_count++;
					}
				}
				GCValueRetained(const GCValueRetained& b)
				{
					value = b.value;
					if(value){
						value->external_ref_count++;
					}
				}
				~GCValueRetained()
				{
					if(value){
						OS_ASSERT(value->external_ref_count > 0);
						value->external_ref_count--;
					}
				}

				T * operator->(){ return value; }
				operator T* (){ return value; }

				GCValueRetained& operator=(const GCValueRetained& b)
				{
					if(value != b.value){
						if(value){
							OS_ASSERT(value->external_ref_count > 0);
							value->external_ref_count--;
						}
						value = b.value;
						if(value){
							value->external_ref_count++;
						}
					}
					return *this;
				}
			};

			struct GCObjectValue: public GCValue
			{
			};

			struct GCStringValue: public GCValue
			{
#ifdef OS_DEBUG
				OS_CHAR * str;
#endif
				int data_size;
				int hash;

				GCStringValue(int p_data_size);
				// ~GCStringValue();

				int getDataSize() const { return data_size; }
				int getLen() const { return data_size/sizeof(OS_CHAR); }
				OS_CHAR * toChar() const { return (OS_CHAR*)(this + 1); }
				OS_BYTE * toBytes() const { return (OS_BYTE*)(this + 1); }
				void * toMemory() const { return (void*)(this + 1); }
				// bool isExternal() const { return str != (OS_CHAR*)(this+1); }

				static GCStringValue * alloc(OS*, const void *, int data_size OS_DBG_FILEPOS_DECL);
				static GCStringValue * alloc(OS*, const void * buf1, int len1, const void * buf2, int len2 OS_DBG_FILEPOS_DECL);
				// static GCStringValue * alloc(OS*, const void * buf1, int len1, const void * buf2, int len2, const void * buf3, int len3);
				static GCStringValue * alloc(OS*, GCStringValue * a, GCStringValue * b OS_DBG_FILEPOS_DECL);
				// static GCStringValue * alloc(OS*, GCStringValue * a, GCStringValue * b, GCStringValue * c);

				bool isFloat(OS_FLOAT*) const;
				// bool isNumber(OS_FLOAT*) const;

				int cmp(GCStringValue*) const;
				int cmp(const OS_CHAR*) const;
				int cmp(const OS_CHAR*, int len) const;
				void calcHash();
			};

			struct GCUserDataValue: public GCValue
			{
				int crc;
				void * ptr;
				OS_UserDataDtor dtor;
				void * user_param;
			};

			struct GCCFunctionValue: public GCValue
			{
				OS_CFunction func;
				void * user_param;
				int num_closure_values;
			};

			struct GCFunctionValue;

			struct WeakRef { WeakRef(){} };
			struct Value
			{
				union {
					int boolean;
					OS_NUMBER number;
					int value_id;
					GCValue * value;
					GCObjectValue * object;
					GCStringValue * string;
					GCUserDataValue * userdata;
					GCFunctionValue * func;
					GCCFunctionValue * cfunc;
				} v;

				OS_EValueType type;

				Value();
				Value(bool);
				Value(OS_INT32);
				Value(OS_INT64);
				Value(float);
				Value(double);
				Value(int, const WeakRef&);
				Value(GCValue*);

				Value& operator=(GCValue*);
				
				void clear();

				GCValue * getGCValue() const;

				bool isFunction() const;
				bool isUserData() const;
			};

			struct ValueRetained: public Value
			{
				typedef Value super;

				ValueRetained();
				ValueRetained(bool);
				ValueRetained(OS_FLOAT);
				ValueRetained(int);
				ValueRetained(int, const WeakRef&);
				ValueRetained(GCValue*);
				ValueRetained(Value);
				~ValueRetained();

				ValueRetained& operator=(Value);
				
				void clear();

				void retain();
				void release();
			};

			class Program;
			struct FunctionDecl;
			struct Upvalues;
			struct GCFunctionValue: public GCValue
			{
				Program * prog; // retained
				FunctionDecl * func_decl;
				Value env;
				Upvalues * upvalues; // retained

				GCFunctionValue();
				~GCFunctionValue();
			};

			struct PropertyIndex
			{
				struct KeepStringIndex
				{
					KeepStringIndex(){}
				};

				Value index;

				PropertyIndex(const PropertyIndex& index);
				PropertyIndex(Value index);
				PropertyIndex(Value index, const KeepStringIndex&);
				PropertyIndex(GCStringValue * index);
				PropertyIndex(GCStringValue * index, const KeepStringIndex&);
				PropertyIndex(const String& index);
				PropertyIndex(const String& index, const KeepStringIndex&);

				void convertIndexStringToNumber();

				bool isEqual(const PropertyIndex& b) const;
				bool isEqual(int hash, const void * b, int size) const;
				bool isEqual(int hash, const void * buf1, int size1, const void * buf2, int size2) const;
				int getHash() const;
			};

			struct Property: public PropertyIndex
			{
				Value value;

				Property * hash_next;
				Property * prev, * next;

				Property(const PropertyIndex& index);
				Property(Value index);
				Property(Value index, const KeepStringIndex&);
				Property(GCStringValue * index);
				Property(GCStringValue * index, const KeepStringIndex&);
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
					EXP_TYPE_DEBUG_LOCALS,
					
					EXP_TYPE_IF,

					EXP_TYPE_ARRAY,

					EXP_TYPE_OBJECT,
					EXP_TYPE_OBJECT_SET_BY_NAME,
					EXP_TYPE_OBJECT_SET_BY_INDEX,
					EXP_TYPE_OBJECT_SET_BY_EXP,
					EXP_TYPE_OBJECT_SET_BY_AUTO_INDEX,

					EXP_TYPE_SUPER,
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
					OS_U16 target_local;
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

					void debugPrint(StringBuffer&, Compiler * compiler, int depth);
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
					int prog_func_index;
					Vector<LocalVar> locals;
					Vector<LocalVarCompiled> locals_compiled;
					int num_params;
					int num_locals;
					int opcodes_pos;
					int opcodes_size;
					int max_up_count;
					int func_depth;
					int func_index;
					int num_local_funcs;

					Vector<LoopBreak> loop_breaks;

					bool parser_started;

					Scope(Scope * parent, ExpressionType, TokenData*);
					virtual ~Scope();

					bool addLoopBreak(int pos, ELoopBreakType);
					void fixLoopBreaks(int scope_start_pos, int scope_end_pos, StreamWriter*);

					void addStdVars();
					void addLocalVar(const String& name);
					void addLocalVar(const String& name, LocalVarDesc&);
				};

				enum ErrorType {
					ERROR_NOTHING,
					ERROR_SYNTAX,
					ERROR_NESTED_ROOT_BLOCK,
					ERROR_LOCAL_VAL_NOT_DECLARED,
					ERROR_VAR_ALREADY_EXIST,
					ERROR_VAR_NAME,
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
				Vector<OS_NUMBER> prog_numbers;
				Vector<String> prog_strings;
				Vector<String> prog_debug_strings;
				Vector<Scope*> prog_functions;
				MemStreamWriter * prog_opcodes;
				MemStreamWriter * prog_debug_info;
				int prog_num_debug_infos;
				int prog_max_up_count;

				int prog_stack_size;

				bool isError();
				void resetError();
				void setError();
				void setError(ErrorType value, TokenData * error_token);
				void setError(TokenType expect_token_type, TokenData * error_token);
				void setError(const String& str, TokenData * error_token);

				void * malloc(int size OS_DBG_FILEPOS_DECL);

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
					bool allow_call;

					Params();
					Params(const Params&);

					Params& setAllowRootBlocks(bool);
					Params& setAllowBinaryOperator(bool);
					Params& setAllowAssign(bool);
					Params& setAllowParams(bool);
					// Params& setAllowLeftSideParams(bool);
					// Params& setAllowRightSideParams(bool);
					Params& setAllowAutoCall(bool);
					Params& setAllowCall(bool);
				};

				Expression * expectSingleExpression(Scope*, const Params& p);
				Expression * expectSingleExpression(Scope*);
					// bool allow_binary_operator, bool allow_param, bool allow_var, bool allow_assign, bool allow_auto_call);

				Expression * expectExpressionValues(Expression * exp, int ret_values);
				Expression * newExpressionFromList(ExpressionList& list, int ret_values);
				Expression * newAssingExpression(Scope * scope, Expression * var_exp, Expression * value_exp);
				// Expression * newIndirectExpression(Scope * scope, Expression * var_exp, Expression * value_exp);
				Expression * newSingleValueExpression(Expression * exp);
				
				Expression * postProcessExpression(Scope * scope, Expression * exp);
				Expression * stepPass2(Scope * scope, Expression * exp);
				Expression * stepPass3(Scope * scope, Expression * exp);

				bool isVarNameValid(const String& name);

				Scope * expectTextExpression();
				Scope * expectCodeExpression(Scope*, int ret_values = 0);
				Expression * expectFunctionExpression(Scope*);
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
				Expression * expectDebugLocalsExpression(Scope*);
				Expression * finishValueExpression(Scope*, Expression*, const Params& p); // bool allow_binary_operator, bool allow_param, bool allow_assign, bool allow_auto_call);
				Expression * finishBinaryOperator(Scope * scope, OpcodeLevel prev_level, Expression * exp, const Params& p, bool& is_finished); // bool allow_param, bool& is_finished);
				Expression * newBinaryExpression(Scope * scope, ExpressionType, TokenData*, Expression * left_exp, Expression * right_exp);

				bool findLocalVar(LocalVarDesc&, Scope * scope, const String& name, int active_locals, bool all_scopes);

				void debugPrintSourceLine(StringBuffer& out, TokenData*);
				static const OS_CHAR * getExpName(ExpressionType);

				int cacheString(Table * strings_table, Vector<String>& strings, const String& str);
				int cacheString(const String& str);
				int cacheDebugString(const String& str);
				int cacheNumber(OS_NUMBER);

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

#ifdef OS_DEBUG
				int prog_func_index;
#endif
				int prog_parent_func_index;
				LocalVar * locals;
				int num_locals;
				int num_params;
				int max_up_count;
				int func_depth;
				int func_index; // in parent space
				int num_local_funcs;
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

					OP_PUSH_NEW_ARRAY,
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

					OP_SUPER,

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

				GCStringValue ** const_strings;
				int num_strings;
				
				OS_NUMBER * const_numbers;
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

			enum {
				ENV_VAR_INDEX,
				GLOBALS_VAR_INDEX,
			};

			struct Upvalues
			{
				int ref_count;
				int gc_time;

				Value * locals;
				int num_locals;

				bool is_stack_locals;
				
				int num_parents;
				// Upvalues * parent;

				// Upvalues();
				// ~Upvalues();

				Upvalues ** getParents();
				Upvalues * getParent(int i);
				void setParent(int i, Upvalues*);

				Upvalues * retain();
			};

			struct StackFunction
			{
				GCFunctionValue * func;
				GCValue * self;

				Upvalues * locals;
				int num_params;
				int num_extra_params;

				GCValue * arguments;
				GCValue * rest_arguments;
				
				int caller_stack_pos;
				int locals_stack_pos;
				int opcode_stack_pos;
				int bottom_stack_pos;

				int need_ret_values;

				int opcodes_pos;
				
				StackFunction();
				~StackFunction();
			};

			struct Values
			{
				GCValue ** heads;
				int head_mask;
				int count;

				int next_id;

				Values();
				~Values();

				// Value * add(Value * obj);
				// Value * remove(int value_id);
				GCValue * get(int value_id);
			};

			OS * allocator;

			struct Strings
			{
				String __construct;
				// String __destruct;
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

				String syntax_super;
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
				String syntax_debuglocals;

				String var_globals;
				String var_env;

				int __dummy__;

				Strings(OS * allocator);
			} * strings;

			Values values;
			int num_created_values;
			int num_destroyed_values;

			Table * string_values_table;
			GCObjectValue * check_recursion;
			GCObjectValue * global_vars;
			GCObjectValue * user_pool;

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

			GCObjectValue * prototypes[PROTOTYPE_COUNT];

			// Vector<Value> stack_values;
			struct StackValues {
				Value * buf;
				int capacity;
				int count;

				StackValues();
				~StackValues();

				Value& operator[](int i)
				{
					OS_ASSERT(i >= 0 && i < count);
					return buf[i];
				}

				Value& lastElement()
				{
					OS_ASSERT(count > 0);
					return buf[count-1];
				}
			} stack_values;

			void reserveStackValues(int new_capacity);

			Vector<StackFunction> call_stack_funcs;

			GCValue * gc_grey_list_first;
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

			enum {
				RAND_STATE_SIZE = 624
			};

			OS_U32 rand_state[RAND_STATE_SIZE+1];
			OS_U32 rand_seed;
			OS_U32 * rand_next;
			int rand_left;

			void randInitialize(OS_U32 seed);
			void randReload();
			double getRand();
			double getRand(double up);
			double getRand(double min, double max);

			void * malloc(int size OS_DBG_FILEPOS_DECL);
			void free(void * p);

			void error(int code, const OS_CHAR * message);
			void error(int code, const String& message);

			void gcInitGreyList();
			void gcResetGreyList();
			void gcAddToGreyList(GCValue*);
			void gcAddToGreyList(Value);
			void gcRemoveFromGreyList(GCValue*);
			void gcMarkProgram(Program * prog);
			void gcMarkTable(Table * table);
			void gcMarkUpvalues(Upvalues*);
			void gcMarkStackFunction(StackFunction*);
			void gcMarkList(int step_size);
			void gcMarkValue(GCValue * value);
			
			// return next gc phase
			int gcStep();
			void gcStepIfNeeded();
			void gcFinishSweepPhase();
			void gcFinishMarkPhase();
			void gcFull();

			void clearValue(Value&);
			void clearValue(GCValue*);
			void deleteValue(GCValue*);

#ifdef OS_DEBUG
			bool isValueUsed(GCValue*);
#endif

			GCFunctionValue * newFunctionValue(StackFunction*, Program*, FunctionDecl*, Value env);
			void clearFunctionValue(GCFunctionValue*);

			// Upvalues * newUpvalues(int num_parents);
			void releaseUpvalues(Upvalues*);
			void deleteUpvalues(Upvalues*);
			void clearStackFunction(StackFunction*);

			// GCValue * newValue();
			// Value newBoolValue(bool);
			// Value newNumberValue(OS_FLOAT);

			GCStringValue * newStringValue(const String&);
			GCStringValue * newStringValue(const String&, const String&);
			GCStringValue * newStringValue(const String&, bool trim_left, bool trim_right);
			GCStringValue * newStringValue(const OS_CHAR*);
			GCStringValue * newStringValue(const OS_CHAR*, int len);
			GCStringValue * newStringValue(const OS_CHAR*, int len, const OS_CHAR*, int len2);
			GCStringValue * newStringValue(const OS_CHAR*, int len, bool trim_left, bool trim_right);
			GCStringValue * newStringValue(const void * buf, int size);
			GCStringValue * newStringValue(const void * buf1, int size1, const void * buf2, int size2);
			GCStringValue * newStringValue(const void * buf1, int size1, const void * buf2, int size2, const void * buf3, int size3);
			GCStringValue * newStringValue(GCStringValue*, GCStringValue*);
			GCStringValue * newStringValue(OS_INT);
			GCStringValue * newStringValue(OS_FLOAT, int);
			GCStringValue * newStringValue(int temp_buf_len, const OS_CHAR * fmt, ...);
			GCStringValue * newStringValueVa(int temp_buf_len, const OS_CHAR * fmt, va_list va);

			GCCFunctionValue * newCFunctionValue(OS_CFunction func, void * user_param);
			GCCFunctionValue * newCFunctionValue(OS_CFunction func, int closure_values, void * user_param);
			GCUserDataValue * newUserDataValue(int crc, int data_size, OS_UserDataDtor dtor, void * user_param);
			GCUserDataValue * newUserPointerValue(int crc, void * data, OS_UserDataDtor dtor, void * user_param);
			GCObjectValue * newObjectValue();
			GCObjectValue * newObjectValue(GCValue * prototype);
			GCObjectValue * newArrayValue();

			template<class T> T * pushValue(T * val){ pushValue(Value(val)); return val; }

			void pushValue(Value val);
			void pushStackValue(int offs);
			void copyValue(int raw_from, int raw_to);
			void insertValue(Value val, int offs);
			void pushNull();
			void pushTrue();
			void pushFalse();
			void pushBool(bool);
			void pushNumber(OS_INT32);
			void pushNumber(OS_INT64);
			void pushNumber(float);
			void pushNumber(double);
			
			GCStringValue * pushStringValue(const String&);
			GCStringValue * pushStringValue(const OS_CHAR*);
			GCCFunctionValue * pushCFunctionValue(OS_CFunction func, void * user_param);
			GCCFunctionValue * pushCFunctionValue(OS_CFunction func, int closure_values, void * user_param);
			GCUserDataValue * pushUserDataValue(int crc, int data_size, OS_UserDataDtor dtor, void * user_param);
			GCUserDataValue * pushUserPointerValue(int crc, void * data, OS_UserDataDtor dtor, void * user_param);
			GCObjectValue * pushObjectValue();
			GCObjectValue * pushObjectValue(GCValue * prototype);
			GCObjectValue * pushArrayValue();

			void pushTypeOf(Value val);
			bool pushNumberOf(Value val);
			bool pushStringOf(Value val);
			bool pushValueOf(Value val);
			GCObjectValue * pushArrayOf(Value val);
			GCObjectValue * pushObjectOf(Value val);
			GCUserDataValue * pushUserDataOf(Value val);
			bool pushFunctionOf(Value val);

			void pushCloneValue(Value val);

			// unary operator
			void pushOpResultValue(int opcode, Value value);

			// binary operator
			void pushOpResultValue(int opcode, Value left_value, Value right_value);

			void setGlobalValue(const String& name, Value value, bool setter_enabled);
			void setGlobalValue(const OS_CHAR * name, Value value, bool setter_enabled);

			int getStackOffs(int offs);
			Value getStackValue(int offs);

			void removeStackValues(int offs, int count);
			void removeStackValue(int offs = -1);
			void removeAllStackValues();
			void pop(int count = 1);
			void moveStackValues(int offs, int count, int new_offs);
			void moveStackValue(int offs, int new_offs);

			void syncStackRetValues(int need_ret_values, int cur_ret_values);

			void registerValue(GCValue * val);
			GCValue * unregisterValue(int value_id);
			void deleteValues(bool del_ref_counted_also);

			bool valueToBool(Value val);
			OS_INT valueToInt(Value val, bool valueof_enabled = false);
			OS_FLOAT valueToNumber(Value val, bool valueof_enabled = false);
			String valueToString(Value val, bool valueof_enabled = false);

			bool isValueNumber(Value val, OS_FLOAT * out = NULL);
			bool isValueString(Value val, String * out = NULL);
			bool isValueString(Value val, OS::String * out = NULL);
			bool isValueInstanceOf(GCValue * val, GCValue * prototype_val);
			bool isValueInstanceOf(Value val, Value prototype_val);

			Table * newTable(OS_DBG_FILEPOS_START_DECL);
			void clearTable(Table*);
			void deleteTable(Table*);
			void addTableProperty(Table * table, Property * prop);
			bool deleteTableProperty(Table * table, const PropertyIndex& index);
			void deleteValueProperty(GCValue * table_value, const PropertyIndex& index, bool prototype_enabled, bool del_method_enabled);
			void deleteValueProperty(Value table_value, const PropertyIndex& index, bool prototype_enabled, bool del_method_enabled);
			void reorderTableNumericKeys(Table * table);
			void reorderTableKeys(Table * table);
			void initTableProperties(Table * dst, Table * src);

			Property * setTableValue(Table * table, const PropertyIndex& index, Value val);
			void setPropertyValue(GCValue * table_value, const PropertyIndex& index, Value val, bool setter_enabled);
			void setPropertyValue(Value table_value, const PropertyIndex& index, Value val, bool setter_enabled);

			bool getPropertyValue(Value& result, Table * table, const PropertyIndex& index);
			bool getPropertyValue(Value& result, GCValue * table_value, const PropertyIndex& index, bool prototype_enabled);
			bool getPropertyValue(Value& result, Value table_value, const PropertyIndex& index, bool prototype_enabled);

			void pushPropertyValue(GCValue * table_value, const PropertyIndex& index, bool prototype_enabled, bool getter_enabled, bool auto_create);
			void pushPropertyValue(Value table_value, const PropertyIndex& index, bool prototype_enabled, bool getter_enabled, bool auto_create);

			void setPrototype(Value val, Value proto);
			void pushPrototype(Value val);

			void enterFunction(GCFunctionValue * func_value, GCValue * self, int params, int extra_remove_from_stack, int need_ret_values);
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

		virtual ~OS();

		// bool init(ObjectScriptExtention * ext, MemoryManager * manager);
		virtual void shutdown();

		void * malloc(int size OS_DBG_FILEPOS_DECL);
		void free(void * p);

		void initGlobalFunctions();
		void initObjectClass();
		void initArrayClass();
		void initFunctionClass();
		void initStringClass();
		void initMathLibrary();
		void initPreScript();
		void initPostScript();

	public:

		class String: public Core::String // this string retains OS
		{
			typedef Core::String super;

		protected:

			OS * allocator;

		public:

			String(OS*);
			String(const String&);
			String(OS*, const Core::String&);
			String(OS*, const OS_CHAR*);
			String(OS*, const OS_CHAR*, int len);
			String(OS*, const OS_CHAR*, int len, const OS_CHAR*, int len2);
			String(OS*, const OS_CHAR*, int len, bool trim_left, bool trim_right);
			String(OS*, const void*, int size);
			String(OS*, const void * buf1, int len1, const void * buf2, int len2);
			// String(OS*, const void * buf1, int len1, const void * buf2, int len2, const void * buf3, int len3);
			String(OS*, OS_INT value);
			String(OS*, OS_FLOAT value, int precision = OS_AUTO_PRECISION);
			~String();

			String& operator=(const Core::String&);
			String& operator=(const String&);
			String& operator+=(const String&);
			String& operator+=(const OS_CHAR*);
			String operator+(const String&) const;
			String operator+(const OS_CHAR*) const;

			String trim(bool trim_left = true, bool trim_right = true) const;
		};

		static OS * create(MemoryManager* = NULL);
		static OS * create(OS *, MemoryManager* = NULL);

		OS();

		virtual OS * start(MemoryManager* = NULL);
		virtual bool init(MemoryManager* = NULL);

		OS * retain();
		void release();

		int getAllocatedBytes();
		int getMaxAllocatedBytes();
		int getCachedBytes();

		void setMemBreakpointId(int id);

		void getProperty(bool prototype_enabled = true, bool getter_enabled = true);
		void getProperty(const OS_CHAR*, bool prototype_enabled = true, bool getter_enabled = true);
		void getProperty(const Core::String&, bool prototype_enabled = true, bool getter_enabled = true);
		
		void setProperty(bool setter_enabled = true);
		void setProperty(const OS_CHAR*, bool setter_enabled = true);
		void setProperty(const Core::String&, bool setter_enabled = true);
		void addProperty();

		void getGlobal(const OS_CHAR*, bool prototype_enabled = true, bool getter_enabled = true);
		void getGlobal(const Core::String&, bool prototype_enabled = true, bool getter_enabled = true);

		void setGlobal(const OS_CHAR*, bool setter_enabled = true);
		void setGlobal(const Core::String&, bool setter_enabled = true);

		void getPrototype();
		void setPrototype();

		int getId(int offs = -1);
		
		void pushNull();
		void pushNumber(OS_INT32);
		void pushNumber(OS_INT64);
		void pushNumber(float);
		void pushNumber(double);
		void pushBool(bool);
		void pushString(const OS_CHAR*);
		void pushString(const Core::String&);
		void pushCFunction(OS_CFunction func, void * user_param = NULL);
		void pushCFunction(OS_CFunction func, int closure_values, void * user_param = NULL);
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

		bool compileFile(const String& filename, bool required = false);
		bool compile(const String& str);
		bool compile();

		int call(int params = 0, int ret_values = 0);
		int eval(const OS_CHAR * str, int params = 0, int ret_values = 0);
		int eval(const String& str, int params = 0, int ret_values = 0);

		int require(const OS_CHAR * filename, bool required = false, int ret_values = 0);
		int require(const String& filename, bool required = false, int ret_values = 0);

		// return next gc phase
		int gc();
		void gcFull();

		struct Func {
			const OS_CHAR * name;
			OS_CFunction func;
		};
		void setFuncs(const Func * list, int closure_values = 0, void * user_param = NULL); // null terminated list

		struct Number {
			const OS_CHAR * name;
			OS_NUMBER value;
		};
		void setNumbers(const Number * list);

		void getObject(const OS_CHAR * name, bool prototype_enabled = true, bool getter_enabled = true);
		void getGlobalObject(const OS_CHAR * name, bool prototype_enabled = true, bool getter_enabled = true);

		String changeFilenameExt(const String& filename, const String& ext);
		String changeFilenameExt(const String& filename, const OS_CHAR * ext);
		
		String getFilenameExt(const String& filename);
		String getFilenameExt(const OS_CHAR * filename);
		String getFilenameExt(const OS_CHAR * filename, int len);
		
		String getFilenamePath(const String& filename);
		String getFilenamePath(const OS_CHAR * filename);
		String getFilenamePath(const OS_CHAR * filename, int len);

		String resolvePath(const String& filename);
		String resolvePath(const String& filename, const String& paths);
		virtual String resolvePath(const String& filename, const String& cur_path, const String& paths);
		virtual String getCompiledFilename(const String& resolved_filename);

		virtual void * openFile(const OS_CHAR * filename, const OS_CHAR * mode);
		virtual int readFile(void * buf, int size, void * f);
		virtual int writeFile(const void * buf, int size, void * f);
		virtual int seekFile(void * f, int offset, int whence);
		virtual void closeFile(void * f);

		virtual void printf(const OS_CHAR * fmt, ...);
	};

} // namespace OS

#endif // __OBJECT_SCRIPT_H__