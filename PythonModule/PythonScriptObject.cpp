#include "StableHeaders.h"
#include "Foundation.h"
#include "PythonEngine.h"
#include "PythonScriptModule.h"

//#include "Python/Python.h" //should be <Python.h> or "Python.h" ?
#include <Python.h> //should be <Python.h> or "Python.h" ?


#include "PythonScriptObject.h"



namespace PythonScript
{



	PythonScriptObject::PythonScriptObject(void) : Foundation::ScriptObject()
	{
	}

	PythonScriptObject::~PythonScriptObject(void)
	{
		if(pythonRef!=NULL){
			delete pythonRef;
			pythonRef = NULL;
		}
		if(pythonObj!=NULL){
			delete pythonObj;
			pythonObj = NULL;
		}
	}


	Foundation::ScriptObject* PythonScriptObject::CallMethod(std::string& methodname, 
															 std::string& syntax, 
															 char* argv[])
	{
		if(syntax==""){
			char *m = new char[methodname.size()+1];
			strcpy(m, methodname.c_str());
			PyObject* pRetValue = PyObject_CallMethod(this->pythonObj, 
													  m, 
													  NULL, 
													  NULL);
			delete m;
			PythonScriptObject* obj = new PythonScriptObject();
			obj->pythonObj = pRetValue;
			return obj;

		} else {
			char *m = new char[methodname.size()+1];
			char *s = new char[syntax.size()+1];


			strcpy(m, methodname.c_str());
			strcpy(s, syntax.c_str());
			PyObject* pRetValue = PyObject_CallMethod(this->pythonObj, 
													  m, 
													  s, 
													  *argv);

			//PyObject* pRetValue = PyObject_CallMethod(this->pythonObj, 
			//										  m, 
			//										  s, 
			//										  argv[0]);
			delete m;
			delete s;
            delete []*argv;
			PythonScriptObject* obj = new PythonScriptObject();
			obj->pythonObj = pRetValue;
			return obj;
		}
	}

	Foundation::ScriptObject* PythonScriptObject::CallMethod(std::string& methodname, 
															 const std::string& syntax, 
															 const Foundation::ScriptObject* args)
	{
		return NULL;
	}

	Foundation::ScriptObject* PythonScriptObject::GetObject(const std::string& objectname)
	{
		PyObject *pDict, *pClass, *pInstance;
		pDict = PyModule_GetDict(this->pythonRef);
		if(pDict==NULL){throw "unable to get module namespace";}
		// Build the name of a callable class 
		pClass = PyDict_GetItemString(pDict, objectname.c_str());
		if(pClass==NULL){throw "unable get class from module namespace";}

		PythonScriptObject* obj = new PythonScriptObject();
		obj->pythonRef = pClass;

		// Create an instance of the class
		if (PyCallable_Check(pClass)) {
			pInstance = PyObject_CallObject(pClass, NULL); 
		} else {
			throw "unable to create instance from class";
		}
		obj->pythonObj = pInstance;
		return obj;
	}

	char* PythonScriptObject::ConvertToChar(){
		return PyString_AsString(this->pythonObj);
	}

}