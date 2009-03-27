// For conditions of distribution and use, see copyright notice in license.txt

#include "StableHeaders.h"
#include "PythonScriptModule.h"
//#include <Poco/ClassLibrary.h>
//#include "Foundation.h"
#include "ServiceManager.h"
#include "ComponentRegistrarInterface.h"
#include "ConsoleCommandServiceInterface.h"

#include <Python/Python.h>

namespace PythonScript
{
	PythonScriptModule::PythonScriptModule() : ModuleInterface_Impl(type_static_)
    {
    }

    PythonScriptModule::~PythonScriptModule()
    {
    }

    // virtual
    void PythonScriptModule::Load()
    {
        using namespace PythonScript;

        LogInfo("Module " + Name() + " loaded.");
        //DECLARE_MODULE_EC(EC_OgreEntity);

        AutoRegisterConsoleCommand(Console::CreateCommand(
            "PyExec", "Execute given code in the embedded Python interpreter. Usage: PyExec(mycodestring)", 
            Console::Bind(this, &PythonScriptModule::ConsoleRunString))); 
		/* NOTE: called 'exec' cause is similar to py shell builtin exec() func.
		 * Also in the IPython shell 'run' refers to running an external file and not the given string
		 */
    }

    // virtual
    void PythonScriptModule::Unload()
    {
        LogInfo("Module " + Name() + " unloaded.");
    }

    // virtual
    void PythonScriptModule::Initialize()
    {
        Py_Initialize();

        LogInfo("Module " + Name() + " initialized.");
		//LogInfo("Py thinks 1 + 1 = " + Py_Eval("1 + 1"));
		RunString("print 'Py thinks 1 + 1 = %d' % (1 + 1)");
    }

    void PythonScriptModule::PostInitialize()
    {
	}


	void PythonScriptModule::RunString(const char* codestr)
	{
		PyRun_SimpleString(codestr);
	}


    Console::CommandResult PythonScriptModule::ConsoleRunString(const Core::StringVector &params)
	{
		if (params.size() != 1)
		{			
			return Console::ResultFailure("Usage: PyExec(print 1 + 1)");
			//how to handle input like this? PyExec(print '1 + 1 = %d' % (1 + 1))");
			//probably better have separate py shell.
		}

		else
		{
			RunString(params[0].c_str());
			return Console::ResultSuccess();
		}

	}

    // virtual 
    void PythonScriptModule::Uninitialize()
    {        
		Py_Finalize();

        LogInfo("Module " + Name() + " uninitialized.");
    }
    
    // virtual
    void PythonScriptModule::Update()
    {
        //renderer_->Update();
    }
}

using namespace PythonScript;

POCO_BEGIN_MANIFEST(Foundation::ModuleInterface)
   POCO_EXPORT_CLASS(PythonScriptModule)
POCO_END_MANIFEST

