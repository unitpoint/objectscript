/*
AUTO-GENERATED FILE. DO NOT MODIFY.

Note: this header is a header template
and must NOT have multiple-inclusion protection.
*/

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

#if OS_BIND_FUNC_NUM_ARGS == 0

#define OS_BIND_FUNC_PARMS_COMMA
#define OS_BIND_FUNC_TEMPLATE_PARMS
#define OS_BIND_FUNC_PARMS
#define OS_BIND_FUNC_ARGS
#define OS_BIND_FUNC_GET_ARGS

#elif OS_BIND_FUNC_NUM_ARGS == 1

#define OS_BIND_FUNC_PARMS_COMMA ,
#define OS_BIND_FUNC_TEMPLATE_PARMS class ARG_TYPE_1
#define OS_BIND_FUNC_PARMS ARG_TYPE_1
#define OS_BIND_FUNC_ARGS arg1
#define OS_BIND_FUNC_GET_ARGS  \
	int cur_param_offs = -params; \
	OS_GET_TEMPLATE_ARG(1, ARG_TYPE_1)

#elif OS_BIND_FUNC_NUM_ARGS == 2

#define OS_BIND_FUNC_PARMS_COMMA ,
#define OS_BIND_FUNC_TEMPLATE_PARMS class ARG_TYPE_1, class ARG_TYPE_2
#define OS_BIND_FUNC_PARMS ARG_TYPE_1, ARG_TYPE_2
#define OS_BIND_FUNC_ARGS arg1, arg2
#define OS_BIND_FUNC_GET_ARGS  \
	int cur_param_offs = -params; \
	OS_GET_TEMPLATE_ARG(1, ARG_TYPE_1); \
	OS_GET_TEMPLATE_ARG(2, ARG_TYPE_2)

#elif OS_BIND_FUNC_NUM_ARGS == 3

#define OS_BIND_FUNC_PARMS_COMMA ,
#define OS_BIND_FUNC_TEMPLATE_PARMS class ARG_TYPE_1, class ARG_TYPE_2, class ARG_TYPE_3
#define OS_BIND_FUNC_PARMS ARG_TYPE_1, ARG_TYPE_2, ARG_TYPE_3
#define OS_BIND_FUNC_ARGS arg1, arg2, arg3
#define OS_BIND_FUNC_GET_ARGS  \
	int cur_param_offs = -params; \
	OS_GET_TEMPLATE_ARG(1, ARG_TYPE_1); \
	OS_GET_TEMPLATE_ARG(2, ARG_TYPE_2); \
	OS_GET_TEMPLATE_ARG(3, ARG_TYPE_3)

#elif OS_BIND_FUNC_NUM_ARGS == 4

#define OS_BIND_FUNC_PARMS_COMMA ,
#define OS_BIND_FUNC_TEMPLATE_PARMS class ARG_TYPE_1, class ARG_TYPE_2, class ARG_TYPE_3, class ARG_TYPE_4
#define OS_BIND_FUNC_PARMS ARG_TYPE_1, ARG_TYPE_2, ARG_TYPE_3, ARG_TYPE_4
#define OS_BIND_FUNC_ARGS arg1, arg2, arg3, arg4
#define OS_BIND_FUNC_GET_ARGS  \
	int cur_param_offs = -params; \
	OS_GET_TEMPLATE_ARG(1, ARG_TYPE_1); \
	OS_GET_TEMPLATE_ARG(2, ARG_TYPE_2); \
	OS_GET_TEMPLATE_ARG(3, ARG_TYPE_3); \
	OS_GET_TEMPLATE_ARG(4, ARG_TYPE_4)

#elif OS_BIND_FUNC_NUM_ARGS == 5

#define OS_BIND_FUNC_PARMS_COMMA ,
#define OS_BIND_FUNC_TEMPLATE_PARMS class ARG_TYPE_1, class ARG_TYPE_2, class ARG_TYPE_3, class ARG_TYPE_4, class ARG_TYPE_5
#define OS_BIND_FUNC_PARMS ARG_TYPE_1, ARG_TYPE_2, ARG_TYPE_3, ARG_TYPE_4, ARG_TYPE_5
#define OS_BIND_FUNC_ARGS arg1, arg2, arg3, arg4, arg5
#define OS_BIND_FUNC_GET_ARGS  \
	int cur_param_offs = -params; \
	OS_GET_TEMPLATE_ARG(1, ARG_TYPE_1); \
	OS_GET_TEMPLATE_ARG(2, ARG_TYPE_2); \
	OS_GET_TEMPLATE_ARG(3, ARG_TYPE_3); \
	OS_GET_TEMPLATE_ARG(4, ARG_TYPE_4); \
	OS_GET_TEMPLATE_ARG(5, ARG_TYPE_5)

#elif OS_BIND_FUNC_NUM_ARGS == 6

#define OS_BIND_FUNC_PARMS_COMMA ,
#define OS_BIND_FUNC_TEMPLATE_PARMS class ARG_TYPE_1, class ARG_TYPE_2, class ARG_TYPE_3, class ARG_TYPE_4, class ARG_TYPE_5, class ARG_TYPE_6
#define OS_BIND_FUNC_PARMS ARG_TYPE_1, ARG_TYPE_2, ARG_TYPE_3, ARG_TYPE_4, ARG_TYPE_5, ARG_TYPE_6
#define OS_BIND_FUNC_ARGS arg1, arg2, arg3, arg4, arg5, arg6
#define OS_BIND_FUNC_GET_ARGS  \
	int cur_param_offs = -params; \
	OS_GET_TEMPLATE_ARG(1, ARG_TYPE_1); \
	OS_GET_TEMPLATE_ARG(2, ARG_TYPE_2); \
	OS_GET_TEMPLATE_ARG(3, ARG_TYPE_3); \
	OS_GET_TEMPLATE_ARG(4, ARG_TYPE_4); \
	OS_GET_TEMPLATE_ARG(5, ARG_TYPE_5); \
	OS_GET_TEMPLATE_ARG(6, ARG_TYPE_6)

#elif OS_BIND_FUNC_NUM_ARGS == 7

#define OS_BIND_FUNC_PARMS_COMMA ,
#define OS_BIND_FUNC_TEMPLATE_PARMS class ARG_TYPE_1, class ARG_TYPE_2, class ARG_TYPE_3, class ARG_TYPE_4, class ARG_TYPE_5, class ARG_TYPE_6, class ARG_TYPE_7
#define OS_BIND_FUNC_PARMS ARG_TYPE_1, ARG_TYPE_2, ARG_TYPE_3, ARG_TYPE_4, ARG_TYPE_5, ARG_TYPE_6, ARG_TYPE_7
#define OS_BIND_FUNC_ARGS arg1, arg2, arg3, arg4, arg5, arg6, arg7
#define OS_BIND_FUNC_GET_ARGS  \
	int cur_param_offs = -params; \
	OS_GET_TEMPLATE_ARG(1, ARG_TYPE_1); \
	OS_GET_TEMPLATE_ARG(2, ARG_TYPE_2); \
	OS_GET_TEMPLATE_ARG(3, ARG_TYPE_3); \
	OS_GET_TEMPLATE_ARG(4, ARG_TYPE_4); \
	OS_GET_TEMPLATE_ARG(5, ARG_TYPE_5); \
	OS_GET_TEMPLATE_ARG(6, ARG_TYPE_6); \
	OS_GET_TEMPLATE_ARG(7, ARG_TYPE_7)

#elif OS_BIND_FUNC_NUM_ARGS == 8

