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

#ifdef __GNUC__
#define OS_BIND_FUNC_CC
#define OS_BIND_FUNC_CC_GNUC __attribute__((cdecl))
#else
#define OS_BIND_FUNC_CC __cdecl
#define OS_BIND_FUNC_CC_GNUC
#endif

#if OS_BIND_FUNC_NUM_ARGS == 0
#define OS_BIND_FUNC_CLASS_NAME OS_FunctionClassImpConst0 ## cdecl
#define OS_BIND_FUNC_RUN_CLASS_NAME OS_FunctionClassImpConst0_run ## cdecl
#elif OS_BIND_FUNC_NUM_ARGS == 1
#define OS_BIND_FUNC_CLASS_NAME OS_FunctionClassImpConst1 ## cdecl
#define OS_BIND_FUNC_RUN_CLASS_NAME OS_FunctionClassImpConst1_run ## cdecl
#elif OS_BIND_FUNC_NUM_ARGS == 2
#define OS_BIND_FUNC_CLASS_NAME OS_FunctionClassImpConst2 ## cdecl
#define OS_BIND_FUNC_RUN_CLASS_NAME OS_FunctionClassImpConst2_run ## cdecl
#elif OS_BIND_FUNC_NUM_ARGS == 3
#define OS_BIND_FUNC_CLASS_NAME OS_FunctionClassImpConst3 ## cdecl
#define OS_BIND_FUNC_RUN_CLASS_NAME OS_FunctionClassImpConst3_run ## cdecl
#elif OS_BIND_FUNC_NUM_ARGS == 4
#define OS_BIND_FUNC_CLASS_NAME OS_FunctionClassImpConst4 ## cdecl
#define OS_BIND_FUNC_RUN_CLASS_NAME OS_FunctionClassImpConst4_run ## cdecl
#elif OS_BIND_FUNC_NUM_ARGS == 5
#define OS_BIND_FUNC_CLASS_NAME OS_FunctionClassImpConst5 ## cdecl
#define OS_BIND_FUNC_RUN_CLASS_NAME OS_FunctionClassImpConst5_run ## cdecl
#elif OS_BIND_FUNC_NUM_ARGS == 6
#define OS_BIND_FUNC_CLASS_NAME OS_FunctionClassImpConst6 ## cdecl
#define OS_BIND_FUNC_RUN_CLASS_NAME OS_FunctionClassImpConst6_run ## cdecl
#elif OS_BIND_FUNC_NUM_ARGS == 7
#define OS_BIND_FUNC_CLASS_NAME OS_FunctionClassImpConst7 ## cdecl
#define OS_BIND_FUNC_RUN_CLASS_NAME OS_FunctionClassImpConst7_run ## cdecl
#elif OS_BIND_FUNC_NUM_ARGS == 8
#define OS_BIND_FUNC_CLASS_NAME OS_FunctionClassImpConst8 ## cdecl
#define OS_BIND_FUNC_RUN_CLASS_NAME OS_FunctionClassImpConst8_run ## cdecl
#elif OS_BIND_FUNC_NUM_ARGS == 9
#define OS_BIND_FUNC_CLASS_NAME OS_FunctionClassImpConst9 ## cdecl
#define OS_BIND_FUNC_RUN_CLASS_NAME OS_FunctionClassImpConst9_run ## cdecl
#elif OS_BIND_FUNC_NUM_ARGS == 10
#define OS_BIND_FUNC_CLASS_NAME OS_FunctionClassImpConst10 ## cdecl
#define OS_BIND_FUNC_RUN_CLASS_NAME OS_FunctionClassImpConst10_run ## cdecl
#elif OS_BIND_FUNC_NUM_ARGS == 11
#define OS_BIND_FUNC_CLASS_NAME OS_FunctionClassImpConst11 ## cdecl
#define OS_BIND_FUNC_RUN_CLASS_NAME OS_FunctionClassImpConst11_run ## cdecl
#elif OS_BIND_FUNC_NUM_ARGS == 12
#define OS_BIND_FUNC_CLASS_NAME OS_FunctionClassImpConst12 ## cdecl
#define OS_BIND_FUNC_RUN_CLASS_NAME OS_FunctionClassImpConst12_run ## cdecl
#elif OS_BIND_FUNC_NUM_ARGS == 13
#define OS_BIND_FUNC_CLASS_NAME OS_FunctionClassImpConst13 ## cdecl
#define OS_BIND_FUNC_RUN_CLASS_NAME OS_FunctionClassImpConst13_run ## cdecl
#elif OS_BIND_FUNC_NUM_ARGS == 14
#define OS_BIND_FUNC_CLASS_NAME OS_FunctionClassImpConst14 ## cdecl
#define OS_BIND_FUNC_RUN_CLASS_NAME OS_FunctionClassImpConst14_run ## cdecl
#elif OS_BIND_FUNC_NUM_ARGS == 15
#define OS_BIND_FUNC_CLASS_NAME OS_FunctionClassImpConst15 ## cdecl
#define OS_BIND_FUNC_RUN_CLASS_NAME OS_FunctionClassImpConst15_run ## cdecl
#endif

#elif defined OS_BIND_FUNC_STDCALL

#ifdef __GNUC__
#define OS_BIND_FUNC_CC
#define OS_BIND_FUNC_CC_GNUC __attribute__((stdcall))
#else
#define OS_BIND_FUNC_CC __stdcall
#define OS_BIND_FUNC_CC_GNUC
#endif

#if OS_BIND_FUNC_NUM_ARGS == 0
#define OS_BIND_FUNC_CLASS_NAME OS_FunctionClassImpConst0 ## stdcall
#define OS_BIND_FUNC_RUN_CLASS_NAME OS_FunctionClassImpConst0_run ## stdcall
#elif OS_BIND_FUNC_NUM_ARGS == 1
#define OS_BIND_FUNC_CLASS_NAME OS_FunctionClassImpConst1 ## stdcall
#define OS_BIND_FUNC_RUN_CLASS_NAME OS_FunctionClassImpConst1_run ## stdcall
#elif OS_BIND_FUNC_NUM_ARGS == 2
#define OS_BIND_FUNC_CLASS_NAME OS_FunctionClassImpConst2 ## stdcall
#define OS_BIND_FUNC_RUN_CLASS_NAME OS_FunctionClassImpConst2_run ## stdcall
#elif OS_BIND_FUNC_NUM_ARGS == 3
#define OS_BIND_FUNC_CLASS_NAME OS_FunctionClassImpConst3 ## stdcall
#define OS_BIND_FUNC_RUN_CLASS_NAME OS_FunctionClassImpConst3_run ## stdcall
#elif OS_BIND_FUNC_NUM_ARGS == 4
#define OS_BIND_FUNC_CLASS_NAME OS_FunctionClassImpConst4 ## stdcall
#define OS_BIND_FUNC_RUN_CLASS_NAME OS_FunctionClassImpConst4_run ## stdcall
#elif OS_BIND_FUNC_NUM_ARGS == 5
#define OS_BIND_FUNC_CLASS_NAME OS_FunctionClassImpConst5 ## stdcall
#define OS_BIND_FUNC_RUN_CLASS_NAME OS_FunctionClassImpConst5_run ## stdcall
#elif OS_BIND_FUNC_NUM_ARGS == 6
#define OS_BIND_FUNC_CLASS_NAME OS_FunctionClassImpConst6 ## stdcall
#define OS_BIND_FUNC_RUN_CLASS_NAME OS_FunctionClassImpConst6_run ## stdcall
#elif OS_BIND_FUNC_NUM_ARGS == 7
#define OS_BIND_FUNC_CLASS_NAME OS_FunctionClassImpConst7 ## stdcall
#define OS_BIND_FUNC_RUN_CLASS_NAME OS_FunctionClassImpConst7_run ## stdcall
#elif OS_BIND_FUNC_NUM_ARGS == 8
#define OS_BIND_FUNC_CLASS_NAME OS_FunctionClassImpConst8 ## stdcall
#define OS_BIND_FUNC_RUN_CLASS_NAME OS_FunctionClassImpConst8_run ## stdcall
#elif OS_BIND_FUNC_NUM_ARGS == 9
#define OS_BIND_FUNC_CLASS_NAME OS_FunctionClassImpConst9 ## stdcall
#define OS_BIND_FUNC_RUN_CLASS_NAME OS_FunctionClassImpConst9_run ## stdcall
#elif OS_BIND_FUNC_NUM_ARGS == 10
#define OS_BIND_FUNC_CLASS_NAME OS_FunctionClassImpConst10 ## stdcall
#define OS_BIND_FUNC_RUN_CLASS_NAME OS_FunctionClassImpConst10_run ## stdcall
#elif OS_BIND_FUNC_NUM_ARGS == 11
#define OS_BIND_FUNC_CLASS_NAME OS_FunctionClassImpConst11 ## stdcall
#define OS_BIND_FUNC_RUN_CLASS_NAME OS_FunctionClassImpConst11_run ## stdcall
#elif OS_BIND_FUNC_NUM_ARGS == 12
#define OS_BIND_FUNC_CLASS_NAME OS_FunctionClassImpConst12 ## stdcall
#define OS_BIND_FUNC_RUN_CLASS_NAME OS_FunctionClassImpConst12_run ## stdcall
#elif OS_BIND_FUNC_NUM_ARGS == 13
#define OS_BIND_FUNC_CLASS_NAME OS_FunctionClassImpConst13 ## stdcall
#define OS_BIND_FUNC_RUN_CLASS_NAME OS_FunctionClassImpConst13_run ## stdcall
#elif OS_BIND_FUNC_NUM_ARGS == 14
#define OS_BIND_FUNC_CLASS_NAME OS_FunctionClassImpConst14 ## stdcall
#define OS_BIND_FUNC_RUN_CLASS_NAME OS_FunctionClassImpConst14_run ## stdcall
#elif OS_BIND_FUNC_NUM_ARGS == 15
#define OS_BIND_FUNC_CLASS_NAME OS_FunctionClassImpConst15 ## stdcall
#define OS_BIND_FUNC_RUN_CLASS_NAME OS_FunctionClassImpConst15_run ## stdcall
#endif

#elif defined OS_BIND_FUNC_FASTCALL

