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
#include <stdlib.h>

#if !defined __GNUC__ || defined IW_SDK
#include <new.h>
#elif defined(__GNUC__)
#include <new>
#else
inline void *operator new(size_t, void * p){ return p; }
inline void operator delete(void *, void *){}
#endif

#if defined _MSC_VER && !defined IW_SDK
#include <vadefs.h>
#endif

#if defined _DEBUG && !defined OS_RELEASE && !defined OS_DEBUG
#define OS_DEBUG
#endif

// select ObjectScript number type here
#ifndef OS_NUMBER
#define OS_NUMBER double
// #define OS_NUMBER float	// could be a bit faster
// #define OS_NUMBER int	// not recomended, math.random returns float value [0..1]
#endif // OS_NUMBER

#define OS_MATH_POW_OPERATOR(a, b) (OS_NUMBER)::pow((double)(a), (double)(b))
// #define OS_MATH_MOD_OPERATOR(a, b) (OS_NUMBER)((OS_INT)(a) % (OS_INT)(b))
#define OS_MATH_MOD_OPERATOR(a, b) (OS_NUMBER)((double)(a) - ::floor((double)(a)/(double)(b))*(double)(b))

#define OS_CHAR char
#define OS_TEXT(s) s

// does disable _G due to security reason ???
#if !defined OS_GLOBAL_VAR_ENABLED && !defined OS_GLOBAL_VAR_DISABLED
#define OS_GLOBAL_VAR_ENABLED
#endif

#define OS_FUNC_VAR_NAME OS_TEXT("_F")
#define OS_THIS_VAR_NAME OS_TEXT("this")
#define OS_ENV_VAR_NAME OS_TEXT("_E")
#define OS_GLOBALS_VAR_NAME OS_TEXT("_G")

#define OS_FLOAT double
#define OS_INT8 signed char
#define OS_BYTE unsigned char
#define OS_INT16 short
#define OS_U16 unsigned short

#if defined __GNUC__ 
#include <inttypes.h>

#define OS_INT int
#define OS_INT32 int32_t
#define OS_INT64 int64_t
#define OS_U32 uint32_t
#define OS_U64 uint64_t

#elif defined IW_SDK

#define OS_INT int
#define OS_INT32 int32
#define OS_INT64 int64
#define OS_U32 uint32
#define OS_U64 uint64

#else

#define OS_INT int
#define OS_INT32 __int32
#define OS_INT64 __int64
#define OS_U32 unsigned __int32
#define OS_U64 unsigned __int64

#endif

#define OS_MEMCMP ::memcmp
#define OS_MEMMOVE ::memmove
#define OS_MEMSET ::memset
#define OS_MEMCPY ::memcpy
#define OS_STRLEN ::strlen
#define OS_STRCMP ::strcmp
#define OS_STRNCMP ::strncmp
#define OS_STRCHR ::strchr
#define OS_STRSTR ::strstr

#define OS_VPRINTF ::vprintf
#define OS_PRINTF ::printf

#define OS_IS_SPACE(c) ((c) > OS_TEXT('\0') && (c) <= OS_TEXT(' '))
#define OS_IS_ALPHA ::isalpha
#define OS_IS_ALNUM(c) ((c) >= OS_TEXT('0') && (c) <= OS_TEXT('9'))
#define OS_IS_SLASH(c) ((c) == OS_TEXT('/') || (c) == OS_TEXT('\\'))

#define OS_CHAR_UPPER(c) toupper(c)
#define OS_CHAR_LOWER(c) tolower(c)

#define OS_AUTO_PRECISION 20
#define OS_TOP_STACK_NULL_VALUES 20
#define OS_DEF_FMT_BUF_LEN (1024*10)
#define OS_PATH_SEPARATOR OS_TEXT("/")

// uncomment it if need
// #define OS_INFINITE_LOOP_OPCODES 100000000

#define OS_CALL_STACK_MAX_SIZE 200

#define OS_VERSION OS_TEXT("0.99-vm4")
#define OS_COMPILED_HEADER OS_TEXT("OBJECTSCRIPT")
#define OS_DEBUGINFO_HEADER OS_TEXT("OBJECTSCRIPT.DEBUGINFO")
#define OS_EXT_SOURCECODE OS_TEXT(".os")
#define OS_EXT_TEMPLATE OS_TEXT(".osh")
#define OS_EXT_COMPILED OS_TEXT(".osc")
#define OS_EXT_DEBUG_INFO OS_TEXT(".osd")
#define OS_EXT_DEBUG_OPCODES OS_TEXT(".txt")

#define OS_MEMORY_MANAGER_PAGE_BLOCKS 32

#define OS_DEBUGGER_SAVE_NUM_LINES 11

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

#if defined _MSC_VER // && !defined IW_SDK
#define DEBUG_BREAK __debugbreak()
#elif !defined __GNUC__
#include <signal.h>
#define DEBUG_BREAK raise(SIGTRAP)
// #define DEBUG_BREAK __builtin_trap()
#else
#define DEBUG_BREAK 
#endif

#ifndef OS_PROFILE_BEGIN_OPCODE
#define OS_PROFILE_BEGIN_OPCODE(a)
#define OS_PROFILE_END_OPCODE(a)
#endif

#ifndef OS_PROFILE_BEGIN_GC
#define OS_PROFILE_BEGIN_GC
#define OS_PROFILE_END_GC
#endif

namespace ObjectScript
{
	class OS;

	typedef void (*OS_UserdataDtor)(OS*, void * data, void * user_param);
	typedef int (*OS_CFunction)(OS*, int params, int closure_values, int need_ret_values, void * user_param);

	enum OS_ESettings
	{
		OS_SETTING_CREATE_DEBUG_OPCODES,
		OS_SETTING_CREATE_DEBUG_EVAL_OPCODES,
		OS_SETTING_CREATE_DEBUG_INFO,
		OS_SETTING_CREATE_COMPILED_FILE,
		OS_SETTING_PRIMARY_COMPILED_FILE,
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

		// internal
		OS_VALUE_TYPE_WEAKREF,
		OS_VALUE_TYPE_UNKNOWN,
	};

	enum OS_ESourceCodeType
	{
		OS_SOURCECODE_AUTO,
		OS_SOURCECODE_PLAIN,
		OS_SOURCECODE_TEMPLATE
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
		OS_E_ERROR		= 1<<0,
		OS_E_WARNING	= 1<<1,
		OS_E_NOTICE		= 1<<2,
		// --------------------------
		OS_ERROR_LEVELS	= 3,
		OS_E_ALL		= (1<<OS_ERROR_LEVELS)-1
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

		OP_COMPARE, // <=>
		OP_ADD, // +
		OP_SUB, // -
		OP_MUL, // *
		OP_DIV, // /
		OP_MOD, // %
		OP_LSHIFT, // <<
		OP_RSHIFT, // >>
		OP_POW, // **

		OP_CONCAT,	// ..

		// unary operators

		OP_BIT_NOT,		// ~
		OP_PLUS,		// +
		OP_NEG,			// -
		OP_LENGTH,		// #
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

				virtual void writeFromStream(StreamReader*);

				virtual void writeBytes(const void*, int len) = 0;
				virtual void writeBytesAtPos(const void*, int len, int pos) = 0;

				virtual void writeByte(int);
				virtual void writeByteAtPos(int value, int pos);

				virtual void writeUVariable(int);

				virtual void writeU16(int);
				virtual void writeU16AtPos(int value, int pos);

