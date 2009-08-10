// For conditions of distribution and use, see copyright notice in license.txt

#ifndef incl_Interfaces_ScriptServiceInterface_h
#define incl_Interfaces_ScriptServiceInterface_h

#include "ServiceInterface.h"

namespace Foundation
{


	class ScriptObject
	{
	public:
        ScriptObject() {}
        virtual ~ScriptObject() {}
        virtual void ReleaseResources() = 0;

		// for syntax parameter see http://docs.python.org/c-api/arg.html
		virtual ScriptObject* CallMethod(std::string& methodname, std::string& syntax, char* argv[]) = 0;
		virtual ScriptObject* CallMethod2(std::string& methodname, std::string& syntax, ...) = 0;
		virtual ScriptObject* CallMethod(std::string& methodname, const std::string& syntax, const ScriptObject* args) = 0;
		virtual ScriptObject* GetObject(const std::string& objectname) = 0;

		virtual char* ConvertToChar() = 0;
	};

	typedef boost::shared_ptr<ScriptObject> ScriptObjectPtr;

	class ScriptEventInterface
	{
	public:
        ScriptEventInterface() {}
        virtual ~ScriptEventInterface() {}
		virtual void SetCallback(void(*f)(char*), std::string key) = 0;
		virtual void NotifyScriptEvent(const std::string& key, const std::string& message) = 0;		
	};

    //! Script service interface.
    /*!
        \ingroup Services_group
    
        Provides execution of interpreted code as a service for any module to use. Used (in 0.0.1 release) by CommunicationsModule to load the Python written, Telepathy libs using impementation for instant messaging services.
        Implemented by the \ref PythonScriptModule.
    */

    class ScriptServiceInterface : public ServiceInterface
    {
    public:
        ScriptServiceInterface() {}
        virtual ~ScriptServiceInterface() {}


        virtual void RunScript(const std::string& scriptname) = 0;
		virtual void RunString(const std::string& codestr) = 0;

		virtual ScriptObject* LoadScript(const std::string& scriptname, std::string& error) = 0;
		virtual ScriptObject* GetObject(const ScriptObject& script, const std::string& objectname, std::string& error) = 0;

    };
}

#endif