#define OS_BIND_FUNC_PARMS_COMMA ,
#define OS_BIND_FUNC_TEMPLATE_PARMS class ARG_TYPE_1, class ARG_TYPE_2, class ARG_TYPE_3, class ARG_TYPE_4, class ARG_TYPE_5, class ARG_TYPE_6, class ARG_TYPE_7, class ARG_TYPE_8
#define OS_BIND_FUNC_PARMS ARG_TYPE_1, ARG_TYPE_2, ARG_TYPE_3, ARG_TYPE_4, ARG_TYPE_5, ARG_TYPE_6, ARG_TYPE_7, ARG_TYPE_8
#define OS_BIND_FUNC_ARGS arg1, arg2, arg3, arg4, arg5, arg6, arg7, arg8
#define OS_BIND_FUNC_GET_ARGS  \
	int cur_param_offs = -params; \
	OS_GET_TEMPLATE_ARG(1, ARG_TYPE_1); \
	OS_GET_TEMPLATE_ARG(2, ARG_TYPE_2); \
	OS_GET_TEMPLATE_ARG(3, ARG_TYPE_3); \
	OS_GET_TEMPLATE_ARG(4, ARG_TYPE_4); \
	OS_GET_TEMPLATE_ARG(5, ARG_TYPE_5); \
	OS_GET_TEMPLATE_ARG(6, ARG_TYPE_6); \
	OS_GET_TEMPLATE_ARG(7, ARG_TYPE_7); \
	OS_GET_TEMPLATE_ARG(8, ARG_TYPE_8)

#elif OS_BIND_FUNC_NUM_ARGS == 9

#define OS_BIND_FUNC_PARMS_COMMA ,
#define OS_BIND_FUNC_TEMPLATE_PARMS class ARG_TYPE_1, class ARG_TYPE_2, class ARG_TYPE_3, class ARG_TYPE_4, class ARG_TYPE_5, class ARG_TYPE_6, class ARG_TYPE_7, class ARG_TYPE_8, class ARG_TYPE_9
#define OS_BIND_FUNC_PARMS ARG_TYPE_1, ARG_TYPE_2, ARG_TYPE_3, ARG_TYPE_4, ARG_TYPE_5, ARG_TYPE_6, ARG_TYPE_7, ARG_TYPE_8, ARG_TYPE_9
#define OS_BIND_FUNC_ARGS arg1, arg2, arg3, arg4, arg5, arg6, arg7, arg8, arg9
#define OS_BIND_FUNC_GET_ARGS  \
	int cur_param_offs = -params; \
	OS_GET_TEMPLATE_ARG(1, ARG_TYPE_1); \
	OS_GET_TEMPLATE_ARG(2, ARG_TYPE_2); \
	OS_GET_TEMPLATE_ARG(3, ARG_TYPE_3); \
	OS_GET_TEMPLATE_ARG(4, ARG_TYPE_4); \
	OS_GET_TEMPLATE_ARG(5, ARG_TYPE_5); \
	OS_GET_TEMPLATE_ARG(6, ARG_TYPE_6); \
	OS_GET_TEMPLATE_ARG(7, ARG_TYPE_7); \
	OS_GET_TEMPLATE_ARG(8, ARG_TYPE_8); \
	OS_GET_TEMPLATE_ARG(9, ARG_TYPE_9)

#elif OS_BIND_FUNC_NUM_ARGS == 10

#define OS_BIND_FUNC_PARMS_COMMA ,
#define OS_BIND_FUNC_TEMPLATE_PARMS class ARG_TYPE_1, class ARG_TYPE_2, class ARG_TYPE_3, class ARG_TYPE_4, class ARG_TYPE_5, class ARG_TYPE_6, class ARG_TYPE_7, class ARG_TYPE_8, class ARG_TYPE_9, class ARG_TYPE_10
#define OS_BIND_FUNC_PARMS ARG_TYPE_1, ARG_TYPE_2, ARG_TYPE_3, ARG_TYPE_4, ARG_TYPE_5, ARG_TYPE_6, ARG_TYPE_7, ARG_TYPE_8, ARG_TYPE_9, ARG_TYPE_10
#define OS_BIND_FUNC_ARGS arg1, arg2, arg3, arg4, arg5, arg6, arg7, arg8, arg9, arg10
#define OS_BIND_FUNC_GET_ARGS  \
	int cur_param_offs = -params; \
	OS_GET_TEMPLATE_ARG(1, ARG_TYPE_1); \
	OS_GET_TEMPLATE_ARG(2, ARG_TYPE_2); \
	OS_GET_TEMPLATE_ARG(3, ARG_TYPE_3); \
	OS_GET_TEMPLATE_ARG(4, ARG_TYPE_4); \
	OS_GET_TEMPLATE_ARG(5, ARG_TYPE_5); \
	OS_GET_TEMPLATE_ARG(6, ARG_TYPE_6); \
	OS_GET_TEMPLATE_ARG(7, ARG_TYPE_7); \
	OS_GET_TEMPLATE_ARG(8, ARG_TYPE_8); \
	OS_GET_TEMPLATE_ARG(9, ARG_TYPE_9); \
	OS_GET_TEMPLATE_ARG(10, ARG_TYPE_10)

#elif OS_BIND_FUNC_NUM_ARGS == 11

#define OS_BIND_FUNC_PARMS_COMMA ,
#define OS_BIND_FUNC_TEMPLATE_PARMS class ARG_TYPE_1, class ARG_TYPE_2, class ARG_TYPE_3, class ARG_TYPE_4, class ARG_TYPE_5, class ARG_TYPE_6, class ARG_TYPE_7, class ARG_TYPE_8, class ARG_TYPE_9, class ARG_TYPE_10, class ARG_TYPE_11
#define OS_BIND_FUNC_PARMS ARG_TYPE_1, ARG_TYPE_2, ARG_TYPE_3, ARG_TYPE_4, ARG_TYPE_5, ARG_TYPE_6, ARG_TYPE_7, ARG_TYPE_8, ARG_TYPE_9, ARG_TYPE_10, ARG_TYPE_11
#define OS_BIND_FUNC_ARGS arg1, arg2, arg3, arg4, arg5, arg6, arg7, arg8, arg9, arg10, arg11
#define OS_BIND_FUNC_GET_ARGS  \
	int cur_param_offs = -params; \
	OS_GET_TEMPLATE_ARG(1, ARG_TYPE_1); \
	OS_GET_TEMPLATE_ARG(2, ARG_TYPE_2); \
	OS_GET_TEMPLATE_ARG(3, ARG_TYPE_3); \
	OS_GET_TEMPLATE_ARG(4, ARG_TYPE_4); \
	OS_GET_TEMPLATE_ARG(5, ARG_TYPE_5); \
	OS_GET_TEMPLATE_ARG(6, ARG_TYPE_6); \
	OS_GET_TEMPLATE_ARG(7, ARG_TYPE_7); \
	OS_GET_TEMPLATE_ARG(8, ARG_TYPE_8); \
	OS_GET_TEMPLATE_ARG(9, ARG_TYPE_9); \
	OS_GET_TEMPLATE_ARG(10, ARG_TYPE_10); \
	OS_GET_TEMPLATE_ARG(11, ARG_TYPE_11)

#elif OS_BIND_FUNC_NUM_ARGS == 12

#define OS_BIND_FUNC_PARMS_COMMA ,
#define OS_BIND_FUNC_TEMPLATE_PARMS class ARG_TYPE_1, class ARG_TYPE_2, class ARG_TYPE_3, class ARG_TYPE_4, class ARG_TYPE_5, class ARG_TYPE_6, class ARG_TYPE_7, class ARG_TYPE_8, class ARG_TYPE_9, class ARG_TYPE_10, class ARG_TYPE_11, class ARG_TYPE_12
#define OS_BIND_FUNC_PARMS ARG_TYPE_1, ARG_TYPE_2, ARG_TYPE_3, ARG_TYPE_4, ARG_TYPE_5, ARG_TYPE_6, ARG_TYPE_7, ARG_TYPE_8, ARG_TYPE_9, ARG_TYPE_10, ARG_TYPE_11, ARG_TYPE_12
#define OS_BIND_FUNC_ARGS arg1, arg2, arg3, arg4, arg5, arg6, arg7, arg8, arg9, arg10, arg11, arg12
#define OS_BIND_FUNC_GET_ARGS  \
	int cur_param_offs = -params; \
	OS_GET_TEMPLATE_ARG(1, ARG_TYPE_1); \
	OS_GET_TEMPLATE_ARG(2, ARG_TYPE_2); \
	OS_GET_TEMPLATE_ARG(3, ARG_TYPE_3); \
	OS_GET_TEMPLATE_ARG(4, ARG_TYPE_4); \
	OS_GET_TEMPLATE_ARG(5, ARG_TYPE_5); \
	OS_GET_TEMPLATE_ARG(6, ARG_TYPE_6); \
	OS_GET_TEMPLATE_ARG(7, ARG_TYPE_7); \
	OS_GET_TEMPLATE_ARG(8, ARG_TYPE_8); \
	OS_GET_TEMPLATE_ARG(9, ARG_TYPE_9); \
	OS_GET_TEMPLATE_ARG(10, ARG_TYPE_10); \
	OS_GET_TEMPLATE_ARG(11, ARG_TYPE_11); \
	OS_GET_TEMPLATE_ARG(12, ARG_TYPE_12)