				virtual void writeInt8(int);
				virtual void writeInt8AtPos(int value, int pos);

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

				void reserveCapacity(int new_capacity);

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

				virtual OS_INT8 readInt8();
				virtual OS_INT8 readInt8AtPos(int pos);

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
				// int pos;
				OS_BYTE * cur;

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

				OS_INT8 readInt8();
				OS_INT16 readInt16();
				OS_INT32 readInt32();
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
				String(OS*, const String&, const String&);
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

				OS_NUMBER toNumber() const;
			};

			class Buffer: public MemStreamWriter
			{
			protected:

				Core::GCStringValue * cacheStr;

			public:

				Buffer(OS*);
				Buffer(const Buffer&);
				~Buffer();

				Buffer& append(OS_CHAR);
				Buffer& append(const OS_CHAR*);
				Buffer& append(const OS_CHAR*, int len);
				Buffer& append(const void*, int size);
				Buffer& append(const Core::String&);
				Buffer& append(const Buffer&);

				Buffer& operator+=(const Core::String&);
				Buffer& operator+=(const OS_CHAR*);

				operator Core::String();
				Core::String toString();
				OS::String toStringOS();

				Core::GCStringValue * toGCStringValue();
				void freeCacheStr();
			};

			class File
			{
			protected:

				OS * os;
				void * f;

			public:

				File(OS*);
				virtual ~File();

				bool open(const OS_CHAR * filename, const OS_CHAR * mode = "rb");
				void close();

				bool isOpen() const;
				int getSize() const;
				int getPos() const;
				void setPos(int);

				String read();
				String read(int len);
				int write(const void * data, int len);
				int write(const Core::String&);
			};

			class Tokenizer
			{
			public:

				enum Error
				{
					ERROR_NOTHING,
					ERROR_MULTI_LINE_COMMENT, // multi line comment not end
					ERROR_CONST_STRING,             // string not end
					ERROR_CONST_STRING_ESCAPE_CHAR, // string escape error
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

					STRING,         // ["].*?["]
					OUTPUT_STRING,
					OUTPUT_NEXT_VALUE,

					NUMBER,      // -?[0..9][.]?[0..9]+(e[+-]?[0..9]+)?

					// [not real operators]
					OPERATOR,
					BINARY_OPERATOR,
					SEPARATOR,
					// [/not real operators]

					OPERATOR_INDIRECT,    // .
					OPERATOR_CONCAT,    // ..
					REST_ARGUMENTS,  // ...

					OPERATOR_THIS, // @

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
					OPERATOR_COMPARE,		// <=>
					OPERATOR_LOGIC_NOT,     // !

					OPERATOR_INC,     // ++
					OPERATOR_DEC,     // --

					OPERATOR_QUESTION,  // ?
					OPERATOR_COLON,     // :

					OPERATOR_IN,		// in
					OPERATOR_ISPROTOTYPEOF,		// is
					OPERATOR_IS,	// is
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
				bool parseLines(OS_ESourceCodeType source_code_type, bool check_utf8_bom);

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

				bool parseText(const OS_CHAR * text, int len, const String& filename, OS_ESourceCodeType source_code_type, bool check_utf8_bom);

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
					bool ascending;

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

				GCValue * gc_grey_next;
#ifdef OS_DEBUG
				int gc_time;
#endif

				OS_EValueType type;
				bool is_object_instance;
				bool is_destructor_called;

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

			struct GCArrayValue;

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

				static GCStringValue * alloc(OS*, const void *, int data_size OS_DBG_FILEPOS_DECL);
				static GCStringValue * alloc(OS*, const void * buf1, int len1, const void * buf2, int len2 OS_DBG_FILEPOS_DECL);
				static GCStringValue * alloc(OS*, GCStringValue * a, GCStringValue * b OS_DBG_FILEPOS_DECL);

				bool isNumber(OS_NUMBER*) const;
				OS_NUMBER toNumber() const;

				int cmp(GCStringValue*) const;
				int cmp(const OS_CHAR*) const;
				int cmp(const OS_CHAR*, int len) const;

				bool isEqual(int hash, const void * b, int size) const;
				bool isEqual(int hash, const void * buf1, int size1, const void * buf2, int size2) const;

				void calcHash();
			};

			struct GCUserdataValue: public GCObjectValue
			{
				int crc;
				void * ptr;
				OS_UserdataDtor dtor;
				void * user_param;
			};

			struct GCCFunctionValue: public GCValue
			{
				GCStringValue * name;
				OS_CFunction func;
				void * user_param;
				int num_closure_values;
			};

			struct GCFunctionValue;

			struct WeakRef { WeakRef(){} };

/* Microsoft compiler on a Pentium (32 bit) ? */
#if defined(_MSC_VER) && defined(_M_IX86)

#define OS_NUMBER_IEEEENDIAN	0
#define OS_NUMBER_NAN_TRICK

/* pentium 32 bits? */
#elif defined(__i386__) || defined(__i386) || defined(__X86__)

#define OS_NUMBER_IEEEENDIAN	1
#define OS_NUMBER_NAN_TRICK

#elif defined(__x86_64)

#define OS_NUMBER_IEEEENDIAN	0

#elif defined(__POWERPC__) || defined(__ppc__)

#define OS_NUMBER_IEEEENDIAN	1

#else

#endif // OS_NUMBER_IEEEENDIAN & OS_NUMBER_NAN_TRICK

			union ValueUnion
			{
				int boolean;
				int value_id;
				GCValue * value;
				GCObjectValue * object;
				GCArrayValue * arr;
				GCStringValue * string;
				GCUserdataValue * userdata;
				GCFunctionValue * func;
				GCCFunctionValue * cfunc;
#ifndef OS_NUMBER_NAN_TRICK
				OS_NUMBER number;
#endif
			};

