#include "StableHeaders.h"
#include "Foundation.h"
#include "PythonEngine.h"
#include "PythonScriptModule.h"
#include "PythonScriptObject.h"

#ifdef unix
#include <cstdarg>
#endif

namespace PythonScript
{



	PythonScriptObject::PythonScriptObject(void) : Foundation::ScriptObject()
	{
        this->pythonObj=NULL;
        this->pythonRef=NULL;
	}

	PythonScriptObject::~PythonScriptObject(void)
	{
        if(pythonObj!=NULL&&pythonRef!=NULL){
            Py_DECREF(pythonObj);
        }
		if(pythonRef!=NULL){
            Py_DECREF(pythonRef);
		}
	}

    void PythonScriptObject::ReleaseResources()
    {
        if(pythonObj!=NULL&&pythonRef!=NULL){
            Py_DECREF(pythonObj);
        }
		if(pythonRef!=NULL){
            Py_DECREF(pythonRef);
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

			delete m;
			delete s;
            delete []*argv;
			PythonScriptObject* obj = new PythonScriptObject();
			obj->pythonObj = pRetValue;
			return obj;
		}
	}

	Foundation::ScriptObject* PythonScriptObject::CallMethod2(std::string& methodname, 
															 std::string& syntax, 
															 ...)
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
#ifndef unix
			PyObject* pRetValue = PyObject_CallMethod(this->pythonObj, 
													  m, 
													  s
													  __VA_ARGS__
													  );
#else
			// Macro __VA_ARGS__ is not defined in unix side, gives compile error, i do not know what is happening here so i just add this for unix 
			// probably it won't work. 

			va_list ap;
			PyObject* pRetValue = PyObject_CallMethod(this->pythonObj, 
								  m, 
								  s, 
								  ap);
#endif
			//PyObject* pRetValue = PyObject_CallMethod(this->pythonObj, 
			//										  m, 
			//										  s, 
			//										  argv[0]);
			delete m;
			delete s;
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

    /**
     * returns pyobject as char array, return value must not be modified (memory freeing is handled by python,
     * when ReleaseResources and destructor methods are called)
     */
	char* PythonScriptObject::ConvertToChar(){
		if (!this->pythonObj)
			return NULL;
		return PyString_AsString(this->pythonObj);
	}

}