#ifdef __GNUC__
#define OS_BIND_FUNC_CC
#define OS_BIND_FUNC_CC_GNUC __attribute__((fastcall))
#else
#define OS_BIND_FUNC_CC __fastcall
#define OS_BIND_FUNC_CC_GNUC
#endif

#if OS_BIND_FUNC_NUM_ARGS == 0
#define OS_BIND_FUNC_CLASS_NAME OS_FunctionClassImpConst0 ## fastcall
#define OS_BIND_FUNC_RUN_CLASS_NAME OS_FunctionClassImpConst0_run ## fastcall
#elif OS_BIND_FUNC_NUM_ARGS == 1
#define OS_BIND_FUNC_CLASS_NAME OS_FunctionClassImpConst1 ## fastcall
#define OS_BIND_FUNC_RUN_CLASS_NAME OS_FunctionClassImpConst1_run ## fastcall
#elif OS_BIND_FUNC_NUM_ARGS == 2
#define OS_BIND_FUNC_CLASS_NAME OS_FunctionClassImpConst2 ## fastcall
#define OS_BIND_FUNC_RUN_CLASS_NAME OS_FunctionClassImpConst2_run ## fastcall
#elif OS_BIND_FUNC_NUM_ARGS == 3
#define OS_BIND_FUNC_CLASS_NAME OS_FunctionClassImpConst3 ## fastcall
#define OS_BIND_FUNC_RUN_CLASS_NAME OS_FunctionClassImpConst3_run ## fastcall
#elif OS_BIND_FUNC_NUM_ARGS == 4
#define OS_BIND_FUNC_CLASS_NAME OS_FunctionClassImpConst4 ## fastcall
#define OS_BIND_FUNC_RUN_CLASS_NAME OS_FunctionClassImpConst4_run ## fastcall
#elif OS_BIND_FUNC_NUM_ARGS == 5
#define OS_BIND_FUNC_CLASS_NAME OS_FunctionClassImpConst5 ## fastcall
#define OS_BIND_FUNC_RUN_CLASS_NAME OS_FunctionClassImpConst5_run ## fastcall
#elif OS_BIND_FUNC_NUM_ARGS == 6
#define OS_BIND_FUNC_CLASS_NAME OS_FunctionClassImpConst6 ## fastcall
#define OS_BIND_FUNC_RUN_CLASS_NAME OS_FunctionClassImpConst6_run ## fastcall
#elif OS_BIND_FUNC_NUM_ARGS == 7
#define OS_BIND_FUNC_CLASS_NAME OS_FunctionClassImpConst7 ## fastcall
#define OS_BIND_FUNC_RUN_CLASS_NAME OS_FunctionClassImpConst7_run ## fastcall
#elif OS_BIND_FUNC_NUM_ARGS == 8
#define OS_BIND_FUNC_CLASS_NAME OS_FunctionClassImpConst8 ## fastcall
#define OS_BIND_FUNC_RUN_CLASS_NAME OS_FunctionClassImpConst8_run ## fastcall
#elif OS_BIND_FUNC_NUM_ARGS == 9
#define OS_BIND_FUNC_CLASS_NAME OS_FunctionClassImpConst9 ## fastcall
#define OS_BIND_FUNC_RUN_CLASS_NAME OS_FunctionClassImpConst9_run ## fastcall
#elif OS_BIND_FUNC_NUM_ARGS == 10
#define OS_BIND_FUNC_CLASS_NAME OS_FunctionClassImpConst10 ## fastcall
#define OS_BIND_FUNC_RUN_CLASS_NAME OS_FunctionClassImpConst10_run ## fastcall
#elif OS_BIND_FUNC_NUM_ARGS == 11
#define OS_BIND_FUNC_CLASS_NAME OS_FunctionClassImpConst11 ## fastcall
#define OS_BIND_FUNC_RUN_CLASS_NAME OS_FunctionClassImpConst11_run ## fastcall
#elif OS_BIND_FUNC_NUM_ARGS == 12
#define OS_BIND_FUNC_CLASS_NAME OS_FunctionClassImpConst12 ## fastcall
#define OS_BIND_FUNC_RUN_CLASS_NAME OS_FunctionClassImpConst12_run ## fastcall
#elif OS_BIND_FUNC_NUM_ARGS == 13
#define OS_BIND_FUNC_CLASS_NAME OS_FunctionClassImpConst13 ## fastcall
#define OS_BIND_FUNC_RUN_CLASS_NAME OS_FunctionClassImpConst13_run ## fastcall
#elif OS_BIND_FUNC_NUM_ARGS == 14
#define OS_BIND_FUNC_CLASS_NAME OS_FunctionClassImpConst14 ## fastcall
#define OS_BIND_FUNC_RUN_CLASS_NAME OS_FunctionClassImpConst14_run ## fastcall
#elif OS_BIND_FUNC_NUM_ARGS == 15
#define OS_BIND_FUNC_CLASS_NAME OS_FunctionClassImpConst15 ## fastcall
#define OS_BIND_FUNC_RUN_CLASS_NAME OS_FunctionClassImpConst15_run ## fastcall
#endif

#elif defined OS_BIND_FUNC_THISCALL

#ifdef __GNUC__
#define OS_BIND_FUNC_CC
#define OS_BIND_FUNC_CC_GNUC __attribute__((thiscall))
#else
#define OS_BIND_FUNC_CC __thiscall
#define OS_BIND_FUNC_CC_GNUC
#endif

#if OS_BIND_FUNC_NUM_ARGS == 0
#define OS_BIND_FUNC_CLASS_NAME OS_FunctionClassImpConst0 ## thiscall
#define OS_BIND_FUNC_RUN_CLASS_NAME OS_FunctionClassImpConst0_run ## thiscall
#elif OS_BIND_FUNC_NUM_ARGS == 1
#define OS_BIND_FUNC_CLASS_NAME OS_FunctionClassImpConst1 ## thiscall
#define OS_BIND_FUNC_RUN_CLASS_NAME OS_FunctionClassImpConst1_run ## thiscall
#elif OS_BIND_FUNC_NUM_ARGS == 2
#define OS_BIND_FUNC_CLASS_NAME OS_FunctionClassImpConst2 ## thiscall
#define OS_BIND_FUNC_RUN_CLASS_NAME OS_FunctionClassImpConst2_run ## thiscall
#elif OS_BIND_FUNC_NUM_ARGS == 3
#define OS_BIND_FUNC_CLASS_NAME OS_FunctionClassImpConst3 ## thiscall
#define OS_BIND_FUNC_RUN_CLASS_NAME OS_FunctionClassImpConst3_run ## thiscall
#elif OS_BIND_FUNC_NUM_ARGS == 4
#define OS_BIND_FUNC_CLASS_NAME OS_FunctionClassImpConst4 ## thiscall
#define OS_BIND_FUNC_RUN_CLASS_NAME OS_FunctionClassImpConst4_run ## thiscall
#elif OS_BIND_FUNC_NUM_ARGS == 5
#define OS_BIND_FUNC_CLASS_NAME OS_FunctionClassImpConst5 ## thiscall
#define OS_BIND_FUNC_RUN_CLASS_NAME OS_FunctionClassImpConst5_run ## thiscall
#elif OS_BIND_FUNC_NUM_ARGS == 6
#define OS_BIND_FUNC_CLASS_NAME OS_FunctionClassImpConst6 ## thiscall
#define OS_BIND_FUNC_RUN_CLASS_NAME OS_FunctionClassImpConst6_run ## thiscall
#elif OS_BIND_FUNC_NUM_ARGS == 7
#define OS_BIND_FUNC_CLASS_NAME OS_FunctionClassImpConst7 ## thiscall
#define OS_BIND_FUNC_RUN_CLASS_NAME OS_FunctionClassImpConst7_run ## thiscall
#elif OS_BIND_FUNC_NUM_ARGS == 8
#define OS_BIND_FUNC_CLASS_NAME OS_FunctionClassImpConst8 ## thiscall
#define OS_BIND_FUNC_RUN_CLASS_NAME OS_FunctionClassImpConst8_run ## thiscall
#elif OS_BIND_FUNC_NUM_ARGS == 9
#define OS_BIND_FUNC_CLASS_NAME OS_FunctionClassImpConst9 ## thiscall
#define OS_BIND_FUNC_RUN_CLASS_NAME OS_FunctionClassImpConst9_run ## thiscall
#elif OS_BIND_FUNC_NUM_ARGS == 10
#define OS_BIND_FUNC_CLASS_NAME OS_FunctionClassImpConst10 ## thiscall
#define OS_BIND_FUNC_RUN_CLASS_NAME OS_FunctionClassImpConst10_run ## thiscall
#elif OS_BIND_FUNC_NUM_ARGS == 11
#define OS_BIND_FUNC_CLASS_NAME OS_FunctionClassImpConst11 ## thiscall
#define OS_BIND_FUNC_RUN_CLASS_NAME OS_FunctionClassImpConst11_run ## thiscall
#elif OS_BIND_FUNC_NUM_ARGS == 12
#define OS_BIND_FUNC_CLASS_NAME OS_FunctionClassImpConst12 ## thiscall
#define OS_BIND_FUNC_RUN_CLASS_NAME OS_FunctionClassImpConst12_run ## thiscall
#elif OS_BIND_FUNC_NUM_ARGS == 13
#define OS_BIND_FUNC_CLASS_NAME OS_FunctionClassImpConst13 ## thiscall
#define OS_BIND_FUNC_RUN_CLASS_NAME OS_FunctionClassImpConst13_run ## thiscall
#elif OS_BIND_FUNC_NUM_ARGS == 14
#define OS_BIND_FUNC_CLASS_NAME OS_FunctionClassImpConst14 ## thiscall
#define OS_BIND_FUNC_RUN_CLASS_NAME OS_FunctionClassImpConst14_run ## thiscall
#elif OS_BIND_FUNC_NUM_ARGS == 15
#define OS_BIND_FUNC_CLASS_NAME OS_FunctionClassImpConst15 ## thiscall
#define OS_BIND_FUNC_RUN_CLASS_NAME OS_FunctionClassImpConst15_run ## thiscall
#endif

#else

