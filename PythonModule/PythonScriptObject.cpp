#include "StableHeaders.h"
#include "Foundation.h"
#include "PythonEngine.h"
#include "PythonScriptModule.h"
#include "PythonScriptObject.h"

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
            ////Py_ssize_t count = pythonObj->ob_refcnt;
            //int count = (int)pythonObj->ob_refcnt;
            //if(count>0){
            //    for(int i=0;i<count;i++){
                    Py_DECREF(pythonObj);
            //    }
            //}
        }
		if(pythonRef!=NULL){
            ////Py_ssize_t count = pythonRef->ob_refcnt;
            //int count = (int)pythonRef->ob_refcnt;
            //if(count>0){
            //    for(int i=0;i<count;i++){
                    Py_DECREF(pythonRef);
            //    }
            //}
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
			PyObject* pRetValue = PyObject_CallMethod(this->pythonObj, 
													  m, 
													  s
													  __VA_ARGS__
													  );

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

	char* PythonScriptObject::ConvertToChar(){
		return PyString_AsString(this->pythonObj);
	}

}