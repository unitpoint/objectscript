#include "os-odbo.h"
#include "../objectscript.h"
#include "../os-binder.h"

#include <soci.h>
#include <soci-mysql.h>
#include <soci-simple.h>

namespace ObjectScript {

class ODBO_OS: public OS
{
public:

	static void triggerError(OS * os, const OS::String& msg)
	{
		os->getGlobal(OS_TEXT("ODBOException"));
		os->pushGlobals();
		os->pushString(msg);
		os->call(1, 1);
		os->setException();
	}

	static void triggerError(OS * os, const char * msg)
	{
		os->getGlobal(OS_TEXT("ODBOException"));
		os->pushGlobals();
		os->pushString(msg);
		os->call(1, 1);
		os->setException();
	}

	struct ODBO
	{
		OS * os;
		session_handle handle;

		ODBO(OS * p_os)
		{
			os = p_os;
			handle = NULL;
		}

		~ODBO()
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
			ODBO * self = new (os->malloc(sizeof(ODBO) OS_DBG_FILEPOS)) ODBO(os);
			if(params >= 2 && os->isObject(-params+1)){
				OS::Core::Buffer connection_str(os);
				connection_str.append(os->toString(-params+0));
				connection_str.append("://");
				if(params > 2){
					os->pop(params-2);
				}
				for(bool i = 0; os->nextIteratorStep(); i++){
					OS::String key = os->toString(-2);
					OS::String value = os->toString(-1);

					if(!isValidOption(key) || !isValidOption(value)){
						triggerError(os, OS::String::format(os, "invalid char of option '%s=%s'", key.toChar(), value.toChar()));
						self->close();
						break;
					}
					if(i > 0){
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
			self->~ODBO();
			os->free(self);
			return 0;
		}
	};

	static void initLibrary(OS* os);

};

template <> struct CtypeName<ODBO_OS::ODBO>{ static const OS_CHAR * getName(){ return OS_TEXT("ODBO"); } };
template <> struct CtypeValue<ODBO_OS::ODBO*>: public CtypeUserClass<ODBO_OS::ODBO*>{};
template <> struct UserDataDestructor<ODBO_OS::ODBO>
{
	static void dtor(ObjectScript::OS * os, void * data, void * user_param)
	{
		OS_ASSERT(data && dynamic_cast<ODBO_OS::ODBO*>((ODBO_OS::ODBO*)data));
		ODBO_OS::ODBO * buf = (ODBO_OS::ODBO*)data;
		buf->~ODBO();
		os->free(buf);
	}
};

void ODBO_OS::initLibrary(OS* os)
{
	soci::register_factory_mysql();
	{
		OS::FuncDef funcs[] = {
			{OS_TEXT("__construct"), ODBO::__construct},
			// {OS_TEXT("query"), Sqlite::query},
			// {OS_TEXT("__get@lastInsertId"), Sqlite::getLastInsertId},
			// {OS_TEXT("getLastInsertId"), Sqlite::getLastInsertId},
			{}
		};

		registerUserClass<ODBO>(os, funcs);
	}
#define OS_AUTO_TEXT(exp) OS_TEXT(#exp)
	os->eval(OS_AUTO_TEXT(
		ODBOException = extends Exception {
		}
		function ODBO.execute(sql, params){
			return @query(sql, params).execute()
		}
		function ODBO.fetch(sql, params){
			return @query(sql, params).fetch()
		}
	));
}

void initODBOLibrary(OS* os)
{
	ODBO_OS::initLibrary(os);
}

} // namespace ObjectScript