#elif OS_BIND_FUNC_NUM_ARGS == 13

#define OS_BIND_FUNC_PARMS_COMMA ,
#define OS_BIND_FUNC_TEMPLATE_PARMS class ARG_TYPE_1, class ARG_TYPE_2, class ARG_TYPE_3, class ARG_TYPE_4, class ARG_TYPE_5, class ARG_TYPE_6, class ARG_TYPE_7, class ARG_TYPE_8, class ARG_TYPE_9, class ARG_TYPE_10, class ARG_TYPE_11, class ARG_TYPE_12, class ARG_TYPE_13
#define OS_BIND_FUNC_PARMS ARG_TYPE_1, ARG_TYPE_2, ARG_TYPE_3, ARG_TYPE_4, ARG_TYPE_5, ARG_TYPE_6, ARG_TYPE_7, ARG_TYPE_8, ARG_TYPE_9, ARG_TYPE_10, ARG_TYPE_11, ARG_TYPE_12, ARG_TYPE_13
#define OS_BIND_FUNC_ARGS arg1, arg2, arg3, arg4, arg5, arg6, arg7, arg8, arg9, arg10, arg11, arg12, arg13
#define OS_BIND_FUNC_GET_ARGS  \
	int cur_param_offs = -params; \
	OS_GET_TEMPLATE_ARG(1, ARG_TYPE_1); \
	OS_GET_TEMPLATE_ARG(2, ARG_TYPE_2); \
	OS_GET_TEMPLATE_ARG(3, ARG_TYPE_3); \
	OS_GET_TEMPLATE_ARG(4, ARG_TYPE_4); \
	OS_GET_TEMPLATE_ARG(5, ARG_TYPE_5); \
	OS_GET_TEMPLATE_ARG(6, ARG_TYPE_6); \
	OS_GET_TEMPLATE_ARG(7, ARG_TYPE_7); \
	OS_GET_TEMPLATE_ARG(8, ARG_TYPE_8); \
	OS_GET_TEMPLATE_ARG(9, ARG_TYPE_9); \
	OS_GET_TEMPLATE_ARG(10, ARG_TYPE_10); \
	OS_GET_TEMPLATE_ARG(11, ARG_TYPE_11); \
	OS_GET_TEMPLATE_ARG(12, ARG_TYPE_12); \
	OS_GET_TEMPLATE_ARG(13, ARG_TYPE_13)

#elif OS_BIND_FUNC_NUM_ARGS == 14

#define OS_BIND_FUNC_PARMS_COMMA ,
#define OS_BIND_FUNC_TEMPLATE_PARMS class ARG_TYPE_1, class ARG_TYPE_2, class ARG_TYPE_3, class ARG_TYPE_4, class ARG_TYPE_5, class ARG_TYPE_6, class ARG_TYPE_7, class ARG_TYPE_8, class ARG_TYPE_9, class ARG_TYPE_10, class ARG_TYPE_11, class ARG_TYPE_12, class ARG_TYPE_13, class ARG_TYPE_14
#define OS_BIND_FUNC_PARMS ARG_TYPE_1, ARG_TYPE_2, ARG_TYPE_3, ARG_TYPE_4, ARG_TYPE_5, ARG_TYPE_6, ARG_TYPE_7, ARG_TYPE_8, ARG_TYPE_9, ARG_TYPE_10, ARG_TYPE_11, ARG_TYPE_12, ARG_TYPE_13, ARG_TYPE_14
#define OS_BIND_FUNC_ARGS arg1, arg2, arg3, arg4, arg5, arg6, arg7, arg8, arg9, arg10, arg11, arg12, arg13, arg14
#define OS_BIND_FUNC_GET_ARGS  \
	int cur_param_offs = -params; \
	OS_GET_TEMPLATE_ARG(1, ARG_TYPE_1); \
	OS_GET_TEMPLATE_ARG(2, ARG_TYPE_2); \
	OS_GET_TEMPLATE_ARG(3, ARG_TYPE_3); \
	OS_GET_TEMPLATE_ARG(4, ARG_TYPE_4); \
	OS_GET_TEMPLATE_ARG(5, ARG_TYPE_5); \
	OS_GET_TEMPLATE_ARG(6, ARG_TYPE_6); \
	OS_GET_TEMPLATE_ARG(7, ARG_TYPE_7); \
	OS_GET_TEMPLATE_ARG(8, ARG_TYPE_8); \
	OS_GET_TEMPLATE_ARG(9, ARG_TYPE_9); \
	OS_GET_TEMPLATE_ARG(10, ARG_TYPE_10); \
	OS_GET_TEMPLATE_ARG(11, ARG_TYPE_11); \
	OS_GET_TEMPLATE_ARG(12, ARG_TYPE_12); \
	OS_GET_TEMPLATE_ARG(13, ARG_TYPE_13); \
	OS_GET_TEMPLATE_ARG(14, ARG_TYPE_14)

#elif OS_BIND_FUNC_NUM_ARGS == 15

#define OS_BIND_FUNC_PARMS_COMMA ,
#define OS_BIND_FUNC_TEMPLATE_PARMS class ARG_TYPE_1, class ARG_TYPE_2, class ARG_TYPE_3, class ARG_TYPE_4, class ARG_TYPE_5, class ARG_TYPE_6, class ARG_TYPE_7, class ARG_TYPE_8, class ARG_TYPE_9, class ARG_TYPE_10, class ARG_TYPE_11, class ARG_TYPE_12, class ARG_TYPE_13, class ARG_TYPE_14, class ARG_TYPE_15
#define OS_BIND_FUNC_PARMS ARG_TYPE_1, ARG_TYPE_2, ARG_TYPE_3, ARG_TYPE_4, ARG_TYPE_5, ARG_TYPE_6, ARG_TYPE_7, ARG_TYPE_8, ARG_TYPE_9, ARG_TYPE_10, ARG_TYPE_11, ARG_TYPE_12, ARG_TYPE_13, ARG_TYPE_14, ARG_TYPE_15
#define OS_BIND_FUNC_ARGS arg1, arg2, arg3, arg4, arg5, arg6, arg7, arg8, arg9, arg10, arg11, arg12, arg13, arg14, arg15
#define OS_BIND_FUNC_GET_ARGS  \
	int cur_param_offs = -params; \
	OS_GET_TEMPLATE_ARG(1, ARG_TYPE_1); \
	OS_GET_TEMPLATE_ARG(2, ARG_TYPE_2); \
	OS_GET_TEMPLATE_ARG(3, ARG_TYPE_3); \
	OS_GET_TEMPLATE_ARG(4, ARG_TYPE_4); \
	OS_GET_TEMPLATE_ARG(5, ARG_TYPE_5); \
	OS_GET_TEMPLATE_ARG(6, ARG_TYPE_6); \
	OS_GET_TEMPLATE_ARG(7, ARG_TYPE_7); \
	OS_GET_TEMPLATE_ARG(8, ARG_TYPE_8); \
	OS_GET_TEMPLATE_ARG(9, ARG_TYPE_9); \
	OS_GET_TEMPLATE_ARG(10, ARG_TYPE_10); \
	OS_GET_TEMPLATE_ARG(11, ARG_TYPE_11); \
	OS_GET_TEMPLATE_ARG(12, ARG_TYPE_12); \
	OS_GET_TEMPLATE_ARG(13, ARG_TYPE_13); \
	OS_GET_TEMPLATE_ARG(14, ARG_TYPE_14); \
	OS_GET_TEMPLATE_ARG(15, ARG_TYPE_15)

