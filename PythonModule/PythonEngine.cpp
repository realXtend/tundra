// For conditions of distribution and use, see copyright notice in license.txt

#include "StableHeaders.h"
#include "PythonEngine.h"
#include "PythonScriptModule.h"

#include "Python/Python.h" //should be <Python.h> or "Python.h" ?

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
	        Py_Initialize();
			RunString("import sys; sys.path.append('pymodules');"); //XXX change to the c equivalent when have network to access the reference
		}
		//else
			//LogWarning("Python already initialized in PythonScriptModule init!");
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

	void PythonEngine::Reset()
	{
		//should clear own internal state etc too i guess XXX
		Py_Finalize();
		Py_Initialize();
		//no ref to mod here now XXX
		//PythonModule::LogInfo("Python interpreter reseted: all memory and state cleared.");
	}


	void PythonEngine::RunString(const std::string& codestr)
	{
		PyRun_SimpleString(codestr.c_str());
	}

	void PythonEngine::RunScript(const std::string& scriptname)
	{
		/* could get a fp* here and pass it to
		int PyRun_SimpleFile(FILE *fp, const char *filename)
		but am unsure whether to use the Poco fs stuff for it an how
		so trying this, why not? we don't need the file on the c++ side?*/
		std::string cmd = "import " + scriptname;
		RunString(cmd);
	}


}