			struct Value
			{
#ifndef OS_NUMBER_NAN_TRICK
				struct {
					ValueUnion v; 
					int type;
				} u;

#define OS_VALUE_VARIANT(a)	(a).u.v
#define OS_VALUE_NUMBER(a)	(a).u.v.number
#define OS_VALUE_TAGGED_TYPE(a)	(a).u.type
#define OS_VALUE_TYPE(a)	OS_VALUE_TAGGED_TYPE(a)

#define OS_IS_VALUE_NUMBER(a)	(OS_VALUE_TYPE(a) == OS_VALUE_TYPE_NUMBER)
#define OS_MAKE_VALUE_TAGGED_TYPE(t)	(t)

#define OS_SET_VALUE_NUMBER(a, n)	((OS_VALUE_NUMBER(a) = (OS_NUMBER)(n)), OS_SET_VALUE_TYPE(a, OS_VALUE_TYPE_NUMBER))
#define OS_SET_VALUE_TYPE(a, t)		(OS_VALUE_TAGGED_TYPE(a) = OS_MAKE_VALUE_TAGGED_TYPE(t))
#define OS_SET_VALUE_NULL(a) (OS_VALUE_VARIANT(a).value = NULL, OS_SET_VALUE_TYPE((a), OS_VALUE_TYPE_NULL))
#define OS_SET_NULL_VALUES(a, c) do{ Value * v = a; for(int count = c; count > 0; --count, ++v) OS_SET_VALUE_NULL(*v); }while(false)

#elif !defined(OS_NUMBER_IEEEENDIAN)
#error option 'OS_NUMBER_NAN_TRICK' needs 'OS_NUMBER_IEEEENDIAN'
#else

#define OS_NUMBER_NAN_MASK	0x7FFFFF00
#define OS_NUMBER_NAN_MARK	0x7FF7A500

#define OS_VALUE_VARIANT(a)	(a).u.i.v
#define OS_VALUE_NUMBER(a)	(a).u.number
#define OS_VALUE_TAGGED_TYPE(a)	(a).u.i.type
#define OS_VALUE_TYPE(a)	(OS_IS_VALUE_NUMBER(a) ? OS_VALUE_TYPE_NUMBER : OS_VALUE_TAGGED_TYPE(a) & 0xff)

#define OS_IS_VALUE_NUMBER(a)	((OS_VALUE_TAGGED_TYPE(a) & OS_NUMBER_NAN_MASK) != OS_NUMBER_NAN_MARK)
#define OS_MAKE_VALUE_TAGGED_TYPE(t)	((t) | OS_NUMBER_NAN_MARK)

#define OS_SET_VALUE_NUMBER(a, n)	(OS_VALUE_NUMBER(a) = (OS_NUMBER)(n))
// #define OS_SET_VALUE_OBJECT(a, v)	(OS_VALUE_VARIANT(a) = (v))
#define OS_SET_VALUE_TYPE(a, t)		(OS_VALUE_TAGGED_TYPE(a) = OS_MAKE_VALUE_TAGGED_TYPE(t))
#define OS_SET_VALUE_NULL(a) (OS_VALUE_VARIANT(a).value = NULL, OS_SET_VALUE_TYPE((a), OS_VALUE_TYPE_NULL))
#define OS_SET_NULL_VALUES(a, c) do{ Value * v = a; for(int count = c; count > 0; --count, ++v) OS_SET_VALUE_NULL(*v); }while(false)

#if OS_NUMBER_IEEEENDIAN == 0
				union {
					struct { ValueUnion v; int type; } i;
					OS_NUMBER number;
				} u;
#else
				union {
					struct { int type; ValueUnion v; } i;
					OS_NUMBER number;
				} u;
#endif
#endif // OS_NUMBER_NAN_TRICK

				Value();
				Value(bool);
				Value(OS_INT32);
				Value(OS_INT64);
				Value(float);
				Value(double);
				Value(GCValue*);
				Value(const String&);
				Value(int, const WeakRef&);

				Value& operator=(GCValue*);
				Value& operator=(bool);
				Value& operator=(OS_INT32);
				Value& operator=(OS_INT64);
				Value& operator=(float);
				Value& operator=(double);

#ifdef OS_NUMBER_NAN_TRICK
				// Value& operator=(const Value& b){ OS_SET_VALUE_NUMBER(*this, OS_VALUE_NUMBER(b)); return *this;  }
#endif
				
				void clear();

				GCValue * getGCValue() const;

				bool isNull() const;
				bool isFunction() const;
				bool isUserdata() const;
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

			struct GCArrayValue: public GCValue
			{
				Vector<Value> values;
			};

			class Program;
			struct FunctionDecl;
			struct Locals;
			struct GCFunctionValue: public GCValue
			{
				Program * prog; // retained
				FunctionDecl * func_decl;
				Value env;
				Locals * locals; // retained
				GCStringValue * name;

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
				PropertyIndex(const Value& index);
				PropertyIndex(const Value& index, const KeepStringIndex&);
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

			enum {
				OP_MULTI_GET_ARGUMENTS,
				OP_MULTI_GET_REST_ARGUMENTS,
				OP_MULTI_SUPER,
				OP_MULTI_DEBUGGER,
			};

			enum OpcodeType
			{
				OP_NOP,
				OP_NEW_FUNCTION,
				OP_NEW_ARRAY,
				OP_NEW_OBJECT,
				OP_RETURN,
				OP_JUMP,
				OP_MULTI,
				OP_MOVE,
				OP_MOVE2,
				OP_GET_XCONST,

				OP_SUPER_CALL,
				OP_CALL,
				OP_CALL_METHOD,

#ifdef OS_TAIL_CALL_ENABLED
				OP_TAIL_CALL,
				OP_TAIL_CALL_METHOD,
#endif

				OP_GET_PROPERTY,
				OP_SET_PROPERTY,

				OP_GET_UPVALUE,
				OP_SET_UPVALUE,

				OP_LOGIC_PTR_EQ,
				OP_LOGIC_EQ,
				OP_LOGIC_GREATER,
				OP_LOGIC_GE,
				OP_LOGIC_BOOL,

				OP_BIT_AND,
				OP_BIT_OR,
				OP_BIT_XOR,

				OP_COMPARE, // <=>
				OP_ADD, // +
				OP_SUB, // -
				OP_MUL, // *
				OP_DIV, // /
				OP_MOD, // %
				OP_LSHIFT, // <<
				OP_RSHIFT, // >>
				OP_POW, // **

				// OP_CONCAT,	// ..

				OP_BIT_NOT,
				OP_PLUS,
				OP_NEG,

				OPCODE_COUNT
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
					EXP_TYPE_POP_VALUE,
					EXP_TYPE_SUPER_CALL,
					EXP_TYPE_CALL,
					EXP_TYPE_CALL_AUTO_PARAM,
					EXP_TYPE_CALL_DIM, // temp
					EXP_TYPE_VALUE,
					EXP_TYPE_PARAMS,
					EXP_TYPE_FUNCTION,
					EXP_TYPE_EXTENDS,
					EXP_TYPE_DELETE,
					EXP_TYPE_RETURN,
					EXP_TYPE_BREAK,
					EXP_TYPE_CONTINUE,
					EXP_TYPE_DEBUGGER,
					EXP_TYPE_DEBUG_LOCALS,
					
					EXP_TYPE_IF,
					EXP_TYPE_QUESTION,

					EXP_TYPE_ARRAY,

					EXP_TYPE_OBJECT,
					EXP_TYPE_OBJECT_SET_BY_NAME,
					EXP_TYPE_OBJECT_SET_BY_INDEX,
					EXP_TYPE_OBJECT_SET_BY_EXP,
					EXP_TYPE_OBJECT_SET_BY_AUTO_INDEX,

					EXP_TYPE_SUPER,

					EXP_TYPE_GET_THIS,
					EXP_TYPE_GET_ARGUMENTS,
					EXP_TYPE_GET_REST_ARGUMENTS,

					EXP_TYPE_GET_LOCAL_VAR,
					EXP_TYPE_GET_LOCAL_VAR_AUTO_CREATE,
					EXP_TYPE_SET_LOCAL_VAR,
					
					EXP_TYPE_SET_LOCAL_VAR_BY_BIN_OPERATOR_LOCALS,
					EXP_TYPE_SET_LOCAL_VAR_BY_BIN_OPERATOR_LOCAL_AND_NUMBER,

					EXP_TYPE_GET_ENV_VAR,
					EXP_TYPE_GET_ENV_VAR_AUTO_CREATE,
					EXP_TYPE_SET_ENV_VAR,

					EXP_TYPE_INDIRECT, // temp

					EXP_TYPE_GET_PROPERTY,
					EXP_TYPE_GET_PROPERTY_AUTO_CREATE,
					EXP_TYPE_SET_PROPERTY,

					EXP_TYPE_GET_THIS_PROPERTY_BY_STRING,

					EXP_TYPE_GET_PROPERTY_BY_LOCALS,
					EXP_TYPE_GET_PROPERTY_BY_LOCAL_AND_NUMBER,
					EXP_TYPE_SET_PROPERTY_BY_LOCALS_AUTO_CREATE,