#endif

#ifdef OS_BIND_FUNC_CDECL

#define OS_BIND_FUNC_CC __cdecl

#if OS_BIND_FUNC_NUM_ARGS == 0
#define OS_BIND_FUNC_CLASS_NAME OS_FunctionClassImpConst0 ## __cdecl
#elif OS_BIND_FUNC_NUM_ARGS == 1
#define OS_BIND_FUNC_CLASS_NAME OS_FunctionClassImpConst1 ## __cdecl
#elif OS_BIND_FUNC_NUM_ARGS == 2
#define OS_BIND_FUNC_CLASS_NAME OS_FunctionClassImpConst2 ## __cdecl
#elif OS_BIND_FUNC_NUM_ARGS == 3
#define OS_BIND_FUNC_CLASS_NAME OS_FunctionClassImpConst3 ## __cdecl
#elif OS_BIND_FUNC_NUM_ARGS == 4
#define OS_BIND_FUNC_CLASS_NAME OS_FunctionClassImpConst4 ## __cdecl
#elif OS_BIND_FUNC_NUM_ARGS == 5
#define OS_BIND_FUNC_CLASS_NAME OS_FunctionClassImpConst5 ## __cdecl
#elif OS_BIND_FUNC_NUM_ARGS == 6
#define OS_BIND_FUNC_CLASS_NAME OS_FunctionClassImpConst6 ## __cdecl
#elif OS_BIND_FUNC_NUM_ARGS == 7
#define OS_BIND_FUNC_CLASS_NAME OS_FunctionClassImpConst7 ## __cdecl
#elif OS_BIND_FUNC_NUM_ARGS == 8
#define OS_BIND_FUNC_CLASS_NAME OS_FunctionClassImpConst8 ## __cdecl
#elif OS_BIND_FUNC_NUM_ARGS == 9
#define OS_BIND_FUNC_CLASS_NAME OS_FunctionClassImpConst9 ## __cdecl
#elif OS_BIND_FUNC_NUM_ARGS == 10
#define OS_BIND_FUNC_CLASS_NAME OS_FunctionClassImpConst10 ## __cdecl
#elif OS_BIND_FUNC_NUM_ARGS == 11
#define OS_BIND_FUNC_CLASS_NAME OS_FunctionClassImpConst11 ## __cdecl
#elif OS_BIND_FUNC_NUM_ARGS == 12
#define OS_BIND_FUNC_CLASS_NAME OS_FunctionClassImpConst12 ## __cdecl
#elif OS_BIND_FUNC_NUM_ARGS == 13
#define OS_BIND_FUNC_CLASS_NAME OS_FunctionClassImpConst13 ## __cdecl
#elif OS_BIND_FUNC_NUM_ARGS == 14
#define OS_BIND_FUNC_CLASS_NAME OS_FunctionClassImpConst14 ## __cdecl
#elif OS_BIND_FUNC_NUM_ARGS == 15
#define OS_BIND_FUNC_CLASS_NAME OS_FunctionClassImpConst15 ## __cdecl
#endif

#elif defined OS_BIND_FUNC_STDCALL

#define OS_BIND_FUNC_CC __stdcall

#if OS_BIND_FUNC_NUM_ARGS == 0
#define OS_BIND_FUNC_CLASS_NAME OS_FunctionClassImpConst0 ## __stdcall
#elif OS_BIND_FUNC_NUM_ARGS == 1
#define OS_BIND_FUNC_CLASS_NAME OS_FunctionClassImpConst1 ## __stdcall
#elif OS_BIND_FUNC_NUM_ARGS == 2
#define OS_BIND_FUNC_CLASS_NAME OS_FunctionClassImpConst2 ## __stdcall
#elif OS_BIND_FUNC_NUM_ARGS == 3
#define OS_BIND_FUNC_CLASS_NAME OS_FunctionClassImpConst3 ## __stdcall
#elif OS_BIND_FUNC_NUM_ARGS == 4
#define OS_BIND_FUNC_CLASS_NAME OS_FunctionClassImpConst4 ## __stdcall
#elif OS_BIND_FUNC_NUM_ARGS == 5
#define OS_BIND_FUNC_CLASS_NAME OS_FunctionClassImpConst5 ## __stdcall
#elif OS_BIND_FUNC_NUM_ARGS == 6
#define OS_BIND_FUNC_CLASS_NAME OS_FunctionClassImpConst6 ## __stdcall
#elif OS_BIND_FUNC_NUM_ARGS == 7
#define OS_BIND_FUNC_CLASS_NAME OS_FunctionClassImpConst7 ## __stdcall
#elif OS_BIND_FUNC_NUM_ARGS == 8
#define OS_BIND_FUNC_CLASS_NAME OS_FunctionClassImpConst8 ## __stdcall
#elif OS_BIND_FUNC_NUM_ARGS == 9
#define OS_BIND_FUNC_CLASS_NAME OS_FunctionClassImpConst9 ## __stdcall
#elif OS_BIND_FUNC_NUM_ARGS == 10
#define OS_BIND_FUNC_CLASS_NAME OS_FunctionClassImpConst10 ## __stdcall
#elif OS_BIND_FUNC_NUM_ARGS == 11
#define OS_BIND_FUNC_CLASS_NAME OS_FunctionClassImpConst11 ## __stdcall
#elif OS_BIND_FUNC_NUM_ARGS == 12
#define OS_BIND_FUNC_CLASS_NAME OS_FunctionClassImpConst12 ## __stdcall
#elif OS_BIND_FUNC_NUM_ARGS == 13
#define OS_BIND_FUNC_CLASS_NAME OS_FunctionClassImpConst13 ## __stdcall
#elif OS_BIND_FUNC_NUM_ARGS == 14
#define OS_BIND_FUNC_CLASS_NAME OS_FunctionClassImpConst14 ## __stdcall
#elif OS_BIND_FUNC_NUM_ARGS == 15
#define OS_BIND_FUNC_CLASS_NAME OS_FunctionClassImpConst15 ## __stdcall
#endif

#elif defined OS_BIND_FUNC_FASTCALL

#define OS_BIND_FUNC_CC __fastcall

