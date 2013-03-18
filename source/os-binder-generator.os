var header = <<<===END==='
/*
AUTO-GENERATED FILE. DO NOT MODIFY.

Note: this header is a header template
and must NOT have multiple-inclusion protection.
*/

/******************************************************************************
* Copyright (C) 2012-2013 Evgeniy Golovin (evgeniy.golovin@unitpoint.ru)
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


===END===

var MAX_ARGS = 11

var content = header .. <<<===END==='
#if OS_BIND_FUNC_NUM_ARGS == 0

#define OS_BIND_FUNC_PARMS_COMMA
#define OS_BIND_FUNC_TEMPLATE_PARMS
#define OS_BIND_FUNC_PARMS
#define OS_BIND_FUNC_ARGS
#define OS_BIND_FUNC_GET_ARGS


===END===

function file_get_contents(filename)
{
	var f = File(filename, "rb")
	var content = f.read()
	f.close()
	return content
}

function file_put_contents(filename, content)
{
	var f = File(filename, "wb")
	f.write(content)
	f.close()
}

function get_template_parms(i)
{
	var items = []
	for(var j = 1; j <= i; j++){
		items[] = "class ARG_TYPE_${j}"
	}
	return items.join(", ")
}

function get_parms(i)
{
	var items = []
	for(var j = 1; j <= i; j++){
		items[] = "ARG_TYPE_${j}"
	}
	return items.join(", ")
}

function get_args(i)
{
	var items = []
	for(var j = 1; j <= i; j++){
		items[] = "arg${j}"
	}
	return items.join(", ")
}

function get_get_args(i)
{
	var items = []
	for(var j = 1; j <= i; j++){
		items[] = "OS_GET_TEMPLATE_ARG(${j}, ARG_TYPE_${j})"
	}
	return " \\\n\tint cur_param_offs = -params; \\\n\t"..items.join("; \\\n\t")
}

for(var i = 1; i <= MAX_ARGS; i++){
	content = content
		.. "#elif OS_BIND_FUNC_NUM_ARGS == ${i}\n\n"
		.. "#define OS_BIND_FUNC_PARMS_COMMA ,\n"
		.. "#define OS_BIND_FUNC_TEMPLATE_PARMS ${get_template_parms(i)}\n"
		.. "#define OS_BIND_FUNC_PARMS ${get_parms(i)}\n"
		.. "#define OS_BIND_FUNC_ARGS ${get_args(i)}\n"
		.. "#define OS_BIND_FUNC_GET_ARGS ${get_get_args(i)}\n\n"
}

content = content .. "#endif\n\n"

var cc_types = ["cdecl", "stdcall", "fastcall", "thiscall"]

for(var _, class_name in [
    "OS_FunctionClassImpConst",
    "OS_FunctionClassImp",
    "OS_FunctionImp",
]){

    for(var i, cc in cc_types){

        content = content 
			.. (i > 0 ? "#elif defined " : "#ifdef ")
			.. "OS_BIND_FUNC_${cc.upper()}\n\n"
			.. "#ifdef __GNUC__\n"
			.. "#define OS_BIND_FUNC_CC\n"
			.. "#define OS_BIND_FUNC_CC_GNUC __attribute__((${cc}))\n"
			.. "#else\n"
			.. "#define OS_BIND_FUNC_CC __${cc}\n"
			.. "#define OS_BIND_FUNC_CC_GNUC\n"
			.. "#endif\n\n"

        for(var j = 0; j <= MAX_ARGS; j++){
			content = content 
				.. (j > 0 ? "#elif " : "#if ").."OS_BIND_FUNC_NUM_ARGS == ${j}\n"
				.. "#define OS_BIND_FUNC_CLASS_NAME ${class_name}${j} ## _${cc}\n"
				.. "#define OS_BIND_FUNC_RUN_CLASS_NAME ${class_name}${j}_run ## _${cc}\n"
        }
        content = content .. "#endif\n\n"
    }
    if(true){
        content = content 
			.. "#else\n\n"
			.. "#define OS_BIND_FUNC_CC\n"
			.. "#define OS_BIND_FUNC_CC_GNUC\n\n"

        for(var j = 0; j <= MAX_ARGS; j++){
			content = content 
				.. (j > 0 ? "#elif " : "#if ").."OS_BIND_FUNC_NUM_ARGS == ${j}\n"
				.. "#define OS_BIND_FUNC_CLASS_NAME ${class_name}${j}\n"
				.. "#define OS_BIND_FUNC_RUN_CLASS_NAME ${class_name}${j}_run\n"
        }
        content = content .. "#endif\n\n"
    }
    content = content .. "#endif\n\n"

    if(class_name == "OS_FunctionClassImpConst"){
        impl = file_get_contents("os-binder-FunctionClassImp.tpl").trim()
        impl = impl.replace("{const}", "const")
    }else if(class_name == "OS_FunctionClassImp"){
        impl = file_get_contents("os-binder-FunctionClassImp.tpl").trim()
        impl = impl.replace("{const}", "")
    }else{
        impl = "#if OS_BIND_FUNC_NUM_ARGS > 0\n\n"
			.. file_get_contents("os-binder-FunctionImp.tpl").trim()
			.. "\n\n#else\n\n"
			.. file_get_contents("os-binder-FunctionImpVoid.tpl").trim()
			.. "\n\n#endif\n\n"
    }
    content = content 
		.. "${impl}\n\n"
		.. "#undef OS_BIND_FUNC_CLASS_NAME\n"
		.. "#undef OS_BIND_FUNC_RUN_CLASS_NAME\n"
		.. "#undef OS_BIND_FUNC_CC\n\n"
		.. "#undef OS_BIND_FUNC_CC_GNUC\n\n"
}

content = content 
	.. "#undef OS_BIND_FUNC_PARMS_COMMA\n"
	.. "#undef OS_BIND_FUNC_TEMPLATE_PARMS\n"
	.. "#undef OS_BIND_FUNC_PARMS\n"
	.. "#undef OS_BIND_FUNC_ARGS\n"
	.. "#undef OS_BIND_FUNC_GET_ARGS\n"

file_put_contents("os-binder-function.h", content)

content = header
	.. "#if defined __GNUC__ || !(defined(__i386__) || defined(__i386) || defined(__X86__))\n"
	.. "#include \"os-binder-function.h\"\n"
	.. "#else\n\n"
for(var _, cc in cc_types){
    content = content 
		.. "#define OS_BIND_FUNC_${cc.upper()}\n"
		.. "#include \"os-binder-function.h\"\n"
		.. "#undef OS_BIND_FUNC_${cc.upper()}\n\n"
}
content = content .. "#endif\n"
file_put_contents("os-binder-cc-functions.h", content)

content = header
for(var i = 0; i <= MAX_ARGS; i++){
    content = content 
		.. "#define OS_BIND_FUNC_NUM_ARGS ${i}\n"
		.. "#include \"os-binder-cc-functions.h\"\n"
		.. "#undef OS_BIND_FUNC_NUM_ARGS\n\n"
}
file_put_contents("os-binder-arg-cc-functions.h", content)