#define OS_BIND_FUNC_CC
#define OS_BIND_FUNC_CC_GNUC

#if OS_BIND_FUNC_NUM_ARGS == 0
#define OS_BIND_FUNC_CLASS_NAME OS_FunctionClassImpConst0
#define OS_BIND_FUNC_RUN_CLASS_NAME OS_FunctionClassImpConst0_run
#elif OS_BIND_FUNC_NUM_ARGS == 1
#define OS_BIND_FUNC_CLASS_NAME OS_FunctionClassImpConst1
#define OS_BIND_FUNC_RUN_CLASS_NAME OS_FunctionClassImpConst1_run
#elif OS_BIND_FUNC_NUM_ARGS == 2
#define OS_BIND_FUNC_CLASS_NAME OS_FunctionClassImpConst2
#define OS_BIND_FUNC_RUN_CLASS_NAME OS_FunctionClassImpConst2_run
#elif OS_BIND_FUNC_NUM_ARGS == 3
#define OS_BIND_FUNC_CLASS_NAME OS_FunctionClassImpConst3
#define OS_BIND_FUNC_RUN_CLASS_NAME OS_FunctionClassImpConst3_run
#elif OS_BIND_FUNC_NUM_ARGS == 4
#define OS_BIND_FUNC_CLASS_NAME OS_FunctionClassImpConst4
#define OS_BIND_FUNC_RUN_CLASS_NAME OS_FunctionClassImpConst4_run
#elif OS_BIND_FUNC_NUM_ARGS == 5
#define OS_BIND_FUNC_CLASS_NAME OS_FunctionClassImpConst5
#define OS_BIND_FUNC_RUN_CLASS_NAME OS_FunctionClassImpConst5_run
#elif OS_BIND_FUNC_NUM_ARGS == 6
#define OS_BIND_FUNC_CLASS_NAME OS_FunctionClassImpConst6
#define OS_BIND_FUNC_RUN_CLASS_NAME OS_FunctionClassImpConst6_run
#elif OS_BIND_FUNC_NUM_ARGS == 7
#define OS_BIND_FUNC_CLASS_NAME OS_FunctionClassImpConst7
#define OS_BIND_FUNC_RUN_CLASS_NAME OS_FunctionClassImpConst7_run
#elif OS_BIND_FUNC_NUM_ARGS == 8
#define OS_BIND_FUNC_CLASS_NAME OS_FunctionClassImpConst8
#define OS_BIND_FUNC_RUN_CLASS_NAME OS_FunctionClassImpConst8_run
#elif OS_BIND_FUNC_NUM_ARGS == 9
#define OS_BIND_FUNC_CLASS_NAME OS_FunctionClassImpConst9
#define OS_BIND_FUNC_RUN_CLASS_NAME OS_FunctionClassImpConst9_run
#elif OS_BIND_FUNC_NUM_ARGS == 10
#define OS_BIND_FUNC_CLASS_NAME OS_FunctionClassImpConst10
#define OS_BIND_FUNC_RUN_CLASS_NAME OS_FunctionClassImpConst10_run
#elif OS_BIND_FUNC_NUM_ARGS == 11
#define OS_BIND_FUNC_CLASS_NAME OS_FunctionClassImpConst11
#define OS_BIND_FUNC_RUN_CLASS_NAME OS_FunctionClassImpConst11_run
#elif OS_BIND_FUNC_NUM_ARGS == 12
#define OS_BIND_FUNC_CLASS_NAME OS_FunctionClassImpConst12
#define OS_BIND_FUNC_RUN_CLASS_NAME OS_FunctionClassImpConst12_run
#elif OS_BIND_FUNC_NUM_ARGS == 13
#define OS_BIND_FUNC_CLASS_NAME OS_FunctionClassImpConst13
#define OS_BIND_FUNC_RUN_CLASS_NAME OS_FunctionClassImpConst13_run
#elif OS_BIND_FUNC_NUM_ARGS == 14
#define OS_BIND_FUNC_CLASS_NAME OS_FunctionClassImpConst14
#define OS_BIND_FUNC_RUN_CLASS_NAME OS_FunctionClassImpConst14_run
#elif OS_BIND_FUNC_NUM_ARGS == 15
#define OS_BIND_FUNC_CLASS_NAME OS_FunctionClassImpConst15
#define OS_BIND_FUNC_RUN_CLASS_NAME OS_FunctionClassImpConst15_run
#endif

#endif

template <class R, class T OS_BIND_FUNC_PARMS_COMMA OS_BIND_FUNC_TEMPLATE_PARMS> 
struct OS_BIND_FUNC_RUN_CLASS_NAME
{
	typedef R(OS_BIND_FUNC_CC T::*F)(OS_BIND_FUNC_PARMS)const OS_BIND_FUNC_CC_GNUC;

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
};

template <class T OS_BIND_FUNC_PARMS_COMMA OS_BIND_FUNC_TEMPLATE_PARMS> 
struct OS_BIND_FUNC_RUN_CLASS_NAME<void, T OS_BIND_FUNC_PARMS_COMMA OS_BIND_FUNC_PARMS>
{
	typedef void(OS_BIND_FUNC_CC T::*F)(OS_BIND_FUNC_PARMS)const OS_BIND_FUNC_CC_GNUC;

	static int run(ObjectScript::OS * os, int params, int, int, void * user_param)
	{
		OS_GET_TEMPLATE_SELF(T*);
		OS_BIND_FUNC_GET_ARGS;
		F& f = *(F*)user_param;
		(self->*f)(OS_BIND_FUNC_ARGS);
		return 0;
	}
};

template <class R, class T OS_BIND_FUNC_PARMS_COMMA OS_BIND_FUNC_TEMPLATE_PARMS>
struct OS_BIND_FUNC_CLASS_NAME
{
	typedef R(OS_BIND_FUNC_CC T::*F)(OS_BIND_FUNC_PARMS)const OS_BIND_FUNC_CC_GNUC;

	const char * name;
	F f;

	OS_BIND_FUNC_CLASS_NAME(const char * _name, F _f): name(_name), f(_f){}
	
	operator ObjectScript::OS::FuncDef() const 
	{ 
		ObjectScript::OS::FuncDef def = {name, 
			OS_BIND_FUNC_RUN_CLASS_NAME<R, T OS_BIND_FUNC_PARMS_COMMA OS_BIND_FUNC_PARMS>::run, 
			&(new OS_FunctionData<F>(f))->f}; 
		return def; 
	}
};

namespace ObjectScript {

template <class R, class T OS_BIND_FUNC_PARMS_COMMA OS_BIND_FUNC_TEMPLATE_PARMS> 
ObjectScript::OS::FuncDef def(const char * name, R(OS_BIND_FUNC_CC T::*f)(OS_BIND_FUNC_PARMS)const OS_BIND_FUNC_CC_GNUC)
{
	typedef OS_BIND_FUNC_CLASS_NAME<R, T OS_BIND_FUNC_PARMS_COMMA OS_BIND_FUNC_PARMS> Func; 
	return Func(name, f);
}

} // namespace ObjectScript

#undef OS_BIND_FUNC_CLASS_NAME
#undef OS_BIND_FUNC_RUN_CLASS_NAME
#undef OS_BIND_FUNC_CC

#undef OS_BIND_FUNC_CC_GNUC

#ifdef OS_BIND_FUNC_CDECL

#ifdef __GNUC__
#define OS_BIND_FUNC_CC
#define OS_BIND_FUNC_CC_GNUC __attribute__((cdecl))
#else
#define OS_BIND_FUNC_CC __cdecl
#define OS_BIND_FUNC_CC_GNUC
#endif

#if OS_BIND_FUNC_NUM_ARGS == 0
#define OS_BIND_FUNC_CLASS_NAME OS_FunctionClassImp0 ## cdecl
#define OS_BIND_FUNC_RUN_CLASS_NAME OS_FunctionClassImp0_run ## cdecl
#elif OS_BIND_FUNC_NUM_ARGS == 1
#define OS_BIND_FUNC_CLASS_NAME OS_FunctionClassImp1 ## cdecl
#define OS_BIND_FUNC_RUN_CLASS_NAME OS_FunctionClassImp1_run ## cdecl
#elif OS_BIND_FUNC_NUM_ARGS == 2
#define OS_BIND_FUNC_CLASS_NAME OS_FunctionClassImp2 ## cdecl
#define OS_BIND_FUNC_RUN_CLASS_NAME OS_FunctionClassImp2_run ## cdecl
#elif OS_BIND_FUNC_NUM_ARGS == 3
#define OS_BIND_FUNC_CLASS_NAME OS_FunctionClassImp3 ## cdecl
#define OS_BIND_FUNC_RUN_CLASS_NAME OS_FunctionClassImp3_run ## cdecl
#elif OS_BIND_FUNC_NUM_ARGS == 4
#define OS_BIND_FUNC_CLASS_NAME OS_FunctionClassImp4 ## cdecl
#define OS_BIND_FUNC_RUN_CLASS_NAME OS_FunctionClassImp4_run ## cdecl
#elif OS_BIND_FUNC_NUM_ARGS == 5
#define OS_BIND_FUNC_CLASS_NAME OS_FunctionClassImp5 ## cdecl
#define OS_BIND_FUNC_RUN_CLASS_NAME OS_FunctionClassImp5_run ## cdecl
#elif OS_BIND_FUNC_NUM_ARGS == 6
#define OS_BIND_FUNC_CLASS_NAME OS_FunctionClassImp6 ## cdecl
#define OS_BIND_FUNC_RUN_CLASS_NAME OS_FunctionClassImp6_run ## cdecl
#elif OS_BIND_FUNC_NUM_ARGS == 7
#define OS_BIND_FUNC_CLASS_NAME OS_FunctionClassImp7 ## cdecl
#define OS_BIND_FUNC_RUN_CLASS_NAME OS_FunctionClassImp7_run ## cdecl
#elif OS_BIND_FUNC_NUM_ARGS == 8
#define OS_BIND_FUNC_CLASS_NAME OS_FunctionClassImp8 ## cdecl
#define OS_BIND_FUNC_RUN_CLASS_NAME OS_FunctionClassImp8_run ## cdecl
#elif OS_BIND_FUNC_NUM_ARGS == 9
#define OS_BIND_FUNC_CLASS_NAME OS_FunctionClassImp9 ## cdecl
#define OS_BIND_FUNC_RUN_CLASS_NAME OS_FunctionClassImp9_run ## cdecl
#elif OS_BIND_FUNC_NUM_ARGS == 10
#define OS_BIND_FUNC_CLASS_NAME OS_FunctionClassImp10 ## cdecl
#define OS_BIND_FUNC_RUN_CLASS_NAME OS_FunctionClassImp10_run ## cdecl
#elif OS_BIND_FUNC_NUM_ARGS == 11
#define OS_BIND_FUNC_CLASS_NAME OS_FunctionClassImp11 ## cdecl
#define OS_BIND_FUNC_RUN_CLASS_NAME OS_FunctionClassImp11_run ## cdecl
#elif OS_BIND_FUNC_NUM_ARGS == 12
#define OS_BIND_FUNC_CLASS_NAME OS_FunctionClassImp12 ## cdecl
#define OS_BIND_FUNC_RUN_CLASS_NAME OS_FunctionClassImp12_run ## cdecl
#elif OS_BIND_FUNC_NUM_ARGS == 13
#define OS_BIND_FUNC_CLASS_NAME OS_FunctionClassImp13 ## cdecl
#define OS_BIND_FUNC_RUN_CLASS_NAME OS_FunctionClassImp13_run ## cdecl
#elif OS_BIND_FUNC_NUM_ARGS == 14
#define OS_BIND_FUNC_CLASS_NAME OS_FunctionClassImp14 ## cdecl
#define OS_BIND_FUNC_RUN_CLASS_NAME OS_FunctionClassImp14_run ## cdecl
#elif OS_BIND_FUNC_NUM_ARGS == 15
#define OS_BIND_FUNC_CLASS_NAME OS_FunctionClassImp15 ## cdecl
#define OS_BIND_FUNC_RUN_CLASS_NAME OS_FunctionClassImp15_run ## cdecl
#endif