#if OS_BIND_FUNC_NUM_ARGS == 0
#define OS_BIND_FUNC_CLASS_NAME OS_FunctionClassImpConst0 ## __fastcall
#elif OS_BIND_FUNC_NUM_ARGS == 1
#define OS_BIND_FUNC_CLASS_NAME OS_FunctionClassImpConst1 ## __fastcall
#elif OS_BIND_FUNC_NUM_ARGS == 2
#define OS_BIND_FUNC_CLASS_NAME OS_FunctionClassImpConst2 ## __fastcall
#elif OS_BIND_FUNC_NUM_ARGS == 3
#define OS_BIND_FUNC_CLASS_NAME OS_FunctionClassImpConst3 ## __fastcall
#elif OS_BIND_FUNC_NUM_ARGS == 4
#define OS_BIND_FUNC_CLASS_NAME OS_FunctionClassImpConst4 ## __fastcall
#elif OS_BIND_FUNC_NUM_ARGS == 5
#define OS_BIND_FUNC_CLASS_NAME OS_FunctionClassImpConst5 ## __fastcall
#elif OS_BIND_FUNC_NUM_ARGS == 6
#define OS_BIND_FUNC_CLASS_NAME OS_FunctionClassImpConst6 ## __fastcall
#elif OS_BIND_FUNC_NUM_ARGS == 7
#define OS_BIND_FUNC_CLASS_NAME OS_FunctionClassImpConst7 ## __fastcall
#elif OS_BIND_FUNC_NUM_ARGS == 8
#define OS_BIND_FUNC_CLASS_NAME OS_FunctionClassImpConst8 ## __fastcall
#elif OS_BIND_FUNC_NUM_ARGS == 9
#define OS_BIND_FUNC_CLASS_NAME OS_FunctionClassImpConst9 ## __fastcall
#elif OS_BIND_FUNC_NUM_ARGS == 10
#define OS_BIND_FUNC_CLASS_NAME OS_FunctionClassImpConst10 ## __fastcall
#elif OS_BIND_FUNC_NUM_ARGS == 11
#define OS_BIND_FUNC_CLASS_NAME OS_FunctionClassImpConst11 ## __fastcall
#elif OS_BIND_FUNC_NUM_ARGS == 12
#define OS_BIND_FUNC_CLASS_NAME OS_FunctionClassImpConst12 ## __fastcall
#elif OS_BIND_FUNC_NUM_ARGS == 13
#define OS_BIND_FUNC_CLASS_NAME OS_FunctionClassImpConst13 ## __fastcall
#elif OS_BIND_FUNC_NUM_ARGS == 14
#define OS_BIND_FUNC_CLASS_NAME OS_FunctionClassImpConst14 ## __fastcall
#elif OS_BIND_FUNC_NUM_ARGS == 15
#define OS_BIND_FUNC_CLASS_NAME OS_FunctionClassImpConst15 ## __fastcall
#endif

#elif defined OS_BIND_FUNC_THISCALL

#define OS_BIND_FUNC_CC __thiscall

#if OS_BIND_FUNC_NUM_ARGS == 0
#define OS_BIND_FUNC_CLASS_NAME OS_FunctionClassImpConst0 ## __thiscall
#elif OS_BIND_FUNC_NUM_ARGS == 1
#define OS_BIND_FUNC_CLASS_NAME OS_FunctionClassImpConst1 ## __thiscall
#elif OS_BIND_FUNC_NUM_ARGS == 2
#define OS_BIND_FUNC_CLASS_NAME OS_FunctionClassImpConst2 ## __thiscall
#elif OS_BIND_FUNC_NUM_ARGS == 3
#define OS_BIND_FUNC_CLASS_NAME OS_FunctionClassImpConst3 ## __thiscall
#elif OS_BIND_FUNC_NUM_ARGS == 4
#define OS_BIND_FUNC_CLASS_NAME OS_FunctionClassImpConst4 ## __thiscall
#elif OS_BIND_FUNC_NUM_ARGS == 5
#define OS_BIND_FUNC_CLASS_NAME OS_FunctionClassImpConst5 ## __thiscall
#elif OS_BIND_FUNC_NUM_ARGS == 6
#define OS_BIND_FUNC_CLASS_NAME OS_FunctionClassImpConst6 ## __thiscall
#elif OS_BIND_FUNC_NUM_ARGS == 7
#define OS_BIND_FUNC_CLASS_NAME OS_FunctionClassImpConst7 ## __thiscall
#elif OS_BIND_FUNC_NUM_ARGS == 8
#define OS_BIND_FUNC_CLASS_NAME OS_FunctionClassImpConst8 ## __thiscall
#elif OS_BIND_FUNC_NUM_ARGS == 9
#define OS_BIND_FUNC_CLASS_NAME OS_FunctionClassImpConst9 ## __thiscall
#elif OS_BIND_FUNC_NUM_ARGS == 10
#define OS_BIND_FUNC_CLASS_NAME OS_FunctionClassImpConst10 ## __thiscall
#elif OS_BIND_FUNC_NUM_ARGS == 11
#define OS_BIND_FUNC_CLASS_NAME OS_FunctionClassImpConst11 ## __thiscall
#elif OS_BIND_FUNC_NUM_ARGS == 12
#define OS_BIND_FUNC_CLASS_NAME OS_FunctionClassImpConst12 ## __thiscall
#elif OS_BIND_FUNC_NUM_ARGS == 13
#define OS_BIND_FUNC_CLASS_NAME OS_FunctionClassImpConst13 ## __thiscall
#elif OS_BIND_FUNC_NUM_ARGS == 14
#define OS_BIND_FUNC_CLASS_NAME OS_FunctionClassImpConst14 ## __thiscall
#elif OS_BIND_FUNC_NUM_ARGS == 15
#define OS_BIND_FUNC_CLASS_NAME OS_FunctionClassImpConst15 ## __thiscall
#endif

#endif

template <class R, class T OS_BIND_FUNC_PARMS_COMMA OS_BIND_FUNC_TEMPLATE_PARMS>
struct OS_BIND_FUNC_CLASS_NAME
{
	typedef R(OS_BIND_FUNC_CC T::*F)(OS_BIND_FUNC_PARMS)const;

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
ObjectScript::OS::FuncDef def(const char * name, R(OS_BIND_FUNC_CC T::*f)(OS_BIND_FUNC_PARMS)const)
{
	typedef OS_BIND_FUNC_CLASS_NAME<R, T OS_BIND_FUNC_PARMS_COMMA OS_BIND_FUNC_PARMS> Func; 
	return Func(name, f);
}

} // namespace ObjectScript

#undef OS_BIND_FUNC_CLASS_NAME
#undef OS_BIND_FUNC_CC

#ifdef OS_BIND_FUNC_CDECL

#define OS_BIND_FUNC_CC __cdecl

#if OS_BIND_FUNC_NUM_ARGS == 0
#define OS_BIND_FUNC_CLASS_NAME OS_FunctionClassImp0 ## __cdecl
#elif OS_BIND_FUNC_NUM_ARGS == 1
#define OS_BIND_FUNC_CLASS_NAME OS_FunctionClassImp1 ## __cdecl
#elif OS_BIND_FUNC_NUM_ARGS == 2
#define OS_BIND_FUNC_CLASS_NAME OS_FunctionClassImp2 ## __cdecl
#elif OS_BIND_FUNC_NUM_ARGS == 3
#define OS_BIND_FUNC_CLASS_NAME OS_FunctionClassImp3 ## __cdecl
#elif OS_BIND_FUNC_NUM_ARGS == 4
#define OS_BIND_FUNC_CLASS_NAME OS_FunctionClassImp4 ## __cdecl
#elif OS_BIND_FUNC_NUM_ARGS == 5
#define OS_BIND_FUNC_CLASS_NAME OS_FunctionClassImp5 ## __cdecl
#elif OS_BIND_FUNC_NUM_ARGS == 6
#define OS_BIND_FUNC_CLASS_NAME OS_FunctionClassImp6 ## __cdecl
#elif OS_BIND_FUNC_NUM_ARGS == 7
#define OS_BIND_FUNC_CLASS_NAME OS_FunctionClassImp7 ## __cdecl
#elif OS_BIND_FUNC_NUM_ARGS == 8
#define OS_BIND_FUNC_CLASS_NAME OS_FunctionClassImp8 ## __cdecl
#elif OS_BIND_FUNC_NUM_ARGS == 9
#define OS_BIND_FUNC_CLASS_NAME OS_FunctionClassImp9 ## __cdecl
#elif OS_BIND_FUNC_NUM_ARGS == 10
#define OS_BIND_FUNC_CLASS_NAME OS_FunctionClassImp10 ## __cdecl
#elif OS_BIND_FUNC_NUM_ARGS == 11
#define OS_BIND_FUNC_CLASS_NAME OS_FunctionClassImp11 ## __cdecl
#elif OS_BIND_FUNC_NUM_ARGS == 12
#define OS_BIND_FUNC_CLASS_NAME OS_FunctionClassImp12 ## __cdecl
#elif OS_BIND_FUNC_NUM_ARGS == 13
#define OS_BIND_FUNC_CLASS_NAME OS_FunctionClassImp13 ## __cdecl
#elif OS_BIND_FUNC_NUM_ARGS == 14
#define OS_BIND_FUNC_CLASS_NAME OS_FunctionClassImp14 ## __cdecl
#elif OS_BIND_FUNC_NUM_ARGS == 15
#define OS_BIND_FUNC_CLASS_NAME OS_FunctionClassImp15 ## __cdecl
#endif

