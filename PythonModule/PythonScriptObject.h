#ifndef incl_PythonScriptObject_Script_h
#define incl_PythonScriptObject_Script_h


//#pragma once
//#include "c:\code\rexng3\interfaces\scriptserviceinterface.h"
//#include "ScriptServiceInterface.h"

namespace PythonScript
{

	class PythonScriptObject : public Foundation::ScriptObject
	{
	public:
		PythonScriptObject(void);
		virtual ~PythonScriptObject(void);
	  
		
		virtual bool PassFunctionPointerToScript(void(*f)(char*), const std::string& methodname, std::string key);
		//virtual bool PassFunctionPointerToScript(void* methodptr, const std::string& methodname);
		virtual ScriptObject* CallMethod(std::string& methodname, std::string& syntax, char* argv[]);
		virtual ScriptObject* CallMethod(std::string& methodname, const std::string& syntax, const ScriptObject* args);
		virtual ScriptObject* GetObject(const std::string& objectname);

		//virtual std::string ConvertToStdString();
		virtual char* ConvertToChar();
	private:
		//static PyObject* ScriptCallbackMethod(PyObject *self, PyObject *args);

	public:
		PyObject* pythonRef; // reference to class or python script
		PyObject* pythonObj; // object instance
	
	private:
		static std::map<std::string, void(*)(char*)> methods;
		//static std::map<std::string, void*> methods;
		//void* methodptr;
		

	};
}

#endif