#elif defined OS_BIND_FUNC_STDCALL

#ifdef __GNUC__
#define OS_BIND_FUNC_CC
#define OS_BIND_FUNC_CC_GNUC __attribute__((stdcall))
#else
#define OS_BIND_FUNC_CC __stdcall
#define OS_BIND_FUNC_CC_GNUC
#endif

#if OS_BIND_FUNC_NUM_ARGS == 0
#define OS_BIND_FUNC_CLASS_NAME OS_FunctionClassImp0 ## stdcall
#define OS_BIND_FUNC_RUN_CLASS_NAME OS_FunctionClassImp0_run ## stdcall
#elif OS_BIND_FUNC_NUM_ARGS == 1
#define OS_BIND_FUNC_CLASS_NAME OS_FunctionClassImp1 ## stdcall
#define OS_BIND_FUNC_RUN_CLASS_NAME OS_FunctionClassImp1_run ## stdcall
#elif OS_BIND_FUNC_NUM_ARGS == 2
#define OS_BIND_FUNC_CLASS_NAME OS_FunctionClassImp2 ## stdcall
#define OS_BIND_FUNC_RUN_CLASS_NAME OS_FunctionClassImp2_run ## stdcall
#elif OS_BIND_FUNC_NUM_ARGS == 3
#define OS_BIND_FUNC_CLASS_NAME OS_FunctionClassImp3 ## stdcall
#define OS_BIND_FUNC_RUN_CLASS_NAME OS_FunctionClassImp3_run ## stdcall
#elif OS_BIND_FUNC_NUM_ARGS == 4
#define OS_BIND_FUNC_CLASS_NAME OS_FunctionClassImp4 ## stdcall
#define OS_BIND_FUNC_RUN_CLASS_NAME OS_FunctionClassImp4_run ## stdcall
#elif OS_BIND_FUNC_NUM_ARGS == 5
#define OS_BIND_FUNC_CLASS_NAME OS_FunctionClassImp5 ## stdcall
#define OS_BIND_FUNC_RUN_CLASS_NAME OS_FunctionClassImp5_run ## stdcall
#elif OS_BIND_FUNC_NUM_ARGS == 6
#define OS_BIND_FUNC_CLASS_NAME OS_FunctionClassImp6 ## stdcall
#define OS_BIND_FUNC_RUN_CLASS_NAME OS_FunctionClassImp6_run ## stdcall
#elif OS_BIND_FUNC_NUM_ARGS == 7
#define OS_BIND_FUNC_CLASS_NAME OS_FunctionClassImp7 ## stdcall
#define OS_BIND_FUNC_RUN_CLASS_NAME OS_FunctionClassImp7_run ## stdcall
#elif OS_BIND_FUNC_NUM_ARGS == 8
#define OS_BIND_FUNC_CLASS_NAME OS_FunctionClassImp8 ## stdcall
#define OS_BIND_FUNC_RUN_CLASS_NAME OS_FunctionClassImp8_run ## stdcall
#elif OS_BIND_FUNC_NUM_ARGS == 9
#define OS_BIND_FUNC_CLASS_NAME OS_FunctionClassImp9 ## stdcall
#define OS_BIND_FUNC_RUN_CLASS_NAME OS_FunctionClassImp9_run ## stdcall
#elif OS_BIND_FUNC_NUM_ARGS == 10
#define OS_BIND_FUNC_CLASS_NAME OS_FunctionClassImp10 ## stdcall
#define OS_BIND_FUNC_RUN_CLASS_NAME OS_FunctionClassImp10_run ## stdcall
#elif OS_BIND_FUNC_NUM_ARGS == 11
#define OS_BIND_FUNC_CLASS_NAME OS_FunctionClassImp11 ## stdcall
#define OS_BIND_FUNC_RUN_CLASS_NAME OS_FunctionClassImp11_run ## stdcall
#elif OS_BIND_FUNC_NUM_ARGS == 12
#define OS_BIND_FUNC_CLASS_NAME OS_FunctionClassImp12 ## stdcall
#define OS_BIND_FUNC_RUN_CLASS_NAME OS_FunctionClassImp12_run ## stdcall
#elif OS_BIND_FUNC_NUM_ARGS == 13
#define OS_BIND_FUNC_CLASS_NAME OS_FunctionClassImp13 ## stdcall
#define OS_BIND_FUNC_RUN_CLASS_NAME OS_FunctionClassImp13_run ## stdcall
#elif OS_BIND_FUNC_NUM_ARGS == 14
#define OS_BIND_FUNC_CLASS_NAME OS_FunctionClassImp14 ## stdcall
#define OS_BIND_FUNC_RUN_CLASS_NAME OS_FunctionClassImp14_run ## stdcall
#elif OS_BIND_FUNC_NUM_ARGS == 15
#define OS_BIND_FUNC_CLASS_NAME OS_FunctionClassImp15 ## stdcall
#define OS_BIND_FUNC_RUN_CLASS_NAME OS_FunctionClassImp15_run ## stdcall
#endif

#elif defined OS_BIND_FUNC_FASTCALL

#ifdef __GNUC__
#define OS_BIND_FUNC_CC
#define OS_BIND_FUNC_CC_GNUC __attribute__((fastcall))
#else
#define OS_BIND_FUNC_CC __fastcall
#define OS_BIND_FUNC_CC_GNUC
#endif

#if OS_BIND_FUNC_NUM_ARGS == 0
#define OS_BIND_FUNC_CLASS_NAME OS_FunctionClassImp0 ## fastcall
#define OS_BIND_FUNC_RUN_CLASS_NAME OS_FunctionClassImp0_run ## fastcall
#elif OS_BIND_FUNC_NUM_ARGS == 1
#define OS_BIND_FUNC_CLASS_NAME OS_FunctionClassImp1 ## fastcall
#define OS_BIND_FUNC_RUN_CLASS_NAME OS_FunctionClassImp1_run ## fastcall
#elif OS_BIND_FUNC_NUM_ARGS == 2
#define OS_BIND_FUNC_CLASS_NAME OS_FunctionClassImp2 ## fastcall
#define OS_BIND_FUNC_RUN_CLASS_NAME OS_FunctionClassImp2_run ## fastcall
#elif OS_BIND_FUNC_NUM_ARGS == 3
#define OS_BIND_FUNC_CLASS_NAME OS_FunctionClassImp3 ## fastcall
#define OS_BIND_FUNC_RUN_CLASS_NAME OS_FunctionClassImp3_run ## fastcall
#elif OS_BIND_FUNC_NUM_ARGS == 4
#define OS_BIND_FUNC_CLASS_NAME OS_FunctionClassImp4 ## fastcall
#define OS_BIND_FUNC_RUN_CLASS_NAME OS_FunctionClassImp4_run ## fastcall
#elif OS_BIND_FUNC_NUM_ARGS == 5
#define OS_BIND_FUNC_CLASS_NAME OS_FunctionClassImp5 ## fastcall
#define OS_BIND_FUNC_RUN_CLASS_NAME OS_FunctionClassImp5_run ## fastcall
#elif OS_BIND_FUNC_NUM_ARGS == 6
#define OS_BIND_FUNC_CLASS_NAME OS_FunctionClassImp6 ## fastcall
#define OS_BIND_FUNC_RUN_CLASS_NAME OS_FunctionClassImp6_run ## fastcall
#elif OS_BIND_FUNC_NUM_ARGS == 7
#define OS_BIND_FUNC_CLASS_NAME OS_FunctionClassImp7 ## fastcall
#define OS_BIND_FUNC_RUN_CLASS_NAME OS_FunctionClassImp7_run ## fastcall
#elif OS_BIND_FUNC_NUM_ARGS == 8
#define OS_BIND_FUNC_CLASS_NAME OS_FunctionClassImp8 ## fastcall
#define OS_BIND_FUNC_RUN_CLASS_NAME OS_FunctionClassImp8_run ## fastcall
#elif OS_BIND_FUNC_NUM_ARGS == 9
#define OS_BIND_FUNC_CLASS_NAME OS_FunctionClassImp9 ## fastcall
#define OS_BIND_FUNC_RUN_CLASS_NAME OS_FunctionClassImp9_run ## fastcall
#elif OS_BIND_FUNC_NUM_ARGS == 10
#define OS_BIND_FUNC_CLASS_NAME OS_FunctionClassImp10 ## fastcall
#define OS_BIND_FUNC_RUN_CLASS_NAME OS_FunctionClassImp10_run ## fastcall
#elif OS_BIND_FUNC_NUM_ARGS == 11
#define OS_BIND_FUNC_CLASS_NAME OS_FunctionClassImp11 ## fastcall
#define OS_BIND_FUNC_RUN_CLASS_NAME OS_FunctionClassImp11_run ## fastcall
#elif OS_BIND_FUNC_NUM_ARGS == 12
#define OS_BIND_FUNC_CLASS_NAME OS_FunctionClassImp12 ## fastcall
#define OS_BIND_FUNC_RUN_CLASS_NAME OS_FunctionClassImp12_run ## fastcall
#elif OS_BIND_FUNC_NUM_ARGS == 13
#define OS_BIND_FUNC_CLASS_NAME OS_FunctionClassImp13 ## fastcall
#define OS_BIND_FUNC_RUN_CLASS_NAME OS_FunctionClassImp13_run ## fastcall
#elif OS_BIND_FUNC_NUM_ARGS == 14
#define OS_BIND_FUNC_CLASS_NAME OS_FunctionClassImp14 ## fastcall
#define OS_BIND_FUNC_RUN_CLASS_NAME OS_FunctionClassImp14_run ## fastcall
#elif OS_BIND_FUNC_NUM_ARGS == 15
#define OS_BIND_FUNC_CLASS_NAME OS_FunctionClassImp15 ## fastcall
#define OS_BIND_FUNC_RUN_CLASS_NAME OS_FunctionClassImp15_run ## fastcall
#endif