#elif defined OS_BIND_FUNC_STDCALL

#define OS_BIND_FUNC_CC __stdcall

#if OS_BIND_FUNC_NUM_ARGS == 0
#define OS_BIND_FUNC_CLASS_NAME OS_FunctionClassImp0 ## __stdcall
#elif OS_BIND_FUNC_NUM_ARGS == 1
#define OS_BIND_FUNC_CLASS_NAME OS_FunctionClassImp1 ## __stdcall
#elif OS_BIND_FUNC_NUM_ARGS == 2
#define OS_BIND_FUNC_CLASS_NAME OS_FunctionClassImp2 ## __stdcall
#elif OS_BIND_FUNC_NUM_ARGS == 3
#define OS_BIND_FUNC_CLASS_NAME OS_FunctionClassImp3 ## __stdcall
#elif OS_BIND_FUNC_NUM_ARGS == 4
#define OS_BIND_FUNC_CLASS_NAME OS_FunctionClassImp4 ## __stdcall
#elif OS_BIND_FUNC_NUM_ARGS == 5
#define OS_BIND_FUNC_CLASS_NAME OS_FunctionClassImp5 ## __stdcall
#elif OS_BIND_FUNC_NUM_ARGS == 6
#define OS_BIND_FUNC_CLASS_NAME OS_FunctionClassImp6 ## __stdcall
#elif OS_BIND_FUNC_NUM_ARGS == 7
#define OS_BIND_FUNC_CLASS_NAME OS_FunctionClassImp7 ## __stdcall
#elif OS_BIND_FUNC_NUM_ARGS == 8
#define OS_BIND_FUNC_CLASS_NAME OS_FunctionClassImp8 ## __stdcall
#elif OS_BIND_FUNC_NUM_ARGS == 9
#define OS_BIND_FUNC_CLASS_NAME OS_FunctionClassImp9 ## __stdcall
#elif OS_BIND_FUNC_NUM_ARGS == 10
#define OS_BIND_FUNC_CLASS_NAME OS_FunctionClassImp10 ## __stdcall
#elif OS_BIND_FUNC_NUM_ARGS == 11
#define OS_BIND_FUNC_CLASS_NAME OS_FunctionClassImp11 ## __stdcall
#elif OS_BIND_FUNC_NUM_ARGS == 12
#define OS_BIND_FUNC_CLASS_NAME OS_FunctionClassImp12 ## __stdcall
#elif OS_BIND_FUNC_NUM_ARGS == 13
#define OS_BIND_FUNC_CLASS_NAME OS_FunctionClassImp13 ## __stdcall
#elif OS_BIND_FUNC_NUM_ARGS == 14
#define OS_BIND_FUNC_CLASS_NAME OS_FunctionClassImp14 ## __stdcall
#elif OS_BIND_FUNC_NUM_ARGS == 15
#define OS_BIND_FUNC_CLASS_NAME OS_FunctionClassImp15 ## __stdcall
#endif

#elif defined OS_BIND_FUNC_FASTCALL

#define OS_BIND_FUNC_CC __fastcall

#if OS_BIND_FUNC_NUM_ARGS == 0
#define OS_BIND_FUNC_CLASS_NAME OS_FunctionClassImp0 ## __fastcall
#elif OS_BIND_FUNC_NUM_ARGS == 1
#define OS_BIND_FUNC_CLASS_NAME OS_FunctionClassImp1 ## __fastcall
#elif OS_BIND_FUNC_NUM_ARGS == 2
#define OS_BIND_FUNC_CLASS_NAME OS_FunctionClassImp2 ## __fastcall
#elif OS_BIND_FUNC_NUM_ARGS == 3
#define OS_BIND_FUNC_CLASS_NAME OS_FunctionClassImp3 ## __fastcall
#elif OS_BIND_FUNC_NUM_ARGS == 4
#define OS_BIND_FUNC_CLASS_NAME OS_FunctionClassImp4 ## __fastcall
#elif OS_BIND_FUNC_NUM_ARGS == 5
#define OS_BIND_FUNC_CLASS_NAME OS_FunctionClassImp5 ## __fastcall
#elif OS_BIND_FUNC_NUM_ARGS == 6
#define OS_BIND_FUNC_CLASS_NAME OS_FunctionClassImp6 ## __fastcall
#elif OS_BIND_FUNC_NUM_ARGS == 7
#define OS_BIND_FUNC_CLASS_NAME OS_FunctionClassImp7 ## __fastcall
#elif OS_BIND_FUNC_NUM_ARGS == 8
#define OS_BIND_FUNC_CLASS_NAME OS_FunctionClassImp8 ## __fastcall
#elif OS_BIND_FUNC_NUM_ARGS == 9
#define OS_BIND_FUNC_CLASS_NAME OS_FunctionClassImp9 ## __fastcall
#elif OS_BIND_FUNC_NUM_ARGS == 10
#define OS_BIND_FUNC_CLASS_NAME OS_FunctionClassImp10 ## __fastcall
#elif OS_BIND_FUNC_NUM_ARGS == 11
#define OS_BIND_FUNC_CLASS_NAME OS_FunctionClassImp11 ## __fastcall
#elif OS_BIND_FUNC_NUM_ARGS == 12
#define OS_BIND_FUNC_CLASS_NAME OS_FunctionClassImp12 ## __fastcall
#elif OS_BIND_FUNC_NUM_ARGS == 13
#define OS_BIND_FUNC_CLASS_NAME OS_FunctionClassImp13 ## __fastcall
#elif OS_BIND_FUNC_NUM_ARGS == 14
#define OS_BIND_FUNC_CLASS_NAME OS_FunctionClassImp14 ## __fastcall
#elif OS_BIND_FUNC_NUM_ARGS == 15
#define OS_BIND_FUNC_CLASS_NAME OS_FunctionClassImp15 ## __fastcall
#endif

#elif defined OS_BIND_FUNC_THISCALL

#define OS_BIND_FUNC_CC __thiscall

#if OS_BIND_FUNC_NUM_ARGS == 0
#define OS_BIND_FUNC_CLASS_NAME OS_FunctionClassImp0 ## __thiscall
#elif OS_BIND_FUNC_NUM_ARGS == 1
#define OS_BIND_FUNC_CLASS_NAME OS_FunctionClassImp1 ## __thiscall
#elif OS_BIND_FUNC_NUM_ARGS == 2
#define OS_BIND_FUNC_CLASS_NAME OS_FunctionClassImp2 ## __thiscall
#elif OS_BIND_FUNC_NUM_ARGS == 3
#define OS_BIND_FUNC_CLASS_NAME OS_FunctionClassImp3 ## __thiscall
#elif OS_BIND_FUNC_NUM_ARGS == 4
#define OS_BIND_FUNC_CLASS_NAME OS_FunctionClassImp4 ## __thiscall
#elif OS_BIND_FUNC_NUM_ARGS == 5
#define OS_BIND_FUNC_CLASS_NAME OS_FunctionClassImp5 ## __thiscall
#elif OS_BIND_FUNC_NUM_ARGS == 6
#define OS_BIND_FUNC_CLASS_NAME OS_FunctionClassImp6 ## __thiscall
#elif OS_BIND_FUNC_NUM_ARGS == 7
#define OS_BIND_FUNC_CLASS_NAME OS_FunctionClassImp7 ## __thiscall
#elif OS_BIND_FUNC_NUM_ARGS == 8
#define OS_BIND_FUNC_CLASS_NAME OS_FunctionClassImp8 ## __thiscall
#elif OS_BIND_FUNC_NUM_ARGS == 9
#define OS_BIND_FUNC_CLASS_NAME OS_FunctionClassImp9 ## __thiscall
#elif OS_BIND_FUNC_NUM_ARGS == 10
#define OS_BIND_FUNC_CLASS_NAME OS_FunctionClassImp10 ## __thiscall
#elif OS_BIND_FUNC_NUM_ARGS == 11
#define OS_BIND_FUNC_CLASS_NAME OS_FunctionClassImp11 ## __thiscall
#elif OS_BIND_FUNC_NUM_ARGS == 12
#define OS_BIND_FUNC_CLASS_NAME OS_FunctionClassImp12 ## __thiscall
#elif OS_BIND_FUNC_NUM_ARGS == 13
#define OS_BIND_FUNC_CLASS_NAME OS_FunctionClassImp13 ## __thiscall
#elif OS_BIND_FUNC_NUM_ARGS == 14
#define OS_BIND_FUNC_CLASS_NAME OS_FunctionClassImp14 ## __thiscall
#elif OS_BIND_FUNC_NUM_ARGS == 15
#define OS_BIND_FUNC_CLASS_NAME OS_FunctionClassImp15 ## __thiscall
#endif

