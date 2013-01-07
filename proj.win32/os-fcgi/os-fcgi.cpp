// os-insight.cpp: определяет точку входа для консольного приложения.
//

#include "stdafx.h"
#include "../../source/objectscript.h"
#include "../../source/os-binder.h"
#include <Windows.h>
#include <string>
#include "fcgi-2.4.1/include/fcgi_stdio.h"
#include <stdlib.h>
#include "MPFDParser-1.0\Parser.h"

#pragma comment (lib, "Ws2_32.lib")

using namespace ObjectScript;

class FCGX_OS: public OS
{
protected:

	FCGX_Request * request;

	virtual ~FCGX_OS()
	{
	}

public:

	FCGX_OS()
	{
		request = NULL;
	}

	void initEnv(const char * var_name, char ** envp)
	{
		newObject();
		for(; *envp; envp++){
			const char * value = *envp;
			const char * split = strchr(value, '=');
			OS_ASSERT(split);
			if(split){
				pushStackValue(-1);
				pushString(value, split - value);
				pushString(split + 1);
				setProperty(false);
			}
		}
		setGlobal(var_name, false);
	}

	void printf(const OS_CHAR * fmt, ...)
	{
		va_list ap;
		va_start(ap, fmt);
		FCGX_VFPrintF(request->out, fmt, ap);
		va_end(ap);
	}

	void processRequest(FCGX_Request * p_request)
	{
		request = p_request;

		initEnv("_SERVER", request->envp);
		initEnv("_ENV", environ);
		
		getGlobal("_SERVER");
		getProperty("SCRIPT_FILENAME", false, false);
		String script_filename = popString();
		
		require(script_filename, true);
	}
};

static void PrintEnv(FCGX_Stream *out, char *label, char **envp)
{
    FCGX_FPrintF(out, "%s:<br>\n<pre>\n", label);
    for( ; *envp != NULL; envp++) {
        FCGX_FPrintF(out, "%s\n", *envp);
    }
    FCGX_FPrintF(out, "</pre><p>\n");
}

int _tmain(int argc, _TCHAR* argv[])
{
	std::string port=":9000"; // Задаем номер порта TCP
    int  listenQueueBacklog = 400; // Глубина стека запросов
    // FCGX_Stream *in, *out, *err;
    // FCGX_ParamArray envp;

    if(FCGX_Init()){ // Инициализируем библиотеку перед работой.
		exit(1); 
	}

    int  listen_socket = FCGX_OpenSocket(port.c_str(), listenQueueBacklog); // Открываем новый слушающий сокет
    if(listen_socket < 0){
		exit(1);
	}

    FCGX_Request request;
    if(FCGX_InitRequest(&request, listen_socket, 0)){
		exit(1); // Инициализируем структуру запроса
	}

	const char * multipartFormData = "multipart/form-data;";
	int multipartFormDataLen = strlen(multipartFormData);
	
	int maxPostDataLen = 5*1024*1024;

    while(FCGX_Accept_r(&request) == 0){
#if 1
		FCGX_OS * os = OS::create(new FCGX_OS());
		os->processRequest(&request);
		os->release();

        FCGX_Finish_r(&request); // Завершаем запрос

		finalizeAllBinds();
#elif 1
        FCGX_FPrintF(request.out, "Content-type: text/html\r\n\r\n<TITLE>fastcgi</TITLE>\n<H1>Fastcgi: Hello world.</H1>\n");

        PrintEnv(request.out, "Request environment", request.envp);
        PrintEnv(request.out, "Initial environment", environ);

		char *contentLength = FCGX_GetParam("CONTENT_LENGTH", request.envp);
		int len = contentLength ? strtol(contentLength, NULL, 10) : 0;
		if(len > maxPostDataLen){
			FCGX_FPrintF(request.out, "Max post data len: %d > %d\n", len, maxPostDataLen);
			continue;
		}
		char * contentType = FCGX_GetParam("CONTENT_TYPE", request.envp);
		if(contentType && len > 0 && strncmp(contentType, multipartFormData, multipartFormDataLen) == 0){
			MPFD::Parser POSTParser = MPFD::Parser();
			POSTParser.SetTempDirForFileUpload("/tmp");
			// POSTParser.SetMaxCollectedDataLength(20*1024);
			POSTParser.SetContentType(contentType);

			char * buf = new char[len];
			len = FCGX_GetStr(buf, len, request.in);

			POSTParser.SetExternalDataBuffer(buf, len);
			POSTParser.FinishData();

			std::map<std::string, MPFD::Field *> fields = POSTParser.GetFieldsMap();
			FCGX_FPrintF(request.out, "Have %d fields<p>\n", fields.size());

			std::map<std::string, MPFD::Field *>::iterator it;
			for(it = fields.begin(); it != fields.end(); it++){
				MPFD::Field * field = fields[it->first];
				if(field->GetType() == MPFD::Field::TextType){
					FCGX_FPrintF(request.out, "Got text field: '%s', value: '%s'<br>\n", it->first.c_str(), field->GetTextTypeContent().c_str());
				}else{
					FCGX_FPrintF(request.out, "Got text field: '%s', filename: '%s', tempfilename: '%s', mime-type: '%s'<br>\n", it->first.c_str(), 
						field->GetFileName().c_str(),
						field->GetTempFileNameEx().c_str(),
						field->GetFileMimeType().c_str()
						// field->GetTextTypeContent().c_str()
						);
				}
			}
		}else{
            FCGX_FPrintF(request.out, "No data from standard input.<p>\n");
		}
        FCGX_Finish_r(&request); // Завершаем запрос
#else
        if (len <= 0) {
            FCGX_FPrintF(request.out, "No data from standard input.<p>\n");
        }
        else {
            int i, ch;

            FCGX_FPrintF(request.out, "Standard input:<br>\n<pre>\n");
            for (i = 0; i < len; i++) {
                if ((ch = FCGX_GetChar(request.in)) < 0) {
                    FCGX_FPrintF(request.out,
                        "Error: Not enough bytes received on standard input<p>\n");
                    break;
                }
				switch(ch){
				case '<':
					FCGX_FPrintF(request.out, "&lt;");
					break;

				case '>':
					FCGX_FPrintF(request.out, "&gt;");
					break;

				default:
					FCGX_PutChar(ch, request.out);
				}
            }
            FCGX_FPrintF(request.out, "\n</pre><p>\n");
        }
        FCGX_Finish_r(&request); // Завершаем запрос
#endif
    }

	return 0;
}