					EXP_TYPE_GET_SET_PROPERTY_BY_LOCALS_AUTO_CREATE,
					
					EXP_TYPE_SET_DIM,

					EXP_TYPE_CALL_METHOD,

					EXP_TYPE_TAIL_CALL,
					EXP_TYPE_TAIL_CALL_METHOD,

					EXP_TYPE_CONST_NULL,
					EXP_TYPE_CONST_NUMBER,
					EXP_TYPE_CONST_STRING,
					EXP_TYPE_CONST_TRUE,
					EXP_TYPE_CONST_FALSE,

					EXP_TYPE_LOGIC_BOOL,    // !!
					EXP_TYPE_LOGIC_NOT,     // !
					EXP_TYPE_BIT_NOT,		// ~
					EXP_TYPE_PLUS,			// +
					EXP_TYPE_NEG,			// -
					EXP_TYPE_LENGTH,		// #
					EXP_TYPE_IN,			// in
					EXP_TYPE_ISPROTOTYPEOF,		// is
					EXP_TYPE_IS,	// is

					EXP_TYPE_BIN_OPERATOR_BY_LOCALS,
					EXP_TYPE_BIN_OPERATOR_BY_LOCAL_AND_NUMBER,

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

					EXP_TYPE_PRE_INC,     // ++
					EXP_TYPE_PRE_DEC,     // --

					EXP_TYPE_POST_INC,    // ++
					EXP_TYPE_POST_DEC,    // --

					EXP_TYPE_BIT_AND, // &
					EXP_TYPE_BIT_OR,  // |
					EXP_TYPE_BIT_XOR, // ^

					EXP_TYPE_BIT_AND_ASSIGN, // &=
					EXP_TYPE_BIT_OR_ASSIGN,  // |=
					EXP_TYPE_BIT_XOR_ASSIGN, // ^=
					EXP_TYPE_BIT_NOT_ASSIGN, // ~=

					EXP_TYPE_COMPARE, // <=>
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

					EXP_TYPE_GET_UPVALUE,
					EXP_TYPE_SET_UPVALUE,

					EXP_TYPE_MOVE,
					EXP_TYPE_GET_XCONST,
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

					Expression * add(Expression* OS_DBG_FILEPOS_DECL);
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

					LocalVarDesc();
				};

				struct Scope;
				struct Expression
				{
					TokenData * token;
					ExpressionList list;
					LocalVarDesc local_var;
					OS_U16 active_locals;
					OS_U16 ret_values;
					struct {
						OS_INT16 a, b, c;
					} slots;
					ExpressionType type;
					
					Expression(ExpressionType type, TokenData*);
					Expression(ExpressionType type, TokenData*, Expression * e1 OS_DBG_FILEPOS_DECL);
					Expression(ExpressionType type, TokenData*, Expression * e1, Expression * e2 OS_DBG_FILEPOS_DECL);
					Expression(ExpressionType type, TokenData*, Expression * e1, Expression * e2, Expression * e3 OS_DBG_FILEPOS_DECL);
					virtual ~Expression();

					OS * getAllocator(){ return list.allocator; }

					OS_NUMBER toNumber();
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

					String getSlotStr(Compiler * compiler, Scope * scope, int slot_num, int up_count = 0);
					void debugPrint(Buffer&, Compiler * compiler, Scope * scope, int depth);
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

					int stack_size;
					int stack_cur_size;

					Vector<LoopBreak> loop_breaks;

					bool parser_started;

					Scope(Scope * parent, ExpressionType, TokenData*);
					virtual ~Scope();

					bool addLoopBreak(int pos, ELoopBreakType);
					void fixLoopBreaks(Compiler*, int scope_start_pos, int scope_end_pos);

					void addPreVars();
					void addPostVars();
					void addLocalVar(const String& name);
					void addLocalVar(const String& name, LocalVarDesc&);

					int allocTempVar();
					void popTempVar(int count = 1);
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
					ERROR_EXPECT_GET_OR_SET,
					ERROR_EXPECT_EXPRESSION,
					ERROR_EXPECT_FUNCTION_SCOPE,
					ERROR_EXPECT_CODE_SEP_BEFORE_NESTED_BLOCK,
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
					OP_LEVEL_13, // ** in as is
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
				Table * prog_numbers_table;
				Table * prog_strings_table;
				Table * prog_debug_strings_table;
				Vector<OS_NUMBER> prog_numbers;
				Vector<String> prog_strings;
				Vector<String> prog_debug_strings;
				Vector<Scope*> prog_functions;
				Vector<OS_U32> prog_opcodes;
				MemStreamWriter * prog_debug_info;
				int prog_num_debug_infos;
				int prog_max_up_count;

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
				TokenData * getPrevToken();
				TokenData * expectToken(TokenType);
				TokenData * expectToken();

				struct Params
				{
					bool allow_root_blocks;
					bool allow_var_decl;
					bool allow_inline_nested_block;
					bool allow_binary_operator;
					bool allow_in_operator;
					bool allow_assing;
					bool allow_params;
					bool allow_auto_call;
					bool allow_call;
					bool allow_nop_result;

					Params();
					Params(const Params&);

					Params& setAllowRootBlocks(bool);
					Params& setAllowVarDecl(bool);
					Params& setAllowInlineNestedBlock(bool);
					Params& setAllowBinaryOperator(bool);
					Params& setAllowInOperator(bool);
					Params& setAllowAssign(bool);
					Params& setAllowParams(bool);
					Params& setAllowAutoCall(bool);
					Params& setAllowCall(bool);
					Params& setAllowNopResult(bool);
				};

				Expression * expectSingleExpression(Scope*, const Params& p);
				Expression * expectSingleExpression(Scope*, bool allow_nop_result = false, bool allow_inline_nested_block = false);

				Expression * expectExpressionValues(Expression * exp, int ret_values, bool auto_no_values = false);
				Expression * newExpressionFromList(ExpressionList& list, int ret_values, bool auto_no_values = false);
				Expression * newAssingExpression(Scope * scope, Expression * var_exp, Expression * value_exp);
				Expression * newSingleValueExpression(Expression * exp);
				
				Expression * postCompileExpression(Scope * scope, Expression * exp);
				Expression * postCompilePass2(Scope * scope, Expression * exp);
				Expression * postCompilePass3(Scope * scope, Expression * exp);
				Expression * postCompileNewVM(Scope * scope, Expression * exp);

				bool isVarNameValid(const String& name);

				Scope * expectTextExpression();
				Scope * expectCodeExpression(Scope*);
				Expression * expectFunctionExpression(Scope*);
				Expression * expectFunctionSugarExpression(Scope*);
				Expression * expectExtendsExpression(Scope*);
				Expression * expectDeleteExpression(Scope*);
				Expression * expectVarExpression(Scope*);
				Expression * expectObjectOrFunctionExpression(Scope*, const Params& p, bool allow_finish_exp = true);
				Expression * expectArrayExpression(Scope*, const Params& p);
				Expression * expectParamsExpression(Scope*);
				Expression * expectReturnExpression(Scope*);
				Expression * expectIfExpression(Scope*);
				Expression * expectForExpression(Scope*);
				Expression * expectDebugLocalsExpression(Scope*);
				Expression * expectBracketExpression(Scope*, const Params& p);
				Expression * finishValueExpression(Scope*, Expression*, const Params& p);
				Expression * finishValueExpressionNoAutoCall(Scope*, Expression*, const Params& p);
				Expression * finishValueExpressionNoNextCall(Scope*, Expression*, const Params& p);
				Expression * finishBinaryOperator(Scope * scope, OpcodeLevel prev_level, Expression * exp, const Params& p, bool& is_finished); // bool allow_param, bool& is_finished);
				Expression * finishQuestionOperator(Scope*, TokenData * token, Expression * left_exp, Expression * right_exp);
				Expression * newBinaryExpression(Scope * scope, ExpressionType, TokenData*, Expression * left_exp, Expression * right_exp);