#elif defined OS_BIND_FUNC_THISCALL

#ifdef __GNUC__
#define OS_BIND_FUNC_CC
#define OS_BIND_FUNC_CC_GNUC __attribute__((thiscall))
#else
#define OS_BIND_FUNC_CC __thiscall
#define OS_BIND_FUNC_CC_GNUC
#endif

#if OS_BIND_FUNC_NUM_ARGS == 0
#define OS_BIND_FUNC_CLASS_NAME OS_FunctionClassImp0 ## thiscall
#define OS_BIND_FUNC_RUN_CLASS_NAME OS_FunctionClassImp0_run ## thiscall
#elif OS_BIND_FUNC_NUM_ARGS == 1
#define OS_BIND_FUNC_CLASS_NAME OS_FunctionClassImp1 ## thiscall
#define OS_BIND_FUNC_RUN_CLASS_NAME OS_FunctionClassImp1_run ## thiscall
#elif OS_BIND_FUNC_NUM_ARGS == 2
#define OS_BIND_FUNC_CLASS_NAME OS_FunctionClassImp2 ## thiscall
#define OS_BIND_FUNC_RUN_CLASS_NAME OS_FunctionClassImp2_run ## thiscall
#elif OS_BIND_FUNC_NUM_ARGS == 3
#define OS_BIND_FUNC_CLASS_NAME OS_FunctionClassImp3 ## thiscall
#define OS_BIND_FUNC_RUN_CLASS_NAME OS_FunctionClassImp3_run ## thiscall
#elif OS_BIND_FUNC_NUM_ARGS == 4
#define OS_BIND_FUNC_CLASS_NAME OS_FunctionClassImp4 ## thiscall
#define OS_BIND_FUNC_RUN_CLASS_NAME OS_FunctionClassImp4_run ## thiscall
#elif OS_BIND_FUNC_NUM_ARGS == 5
#define OS_BIND_FUNC_CLASS_NAME OS_FunctionClassImp5 ## thiscall
#define OS_BIND_FUNC_RUN_CLASS_NAME OS_FunctionClassImp5_run ## thiscall
#elif OS_BIND_FUNC_NUM_ARGS == 6
#define OS_BIND_FUNC_CLASS_NAME OS_FunctionClassImp6 ## thiscall
#define OS_BIND_FUNC_RUN_CLASS_NAME OS_FunctionClassImp6_run ## thiscall
#elif OS_BIND_FUNC_NUM_ARGS == 7
#define OS_BIND_FUNC_CLASS_NAME OS_FunctionClassImp7 ## thiscall
#define OS_BIND_FUNC_RUN_CLASS_NAME OS_FunctionClassImp7_run ## thiscall
#elif OS_BIND_FUNC_NUM_ARGS == 8
#define OS_BIND_FUNC_CLASS_NAME OS_FunctionClassImp8 ## thiscall
#define OS_BIND_FUNC_RUN_CLASS_NAME OS_FunctionClassImp8_run ## thiscall
#elif OS_BIND_FUNC_NUM_ARGS == 9
#define OS_BIND_FUNC_CLASS_NAME OS_FunctionClassImp9 ## thiscall
#define OS_BIND_FUNC_RUN_CLASS_NAME OS_FunctionClassImp9_run ## thiscall
#elif OS_BIND_FUNC_NUM_ARGS == 10
#define OS_BIND_FUNC_CLASS_NAME OS_FunctionClassImp10 ## thiscall
#define OS_BIND_FUNC_RUN_CLASS_NAME OS_FunctionClassImp10_run ## thiscall
#elif OS_BIND_FUNC_NUM_ARGS == 11
#define OS_BIND_FUNC_CLASS_NAME OS_FunctionClassImp11 ## thiscall
#define OS_BIND_FUNC_RUN_CLASS_NAME OS_FunctionClassImp11_run ## thiscall
#elif OS_BIND_FUNC_NUM_ARGS == 12
#define OS_BIND_FUNC_CLASS_NAME OS_FunctionClassImp12 ## thiscall
#define OS_BIND_FUNC_RUN_CLASS_NAME OS_FunctionClassImp12_run ## thiscall
#elif OS_BIND_FUNC_NUM_ARGS == 13
#define OS_BIND_FUNC_CLASS_NAME OS_FunctionClassImp13 ## thiscall
#define OS_BIND_FUNC_RUN_CLASS_NAME OS_FunctionClassImp13_run ## thiscall
#elif OS_BIND_FUNC_NUM_ARGS == 14
#define OS_BIND_FUNC_CLASS_NAME OS_FunctionClassImp14 ## thiscall
#define OS_BIND_FUNC_RUN_CLASS_NAME OS_FunctionClassImp14_run ## thiscall
#elif OS_BIND_FUNC_NUM_ARGS == 15
#define OS_BIND_FUNC_CLASS_NAME OS_FunctionClassImp15 ## thiscall
#define OS_BIND_FUNC_RUN_CLASS_NAME OS_FunctionClassImp15_run ## thiscall
#endif

#else

#define OS_BIND_FUNC_CC
#define OS_BIND_FUNC_CC_GNUC

#if OS_BIND_FUNC_NUM_ARGS == 0
#define OS_BIND_FUNC_CLASS_NAME OS_FunctionClassImp0
#define OS_BIND_FUNC_RUN_CLASS_NAME OS_FunctionClassImp0_run
#elif OS_BIND_FUNC_NUM_ARGS == 1
#define OS_BIND_FUNC_CLASS_NAME OS_FunctionClassImp1
#define OS_BIND_FUNC_RUN_CLASS_NAME OS_FunctionClassImp1_run
#elif OS_BIND_FUNC_NUM_ARGS == 2
#define OS_BIND_FUNC_CLASS_NAME OS_FunctionClassImp2
#define OS_BIND_FUNC_RUN_CLASS_NAME OS_FunctionClassImp2_run
#elif OS_BIND_FUNC_NUM_ARGS == 3
#define OS_BIND_FUNC_CLASS_NAME OS_FunctionClassImp3
#define OS_BIND_FUNC_RUN_CLASS_NAME OS_FunctionClassImp3_run
#elif OS_BIND_FUNC_NUM_ARGS == 4
#define OS_BIND_FUNC_CLASS_NAME OS_FunctionClassImp4
#define OS_BIND_FUNC_RUN_CLASS_NAME OS_FunctionClassImp4_run
#elif OS_BIND_FUNC_NUM_ARGS == 5
#define OS_BIND_FUNC_CLASS_NAME OS_FunctionClassImp5
#define OS_BIND_FUNC_RUN_CLASS_NAME OS_FunctionClassImp5_run
#elif OS_BIND_FUNC_NUM_ARGS == 6
#define OS_BIND_FUNC_CLASS_NAME OS_FunctionClassImp6
#define OS_BIND_FUNC_RUN_CLASS_NAME OS_FunctionClassImp6_run
#elif OS_BIND_FUNC_NUM_ARGS == 7
#define OS_BIND_FUNC_CLASS_NAME OS_FunctionClassImp7
#define OS_BIND_FUNC_RUN_CLASS_NAME OS_FunctionClassImp7_run
#elif OS_BIND_FUNC_NUM_ARGS == 8
#define OS_BIND_FUNC_CLASS_NAME OS_FunctionClassImp8
#define OS_BIND_FUNC_RUN_CLASS_NAME OS_FunctionClassImp8_run
#elif OS_BIND_FUNC_NUM_ARGS == 9
#define OS_BIND_FUNC_CLASS_NAME OS_FunctionClassImp9
#define OS_BIND_FUNC_RUN_CLASS_NAME OS_FunctionClassImp9_run
#elif OS_BIND_FUNC_NUM_ARGS == 10
#define OS_BIND_FUNC_CLASS_NAME OS_FunctionClassImp10
#define OS_BIND_FUNC_RUN_CLASS_NAME OS_FunctionClassImp10_run
#elif OS_BIND_FUNC_NUM_ARGS == 11
#define OS_BIND_FUNC_CLASS_NAME OS_FunctionClassImp11
#define OS_BIND_FUNC_RUN_CLASS_NAME OS_FunctionClassImp11_run
#elif OS_BIND_FUNC_NUM_ARGS == 12
#define OS_BIND_FUNC_CLASS_NAME OS_FunctionClassImp12
#define OS_BIND_FUNC_RUN_CLASS_NAME OS_FunctionClassImp12_run
#elif OS_BIND_FUNC_NUM_ARGS == 13
#define OS_BIND_FUNC_CLASS_NAME OS_FunctionClassImp13
#define OS_BIND_FUNC_RUN_CLASS_NAME OS_FunctionClassImp13_run
#elif OS_BIND_FUNC_NUM_ARGS == 14
#define OS_BIND_FUNC_CLASS_NAME OS_FunctionClassImp14
#define OS_BIND_FUNC_RUN_CLASS_NAME OS_FunctionClassImp14_run
#elif OS_BIND_FUNC_NUM_ARGS == 15
#define OS_BIND_FUNC_CLASS_NAME OS_FunctionClassImp15
#define OS_BIND_FUNC_RUN_CLASS_NAME OS_FunctionClassImp15_run
#endif

