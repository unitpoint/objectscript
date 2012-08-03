#ifndef __OBJECT_SCRIPT__
#define __OBJECT_SCRIPT__

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
#define OS_MEMSET memset
#define OS_MEMCPY memcpy
#define OS_STRLEN strlen
#define OS_STRCMP strcmp
#define OS_STRNCMP strncmp
#define OS_STRCHR strchr
#define OS_STRSTR strstr
#define OS_ISSPACE isspace
#define OS_VSNPRINTF vsnprintf
#define OS_SNPRINTF __snprintf__

#define OS_MATH_POW pow
#define OS_MATH_FLOOR floor
#define OS_MATH_FMOD fmod

#define OS_CHAR char
#define OS_INT __int64
#define OS_FLOAT double
#define OS_INT32 __int32
#define OS_BYTE unsigned char
#define OS_USHORT unsigned short

#define OS_TEXT(s) s

#define OS_DEF_VAR_HASH_SIZE 4
#define OS_DEF_VALUES_HASH_SIZE 16
#define OS_DEF_PRECISION 16

#define OS_DEF_FMT_BUF_SIZE 1024*10

// #define OS_NEW(classname, params) new (allocator->malloc(sizeof(classname))) classname params

namespace ObjectScript
{

	class OS;
	typedef void * (*OS_HeapFunction)(void*, int);
	typedef void (*OS_UserDataDtor)(OS*, void*);
	typedef int (*OS_CFunction)(OS*, void*);

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
		OS_VALUE_TYPE_THREAD,
	};

	enum // OS_ValueRegister
	{
		OS_GLOBALS = 0x10000000,
	};

	enum OS_EFatalError
	{
		OS_FATAL_ERROR_OPCODE,
	};

	class OS
#ifdef OBJECT_SCRIPT_EXTENDS_CLASS
		: public OBJECT_SCRIPT_EXTENDS_CLASS