				bool findLocalVar(LocalVarDesc&, Scope * scope, const String& name, int active_locals, bool all_scopes);

				void debugPrintSourceLine(Buffer& out, TokenData*);
				static const OS_CHAR * getExpName(ExpressionType);

				int cacheString(Table * strings_table, Vector<String>& strings, const String& str);
				int cacheString(const String& str);
				int cacheDebugString(const String& str);
				int cacheNumber(OS_NUMBER);

				void writeJumpOpcode(int offs);
				void fixJumpOpcode(int offs, int pos);

				int getOpcodePos();
				int writeOpcode(OS_U32 opcode);
				int writeOpcode(OpcodeType opcode);
				int writeOpcodeABC(OpcodeType opcode, int a, int b = 0, int c = 0);
				int writeOpcodeABx(OpcodeType opcode, int a, int b);
				void writeOpcodeAt(OS_U32 opcode, int pos);

				bool writeOpcodes(Scope*, Expression*);
				bool writeOpcodes(Scope*, ExpressionList&, bool optimization_enabled = false);

				void writeJumpOpcodeOld(int offs);
				void fixJumpOpcodeOld(StreamWriter * writer, int offs, int pos);
				void fixJumpOpcodeOld(StreamWriter * writer, int offs, int pos, int opcode);

				bool writeOpcodesOld(Scope*, Expression*);
				bool writeOpcodesOld(Scope*, ExpressionList&);
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

					LocalVar(const String&);
					~LocalVar();
				};

#ifdef OS_DEBUG
				int prog_func_index;
#endif
				int prog_parent_func_index;
				LocalVar * locals;
				int stack_size;
				int num_locals;
				int num_params; // this included
				int max_up_count;
				int func_depth;
				int func_index; // in parent space
				int num_local_funcs;
				int opcodes_pos;
				int opcodes_size;

				FunctionDecl(); // Program*);
				~FunctionDecl();
			};

			class Program
			{
			protected:

				int ref_count;

				virtual ~Program();

			public:

				OS * allocator;
				String filename;

				int num_strings;
				int num_numbers;

				Value * const_values;

				FunctionDecl * functions;
				int num_functions;

				Vector<OS_U32> opcodes;

				struct DebugInfoItem
				{
					int opcode_pos;
					int line;
					int pos;
					String token;

					DebugInfoItem(int opcode_pos, int line, int pos, const String&);
				};
				Vector<DebugInfoItem> debug_info;

				Program(OS * allocator);

				Program * retain();
				void release();

				static OpcodeType getOpcodeType(Compiler::ExpressionType);

				bool loadFromStream(StreamReader * reader, StreamReader * debuginfo_reader);
				DebugInfoItem * getDebugInfo(int opcode_pos);

				void pushStartFunction();
			};

			enum {
				PRE_VAR_FUNC,
				PRE_VAR_THIS,
				// -----------------
				PRE_VARS
			};

			enum {
				POST_VAR_ENV,
#ifdef OS_GLOBAL_VAR_ENABLED
				POST_VAR_GLOBALS,
#endif
			};

			enum {
				CONST_NULL,
				CONST_TRUE,
				CONST_FALSE,
				// -----------------
				CONST_STD_VALUES
			};

			struct Locals
			{
				int ref_count;
				int gc_time;

				Program * prog; // retained
				FunctionDecl * func_decl;

				Value * values;
				bool is_stack_locals;
				
				int num_parents;

				Locals ** getParents();
				Locals * getParent(int i);
				void setParent(int i, Locals*);

				Locals * retain();
			};

			struct StackFunction
			{
				GCFunctionValue * func;
				GCValue * self_for_proto;

				Locals * locals;
				int num_params; // func + this + params

				GCArrayValue * arguments;
				GCArrayValue * rest_arguments;
				
				int caller_stack_size;
				int locals_stack_pos;
				
				int need_ret_values;
				OS_U32 * opcodes;
			};

			struct StringRef
			{
				int string_hash;
				int string_value_id;
				StringRef * hash_next;
			};

			struct StringRefs
			{
				StringRef ** heads;
				int head_mask;
				int count;

				StringRefs();
				~StringRefs();
			};

			struct UserptrRef
			{
				int userptr_hash;
				int userptr_value_id;
				UserptrRef * hash_next;
			};

			struct UserptrRefs
			{
				UserptrRef ** heads;
				int head_mask;
				int count;

				UserptrRefs();
				~UserptrRefs();
			};

			struct Values
			{
				GCValue ** heads;
				int head_mask;
				int count;

				int next_id;

				Values();
				~Values();

				GCValue * get(int value_id);
			};

			OS * allocator;

			struct Strings
			{
				String __construct;
				String __destruct;
				String __object;
				String __get;
				String __set;
				String __isset;
				String __getAt;
				String __setAt;
				String __issetAt;
				String __del;
				String __delAt;
				String __getempty;
				String __setempty;
				String __delempty;
				String __getdim;
				String __setdim;
				String __deldim;
				String __cmp;
				String __iter;
				// String __concat;
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
				
				String func_extends;
				String func_delete;
				String func_in;
				String func_is;
				String func_isprototypeof;
				String func_push;
				String func_valueOf;
				String func_clone;
				String func_concat;
				String func_echo;
				String func_require;

				String typeof_null;
				String typeof_boolean;
				String typeof_number;
				String typeof_string;
				String typeof_object;
				String typeof_array;
				String typeof_userdata;
				String typeof_function;

				String syntax_get;
				String syntax_set;
				String syntax_super;
				String syntax_is;
				String syntax_isprototypeof;
				String syntax_extends;
				String syntax_delete;
				String syntax_prototype;
				String syntax_var;
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
				String syntax_try;
				String syntax_catch;
				String syntax_finally;
				String syntax_throw;
				String syntax_new;
				String syntax_while;
				String syntax_do;
				String syntax_const;
				String syntax_public;
				String syntax_protected;
				String syntax_private;
				String syntax_yield;
				String syntax_static;
				String syntax_debugger;
				String syntax_debuglocals;

#ifdef OS_GLOBAL_VAR_ENABLED
				String var_globals;
#endif
				String var_func;
				String var_this;
				String var_env;
				String var_temp_prefix;

				int __dummy__;

				Strings(OS * allocator);
			} * strings;

			Values values;
			int num_created_values;
			int num_destroyed_values;

			StringRefs string_refs;
			UserptrRefs userptr_refs;

			GCObjectValue * check_recursion;
			Value global_vars;
			Value user_pool;
			Value error_handlers[OS_ERROR_LEVELS];

			enum {
				PROTOTYPE_BOOL,
				PROTOTYPE_NUMBER,
				PROTOTYPE_STRING,
				PROTOTYPE_OBJECT,
				PROTOTYPE_ARRAY,
				PROTOTYPE_FUNCTION,
				PROTOTYPE_USERDATA,
				// -----------------
				PROTOTYPE_COUNT
			};

