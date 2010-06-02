// For conditions of distribution and use, see copyright notice in license.txt

#include "StableHeaders.h"
#include "DebugOperatorNew.h"
#include "PythonScriptModule.h"
#include "PythonEngine.h"
//#include <stdlib.h> //_putenv for PYTHONHOME on windows
#include <Poco/Environment.h>

#include "MemoryLeakCheck.h"

using namespace Foundation;

namespace PythonScript
{
    PythonEngine::PythonEngine(Framework* framework) :
        framework_(framework)
    {
    }

    PythonEngine::~PythonEngine()
    {
        //deinit / free
    }

    void PythonEngine::Initialize()
    {
        if (!Py_IsInitialized())
        {
#ifdef _WIN32
            Poco::Environment::set("PYTHONHOME", ".\\pymodules");
            //_putenv("PYTHONHOME = .\\pymodules");
            //loads pymodules/lib/site.py which is the windows 
            //std::cout << "PYTHONHOME SET";
#endif
            Py_Initialize();
#ifdef _WIN32
            //for some reason setting env vars has no effect when running from inside visual studio,
            //so for VS use, the PYTHONHOME env var had to be set in the project file
            //.. that is inconvenient, so changed the path manipulation to back here.
            RunString("import sys; sys.path.append('pymodules/python26_Lib.zip');");

#endif
            RunString("import sys; sys.path.append('pymodules');"); //XXX change to the c equivalent
            RunString("import sys; sys.path.append('pymodules/lib');"); // libraries directory
            RunString("import sys; sys.path.append('pymodules/DLLs');"); // dll directory (PYTHONHOME doesent seem to work to get these)
        }
        else
        {
            //LogWarning() //XXX add module ref here to be able to do logging
            std::cout << "Python already initialized in PythonScriptModule init!";
        }
    }

    /* load modules and get pointers to py functions - used to be in module postinitialize
        pModule = PyImport_Import(pName);
        Py_DECREF(pName);

        if (pModule != NULL) {
            pFunc = PyObject_GetAttrString(pModule, "onChat");
            //pFunc is a new reference

            if (pFunc && PyCallable_Check(pFunc)) {
                LogInfo("Registered callback onChat from chathandler.py");
            }
            else {
                if (PyErr_Occurred())
                    PyErr_Print();
                LogInfo("Cannot find function");
            }
        }
        else 
            LogInfo("chathandler.py not found");
    */

    void PythonEngine::Uninitialize()
    {
        /*Py_XDECREF(pFunc);
        Py_DECREF(pModule);*/

        Py_Finalize();
    }

    /*void PythonEngine::Reset()
    {
        //should clear own internal state etc too i guess XXX
        Py_Finalize();
        Py_Initialize();
        //no ref to mod here now XXX
        //PythonModule::LogInfo("Python interpreter reseted: all memory and state cleared.");
    }*/

    void PythonEngine::RunString(QString codestr)
    {
        PyRun_SimpleString(codestr.toAscii().data());
    }

    void PythonEngine::RunScript(QString scriptname)
    {
        FILE *fp = fopen(scriptname.toAscii().data(), "r");
	if (!fp) {
            PythonScriptModule::LogInfo("Failed to open script " + scriptname.toStdString());
	    return;
	}
	PyRun_SimpleFile(fp, scriptname.toAscii().data());
	fclose(fp);
    }

    //===============================================================================================//
    
    //                                    //
    //         "GENERIC INTERFACE"        //
    //                                    //
    /* deprecated - PythonQt does this too

    Foundation::ScriptObject* PythonEngine::LoadScript(const std::string& scriptname, std::string& error)
    {
        PyObject *pName, *pModule;
        error = "None";
        //std::string scriptPath("pymodules");
        //PyImport_ImportModule(scriptPath.c_str());
        pName = PyString_FromString(scriptname.c_str());
        //pName = PyString_FromString(scriptPath.c_str());
        if(pName==NULL){ error = "name parsing failed"; return NULL;}
        pModule = PyImport_Import(pName);
        if(pModule==NULL){ error = "module loading failed"; return NULL;}
        PythonScriptObject* obj = new PythonScriptObject();
        obj->pythonRef = pModule;
        return obj;
    }
    
    Foundation::ScriptObject* PythonEngine::GetObject(const Foundation::ScriptObject& script, 
                                                      const std::string& objectname, 
                                                      std::string& error)
    {
        PyObject *pDict, *pClass, *pInstance;
        error = "None";
        // try casting script to PythonScriptObject
        PythonScriptObject* pythonscript = (PythonScriptObject*)&script;
        pDict = PyModule_GetDict(pythonscript->pythonRef);
        if(pDict==NULL){error = "unable to get module namespace"; return NULL;}
        // Build the name of a callable class 
        pClass = PyDict_GetItemString(pDict, objectname.c_str());
        if(pClass==NULL){error = "unable get class from module namespace"; return NULL;}
        PythonScriptObject* obj = new PythonScriptObject();
        obj->pythonRef = pClass;

        // Create an instance of the class
        if (PyCallable_Check(pClass)) {
            pInstance = PyObject_CallObject(pClass, NULL); 
        } else {
        error = "unable to create instance from class"; return NULL;
        }
        obj->pythonObj = pInstance;
        return obj;
    }

    */
}
