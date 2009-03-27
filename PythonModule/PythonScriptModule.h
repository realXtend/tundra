// For conditions of distribution and use, see copyright notice in license.txt

#ifndef incl_PythonScriptingModule_h
#define incl_PythonScriptingModule_h

#include "Foundation.h"
#include "StableHeaders.h"
#include "ModuleInterface.h"
#include "ComponentRegistrarInterface.h"
#include "ServiceManager.h"
//#include "Script.h"

namespace Foundation
{
    class Framework;
}

namespace PythonScript
{
    //! A scripting module using Python
    class MODULE_API PythonScriptModule : public Foundation::ModuleInterface_Impl
    {
    public:
        PythonScriptModule();
        virtual ~PythonScriptModule();

		//the module interface
        virtual void Load();
        virtual void Unload();
        virtual void Initialize();
        virtual void PostInitialize();
        virtual void Uninitialize();
        virtual void Update();


		//! callback for console command
        Console::CommandResult ConsoleRunString(const Core::StringVector &params);
        Console::CommandResult ConsoleRunFile(const Core::StringVector &params);
        Console::CommandResult ConsoleReset(const Core::StringVector &params);
        
        MODULE_LOGGING_FUNCTIONS

        //! returns name of this module. Needed for logging.
        static const std::string &NameStatic() { return Foundation::Module::NameFromType(type_static_); }
        static const Foundation::Module::Type type_static_ = Foundation::Module::MT_Script;

	private:
		
		//basic feats
		void PythonScriptModule::RunString(const char* codestr);
		void PythonScriptModule::RunFile(const std::string &modulename);
		void PythonScriptModule::Reset();

    /* python interpreter? 
        OgreRenderer::RendererPtr renderer_;*/
    };
}

#endif