			GCObjectValue * prototypes[PROTOTYPE_COUNT];

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
			StackFunction * stack_func;
			Value * stack_func_locals;
			int stack_func_env_index;
			Value * stack_func_prog_values;

			GCValue * gc_grey_list_first;
			bool gc_grey_root_initialized;
			int gc_values_head_index;
			int gc_start_allocated_bytes;
			int gc_max_allocated_bytes;
			int gc_keep_heap_count;
			int gc_continuous_count;
			bool gc_continuous;
			int gc_time;
			bool gc_in_process;
			int gc_grey_added_count;
			
			float gc_start_values_mult;
			float gc_step_size_mult;
			float gc_step_size_auto_mult;
			int gc_start_next_values;
			int gc_step_size;

			struct {
				bool create_debug_opcodes;
				bool create_debug_eval_opcodes;
				bool create_debug_info;
				bool create_compiled_file;
				bool primary_compiled_file;
			} settings;

			enum {
				RAND_STATE_SIZE = 624
			};

			OS_U32 rand_state[RAND_STATE_SIZE+1];
			OS_U32 rand_seed;
			OS_U32 * rand_next;
			int rand_left;

			bool terminated;
			int terminated_code;

			void randInitialize(OS_U32 seed);
			void randReload();
			double getRand();
			double getRand(double up);
			double getRand(double min, double max);

			void * malloc(int size OS_DBG_FILEPOS_DECL);
			void free(void * p);

			void error(int code, const OS_CHAR * message);
			void error(int code, const String& message);
			void errorDivisionByZero();

			void gcInitGreyList();
			void gcResetGreyList();
			void gcAddToGreyList(GCValue*);
			void gcAddToGreyList(const Value&);
			void gcRemoveFromGreyList(GCValue*);
			void gcMarkProgram(Program * prog);
			void gcMarkTable(Table * table);
			void gcMarkLocals(Locals*);
			void gcMarkStackFunction(StackFunction*);
			void gcMarkList(int step_size);
			void gcMarkValue(GCValue * value);
			
			// return next gc phase
			int gcStep();
			void gcStepIfNeeded();
			void gcFinishSweepPhase();
			void gcFinishMarkPhase();
			void gcFull();

			void triggerValueDestructor(GCValue*);
			void clearValue(GCValue*);
			void deleteValue(GCValue*);

#ifdef OS_DEBUG
			bool isValueUsed(GCValue*);
#endif

			GCFunctionValue * newFunctionValue(StackFunction*, Program*, FunctionDecl*, Value env);
			void clearFunctionValue(GCFunctionValue*);

			void releaseLocals(Locals*);
			void deleteLocals(Locals*);
			void clearStackFunction(StackFunction*);

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
			GCUserdataValue * newUserdataValue(int crc, int data_size, OS_UserdataDtor dtor, void * user_param);
			GCUserdataValue * newUserPointerValue(int crc, void * data, OS_UserdataDtor dtor, void * user_param);
			GCObjectValue * newObjectValue();
			GCObjectValue * newObjectValue(GCValue * prototype);
			GCArrayValue * newArrayValue(int initial_capacity = 0);

			GCObjectValue * initObjectInstance(GCObjectValue*);

			template<class T> T * pushValue(T * val){ pushValue(Value(val)); return val; }

			void pushValue(const Value& val);
			void pushStackValue(int offs);
			void copyValue(int raw_from, int raw_to);
			void insertValue(Value val, int offs);
			void pushNull();
			void pushBool(bool);
			void pushNumber(OS_INT32);
			void pushNumber(OS_INT64);
			void pushNumber(float);
			void pushNumber(double);
			
			GCStringValue * pushStringValue(const String&);
			GCStringValue * pushStringValue(const OS_CHAR*);
			GCStringValue * pushStringValue(const OS_CHAR*, int len);
			GCCFunctionValue * pushCFunctionValue(OS_CFunction func, void * user_param);
			GCCFunctionValue * pushCFunctionValue(OS_CFunction func, int closure_values, void * user_param);
			GCUserdataValue * pushUserdataValue(int crc, int data_size, OS_UserdataDtor dtor, void * user_param);
			GCUserdataValue * pushUserPointerValue(int crc, void * data, OS_UserdataDtor dtor, void * user_param);
			GCObjectValue * pushObjectValue();
			GCObjectValue * pushObjectValue(GCValue * prototype);
			GCArrayValue * pushArrayValue(int initial_capacity = 0);

			void pushTypeOf(const Value& val);
			bool pushNumberOf(const Value& val);
			bool pushStringOf(const Value& val);
			bool pushValueOf(Value val);
			GCArrayValue * pushArrayOf(const Value& val);
			GCObjectValue * pushObjectOf(const Value& val);
			GCUserdataValue * pushUserdataOf(const Value& val);
			bool pushFunctionOf(const Value& val);

			void pushCloneValue(Value val);

			// unary operator
			void pushOpResultValue(OpcodeType opcode, const Value& value);

			// binary operator
			void pushOpResultValue(OpcodeType opcode, const Value& left_value, const Value& right_value);
			bool isEqualExactly(const Value& left_value, const Value& right_value);

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

			int syncRetValues(int need_ret_values, int cur_ret_values);

			void registerStringRef(StringRef*);
			void unregisterStringRef(StringRef*);
			void deleteStringRefs();

			void registerUserptrRef(UserptrRef*);
			void unregisterUserptrRef(UserptrRef*);
			void unregisterUserptrRef(void*, int);
			void deleteUserptrRefs();

			void registerValue(GCValue * val);
			GCValue * unregisterValue(int value_id);
			void deleteValues(bool del_ref_counted_also);
			static int compareGCValues(const void * a, const void * b);

			bool valueToBool(const Value& val);
			OS_INT valueToInt(const Value& val, bool valueof_enabled = false);
			OS_NUMBER valueToNumber(const Value& val, bool valueof_enabled = false);
			String valueToString(const Value& val, bool valueof_enabled = false);
			OS::String valueToStringOS(const Value& val, bool valueof_enabled = false);

			bool isValueNumber(const Value& val, OS_NUMBER * out = NULL);
			bool isValueString(const Value& val, String * out = NULL);
			bool isValueStringOS(const Value& val, OS::String * out = NULL);
			bool isValueInstanceOf(GCValue * val, GCValue * prototype_val);
			bool isValueInstanceOf(const Value& val, const Value& prototype_val);
			bool isValuePrototypeOf(GCValue * val, GCValue * prototype_val);
			bool isValuePrototypeOfUserdata(GCValue * val, int prototype_crc);
			bool isValuePrototypeOf(const Value& val, const Value& prototype_val);

			Table * newTable(OS_DBG_FILEPOS_START_DECL);
			void clearTable(Table*);
			void deleteTable(Table*);
			void addTableProperty(Table * table, Property * prop);
			Property * removeTableProperty(Table * table, const PropertyIndex& index);
			void changePropertyIndex(Table * table, Property * prop, const PropertyIndex& new_index);
			bool deleteTableProperty(Table * table, const PropertyIndex& index);
			void deleteValueProperty(GCValue * table_value, const PropertyIndex& index, bool del_enabled, bool prototype_enabled);
			void deleteValueProperty(const Value& table_value, const PropertyIndex& index, bool del_enabled, bool prototype_enabled);
			
			void copyTableProperties(Table * dst, Table * src);
			void copyTableProperties(GCValue * dst_value, GCValue * src_value, bool setter_enabled);