#endif
	{
	public:

	protected:

		OS_HeapFunction heap_func; // make it the first field to be initialized firstly
		int allocated_bytes;
		int cached_bytes;
		int max_allocated_bytes;
		
	protected:

		// friend void * operator new(void *p){ return p; }

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
			T& operator [] (int i){ return buf[i]; }
			const T& operator [] (int i) const { return buf[i]; }

			bool contains(const T& val) const 
			{
				for(int i = count-1; i >= 0; i--){
					if(buf[i] == val){
						return true;
					}
				}
				return false;
			}
		};

	public:

		enum EParseNumType
		{
			PARSE_NUM_TYPE_ERROR,
			PARSE_NUM_TYPE_FLOAT,
			PARSE_NUM_TYPE_INT,
		};

		enum
		{
			PARSE_NUM_FLAG_INT = 0,
			PARSE_NUM_FLAG_INT_EXPANDED = 1<<0,
			PARSE_NUM_FLAG_FLOAT = 1<<1,
		};

		struct Utils
		{
			static EParseNumType parseNum(const OS_CHAR *& str, OS_FLOAT& fval, OS_INT& ival, int flags);

			static OS_CHAR * numToStr(OS_CHAR*, OS_INT value);
			static OS_CHAR * numToStr(OS_CHAR*, OS_FLOAT value, int precision);

			static OS_INT strToInt(const OS_CHAR*);
			static OS_FLOAT strToFloat(const OS_CHAR*);

			static int keyToHash(const OS_CHAR*, int size);

			static int cmp(const void * buf1, int len1, const void * buf2, int len2);
			static int cmp(const void * buf1, int len1, const void * buf2, int len2, int maxLen);
		};

	protected:

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

			int allocatedBytes() const { return allocated_bytes; }
			int getDataSize() const { return data_size; }

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

		class StringLocal // doesn't retain OS, strings inside of OS mush use StringLocal instead of String
		{
		protected:

			// StringData * data;
			const OS_CHAR * str;

		public:

			StringLocal(OS*);
			StringLocal(OS*, const OS_CHAR*);
			StringLocal(OS*, OS_CHAR, int count);
			StringLocal(OS*, const void*, int size);
			StringLocal(OS*, const void * buf1, int len1, const void * buf2, int len2);
			StringLocal(const StringLocal&);
			StringLocal(StringData*);
			StringLocal(OS*, OS_INT value);
			StringLocal(OS*, OS_FLOAT value, int precision = OS_DEF_PRECISION);
			~StringLocal();

			static StringLocal format(OS*, int temp_buf_size, const OS_CHAR * fmt, ...);
			static StringLocal format(OS*, int temp_buf_size, const OS_CHAR * fmt, va_list);
			static StringLocal format(OS*, const OS_CHAR * fmt, ...);
			static StringLocal format(OS*, const OS_CHAR * fmt, va_list);

			StringLocal& setFormat(int temp_buf_size, const OS_CHAR * fmt, ...);
			StringLocal& setFormat(int temp_buf_size, const OS_CHAR * fmt, va_list);
			StringLocal& setFormat(const OS_CHAR * fmt, ...);
			StringLocal& setFormat(const OS_CHAR * fmt, va_list);

			StringData * toData() const { return StringData::toData(str); }
			StringData * toData(){ return StringData::toData(str); }

			OS * getAllocator(){ return toData()->allocator; }
			OS * getAllocator() const { return toData()->allocator; }

			const OS_CHAR * toChar() const { return str; } // toData()->toChar(); }
			operator const OS_CHAR*() const { return str; } // toChar(); }
			// operator StringData*() { return data; }

			// bool isNull() const { return data == NULL; }
			int allocatedBytes() const { return toData()->allocated_bytes; }
			int getDataSize() const { return toData()->data_size; }

			void clear();

			OS_INT toInt() const;
			OS_FLOAT toFloat() const;

			StringLocal& operator=(const StringLocal&);
			StringLocal& operator=(const OS_CHAR*);

			StringLocal& operator+=(const StringLocal&);
			StringLocal& operator+=(const OS_CHAR*);
			StringLocal& append(const void*, int size);
			StringLocal& append(const OS_CHAR*);

			StringLocal operator+(const StringLocal&);
			StringLocal operator+(const OS_CHAR*);

			bool operator==(const StringLocal&);
			bool operator==(const OS_CHAR*);

			bool operator!=(const StringLocal&);
			bool operator!=(const OS_CHAR*);

			bool operator<=(const StringLocal&);
			bool operator<=(const OS_CHAR*);

			bool operator<(const StringLocal&);
			bool operator<(const OS_CHAR*);

			bool operator>=(const StringLocal&);
			bool operator>=(const OS_CHAR*);

			bool operator>(const StringLocal&);
			bool operator>(const OS_CHAR*);

			StringLocal trim(bool trim_left = true, bool trim_right = true) const;

			int cmp(const StringLocal&) const;
			int cmp(const OS_CHAR*) const;
			int hash() const;

			StringLocal clone() const;
		};

	public:

		class String: public StringLocal // retains os, external strings must use String instead of StringLocal
		{
			typedef StringLocal super;

		public:

			String(OS*);
			String(OS*, const OS_CHAR*);
			String(OS*, OS_CHAR, int count);
			String(OS*, const void*, int size);
			String(OS*, const void * buf1, int len1, const void * buf2, int len2);
			String(const StringLocal&);
			String(const String&);
			String(StringData*);
			String(OS*, OS_INT value);
			String(OS*, OS_FLOAT value, int precision);
			~String();

			// operator const StringLocal&() const { return *this; }

			String& operator=(const StringLocal&);
			String& operator=(const OS_CHAR*);

			String& operator+=(const StringLocal&);
			String& operator+=(const OS_CHAR*);
			String& append(const void*, int size);
			String& append(const OS_CHAR*);
		};

	protected:

		class Tokenizer
		{
		public:

			enum Error
			{
				ERROR_NOTHING,
				ERROR_MULTI_LINE_COMMENT, // multi line comment not end
				ERROR_CONST_VECTOR_3,       // cant parse const vec3
				ERROR_CONST_VECTOR_4,     // cant parse const vec4
				ERROR_CONST_STRING,             // string not end
				ERROR_CONST_STRING_ESCAPE_CHAR, // string escape error
				ERROR_NAME,               // error name, _ - this is error
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

				NUM_INT,        // -?[0..9]+
				NUM_FLOAT,      // -?[0..9][.]?[0..9]+(e[+-]?[0..9]+)?
				NUM_VECTOR_3,   // 'NUM_FLOAT NUM_FLOAT NUM_FLOAT'
				NUM_VECTOR_4,   // 'NUM_FLOAT NUM_FLOAT NUM_FLOAT NUM_FLOAT'
				// NULL,           // used in compiler

				// [not real operators]
				OPERATOR,
				BINARY_OPERATOR,
				SEPARATOR,
				// [/not real operators]

				OPERATOR_INDIRECT,    // .
				OPERATOR_CONCAT,    // ..
				OPERATOR_ANY_PARAMS,  // ...

				// OPERATOR_PRECOMP,   // #
				// OPERATOR_DOLLAR,    // $

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

				OPERATOR_BIN_AND, // &
				OPERATOR_BIN_OR,  // |
				OPERATOR_BIN_XOR, // ^
				OPERATOR_BIN_NOT, // ~
				OPERATOR_ADD, // +
				OPERATOR_SUB, // -
				OPERATOR_MUL, // *
				OPERATOR_DIV, // /
				OPERATOR_MOD, // %
				OPERATOR_MUL_SHIFT, // <<
				OPERATOR_DIV_SHIFT, // >>
				OPERATOR_POW, // **

				OPERATOR_DOT,   // dot
				OPERATOR_CROSS, // cross
				OPERATOR_SWAP,  // swap
				OPERATOR_IS,    // is
				OPERATOR_AS,    // as
				OPERATOR_IN,    // in

				OPERATOR_BIN_AND_ASSIGN, // &=
				OPERATOR_BIN_OR_ASSIGN,  // |=
				OPERATOR_BIN_XOR_ASSIGN, // ^=
				OPERATOR_BIN_NOT_ASSIGN, // ~=
				OPERATOR_ADD_ASSIGN, // +=
				OPERATOR_SUB_ASSIGN, // -=
				OPERATOR_MUL_ASSIGN, // *=
				OPERATOR_DIV_ASSIGN, // /=
				OPERATOR_MOD_ASSIGN, // %=
				OPERATOR_MUL_SHIFT_ASSIGN, // <<=
				OPERATOR_DIV_SHIFT_ASSIGN, // >>=
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

				StringLocal filename;
				Vector<StringLocal> lines;
				
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
					OS_INT int_value;
					OS_FLOAT float_value;
					OS_FLOAT * vec3;
					OS_FLOAT * vec4;
				};
				TokenType type;

				~TokenData();

			public:

				TextData * text_data;

				StringLocal str;
				int line, pos;
				int ref_count;

				OS * getAllocator() const;
				TokenType getType() const { return type; }
				OS_INT getInt() const;
				OS_FLOAT getFloat() const;
				const OS_FLOAT * getVec3() const;
				const OS_FLOAT * getVec4() const;

				TokenData(TextData * text_data, const StringLocal& p_str, TokenType p_type, int p_line, int p_pos);

				TokenData * retain();
				void release();

				void setVec4(OS_FLOAT values[4]);
				void setVec3(OS_FLOAT values[3]);
				void setFloat(OS_FLOAT value);
				void setInt(OS_INT value);

				operator const StringLocal& () const { return str; }

				bool isTypeOf(TokenType tokenType) const;
			};

		protected:

			struct Settings
			{
				bool parseVector;
				bool parseStringOperator;
				// bool parsePreprocessor;
				bool saveComment;
			} settings;

			TextData * text_data;
			
			int cur_line, cur_pos;
			bool loaded;
			bool compiled;

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
			static void initOperatorTable();

			TokenData * addToken(const StringLocal& token, TokenType type, int line, int pos);

			TokenType parseNum(const OS_CHAR *& str, OS_FLOAT& fval, OS_INT& ival, bool parse_end_spaces);
			bool parseLines();

		public:

			Tokenizer(OS*);
			~Tokenizer();

			void reset();

			OS * getAllocator();
			TextData * getTextData() const { return text_data; }

			bool isLoaded() const { return loaded; }

			bool isCompiled() const { return compiled; }
			void setCompiled(bool value){ compiled = value; }

			bool isError() const { return error != ERROR_NOTHING; }
			Error getErrorCode() const { return error; }
			int getErrorLine() const { return cur_line; }
			int getErrorPos() const { return cur_pos; }

			static const OS_CHAR * getTokenTypeName(TokenType tokenType);

			StringLocal getFilename() const { return text_data->filename; }
			StringLocal getLineString(int i) const { return text_data->lines[i]; }
			int getNumLines() const { return text_data->lines.count; }

			bool getSettingParseVector() const { return settings.parseVector; }
			void setSettingParseVector(bool value){ settings.parseVector = value; }

			bool getSettingParseStringOperator() const { return settings.parseStringOperator; }
			void setSettingParseStringOperator(bool value){ settings.parseStringOperator = value; }

			//bool SettingParsePreprocessor() const { return settings.parsePreprocessor; }
			//void SetSettingParsePreprocessor(bool value){ settings.parsePreprocessor = value; }

			bool getSettingSaveComment() const { return settings.saveComment; }
			void setSettingSaveComment(bool value){ settings.saveComment = value; }

			bool parseText(const StringLocal& text);

			int getNumTokens() const { return tokens.count; }
			TokenData * getToken(int i) const { return tokens[i]; }
			void insertToken(int i, TokenData * token);
		};

		typedef Tokenizer::TokenType TokenType;
		typedef Tokenizer::TokenData TokenData;
		typedef Tokenizer::TextData TextData;

		struct Value;
		struct FunctionDecl
		{
			struct LocalVarDecl;
			/*
			struct LocalVarDecl
			{
				StringLocal name;
				int start;
				int end;

				LocalVarDecl();
				~LocalVarDecl();
			};
			*/

			Value * env; // retained

			FunctionDecl * parent; // retained
			int parent_opcode_num;

			int num_params;

			LocalVarDecl * local_decls;
			int num_locals;

			int * opcodes;
			int num_opcodes;

			int ref_count;

			FunctionDecl();
			~FunctionDecl();
		};

		struct VariableIndex
		{
			StringLocal string_index;
			OS_INT int_index;
			int hash_value;
			bool is_string_index;
			bool int_valid;

			VariableIndex(const VariableIndex& index);
			VariableIndex(const StringLocal& index);
			VariableIndex(StringData * index);
			VariableIndex(OS*, const OS_CHAR * index);
			VariableIndex(OS*, OS_INT index);
			VariableIndex(OS*, int index);
			VariableIndex(OS*, OS_FLOAT index, int precision = OS_DEF_PRECISION);
			~VariableIndex();

			OS * getAllocator() const { return string_index.getAllocator(); }

			int cmp(const VariableIndex& b) const;
			int hash() const;

			StringLocal toString() const;

			bool checkIntIndex() const;
			void fixName();
		};

		struct Value
		{
			struct Variable: public VariableIndex
			{
				int value_id;  // allow weak usage
				// Value * value;

				Variable * hash_next;
				Variable * prev, * next;

				Variable(const VariableIndex& index);
				Variable(const StringLocal& index);
				Variable(OS*, const OS_CHAR * index);
				Variable(OS*, OS_INT index);
				Variable(OS*, int index);
				Variable(OS*, OS_FLOAT index, int precision = OS_DEF_PRECISION);
				~Variable();
			};

			struct Table
			{
				Variable ** heads;
				int head_mask;
				int count;

				OS_INT next_id;

				Variable * first, * last;

				Table();    
				virtual ~Table();

				Variable * get(const VariableIndex& index);
				// void add(Variable * var);
				// void free(const VariableIndex& index);
			};

			struct Array: public Table
			{
				Vector<int> values; // allow weak usage, value id list

				Array();
				virtual ~Array();
			};

			int value_id; // allow weak usage
			int ref_count;
			Value * prototype; // retained
			Value * hash_next;
			// int gc_time;

			union {
				// bool boolean;
				OS_FLOAT number;
				StringData * string_data;
				Table * table;
				Array * arr;

				struct {
					void * ptr;
					OS_UserDataDtor dtor;
				} userdata;

				struct {
					// Value * parent_func; // retained
					// Value * env;
					FunctionDecl * func_decl;
				} func;

				struct {
					OS_CFunction func;
					void * user_param;
				} cfunc;
			} value;

			OS_EValueType type;

			Value(int id);
			~Value();

			Value * retain()
			{
				ref_count++;
				return this;
			}
			
			/* Value * retainSafely()
			{
				if(this){
					ref_count++;
				}
				return this;
			} */
		};

		class Program;
		class Compiler
		{
		protected:

			enum ExpressionType {
				EXP_TYPE_UNKNOWN,
				EXP_TYPE_NOP,
				EXP_TYPE_SCOPE,
				EXP_TYPE_CODE_LIST,
				EXP_TYPE_NAME, // temp
				EXP_TYPE_POST_IF,
				EXP_TYPE_POP_VALUE,
				EXP_TYPE_CALL,
				EXP_TYPE_CALL_DIM,
				EXP_TYPE_VALUE,
				EXP_TYPE_PARAMS,
				EXP_TYPE_FUNCTION,
				EXP_TYPE_RETURN,
				EXP_TYPE_ARRAY,
				EXP_TYPE_OBJECT,
				EXP_TYPE_OBJECT_SET_BY_NAME,
				EXP_TYPE_OBJECT_SET_BY_INDEX,
				EXP_TYPE_OBJECT_SET_BY_EXP,
				EXP_TYPE_OBJECT_SET_BY_AUTO,

				EXP_TYPE_GET_LOCAL_VAR,
				EXP_TYPE_SET_LOCAL_VAR,

				EXP_TYPE_GET_AUTO_VAR,
				EXP_TYPE_SET_AUTO_VAR,

				EXP_TYPE_INDIRECT, // temp
				EXP_TYPE_GET_PROPERTY,
				EXP_TYPE_SET_PROPERTY,

				EXP_TYPE_GET_PROPERTY_DIM,
				EXP_TYPE_SET_PROPERTY_DIM,

				EXP_TYPE_CALL_PROPERTY,

				// EXP_CONST_STRING,
				// EXP_CONST_FLOAT,
				// EXP_VAR_ASSING,

				EXP_TYPE_CONST_NULL,
				// EXP_TYPE_CONST_VALUE,
				EXP_TYPE_CONST_NUMBER,
				EXP_TYPE_CONST_STRING,
				EXP_TYPE_CONST_TRUE,
				EXP_TYPE_CONST_FALSE,

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
				EXP_TYPE_LOGIC_NOT,     // !

				EXP_TYPE_PLUS,    // +
				EXP_TYPE_NEG,     // -

				// EXP_TYPE_INC,     // ++
				// EXP_TYPE_DEC,     // --

				EXP_TYPE_PRE_INC,     // ++
				EXP_TYPE_PRE_DEC,     // --

				EXP_TYPE_POST_INC,    // ++
				EXP_TYPE_POST_DEC,    // --

				// EXP_TYPE_QUESTION,

				EXP_TYPE_BIN_AND, // &
				EXP_TYPE_BIN_OR,  // |
				EXP_TYPE_BIN_XOR, // ^
				EXP_TYPE_BIN_NOT, // ~

				EXP_TYPE_BIN_AND_ASSIGN, // &=
				EXP_TYPE_BIN_OR_ASSIGN,  // |=
				EXP_TYPE_BIN_XOR_ASSIGN, // ^=
				EXP_TYPE_BIN_NOT_ASSIGN, // ~=

				EXP_TYPE_ADD, // +
				EXP_TYPE_SUB, // -
				EXP_TYPE_MUL, // *
				EXP_TYPE_DIV, // /
				EXP_TYPE_MOD, // %
				EXP_TYPE_MUL_SHIFT, // <<
				EXP_TYPE_DIV_SHIFT, // >>
				EXP_TYPE_POW, // **

				EXP_TYPE_ADD_ASSIGN, // +=
				EXP_TYPE_SUB_ASSIGN, // -=
				EXP_TYPE_MUL_ASSIGN, // *=
				EXP_TYPE_DIV_ASSIGN, // /=
				EXP_TYPE_MOD_ASSIGN, // %=
				EXP_TYPE_MUL_SHIFT_ASSIGN, // <<=
				EXP_TYPE_DIV_SHIFT_ASSIGN, // >>=
				EXP_TYPE_POW_ASSIGN, // **=

				EXP_TYPE_ASSIGN,

				EXP_TYPE_DOT, // dot
				EXP_TYPE_CROSS, // cross

				EXP_TYPE_SWAP, // swap

				EXP_TYPE_AS, // as
				EXP_TYPE_IS,  // is

				EXP_TYPE_IN,  // in
			};

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

			struct Expression
			{
				TokenData * token;
				ExpressionType type;
				int ret_values;
				// bool is_value;
				
				ExpressionList list;

				Expression(ExpressionType type, TokenData*);
				Expression(ExpressionType type, TokenData*, Expression * e1);
				Expression(ExpressionType type, TokenData*, Expression * e1, Expression * e2);
				Expression(ExpressionType type, TokenData*, Expression * e1, Expression * e2, Expression * e3);
				virtual ~Expression();

				OS * getAllocator(){ return list.allocator; }

				OS_FLOAT toNumber();
				OS_INT toInt();
				StringLocal toString();

				bool isConstValue() const;
				bool isValue() const;
				bool isClear() const;
				bool isWriteable() const;
				bool isOperator() const;
				bool isUnaryOperator() const;
				bool isBinaryOperator() const;
				bool isAssignOperator() const;
				bool isLogicOperator() const;

				StringLocal debugPrint(OS::Compiler * compiler, int depth);
			};

			struct Scope: public Expression
			{
				Scope * parent;

				enum Type {
					// LIBRARY,
					TEXT,
					FUNCTION,
					BLOCK,
					SWITCH,
				} type;

				struct LocalVar
				{
					StringLocal name;
					int start_pos;
					int end_pos;
				};

				Vector<LocalVar> locals;

				Scope(Type, TokenData*);
				virtual ~Scope();
			};

			enum ErrorType {
				ERROR_NOTHING,
				ERROR_SYNTAX,
				ERROR_EXPECT_TOKEN_TYPE,
				ERROR_EXPECT_TOKEN_STR,
				ERROR_EXPECT_TOKEN,
				ERROR_EXPECT_VALUE,
				ERROR_EXPECT_WRITEABLE,
				ERROR_EXPECT_EXPRESSION,
				ERROR_EXPECT_FUNCTION_SCOPE,
				ERROR_EXPECT_SWITCH_SCOPE,
				ERROR_FINISH_BIN_OP,
				ERROR_FINISH_UNARY_OP,
			};

			enum OpcodeLevel {
				OP_LEVEL_NOTHING = -1,
				OP_LEVEL_0, // ||
				OP_LEVEL_1, // &&
				OP_LEVEL_2, // <= >= ...
				OP_LEVEL_3, // = += &= ..
				OP_LEVEL_4, // |
				OP_LEVEL_5, // & ^ ~ ?
				OP_LEVEL_6, // + -
				OP_LEVEL_7, // * / % << >> dot cross
				OP_LEVEL_8, // ** as is
				OP_LEVEL_9, // ++ -- ! 
				OP_LEVEL_10, // .
				OP_LEVEL_COUNT
			};

			OS * allocator;
			Tokenizer * tokenizer;

			ErrorType error;
			TokenData * error_token;
			TokenType expect_token_type;
			StringLocal expect_token;

			TokenData * recent_token;
			int next_token_index;

			// StringLocal recent_printed_filename;
			TextData * recent_printed_text_data;
			int recent_printed_line;

			// code generation
			Program * prog;
			Value::Table * strings_cache;
			Value::Table * numbers_cache;

			bool isError();
			void resetError();
			void setError();
			void setError(ErrorType value, TokenData * error_token);
			void setError(TokenType expect_token_type, TokenData * error_token);
			void setError(const StringLocal& str, TokenData * error_token);

			void * malloc(int size);

			void setNextTokenIndex(int i);
			void setNextToken(TokenData * token);
			void putNextTokenType(TokenType tokenType);
			void ungetToken();

			bool isNextTokens(TokenType * list, int count);
			bool isNextToken(TokenType t0);
			bool isNextTokens(TokenType t0, TokenType t1);
			bool isNextTokens(TokenType t0, TokenType t1, TokenType t2);
			bool isNextTokens(TokenType t0, TokenType t1, TokenType t2, TokenType t3);

			void deleteNops(ExpressionList& list);

			ExpressionType tokenTypeToExp(TokenType);
			OpcodeLevel getOpcodeLevel(ExpressionType exp_type);

			TokenData * readToken();
			TokenData * expectToken(TokenType);
			TokenData * expectToken();

			Expression * expectExpressionValues(Expression * exp, int ret_values);
			Expression * newExpressionFromList(ExpressionList& list, int ret_values); // , bool allow_auto_nop);
			Expression * newAssingExpression(Scope * scope, Expression * var_exp, Expression * value_exp);
			// Expression * newIndirectExpression(Scope * scope, Expression * var_exp, Expression * value_exp);
			Expression * newSingleValueExpression(Expression * exp);
			Expression * secondPhaseExpression(Expression * exp);

			Expression * expectTextExpression(Scope*, int ret_values);
			Expression * expectCodeExpression(Scope*, int ret_values);
			Expression * expectSingleExpression(Scope*, bool allow_binary_operator, bool allow_param);
			Expression * expectObjectExpression(Scope*);
			Expression * expectArrayExpression(Scope*);
			Expression * finishParamsExpression(Expression * params);
			Expression * expectParamsExpression(Scope*);
			Expression * expectParamsExpression(Scope*, Expression * first_param);
			Expression * expectFunctionExpression(Scope*);
			Expression * expectReturnExpression(Scope*);
			Expression * finishValueExpression(Scope*, Expression*, bool allow_binary_operator, bool allow_param);
			Expression * finishBinaryOperator(Scope * scope, OpcodeLevel prev_level, Expression * exp, bool allow_param);
			Expression * newBinaryExpression(Scope * scope, ExpressionType, TokenData*, Expression * left_exp, Expression * right_exp);

			int findLocalVarIndex(Scope * scope, const StringLocal& name);
			
			StringLocal debugPrintSourceLine(TokenData*);
			static const OS_CHAR * getExpName(ExpressionType);

			int getCachedStringIndex(const StringLocal& str);
			int getCachedNumberIndex(OS_FLOAT);
			bool generateOpcodes(Expression*, Program*);
			bool generateOpcodes(Expression*);

		public:

			Compiler(Tokenizer*);
			virtual ~Compiler();

			bool compile(); // push compiled function
		};

		class Program
		{
		protected:

			int ref_count;

			virtual ~Program();

		public:

			enum {
				OP_UNKNOWN,
				OP_PUSH_NUMBER,
				OP_PUSH_STRING,
				OP_PUSH_NULL,
				OP_PUSH_TRUE,
				OP_PUSH_FALSE,
				OP_PUSH_VAR_BY_NAME,

				OP_FUNCTION,
				// OP_PUSH_VAR, // allow name & dim
				// OP_PUSH_VAR_BY_NAME,
				// OP_PUSH_VAR_BY_NAME_DIM,
				OP_INDIRECT_BY_NAME,
				OP_INDIRECT_BY_NAME_DIM,
				OP_SET_VAR_BY_NAME,
				OP_SET_VAR_BY_NAME_DIM,
				OP_ADD,
				OP_SUB,
				OP_JUMP,
			};

			struct Opcode
			{
				virtual ~Opcode();
			};

			OS * allocator;
			StringLocal filename;
			Vector<StringLocal> strings;
			Vector<OS_FLOAT> numbers;
			Vector<OS_BYTE> opcodes;

			Program(OS * allocator);

			Program * retain();
			void release();

			void writeCodeByte(int);
			void writeCodeUShort(int);
			// void writeCodeFloat(OS_FLOAT);
		};

		struct Values
		{
			Value ** heads;
			int head_mask;
			int count;

			int next_id;

			Values();
			~Values();

			// Value * add(Value * obj);
			// Value * remove(int value_id);
			Value * get(int value_id);
		};

		struct StackFunction
		{
			Value * func; // retained
			int cur_opcode_num;

			Value ** local_vars;

			StackFunction();
			~StackFunction();
		};

		template<class T> void vectorReserve(Vector<T>& vec, int capacity)
		{
			if(vec.capacity < capacity){
				vec.capacity = vec.capacity > 0 ? vec.capacity*2 : 4;
				if(vec.capacity < capacity){
					vec.capacity = (capacity+3) & ~3;
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

		template<class T> void addVectorItem(Vector<T>& vec, const T& val)
		{
			if(vec.count >= vec.capacity){
				vectorReserve(vec, vec.capacity > 0 ? vec.capacity*2 : 4);
			}
			new (vec.buf + vec.count++) T(val);
		}

		template<class T> void clearVector(Vector<T>& vec)
		{
			for(int i = 0; i < vec.count; i++){
				vec.buf[i].~T();
			}
			free(vec.buf);
			vec.buf = NULL;
			vec.capacity = 0;
			vec.count = 0;
		}

		template<class T> void releaseVectorItems(Vector<T>& vec)
		{
			for(int i = 0; i < vec.count; i++){
				vec.buf[i]->release();
			}
			// free(vec.buf);
			// vec.buf = NULL;
			// vec.capacity = 0;
			vec.count = 0;
		}

		template<class T> void releaseVectorValues(Vector<T>& vec)
		{
			while(vec.count > 0){
				releaseValue(vec.buf[--vec.count]);
			}
		}

		template<class T> void deleteVectorItems(Vector<T*>& vec)
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
				vectorReserve(vec, vec.capacity > 0 ? vec.capacity*2 : 4);
			}
			for(int j = vec.count-1; j >= i; j--){
				new (vec.buf+j+1) T(vec.buf[j]);
				vec.buf[j].~T();
			}
			new (vec.buf+i) T(val);
			vec.count++;
		}

		template<class T> T vectorRemoveAtIndex(Vector<T>& vec, int i)
		{
			OS_ASSERT(i >= 0 && i < vec.count);
			T val = vec.buf[i];
			vec.buf[i].~T();
			for(i++; i < vec.count; i++){
				new (vec.buf+i-1) T(vec.buf[i]);
				vec.buf[i].~T();
			}
			vec.count--;
			return val;
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

		int ref_count;
		StringData * empty_string_data;

		struct Strings
		{
			StringLocal __get;
			StringLocal __set;
			StringLocal __construct;
			StringLocal __destruct;
			StringLocal __cmp;
			StringLocal __tostring;
			StringLocal __tobool;
			StringLocal __add;
			StringLocal __sub;
			StringLocal __mul;
			StringLocal __div;
			StringLocal __mod;

			StringLocal syntax_function;
			StringLocal syntax_null;
			StringLocal syntax_true;
			StringLocal syntax_false;
			StringLocal syntax_return;
			StringLocal syntax_class;
			StringLocal syntax_enum;
			StringLocal syntax_switch;
			StringLocal syntax_case;
			StringLocal syntax_default;
			StringLocal syntax_if;
			StringLocal syntax_else;
			StringLocal syntax_elseif;
			StringLocal syntax_for;
			StringLocal syntax_do;
			StringLocal syntax_while;
			StringLocal syntax_break;
			StringLocal syntax_continue;

			int __dummy__;

			Strings(OS * allocator);
		} * strings;

		Vector<FunctionDecl*> func_decls;

		Values values;
		Value * global_vars;

		enum {
			PROTOTYPE_BOOL,
			PROTOTYPE_NUMBER,
			PROTOTYPE_STRING,
			PROTOTYPE_OBJECT,
			PROTOTYPE_ARRAY,
			PROTOTYPE_FUNCTION,
			// -----------------
			PROTOTYPES_NUMBER
		};

		Value * prototypes[PROTOTYPES_NUMBER];
		
		// Vector<Value*> autorelease_values;
		Vector<Value*> stack_values;
		Vector<Value*> temp_values;
		Vector<StackFunction*> call_stack_funcs;

		// Vector<Value*> cache_values;

		int fatal_error;

		enum {
			MEM_CACHE_BLOCK_SIZE = 8,
			MEM_CACHE_BLOCKS_NUMBER = 128,
		};

		struct MemCacheBlock
		{
			MemCacheBlock * next;
		};
		MemCacheBlock * mem_cache_blocks[MEM_CACHE_BLOCKS_NUMBER];
		// int gc_on_allocated_size;

		void freeMemCacheBlocks(int new_cached_bytes = 0);

		void * malloc(int size);
		void * realloc(void * p, int size);
		void free(void * p);

		void releaseFuncDecl(FunctionDecl*);

		void resetValue(Value*);
		void deleteValue(Value*);
		void releaseValue(Value*);

		void releaseValue(int value_id);

		Value * pushValue(Value * val);
		Value * pushValueAutoNull(Value * val);
		Value * pushNullValue();
		
		Value * pushBoolValue(bool);
		Value * pushNumberValue(OS_FLOAT);
		Value * pushStringValue(const StringLocal&);
		Value * pushStringValue(const OS_CHAR*);
		Value * pushCFunctionValue(OS_CFunction func, void * user_param);
		Value * pushUserdataValue(int data_size, OS_UserDataDtor dtor);
		Value * pushUserPointerValue(void * data, OS_UserDataDtor dtor);
		Value * pushObjectValue();
		Value * pushArrayValue();

		void removeStackValues(int offs, int count);
		void removeStackValues(int count);

		/*
		Value * setValue(Value*, bool);
		Value * setValue(Value*, int);
		Value * setValue(Value*, OS_FLOAT);
		Value * setValue(Value*, const StringLocal&);
		*/

		/*
		Value * push(Value*);
		Value * top(int offs = 0);
		Value * pop(int count = 1);
		*/

		Value * registerValue(Value * val);
		Value * unregisterValue(int value_id);
		void deleteValues();

		int valueToInt(Value * val);
		OS_FLOAT valueToNumber(Value * val);
		StringLocal valueToString(Value * val);

		bool isValueNumber(Value * val, OS_FLOAT * out = NULL);
		bool isValueString(Value * val, StringLocal * out = NULL);
		bool isValueInstanceOf(Value * val, Value * prototype_val);

		Value::Table * newTable();
		void deleteTable(Value::Table*);
		void addTableVariable(Value::Table * table, Value::Variable * var);
		bool deleteTableVariable(Value::Table * table, const VariableIndex& index);

		Value::Array * newArray();
		void deleteArray(Value::Array*);

		Value::Variable * setTableValue(Value::Table * table, VariableIndex& index, Value * val, bool prototype_enabled, bool setter_enabled);
		void pushObjectField(Value * table_value, VariableIndex& index, bool prototype_enabled, bool getter_enabled);

		Value * getOffsValue(int offs);

		void enterFunction();
		void leaveFunction();

		OS(OS_HeapFunction p_heap_func);
		virtual ~OS();

		virtual bool init();
		virtual void shutdown();

	public:

		static OS * create(OS_HeapFunction heap_func = NULL);

		OS * retain();
		void release();

		int getAllocatedBytes();
		int getMaxAllocatedBytes();

		void pushNull();
		void pushInt(int);
		void pushFloat(double);
		void pushBool(bool);
		void pushString(const OS_CHAR*);
		void pushString(const StringLocal&);
		void pushCFunction(OS_CFunction func, void * user_param);
		void * pushUserdata(int data_size, OS_UserDataDtor dtor = NULL);
		void * pushUserPointer(void * data, OS_UserDataDtor dtor = NULL);
		void pushObject();
		void pushArray();
		void pushStackValue(int offs = -1);
		void pushValueById(int id);

		int getStackValueId(int offs = -1);
		int retainValueById(int id);
		void releaseValueById(int id);

		void remove(int start_offs = -1, int count = 1);
		void pop(int count = 1);
		void popAll();

		void objectSet(int table_offs = -3, int pop_count = 2, bool prototype_enabled = true, bool setter_enabled = true);
		void objectGet(int table_offs = -2, int pop_count = 2, bool prototype_enabled = true, bool getter_enabled = true);

		OS_EValueType getType(int offs = -1);
		OS_EValueType getTypeById(int id);
		bool isNumber(int offs = -1, OS_FLOAT * out = NULL);
		bool isString(int offs = -1, String * out = NULL);
		bool isType(OS_EValueType, int offs = -1);
		bool isObject(int offs = -1);
		bool isArray(int offs = -1);
		bool isFunction(int offs = -1);
		bool isInstanceOf(int value_offs = -2, int prototype_offs = -1);
		// bool checkType(int offs, OS_EValueType type);

		OS_FLOAT toNumber(int offs = -1);
		String toString(int offs = -1);

		int compile(int offs, int pop_count);
		int compile(int offs = -1);
		int call(int params = 0);
		int eval(OS_CHAR * str);
		int eval(const StringLocal& str);
	};

} // namespace OS

#endif // __OBJECT_SCRIPT__