#endif

template <class R, class T OS_BIND_FUNC_PARMS_COMMA OS_BIND_FUNC_TEMPLATE_PARMS>
struct OS_BIND_FUNC_CLASS_NAME
{
	typedef R(OS_BIND_FUNC_CC T::*F)(OS_BIND_FUNC_PARMS);

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
ObjectScript::OS::FuncDef def(const char * name, R(OS_BIND_FUNC_CC T::*f)(OS_BIND_FUNC_PARMS))
{
	typedef OS_BIND_FUNC_CLASS_NAME<R, T OS_BIND_FUNC_PARMS_COMMA OS_BIND_FUNC_PARMS> Func; 
	return Func(name, f);
}

} // namespace ObjectScript

#undef OS_BIND_FUNC_CLASS_NAME
#undef OS_BIND_FUNC_CC

#ifdef OS_BIND_FUNC_CDECL

#define OS_BIND_FUNC_CC __cdecl

#if OS_BIND_FUNC_NUM_ARGS == 0
#define OS_BIND_FUNC_CLASS_NAME OS_FunctionImp0 ## __cdecl
#elif OS_BIND_FUNC_NUM_ARGS == 1
#define OS_BIND_FUNC_CLASS_NAME OS_FunctionImp1 ## __cdecl
#elif OS_BIND_FUNC_NUM_ARGS == 2
#define OS_BIND_FUNC_CLASS_NAME OS_FunctionImp2 ## __cdecl
#elif OS_BIND_FUNC_NUM_ARGS == 3
#define OS_BIND_FUNC_CLASS_NAME OS_FunctionImp3 ## __cdecl
#elif OS_BIND_FUNC_NUM_ARGS == 4
#define OS_BIND_FUNC_CLASS_NAME OS_FunctionImp4 ## __cdecl
#elif OS_BIND_FUNC_NUM_ARGS == 5
#define OS_BIND_FUNC_CLASS_NAME OS_FunctionImp5 ## __cdecl
#elif OS_BIND_FUNC_NUM_ARGS == 6
#define OS_BIND_FUNC_CLASS_NAME OS_FunctionImp6 ## __cdecl
#elif OS_BIND_FUNC_NUM_ARGS == 7
#define OS_BIND_FUNC_CLASS_NAME OS_FunctionImp7 ## __cdecl
#elif OS_BIND_FUNC_NUM_ARGS == 8
#define OS_BIND_FUNC_CLASS_NAME OS_FunctionImp8 ## __cdecl
#elif OS_BIND_FUNC_NUM_ARGS == 9
#define OS_BIND_FUNC_CLASS_NAME OS_FunctionImp9 ## __cdecl
#elif OS_BIND_FUNC_NUM_ARGS == 10
#define OS_BIND_FUNC_CLASS_NAME OS_FunctionImp10 ## __cdecl
#elif OS_BIND_FUNC_NUM_ARGS == 11
#define OS_BIND_FUNC_CLASS_NAME OS_FunctionImp11 ## __cdecl
#elif OS_BIND_FUNC_NUM_ARGS == 12
#define OS_BIND_FUNC_CLASS_NAME OS_FunctionImp12 ## __cdecl
#elif OS_BIND_FUNC_NUM_ARGS == 13
#define OS_BIND_FUNC_CLASS_NAME OS_FunctionImp13 ## __cdecl
#elif OS_BIND_FUNC_NUM_ARGS == 14
#define OS_BIND_FUNC_CLASS_NAME OS_FunctionImp14 ## __cdecl
#elif OS_BIND_FUNC_NUM_ARGS == 15
#define OS_BIND_FUNC_CLASS_NAME OS_FunctionImp15 ## __cdecl
#endif

#elif defined OS_BIND_FUNC_STDCALL

#define OS_BIND_FUNC_CC __stdcall

#if OS_BIND_FUNC_NUM_ARGS == 0
#define OS_BIND_FUNC_CLASS_NAME OS_FunctionImp0 ## __stdcall
#elif OS_BIND_FUNC_NUM_ARGS == 1
#define OS_BIND_FUNC_CLASS_NAME OS_FunctionImp1 ## __stdcall
#elif OS_BIND_FUNC_NUM_ARGS == 2
#define OS_BIND_FUNC_CLASS_NAME OS_FunctionImp2 ## __stdcall
#elif OS_BIND_FUNC_NUM_ARGS == 3
#define OS_BIND_FUNC_CLASS_NAME OS_FunctionImp3 ## __stdcall
#elif OS_BIND_FUNC_NUM_ARGS == 4
#define OS_BIND_FUNC_CLASS_NAME OS_FunctionImp4 ## __stdcall
#elif OS_BIND_FUNC_NUM_ARGS == 5
#define OS_BIND_FUNC_CLASS_NAME OS_FunctionImp5 ## __stdcall
#elif OS_BIND_FUNC_NUM_ARGS == 6
#define OS_BIND_FUNC_CLASS_NAME OS_FunctionImp6 ## __stdcall
#elif OS_BIND_FUNC_NUM_ARGS == 7
#define OS_BIND_FUNC_CLASS_NAME OS_FunctionImp7 ## __stdcall
#elif OS_BIND_FUNC_NUM_ARGS == 8
#define OS_BIND_FUNC_CLASS_NAME OS_FunctionImp8 ## __stdcall
#elif OS_BIND_FUNC_NUM_ARGS == 9
#define OS_BIND_FUNC_CLASS_NAME OS_FunctionImp9 ## __stdcall
#elif OS_BIND_FUNC_NUM_ARGS == 10
#define OS_BIND_FUNC_CLASS_NAME OS_FunctionImp10 ## __stdcall
#elif OS_BIND_FUNC_NUM_ARGS == 11
#define OS_BIND_FUNC_CLASS_NAME OS_FunctionImp11 ## __stdcall
#elif OS_BIND_FUNC_NUM_ARGS == 12
#define OS_BIND_FUNC_CLASS_NAME OS_FunctionImp12 ## __stdcall
#elif OS_BIND_FUNC_NUM_ARGS == 13
#define OS_BIND_FUNC_CLASS_NAME OS_FunctionImp13 ## __stdcall
#elif OS_BIND_FUNC_NUM_ARGS == 14
#define OS_BIND_FUNC_CLASS_NAME OS_FunctionImp14 ## __stdcall
#elif OS_BIND_FUNC_NUM_ARGS == 15
#define OS_BIND_FUNC_CLASS_NAME OS_FunctionImp15 ## __stdcall
#endif

#elif defined OS_BIND_FUNC_FASTCALL

#define OS_BIND_FUNC_CC __fastcall

