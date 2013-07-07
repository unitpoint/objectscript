#include "os-dbal.h"
#include "../objectscript.h"
#include "../os-binder.h"

#include <soci.h>
#include <soci-mysql.h>
#include <soci-simple.h>

namespace ObjectScript {

class DBAL_OS: public OS
{
public:

	static void triggerError(OS * os, const OS::String& msg)
	{
		os->getGlobal(OS_TEXT("DBALException"));
		os->pushGlobals();
		os->pushString(msg);
		os->call(1, 1);
		os->setException();
	}

	static void triggerError(OS * os, const char * msg)
	{
		os->getGlobal(OS_TEXT("DBALException"));
		os->pushGlobals();
		os->pushString(msg);
		os->call(1, 1);
		os->setException();
	}

	struct DBAL
	{
		OS * os;
		session_handle handle;

		DBAL(OS * p_os)
		{
			os = p_os;
			handle = NULL;
		}

		~DBAL()
		{
			close();
		}

		bool isOpen() const { return handle != NULL; }

		void close()
		{
			if(handle){
				soci_destroy_session(handle);
				handle = NULL;
			}
		}

		bool open(const char * connection_string)
		{
			close();
			handle = soci_create_session(connection_string);
			if(handle && !soci_session_state(handle)){
				triggerError(os, soci_session_error_message(handle));
				close();
			}
			return isOpen();
		}

		static bool isValidOption(const char * s, int len)
		{
			for(int i = 0; i < len; i++){
				if(!s[i] || s[i] <= ' '){
					return false;
				}
			}
			return true;
		}

		static bool isValidOption(const OS::String& s)
		{
			return isValidOption(s, s.getLen());
		}

		static int __construct(OS * os, int params, int, int, void * user_param)
		{
			if(params < 1){
				triggerError(os, OS_TEXT("driver parameter requied"));
				return 0;
			}
			DBAL * self = new (os->malloc(sizeof(DBAL) OS_DBG_FILEPOS)) DBAL(os);
			if(params >= 2 && os->isObject(-params+1)){
				OS::Core::Buffer connection_str(os);
				connection_str.append(os->toString(-params+0));
				connection_str.append("://");
				if(params > 2){
					os->pop(params-2);
				}
				while(os->nextIteratorStep()){
					OS::String key = os->toString(-2);
					OS::String value = os->toString(-1);

					if(!isValidOption(key) || !isValidOption(value)){
						triggerError(os, OS::String::format(os, "invalid char of option '%s=%s'", key.toChar(), value.toChar()));
						self->close();
						break;
					}
					if(!connection_str.getSize()){
						connection_str.append(" ");
					}
					connection_str.append(key);
					connection_str.append("=");
					connection_str.append(value);

					os->pop(2);
				}
				self->open(connection_str.toString());
			}else{
				self->open(os->toString(-params+0));
			}
			if(self->isOpen()){
				pushCtypeValue(os, self);
				return 1;
			}
			self->~DBAL();
			os->free(self);
			return 0;
		}
	};

	static void initLibrary(OS* os);

};

template <> struct CtypeName<DBAL_OS::DBAL>{ static const OS_CHAR * getName(){ return OS_TEXT("DBAL"); } };
template <> struct CtypeValue<DBAL_OS::DBAL*>: public CtypeUserClass<DBAL_OS::DBAL*>{};
template <> struct UserDataDestructor<DBAL_OS::DBAL>
{
	static void dtor(ObjectScript::OS * os, void * data, void * user_param)
	{
		OS_ASSERT(data && dynamic_cast<DBAL_OS::DBAL*>((DBAL_OS::DBAL*)data));
		DBAL_OS::DBAL * buf = (DBAL_OS::DBAL*)data;
		buf->~DBAL();
		os->free(buf);
	}
};

void DBAL_OS::initLibrary(OS* os)
{
	soci::register_factory_mysql();
	{
		OS::FuncDef funcs[] = {
			{OS_TEXT("__construct"), DBAL::__construct},
			// {OS_TEXT("query"), Sqlite::query},
			// {OS_TEXT("__get@lastInsertId"), Sqlite::getLastInsertId},
			// {OS_TEXT("getLastInsertId"), Sqlite::getLastInsertId},
			{}
		};

		registerUserClass<DBAL>(os, funcs);
	}
#define OS_AUTO_TEXT(exp) OS_TEXT(#exp)
	os->eval(OS_AUTO_TEXT(
		DBALException = extends Exception {
		}
		function DBAL.execute(sql, params){
			return @query(sql, params).execute()
		}
		function DBAL.fetch(sql, params){
			return @query(sql, params).fetch()
		}
	));
}

void initDBALLibrary(OS* os)
{
	DBAL_OS::initLibrary(os);
}

} // namespace ObjectScript