#endif

template <class R, class T OS_BIND_FUNC_PARMS_COMMA OS_BIND_FUNC_TEMPLATE_PARMS> 
struct OS_BIND_FUNC_RUN_CLASS_NAME
{
	typedef R(OS_BIND_FUNC_CC T::*F)(OS_BIND_FUNC_PARMS) OS_BIND_FUNC_CC_GNUC;

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
};

template <class T OS_BIND_FUNC_PARMS_COMMA OS_BIND_FUNC_TEMPLATE_PARMS> 
struct OS_BIND_FUNC_RUN_CLASS_NAME<void, T OS_BIND_FUNC_PARMS_COMMA OS_BIND_FUNC_PARMS>
{
	typedef void(OS_BIND_FUNC_CC T::*F)(OS_BIND_FUNC_PARMS) OS_BIND_FUNC_CC_GNUC;

	static int run(ObjectScript::OS * os, int params, int, int, void * user_param)
	{
		OS_GET_TEMPLATE_SELF(T*);
		OS_BIND_FUNC_GET_ARGS;
		F& f = *(F*)user_param;
		(self->*f)(OS_BIND_FUNC_ARGS);
		return 0;
	}
};

template <class R, class T OS_BIND_FUNC_PARMS_COMMA OS_BIND_FUNC_TEMPLATE_PARMS>
struct OS_BIND_FUNC_CLASS_NAME
{
	typedef R(OS_BIND_FUNC_CC T::*F)(OS_BIND_FUNC_PARMS) OS_BIND_FUNC_CC_GNUC;

	const char * name;
	F f;

	OS_BIND_FUNC_CLASS_NAME(const char * _name, F _f): name(_name), f(_f){}
	
	operator ObjectScript::OS::FuncDef() const 
	{ 
		ObjectScript::OS::FuncDef def = {name, 
			OS_BIND_FUNC_RUN_CLASS_NAME<R, T OS_BIND_FUNC_PARMS_COMMA OS_BIND_FUNC_PARMS>::run, 
			&(new OS_FunctionData<F>(f))->f}; 
		return def; 
	}
};

namespace ObjectScript {

template <class R, class T OS_BIND_FUNC_PARMS_COMMA OS_BIND_FUNC_TEMPLATE_PARMS> 
ObjectScript::OS::FuncDef def(const char * name, R(OS_BIND_FUNC_CC T::*f)(OS_BIND_FUNC_PARMS) OS_BIND_FUNC_CC_GNUC)
{
	typedef OS_BIND_FUNC_CLASS_NAME<R, T OS_BIND_FUNC_PARMS_COMMA OS_BIND_FUNC_PARMS> Func; 
	return Func(name, f);
}

} // namespace ObjectScript

#undef OS_BIND_FUNC_CLASS_NAME
#undef OS_BIND_FUNC_RUN_CLASS_NAME
#undef OS_BIND_FUNC_CC

#undef OS_BIND_FUNC_CC_GNUC

#ifdef OS_BIND_FUNC_CDECL

#ifdef __GNUC__
#define OS_BIND_FUNC_CC
#define OS_BIND_FUNC_CC_GNUC __attribute__((cdecl))
#else
#define OS_BIND_FUNC_CC __cdecl
#define OS_BIND_FUNC_CC_GNUC
#endif

#if OS_BIND_FUNC_NUM_ARGS == 0
#define OS_BIND_FUNC_CLASS_NAME OS_FunctionImp0 ## cdecl
#define OS_BIND_FUNC_RUN_CLASS_NAME OS_FunctionImp0_run ## cdecl
#elif OS_BIND_FUNC_NUM_ARGS == 1
#define OS_BIND_FUNC_CLASS_NAME OS_FunctionImp1 ## cdecl
#define OS_BIND_FUNC_RUN_CLASS_NAME OS_FunctionImp1_run ## cdecl
#elif OS_BIND_FUNC_NUM_ARGS == 2
#define OS_BIND_FUNC_CLASS_NAME OS_FunctionImp2 ## cdecl
#define OS_BIND_FUNC_RUN_CLASS_NAME OS_FunctionImp2_run ## cdecl
#elif OS_BIND_FUNC_NUM_ARGS == 3
#define OS_BIND_FUNC_CLASS_NAME OS_FunctionImp3 ## cdecl
#define OS_BIND_FUNC_RUN_CLASS_NAME OS_FunctionImp3_run ## cdecl
#elif OS_BIND_FUNC_NUM_ARGS == 4
#define OS_BIND_FUNC_CLASS_NAME OS_FunctionImp4 ## cdecl
#define OS_BIND_FUNC_RUN_CLASS_NAME OS_FunctionImp4_run ## cdecl
#elif OS_BIND_FUNC_NUM_ARGS == 5
#define OS_BIND_FUNC_CLASS_NAME OS_FunctionImp5 ## cdecl
#define OS_BIND_FUNC_RUN_CLASS_NAME OS_FunctionImp5_run ## cdecl
#elif OS_BIND_FUNC_NUM_ARGS == 6
#define OS_BIND_FUNC_CLASS_NAME OS_FunctionImp6 ## cdecl
#define OS_BIND_FUNC_RUN_CLASS_NAME OS_FunctionImp6_run ## cdecl
#elif OS_BIND_FUNC_NUM_ARGS == 7
#define OS_BIND_FUNC_CLASS_NAME OS_FunctionImp7 ## cdecl
#define OS_BIND_FUNC_RUN_CLASS_NAME OS_FunctionImp7_run ## cdecl
#elif OS_BIND_FUNC_NUM_ARGS == 8
#define OS_BIND_FUNC_CLASS_NAME OS_FunctionImp8 ## cdecl
#define OS_BIND_FUNC_RUN_CLASS_NAME OS_FunctionImp8_run ## cdecl
#elif OS_BIND_FUNC_NUM_ARGS == 9
#define OS_BIND_FUNC_CLASS_NAME OS_FunctionImp9 ## cdecl
#define OS_BIND_FUNC_RUN_CLASS_NAME OS_FunctionImp9_run ## cdecl
#elif OS_BIND_FUNC_NUM_ARGS == 10
#define OS_BIND_FUNC_CLASS_NAME OS_FunctionImp10 ## cdecl
#define OS_BIND_FUNC_RUN_CLASS_NAME OS_FunctionImp10_run ## cdecl
#elif OS_BIND_FUNC_NUM_ARGS == 11
#define OS_BIND_FUNC_CLASS_NAME OS_FunctionImp11 ## cdecl
#define OS_BIND_FUNC_RUN_CLASS_NAME OS_FunctionImp11_run ## cdecl
#elif OS_BIND_FUNC_NUM_ARGS == 12
#define OS_BIND_FUNC_CLASS_NAME OS_FunctionImp12 ## cdecl
#define OS_BIND_FUNC_RUN_CLASS_NAME OS_FunctionImp12_run ## cdecl
#elif OS_BIND_FUNC_NUM_ARGS == 13
#define OS_BIND_FUNC_CLASS_NAME OS_FunctionImp13 ## cdecl
#define OS_BIND_FUNC_RUN_CLASS_NAME OS_FunctionImp13_run ## cdecl
#elif OS_BIND_FUNC_NUM_ARGS == 14
#define OS_BIND_FUNC_CLASS_NAME OS_FunctionImp14 ## cdecl
#define OS_BIND_FUNC_RUN_CLASS_NAME OS_FunctionImp14_run ## cdecl
#elif OS_BIND_FUNC_NUM_ARGS == 15
#define OS_BIND_FUNC_CLASS_NAME OS_FunctionImp15 ## cdecl
#define OS_BIND_FUNC_RUN_CLASS_NAME OS_FunctionImp15_run ## cdecl
#endif

#elif defined OS_BIND_FUNC_STDCALL

#ifdef __GNUC__
#define OS_BIND_FUNC_CC
#define OS_BIND_FUNC_CC_GNUC __attribute__((stdcall))
#else
#define OS_BIND_FUNC_CC __stdcall
#define OS_BIND_FUNC_CC_GNUC
#endif