#if OS_BIND_FUNC_NUM_ARGS == 0
#define OS_BIND_FUNC_CLASS_NAME OS_FunctionImp0 ## __fastcall
#elif OS_BIND_FUNC_NUM_ARGS == 1
#define OS_BIND_FUNC_CLASS_NAME OS_FunctionImp1 ## __fastcall
#elif OS_BIND_FUNC_NUM_ARGS == 2
#define OS_BIND_FUNC_CLASS_NAME OS_FunctionImp2 ## __fastcall
#elif OS_BIND_FUNC_NUM_ARGS == 3
#define OS_BIND_FUNC_CLASS_NAME OS_FunctionImp3 ## __fastcall
#elif OS_BIND_FUNC_NUM_ARGS == 4
#define OS_BIND_FUNC_CLASS_NAME OS_FunctionImp4 ## __fastcall
#elif OS_BIND_FUNC_NUM_ARGS == 5
#define OS_BIND_FUNC_CLASS_NAME OS_FunctionImp5 ## __fastcall
#elif OS_BIND_FUNC_NUM_ARGS == 6
#define OS_BIND_FUNC_CLASS_NAME OS_FunctionImp6 ## __fastcall
#elif OS_BIND_FUNC_NUM_ARGS == 7
#define OS_BIND_FUNC_CLASS_NAME OS_FunctionImp7 ## __fastcall
#elif OS_BIND_FUNC_NUM_ARGS == 8
#define OS_BIND_FUNC_CLASS_NAME OS_FunctionImp8 ## __fastcall
#elif OS_BIND_FUNC_NUM_ARGS == 9
#define OS_BIND_FUNC_CLASS_NAME OS_FunctionImp9 ## __fastcall
#elif OS_BIND_FUNC_NUM_ARGS == 10
#define OS_BIND_FUNC_CLASS_NAME OS_FunctionImp10 ## __fastcall
#elif OS_BIND_FUNC_NUM_ARGS == 11
#define OS_BIND_FUNC_CLASS_NAME OS_FunctionImp11 ## __fastcall
#elif OS_BIND_FUNC_NUM_ARGS == 12
#define OS_BIND_FUNC_CLASS_NAME OS_FunctionImp12 ## __fastcall
#elif OS_BIND_FUNC_NUM_ARGS == 13
#define OS_BIND_FUNC_CLASS_NAME OS_FunctionImp13 ## __fastcall
#elif OS_BIND_FUNC_NUM_ARGS == 14
#define OS_BIND_FUNC_CLASS_NAME OS_FunctionImp14 ## __fastcall
#elif OS_BIND_FUNC_NUM_ARGS == 15
#define OS_BIND_FUNC_CLASS_NAME OS_FunctionImp15 ## __fastcall
#endif

#elif defined OS_BIND_FUNC_THISCALL

#define OS_BIND_FUNC_CC __thiscall

#if OS_BIND_FUNC_NUM_ARGS == 0
#define OS_BIND_FUNC_CLASS_NAME OS_FunctionImp0 ## __thiscall
#elif OS_BIND_FUNC_NUM_ARGS == 1
#define OS_BIND_FUNC_CLASS_NAME OS_FunctionImp1 ## __thiscall
#elif OS_BIND_FUNC_NUM_ARGS == 2
#define OS_BIND_FUNC_CLASS_NAME OS_FunctionImp2 ## __thiscall
#elif OS_BIND_FUNC_NUM_ARGS == 3
#define OS_BIND_FUNC_CLASS_NAME OS_FunctionImp3 ## __thiscall
#elif OS_BIND_FUNC_NUM_ARGS == 4
#define OS_BIND_FUNC_CLASS_NAME OS_FunctionImp4 ## __thiscall
#elif OS_BIND_FUNC_NUM_ARGS == 5
#define OS_BIND_FUNC_CLASS_NAME OS_FunctionImp5 ## __thiscall
#elif OS_BIND_FUNC_NUM_ARGS == 6
#define OS_BIND_FUNC_CLASS_NAME OS_FunctionImp6 ## __thiscall
#elif OS_BIND_FUNC_NUM_ARGS == 7
#define OS_BIND_FUNC_CLASS_NAME OS_FunctionImp7 ## __thiscall
#elif OS_BIND_FUNC_NUM_ARGS == 8
#define OS_BIND_FUNC_CLASS_NAME OS_FunctionImp8 ## __thiscall
#elif OS_BIND_FUNC_NUM_ARGS == 9
#define OS_BIND_FUNC_CLASS_NAME OS_FunctionImp9 ## __thiscall
#elif OS_BIND_FUNC_NUM_ARGS == 10
#define OS_BIND_FUNC_CLASS_NAME OS_FunctionImp10 ## __thiscall
#elif OS_BIND_FUNC_NUM_ARGS == 11
#define OS_BIND_FUNC_CLASS_NAME OS_FunctionImp11 ## __thiscall
#elif OS_BIND_FUNC_NUM_ARGS == 12
#define OS_BIND_FUNC_CLASS_NAME OS_FunctionImp12 ## __thiscall
#elif OS_BIND_FUNC_NUM_ARGS == 13
#define OS_BIND_FUNC_CLASS_NAME OS_FunctionImp13 ## __thiscall
#elif OS_BIND_FUNC_NUM_ARGS == 14
#define OS_BIND_FUNC_CLASS_NAME OS_FunctionImp14 ## __thiscall
#elif OS_BIND_FUNC_NUM_ARGS == 15
#define OS_BIND_FUNC_CLASS_NAME OS_FunctionImp15 ## __thiscall
#endif

#endif

#if OS_BIND_FUNC_NUM_ARGS > 0

template <class R OS_BIND_FUNC_PARMS_COMMA OS_BIND_FUNC_TEMPLATE_PARMS>
struct OS_BIND_FUNC_CLASS_NAME
{
	typedef R(OS_BIND_FUNC_CC *F)(OS_BIND_FUNC_PARMS);

	const char * name;
	F f;

	OS_BIND_FUNC_CLASS_NAME(const char * _name, F _f): name(_name), f(_f){}

	template <class R>
	static int run(ObjectScript::OS * os, int params, int, int, void * user_param)
	{
		OS_BIND_FUNC_GET_ARGS;
		typedef typename RemoveConst<R>::type type;
		F& f = *(F*)user_param;
		// CtypeValue<type>::push(os, CtypeValue<type>::to((*f)(OS_BIND_FUNC_ARGS)));
		CtypeValue<type>::push(os, (*f)(OS_BIND_FUNC_ARGS));
		return 1;
	}

	template <>
	static int run<void>(ObjectScript::OS * os, int params, int, int, void * user_param)
	{
		OS_BIND_FUNC_GET_ARGS;
		typedef typename RemoveConst<R>::type type;
		F& f = *(F*)user_param;
		(*f)(OS_BIND_FUNC_ARGS);
		return 0;
	}

	operator ObjectScript::OS::FuncDef() const 
	{ 
		ObjectScript::OS::FuncDef def = {name, run<R>, &(new OS_FunctionData<F>(f))->f}; 
		return def; 
	}
};

namespace ObjectScript {

template <class R OS_BIND_FUNC_PARMS_COMMA OS_BIND_FUNC_TEMPLATE_PARMS> 
ObjectScript::OS::FuncDef def(const char * name, R(OS_BIND_FUNC_CC *f)(OS_BIND_FUNC_PARMS))
{
	typedef OS_BIND_FUNC_CLASS_NAME<R OS_BIND_FUNC_PARMS_COMMA OS_BIND_FUNC_PARMS> Func; 
	return Func(name, f);
}

} // namespace ObjectScript

#else

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

#endif



#undef OS_BIND_FUNC_CLASS_NAME
#undef OS_BIND_FUNC_CC

#undef OS_BIND_FUNC_PARMS_COMMA
#undef OS_BIND_FUNC_TEMPLATE_PARMS
#undef OS_BIND_FUNC_PARMS
#undef OS_BIND_FUNC_ARGS
#undef OS_BIND_FUNC_GET_ARGS