			void sortTable(Table * table, int(*comp)(OS*, const void*, const void*, void*), void* = NULL, bool reorder_keys = false);
			void sortArray(GCArrayValue * arr, int(*comp)(OS*, const void*, const void*, void*), void* = NULL);

			static int comparePropValues(OS*, const void*, const void*, void*);
			static int comparePropValuesReverse(OS*, const void*, const void*, void*);
			static int compareObjectProperties(OS*, const void*, const void*, void*);
			static int compareObjectPropertiesReverse(OS*, const void*, const void*, void*);
			static int compareUserPropValues(OS*, const void*, const void*, void*);
			static int compareUserPropValuesReverse(OS*, const void*, const void*, void*);
			
			static int comparePropKeys(OS*, const void*, const void*, void*);
			static int comparePropKeysReverse(OS*, const void*, const void*, void*);
			static int compareUserPropKeys(OS*, const void*, const void*, void*);
			static int compareUserPropKeysReverse(OS*, const void*, const void*, void*);

			static int compareArrayValues(OS*, const void*, const void*, void*);
			static int compareArrayValuesReverse(OS*, const void*, const void*, void*);
			static int compareUserArrayValues(OS*, const void*, const void*, void*);
			static int compareUserArrayValuesReverse(OS*, const void*, const void*, void*);

			static int compareUserReverse(OS*, const void*, const void*, void*);

			bool hasSpecialPrefix(const Value&);

			Property * setTableValue(Table * table, const PropertyIndex& index, Value val);
			void setPropertyValue(GCValue * table_value, const PropertyIndex& index, Value val, bool setter_enabled);
			void setPropertyValue(const Value& table_value, const PropertyIndex& index, const Value& val, bool setter_enabled);

			bool getPropertyValue(Value& result, Table * table, const PropertyIndex& index);
			bool getPropertyValue(Value& result, GCValue * table_value, const PropertyIndex& index, bool prototype_enabled);
			bool getPropertyValue(Value& result, const Value& table_value, const PropertyIndex& index, bool prototype_enabled);

			bool hasProperty(GCValue * table_value, const PropertyIndex& index, bool getter_enabled, bool prototype_enabled);
			void pushPropertyValue(GCValue * table_value, const PropertyIndex& index, bool getter_enabled, bool prototype_enabled);
			void pushPropertyValueForPrimitive(Value self, const PropertyIndex& index, bool getter_enabled, bool prototype_enabled);
			void pushPropertyValue(Value table_value, const PropertyIndex& index, bool getter_enabled, bool prototype_enabled);

			void setPrototype(const Value& val, const Value& proto, int userdata_crc);
			void pushPrototype(const Value& val);

			void pushBackTrace(int skip_funcs, int max_trace_funcs);
			void pushArguments(StackFunction*);
			void pushArgumentsWithNames(StackFunction*);
			void pushRestArguments(StackFunction*);

			int execute();
			void reloadStackFunctionCache();

			int call(int params, int ret_values, GCValue * self_for_proto = NULL, bool allow_only_enter_func = false);
			int call(int start_pos, int call_params, int ret_values, GCValue * self_for_proto = NULL, bool allow_only_enter_func = false);

			Core(OS*);
			~Core();

			bool init();
			void shutdown();
		};

		MemoryManager * memory_manager;
		Core * core;
		int ref_count;

#ifdef OS_DEBUG
		int native_stack_start_mark;
		int native_stack_max_usage;

		void checkNativeStackUsage(const OS_CHAR * func_name);
#endif

		virtual ~OS();

		virtual void shutdown();

		void qsort(void *base, unsigned num, unsigned width, int (*comp)(OS*, const void *, const void *, void*), void*);

		void initGlobalFunctions();
		void initObjectClass();
		void initArrayClass();
		void initFunctionClass();
		void initStringClass();
		void initBufferClass();
		void initFileClass();
		void initMathModule();
		void initGCModule();
		void initLangTokenizerModule();
		virtual void initPreScript();
		virtual void initPostScript();

		template<class Core> friend struct UserDataDestructor;

	public:

		typedef Core::Buffer CoreBuffer;
		typedef Core::File CoreFile;

		class String: public Core::String // this string retains OS
		{
			typedef Core::String super;
			friend class Core;
			friend class Buffer;

		protected:

			OS * allocator;
			String(OS*, Core::GCStringValue*);

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

		template <class T>
		static T * create(T * os, MemoryManager * manager = NULL)
		{
			OS_ASSERT(dynamic_cast<OS*>(os));
			return (T*)os->start(manager);
		}

		OS();

		virtual OS * start(MemoryManager* = NULL);
		virtual bool init(MemoryManager* = NULL);

		OS * retain();
		void release();

		virtual void * malloc(int size OS_DBG_FILEPOS_DECL);
		virtual void free(void * p);

		int getAllocatedBytes();
		int getMaxAllocatedBytes();
		int getCachedBytes();

		void setMemBreakpointId(int id);

		bool isTerminated();
		int getTerminatedCode();
		void setTerminated(bool = true, int = 0);
		void resetTerminated();

		void getProperty(bool getter_enabled = true, bool prototype_enabled = true);
		void getProperty(const OS_CHAR*, bool getter_enabled = true, bool prototype_enabled = true);
		void getProperty(const Core::String&, bool getter_enabled = true, bool prototype_enabled = true);
		void getProperty(int offs, const OS_CHAR*, bool getter_enabled = true, bool prototype_enabled = true);
		void getProperty(int offs, const Core::String&, bool getter_enabled = true, bool prototype_enabled = true);
		
		void setProperty(bool setter_enabled = true);
		void setProperty(const OS_CHAR*, bool setter_enabled = true);
		void setProperty(const Core::String&, bool setter_enabled = true);
		void setProperty(int offs, const OS_CHAR*, bool setter_enabled = true);
		void setProperty(int offs, const Core::String&, bool setter_enabled = true);
		void addProperty(bool setter_enabled = true);

		void deleteProperty(bool del_enabled = true);
		void deleteProperty(const OS_CHAR*, bool del_enabled = true);
		void deleteProperty(const Core::String&, bool del_enabled = true);

		void getGlobal(const OS_CHAR*, bool getter_enabled = true, bool prototype_enabled = true);
		void getGlobal(const Core::String&, bool getter_enabled = true, bool prototype_enabled = true);

		void setGlobal(const OS_CHAR*, bool setter_enabled = true);
		void setGlobal(const Core::String&, bool setter_enabled = true);

		struct FuncDef;
		
		void setGlobal(const FuncDef& func, bool setter_enabled = true);

		void getPrototype();
		void setPrototype();
		void setPrototype(int userdata_crc);

		int getValueId(int offs = -1);

		void pushNull();
		void pushNumber(OS_INT32);
		void pushNumber(OS_INT64);
		void pushNumber(float);
		void pushNumber(double);
		void pushBool(bool);
		void pushString(const OS_CHAR*);
		void pushString(const OS_CHAR*, int len);
		void pushString(const Core::String&);
		void pushCFunction(OS_CFunction func, void * user_param = NULL);
		void pushCFunction(OS_CFunction func, int closure_values, void * user_param = NULL);
		void * pushUserdata(int crc, int data_size, OS_UserdataDtor dtor = NULL, void * user_param = NULL);
		void * pushUserdata(int data_size, OS_UserdataDtor dtor = NULL, void * user_param = NULL);
		void * pushUserPointer(int crc, void * data, OS_UserdataDtor dtor = NULL, void * user_param = NULL);
		void * pushUserPointer(void * data, OS_UserdataDtor dtor = NULL, void * user_param = NULL);
		void newObject();
		void newArray(int initial_capacity = 0);