#if OS_BIND_FUNC_NUM_ARGS == 0
#define OS_BIND_FUNC_CLASS_NAME OS_FunctionImp0 ## stdcall
#define OS_BIND_FUNC_RUN_CLASS_NAME OS_FunctionImp0_run ## stdcall
#elif OS_BIND_FUNC_NUM_ARGS == 1
#define OS_BIND_FUNC_CLASS_NAME OS_FunctionImp1 ## stdcall
#define OS_BIND_FUNC_RUN_CLASS_NAME OS_FunctionImp1_run ## stdcall
#elif OS_BIND_FUNC_NUM_ARGS == 2
#define OS_BIND_FUNC_CLASS_NAME OS_FunctionImp2 ## stdcall
#define OS_BIND_FUNC_RUN_CLASS_NAME OS_FunctionImp2_run ## stdcall
#elif OS_BIND_FUNC_NUM_ARGS == 3
#define OS_BIND_FUNC_CLASS_NAME OS_FunctionImp3 ## stdcall
#define OS_BIND_FUNC_RUN_CLASS_NAME OS_FunctionImp3_run ## stdcall
#elif OS_BIND_FUNC_NUM_ARGS == 4
#define OS_BIND_FUNC_CLASS_NAME OS_FunctionImp4 ## stdcall
#define OS_BIND_FUNC_RUN_CLASS_NAME OS_FunctionImp4_run ## stdcall
#elif OS_BIND_FUNC_NUM_ARGS == 5
#define OS_BIND_FUNC_CLASS_NAME OS_FunctionImp5 ## stdcall
#define OS_BIND_FUNC_RUN_CLASS_NAME OS_FunctionImp5_run ## stdcall
#elif OS_BIND_FUNC_NUM_ARGS == 6
#define OS_BIND_FUNC_CLASS_NAME OS_FunctionImp6 ## stdcall
#define OS_BIND_FUNC_RUN_CLASS_NAME OS_FunctionImp6_run ## stdcall
#elif OS_BIND_FUNC_NUM_ARGS == 7
#define OS_BIND_FUNC_CLASS_NAME OS_FunctionImp7 ## stdcall
#define OS_BIND_FUNC_RUN_CLASS_NAME OS_FunctionImp7_run ## stdcall
#elif OS_BIND_FUNC_NUM_ARGS == 8
#define OS_BIND_FUNC_CLASS_NAME OS_FunctionImp8 ## stdcall
#define OS_BIND_FUNC_RUN_CLASS_NAME OS_FunctionImp8_run ## stdcall
#elif OS_BIND_FUNC_NUM_ARGS == 9
#define OS_BIND_FUNC_CLASS_NAME OS_FunctionImp9 ## stdcall
#define OS_BIND_FUNC_RUN_CLASS_NAME OS_FunctionImp9_run ## stdcall
#elif OS_BIND_FUNC_NUM_ARGS == 10
#define OS_BIND_FUNC_CLASS_NAME OS_FunctionImp10 ## stdcall
#define OS_BIND_FUNC_RUN_CLASS_NAME OS_FunctionImp10_run ## stdcall
#elif OS_BIND_FUNC_NUM_ARGS == 11
#define OS_BIND_FUNC_CLASS_NAME OS_FunctionImp11 ## stdcall
#define OS_BIND_FUNC_RUN_CLASS_NAME OS_FunctionImp11_run ## stdcall
#elif OS_BIND_FUNC_NUM_ARGS == 12
#define OS_BIND_FUNC_CLASS_NAME OS_FunctionImp12 ## stdcall
#define OS_BIND_FUNC_RUN_CLASS_NAME OS_FunctionImp12_run ## stdcall
#elif OS_BIND_FUNC_NUM_ARGS == 13
#define OS_BIND_FUNC_CLASS_NAME OS_FunctionImp13 ## stdcall
#define OS_BIND_FUNC_RUN_CLASS_NAME OS_FunctionImp13_run ## stdcall
#elif OS_BIND_FUNC_NUM_ARGS == 14
#define OS_BIND_FUNC_CLASS_NAME OS_FunctionImp14 ## stdcall
#define OS_BIND_FUNC_RUN_CLASS_NAME OS_FunctionImp14_run ## stdcall
#elif OS_BIND_FUNC_NUM_ARGS == 15
#define OS_BIND_FUNC_CLASS_NAME OS_FunctionImp15 ## stdcall
#define OS_BIND_FUNC_RUN_CLASS_NAME OS_FunctionImp15_run ## stdcall
#endif

#elif defined OS_BIND_FUNC_FASTCALL

#ifdef __GNUC__
#define OS_BIND_FUNC_CC
#define OS_BIND_FUNC_CC_GNUC __attribute__((fastcall))
#else
#define OS_BIND_FUNC_CC __fastcall
#define OS_BIND_FUNC_CC_GNUC
#endif

#if OS_BIND_FUNC_NUM_ARGS == 0
#define OS_BIND_FUNC_CLASS_NAME OS_FunctionImp0 ## fastcall
#define OS_BIND_FUNC_RUN_CLASS_NAME OS_FunctionImp0_run ## fastcall
#elif OS_BIND_FUNC_NUM_ARGS == 1
#define OS_BIND_FUNC_CLASS_NAME OS_FunctionImp1 ## fastcall
#define OS_BIND_FUNC_RUN_CLASS_NAME OS_FunctionImp1_run ## fastcall
#elif OS_BIND_FUNC_NUM_ARGS == 2
#define OS_BIND_FUNC_CLASS_NAME OS_FunctionImp2 ## fastcall
#define OS_BIND_FUNC_RUN_CLASS_NAME OS_FunctionImp2_run ## fastcall
#elif OS_BIND_FUNC_NUM_ARGS == 3
#define OS_BIND_FUNC_CLASS_NAME OS_FunctionImp3 ## fastcall
#define OS_BIND_FUNC_RUN_CLASS_NAME OS_FunctionImp3_run ## fastcall
#elif OS_BIND_FUNC_NUM_ARGS == 4
#define OS_BIND_FUNC_CLASS_NAME OS_FunctionImp4 ## fastcall
#define OS_BIND_FUNC_RUN_CLASS_NAME OS_FunctionImp4_run ## fastcall
#elif OS_BIND_FUNC_NUM_ARGS == 5
#define OS_BIND_FUNC_CLASS_NAME OS_FunctionImp5 ## fastcall
#define OS_BIND_FUNC_RUN_CLASS_NAME OS_FunctionImp5_run ## fastcall
#elif OS_BIND_FUNC_NUM_ARGS == 6
#define OS_BIND_FUNC_CLASS_NAME OS_FunctionImp6 ## fastcall
#define OS_BIND_FUNC_RUN_CLASS_NAME OS_FunctionImp6_run ## fastcall
#elif OS_BIND_FUNC_NUM_ARGS == 7
#define OS_BIND_FUNC_CLASS_NAME OS_FunctionImp7 ## fastcall
#define OS_BIND_FUNC_RUN_CLASS_NAME OS_FunctionImp7_run ## fastcall
#elif OS_BIND_FUNC_NUM_ARGS == 8
#define OS_BIND_FUNC_CLASS_NAME OS_FunctionImp8 ## fastcall
#define OS_BIND_FUNC_RUN_CLASS_NAME OS_FunctionImp8_run ## fastcall
#elif OS_BIND_FUNC_NUM_ARGS == 9
#define OS_BIND_FUNC_CLASS_NAME OS_FunctionImp9 ## fastcall
#define OS_BIND_FUNC_RUN_CLASS_NAME OS_FunctionImp9_run ## fastcall
#elif OS_BIND_FUNC_NUM_ARGS == 10
#define OS_BIND_FUNC_CLASS_NAME OS_FunctionImp10 ## fastcall
#define OS_BIND_FUNC_RUN_CLASS_NAME OS_FunctionImp10_run ## fastcall
#elif OS_BIND_FUNC_NUM_ARGS == 11
#define OS_BIND_FUNC_CLASS_NAME OS_FunctionImp11 ## fastcall
#define OS_BIND_FUNC_RUN_CLASS_NAME OS_FunctionImp11_run ## fastcall
#elif OS_BIND_FUNC_NUM_ARGS == 12
#define OS_BIND_FUNC_CLASS_NAME OS_FunctionImp12 ## fastcall
#define OS_BIND_FUNC_RUN_CLASS_NAME OS_FunctionImp12_run ## fastcall
#elif OS_BIND_FUNC_NUM_ARGS == 13
#define OS_BIND_FUNC_CLASS_NAME OS_FunctionImp13 ## fastcall
#define OS_BIND_FUNC_RUN_CLASS_NAME OS_FunctionImp13_run ## fastcall
#elif OS_BIND_FUNC_NUM_ARGS == 14
#define OS_BIND_FUNC_CLASS_NAME OS_FunctionImp14 ## fastcall
#define OS_BIND_FUNC_RUN_CLASS_NAME OS_FunctionImp14_run ## fastcall
#elif OS_BIND_FUNC_NUM_ARGS == 15
#define OS_BIND_FUNC_CLASS_NAME OS_FunctionImp15 ## fastcall
#define OS_BIND_FUNC_RUN_CLASS_NAME OS_FunctionImp15_run ## fastcall
#endif

#elif defined OS_BIND_FUNC_THISCALL

#ifdef __GNUC__
#define OS_BIND_FUNC_CC
#define OS_BIND_FUNC_CC_GNUC __attribute__((thiscall))
#else
#define OS_BIND_FUNC_CC __thiscall
#define OS_BIND_FUNC_CC_GNUC
#endif

