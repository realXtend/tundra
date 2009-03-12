// For conditions of distribution and use, see copyright notice in license.txt

//#include "StableHeaders.h"
#include "PythonScriptModule.h"
//#include <Poco/ClassLibrary.h>
#include "Foundation.h"
#include "ComponentRegistrarInterface.h"

#include <Python/Python.h>

namespace PythonScript
{
	PythonScriptModule::PythonScriptModule() : ModuleInterface_Impl(Foundation::Module::Type_Python)
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
    }

    // virtual
    void PythonScriptModule::Unload()
    {
        LogInfo("Module " + Name() + " unloaded.");
    }

    // virtual
    void PythonScriptModule::Initialize(Foundation::Framework *framework)
    {
        Py_Initialize();

        LogInfo("Module " + Name() + " initialized.");
		//LogInfo("Py thinks 1 + 1 = " + Py_Eval("1 + 1"));
		PyRun_SimpleString("print 'Py thinks 1 + 1 = %d' % (1 + 1)");
    }

    // virtual 
    void PythonScriptModule::Uninitialize(Foundation::Framework *framework)
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