		void pushStackValue(int offs = -1);
		void pushGlobals();
		void pushUserPool();
		void pushValueById(int id);

		void retainValueById(int id);
		void releaseValueById(int id);

		void clone(int offs = -1);

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
		String getTypeStr(int offs = -1);
		String getTypeStrById(int id);
		bool isNumber(int offs = -1, OS_NUMBER * out = NULL);
		bool isString(int offs = -1, String * out = NULL);
		bool isType(OS_EValueType, int offs = -1);
		bool isNull(int offs = -1);
		bool isObject(int offs = -1);
		bool isArray(int offs = -1);
		bool isFunction(int offs = -1);
		bool isUserdata(int crc, int offs, int prototype_crc = 0);
		bool isPrototypeOf(int value_offs = -2, int prototype_offs = -1);
		bool is(int value_offs = -2, int prototype_offs = -1);

		bool		toBool(int offs = -1);
		OS_NUMBER	toNumber(int offs = -1, bool valueof_enabled = true);
		float		toFloat(int offs = -1, bool valueof_enabled = true);
		double		toDouble(int offs = -1, bool valueof_enabled = true);
		int			toInt(int offs = -1, bool valueof_enabled = true);
		String		toString(int offs = -1, bool valueof_enabled = true);
		
		void * toUserdata(int crc, int offs = -1, int prototype_crc = 0);
		void clearUserdata(int crc, int offs = -1, int prototype_crc = 0);

		bool		toBool(int offs, bool def);
		OS_NUMBER	toNumber(int offs, OS_NUMBER def, bool valueof_enabled = true);
		float		toFloat(int offs, float def, bool valueof_enabled = true);
		double		toDouble(int offs, double def, bool valueof_enabled = true);
		int			toInt(int offs, int def, bool valueof_enabled = true);
		String		toString(int offs, const String& def, bool valueof_enabled = true);

		bool		popBool();
		OS_NUMBER	popNumber(bool valueof_enabled = true);
		float		popFloat(bool valueof_enabled = true);
		double		popDouble(bool valueof_enabled = true);
		int			popInt(bool valueof_enabled = true);
		String		popString(bool valueof_enabled = true);

		bool		popBool(bool def);
		OS_NUMBER	popNumber(OS_NUMBER def, bool valueof_enabled = true);
		float		popFloat(float def, bool valueof_enabled = true);
		double		popDouble(double def, bool valueof_enabled = true);
		int			popInt(int def, bool valueof_enabled = true);
		String		popString(const String& def, bool valueof_enabled = true);

		int getSetting(OS_ESettings);
		int setSetting(OS_ESettings, int);

		bool compileFile(const String& filename, bool required = false, OS_ESourceCodeType source_code_type = OS_SOURCECODE_AUTO, bool check_utf8_bom = true);
		bool compile(const String& str, OS_ESourceCodeType source_code_type = OS_SOURCECODE_AUTO, bool check_utf8_bom = true);
		bool compile(OS_ESourceCodeType source_code_type = OS_SOURCECODE_AUTO, bool check_utf8_bom = true);

		int call(int params = 0, int ret_values = 0);
		int eval(const OS_CHAR * str, int params = 0, int ret_values = 0, OS_ESourceCodeType source_code_type = OS_SOURCECODE_AUTO, bool check_utf8_bom = true);
		int eval(const String& str, int params = 0, int ret_values = 0, OS_ESourceCodeType source_code_type = OS_SOURCECODE_AUTO, bool check_utf8_bom = true);

		int require(const OS_CHAR * filename, bool required = false, int ret_values = 0, OS_ESourceCodeType source_code_type = OS_SOURCECODE_AUTO, bool check_utf8_bom = true);
		int require(const String& filename, bool required = false, int ret_values = 0, OS_ESourceCodeType source_code_type = OS_SOURCECODE_AUTO, bool check_utf8_bom = true);

		void getErrorHandler(int code);
		void setErrorHandler(int code = OS_E_ALL);

		// return next gc phase
		int gc();
		void gcFull();

		struct FuncDef {
			const OS_CHAR * name;
			OS_CFunction func;
			void * user_param;
		};
		
		struct NumberDef {
			const OS_CHAR * name;
			OS_NUMBER value;
		};
		
		struct StringDef {
			const OS_CHAR * name;
			const OS_CHAR * value;
		};

		struct NullDef {
			const OS_CHAR * name;
		};
		
		void setFuncs(const FuncDef * list, bool setter_enabled = true, int closure_values = 0, void * user_param = NULL); // null terminated list
		void setFunc(const FuncDef& def, bool setter_enabled = true, int closure_values = 0, void * user_param = NULL); // null terminated list
		void setNumbers(const NumberDef * list, bool setter_enabled = true);
		void setNumber(const NumberDef& def, bool setter_enabled = true);
		void setStrings(const StringDef * list, bool setter_enabled = true);
		void setString(const StringDef& def, bool setter_enabled = true);
		void setNulls(const NullDef * list, bool setter_enabled = true);
		void setNull(const NullDef& def, bool setter_enabled = true);

		void getObject(const OS_CHAR * name, bool getter_enabled = true, bool prototype_enabled = true);
		void getGlobalObject(const OS_CHAR * name, bool getter_enabled = true, bool prototype_enabled = true);
		void getModule(const OS_CHAR * name, bool getter_enabled = true, bool prototype_enabled = true);

		void triggerError(int code, const OS_CHAR * message);
		void triggerError(int code, const String& message);

		void triggerError(const OS_CHAR * message);
		void triggerError(const String& message);

		String changeFilenameExt(const String& filename, const String& ext);
		String changeFilenameExt(const String& filename, const OS_CHAR * ext);
		
		String getFilenameExt(const String& filename);
		String getFilenameExt(const OS_CHAR * filename);
		String getFilenameExt(const OS_CHAR * filename, int len);
		
		String getFilename(const String& filename);
		String getFilename(const OS_CHAR * filename);
		String getFilename(const OS_CHAR * filename, int len);
		
		String getFilenamePath(const String& filename);
		String getFilenamePath(const OS_CHAR * filename);
		String getFilenamePath(const OS_CHAR * filename, int len);

		bool isAbsolutePath(const String& filename);
		String resolvePath(const String& filename);
		virtual String resolvePath(const String& filename, const String& cur_path);
		virtual String getCompiledFilename(const String& resolved_filename);
		virtual String getDebugInfoFilename(const String& resolved_filename);
		virtual String getDebugOpcodesFilename(const String& resolved_filename);

		enum EFileUseType
		{
			COMPILE_SOURCECODE_FILE,
			LOAD_COMPILED_FILE
		};
		virtual EFileUseType checkFileUsage(const String& sourcecode_filename, const String& compiled_filename);

		virtual bool isFileExist(const OS_CHAR * filename);
		virtual int getFileSize(const OS_CHAR * filename);
		virtual int getFileSize(void * f);
		virtual void * openFile(const OS_CHAR * filename, const OS_CHAR * mode);
		virtual int readFile(void * buf, int size, void * f);
		virtual int writeFile(const void * buf, int size, void * f);
		virtual int seekFile(void * f, int offset, int whence);
		virtual void closeFile(void * f);

		virtual void echo(const OS_CHAR * str);
		virtual void printf(const OS_CHAR * fmt, ...);

	};
} // namespace ObjectScript

#endif // __OBJECT_SCRIPT_H__