#if OS_BIND_FUNC_NUM_ARGS == 0
#define OS_BIND_FUNC_CLASS_NAME OS_FunctionImp0 ## thiscall
#define OS_BIND_FUNC_RUN_CLASS_NAME OS_FunctionImp0_run ## thiscall
#elif OS_BIND_FUNC_NUM_ARGS == 1
#define OS_BIND_FUNC_CLASS_NAME OS_FunctionImp1 ## thiscall
#define OS_BIND_FUNC_RUN_CLASS_NAME OS_FunctionImp1_run ## thiscall
#elif OS_BIND_FUNC_NUM_ARGS == 2
#define OS_BIND_FUNC_CLASS_NAME OS_FunctionImp2 ## thiscall
#define OS_BIND_FUNC_RUN_CLASS_NAME OS_FunctionImp2_run ## thiscall
#elif OS_BIND_FUNC_NUM_ARGS == 3
#define OS_BIND_FUNC_CLASS_NAME OS_FunctionImp3 ## thiscall
#define OS_BIND_FUNC_RUN_CLASS_NAME OS_FunctionImp3_run ## thiscall
#elif OS_BIND_FUNC_NUM_ARGS == 4
#define OS_BIND_FUNC_CLASS_NAME OS_FunctionImp4 ## thiscall
#define OS_BIND_FUNC_RUN_CLASS_NAME OS_FunctionImp4_run ## thiscall
#elif OS_BIND_FUNC_NUM_ARGS == 5
#define OS_BIND_FUNC_CLASS_NAME OS_FunctionImp5 ## thiscall
#define OS_BIND_FUNC_RUN_CLASS_NAME OS_FunctionImp5_run ## thiscall
#elif OS_BIND_FUNC_NUM_ARGS == 6
#define OS_BIND_FUNC_CLASS_NAME OS_FunctionImp6 ## thiscall
#define OS_BIND_FUNC_RUN_CLASS_NAME OS_FunctionImp6_run ## thiscall
#elif OS_BIND_FUNC_NUM_ARGS == 7
#define OS_BIND_FUNC_CLASS_NAME OS_FunctionImp7 ## thiscall
#define OS_BIND_FUNC_RUN_CLASS_NAME OS_FunctionImp7_run ## thiscall
#elif OS_BIND_FUNC_NUM_ARGS == 8
#define OS_BIND_FUNC_CLASS_NAME OS_FunctionImp8 ## thiscall
#define OS_BIND_FUNC_RUN_CLASS_NAME OS_FunctionImp8_run ## thiscall
#elif OS_BIND_FUNC_NUM_ARGS == 9
#define OS_BIND_FUNC_CLASS_NAME OS_FunctionImp9 ## thiscall
#define OS_BIND_FUNC_RUN_CLASS_NAME OS_FunctionImp9_run ## thiscall
#elif OS_BIND_FUNC_NUM_ARGS == 10
#define OS_BIND_FUNC_CLASS_NAME OS_FunctionImp10 ## thiscall
#define OS_BIND_FUNC_RUN_CLASS_NAME OS_FunctionImp10_run ## thiscall
#elif OS_BIND_FUNC_NUM_ARGS == 11
#define OS_BIND_FUNC_CLASS_NAME OS_FunctionImp11 ## thiscall
#define OS_BIND_FUNC_RUN_CLASS_NAME OS_FunctionImp11_run ## thiscall
#elif OS_BIND_FUNC_NUM_ARGS == 12
#define OS_BIND_FUNC_CLASS_NAME OS_FunctionImp12 ## thiscall
#define OS_BIND_FUNC_RUN_CLASS_NAME OS_FunctionImp12_run ## thiscall
#elif OS_BIND_FUNC_NUM_ARGS == 13
#define OS_BIND_FUNC_CLASS_NAME OS_FunctionImp13 ## thiscall
#define OS_BIND_FUNC_RUN_CLASS_NAME OS_FunctionImp13_run ## thiscall
#elif OS_BIND_FUNC_NUM_ARGS == 14
#define OS_BIND_FUNC_CLASS_NAME OS_FunctionImp14 ## thiscall
#define OS_BIND_FUNC_RUN_CLASS_NAME OS_FunctionImp14_run ## thiscall
#elif OS_BIND_FUNC_NUM_ARGS == 15
#define OS_BIND_FUNC_CLASS_NAME OS_FunctionImp15 ## thiscall
#define OS_BIND_FUNC_RUN_CLASS_NAME OS_FunctionImp15_run ## thiscall
#endif

#else

#define OS_BIND_FUNC_CC
#define OS_BIND_FUNC_CC_GNUC

#if OS_BIND_FUNC_NUM_ARGS == 0
#define OS_BIND_FUNC_CLASS_NAME OS_FunctionImp0
#define OS_BIND_FUNC_RUN_CLASS_NAME OS_FunctionImp0_run
#elif OS_BIND_FUNC_NUM_ARGS == 1
#define OS_BIND_FUNC_CLASS_NAME OS_FunctionImp1
#define OS_BIND_FUNC_RUN_CLASS_NAME OS_FunctionImp1_run
#elif OS_BIND_FUNC_NUM_ARGS == 2
#define OS_BIND_FUNC_CLASS_NAME OS_FunctionImp2
#define OS_BIND_FUNC_RUN_CLASS_NAME OS_FunctionImp2_run
#elif OS_BIND_FUNC_NUM_ARGS == 3
#define OS_BIND_FUNC_CLASS_NAME OS_FunctionImp3
#define OS_BIND_FUNC_RUN_CLASS_NAME OS_FunctionImp3_run
#elif OS_BIND_FUNC_NUM_ARGS == 4
#define OS_BIND_FUNC_CLASS_NAME OS_FunctionImp4
#define OS_BIND_FUNC_RUN_CLASS_NAME OS_FunctionImp4_run
#elif OS_BIND_FUNC_NUM_ARGS == 5
#define OS_BIND_FUNC_CLASS_NAME OS_FunctionImp5
#define OS_BIND_FUNC_RUN_CLASS_NAME OS_FunctionImp5_run
#elif OS_BIND_FUNC_NUM_ARGS == 6
#define OS_BIND_FUNC_CLASS_NAME OS_FunctionImp6
#define OS_BIND_FUNC_RUN_CLASS_NAME OS_FunctionImp6_run
#elif OS_BIND_FUNC_NUM_ARGS == 7
#define OS_BIND_FUNC_CLASS_NAME OS_FunctionImp7
#define OS_BIND_FUNC_RUN_CLASS_NAME OS_FunctionImp7_run
#elif OS_BIND_FUNC_NUM_ARGS == 8
#define OS_BIND_FUNC_CLASS_NAME OS_FunctionImp8
#define OS_BIND_FUNC_RUN_CLASS_NAME OS_FunctionImp8_run
#elif OS_BIND_FUNC_NUM_ARGS == 9
#define OS_BIND_FUNC_CLASS_NAME OS_FunctionImp9
#define OS_BIND_FUNC_RUN_CLASS_NAME OS_FunctionImp9_run
#elif OS_BIND_FUNC_NUM_ARGS == 10
#define OS_BIND_FUNC_CLASS_NAME OS_FunctionImp10
#define OS_BIND_FUNC_RUN_CLASS_NAME OS_FunctionImp10_run
#elif OS_BIND_FUNC_NUM_ARGS == 11
#define OS_BIND_FUNC_CLASS_NAME OS_FunctionImp11
#define OS_BIND_FUNC_RUN_CLASS_NAME OS_FunctionImp11_run
#elif OS_BIND_FUNC_NUM_ARGS == 12
#define OS_BIND_FUNC_CLASS_NAME OS_FunctionImp12
#define OS_BIND_FUNC_RUN_CLASS_NAME OS_FunctionImp12_run
#elif OS_BIND_FUNC_NUM_ARGS == 13
#define OS_BIND_FUNC_CLASS_NAME OS_FunctionImp13
#define OS_BIND_FUNC_RUN_CLASS_NAME OS_FunctionImp13_run
#elif OS_BIND_FUNC_NUM_ARGS == 14
#define OS_BIND_FUNC_CLASS_NAME OS_FunctionImp14
#define OS_BIND_FUNC_RUN_CLASS_NAME OS_FunctionImp14_run
#elif OS_BIND_FUNC_NUM_ARGS == 15
#define OS_BIND_FUNC_CLASS_NAME OS_FunctionImp15
#define OS_BIND_FUNC_RUN_CLASS_NAME OS_FunctionImp15_run
#endif

#endif

#if OS_BIND_FUNC_NUM_ARGS > 0

template <class R OS_BIND_FUNC_PARMS_COMMA OS_BIND_FUNC_TEMPLATE_PARMS> 
struct OS_BIND_FUNC_RUN_CLASS_NAME
{
	typedef R(OS_BIND_FUNC_CC *F)(OS_BIND_FUNC_PARMS) OS_BIND_FUNC_CC_GNUC;

	static int run(ObjectScript::OS * os, int params, int, int, void * user_param)
	{
		OS_BIND_FUNC_GET_ARGS;
		typedef typename RemoveConst<R>::type type;
		F& f = *(F*)user_param;
		CtypeValue<type>::push(os, (*f)(OS_BIND_FUNC_ARGS));
		return 1;
	}
};

template <OS_BIND_FUNC_TEMPLATE_PARMS> 
struct OS_BIND_FUNC_RUN_CLASS_NAME<void OS_BIND_FUNC_PARMS_COMMA OS_BIND_FUNC_PARMS>
{
	typedef void(OS_BIND_FUNC_CC *F)(OS_BIND_FUNC_PARMS) OS_BIND_FUNC_CC_GNUC;

	static int run(ObjectScript::OS * os, int params, int, int, void * user_param)
	{
		OS_BIND_FUNC_GET_ARGS;
		F& f = *(F*)user_param;
		(*f)(OS_BIND_FUNC_ARGS);
		return 0;
	}
};

template <class R OS_BIND_FUNC_PARMS_COMMA OS_BIND_FUNC_TEMPLATE_PARMS>
struct OS_BIND_FUNC_CLASS_NAME
{
	typedef R(OS_BIND_FUNC_CC *F)(OS_BIND_FUNC_PARMS) OS_BIND_FUNC_CC_GNUC;

	const char * name;
	F f;

	OS_BIND_FUNC_CLASS_NAME(const char * _name, F _f): name(_name), f(_f){}

	operator ObjectScript::OS::FuncDef() const 
	{ 
		ObjectScript::OS::FuncDef def = {name, 
			OS_BIND_FUNC_RUN_CLASS_NAME<R OS_BIND_FUNC_PARMS_COMMA OS_BIND_FUNC_PARMS>::run, 
			&(new OS_FunctionData<F>(f))->f}; 
		return def; 
	}
};

namespace ObjectScript {

template <class R OS_BIND_FUNC_PARMS_COMMA OS_BIND_FUNC_TEMPLATE_PARMS> 
ObjectScript::OS::FuncDef def(const char * name, R(OS_BIND_FUNC_CC *f)(OS_BIND_FUNC_PARMS) OS_BIND_FUNC_CC_GNUC)
{
	typedef OS_BIND_FUNC_CLASS_NAME<R OS_BIND_FUNC_PARMS_COMMA OS_BIND_FUNC_PARMS> Func; 
	return Func(name, f);
}

} // namespace ObjectScript

#else

struct OS_BIND_FUNC_CLASS_NAME
{
	typedef void(OS_BIND_FUNC_CC *F)() OS_BIND_FUNC_CC_GNUC;

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

inline ObjectScript::OS::FuncDef def(const char * name, void(OS_BIND_FUNC_CC *f)() OS_BIND_FUNC_CC_GNUC)
{
	typedef OS_BIND_FUNC_CLASS_NAME Func; 
	return Func(name, f);
}

} // namespace ObjectScript

#endif



#undef OS_BIND_FUNC_CLASS_NAME
#undef OS_BIND_FUNC_RUN_CLASS_NAME
#undef OS_BIND_FUNC_CC

#undef OS_BIND_FUNC_CC_GNUC

#undef OS_BIND_FUNC_PARMS_COMMA
#undef OS_BIND_FUNC_TEMPLATE_PARMS
#undef OS_BIND_FUNC_PARMS
#undef OS_BIND_FUNC_ARGS
#undef OS_BIND_FUNC_GET_ARGS
