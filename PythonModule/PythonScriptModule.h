// For conditions of distribution and use, see copyright notice in license.txt

#ifndef incl_PythonScriptingModule_h
#define incl_PythonScriptingModule_h

#include "Foundation.h"
#include "StableHeaders.h"
#include "ModuleInterface.h"
#include "ComponentRegistrarInterface.h"
#include "ServiceManager.h"

#ifdef PYTHON_FORCE_RELEASE_VERSION
  #ifdef _DEBUG
    #undef _DEBUG
    #include "Python.h"
    #define _DEBUG
  #else
    #include "Python.h"
  #endif 
#else
    #include "Python.h"
#endif


namespace Foundation
{
    class Framework;
}

namespace RexLogic
{
    class RexLogicModule;
	class EC_OpenSimPrim;
}

namespace PythonScript
{
	//hack to have a ref to framework so can get the module in api funcs
	static Foundation::Framework *staticframework;

	class PythonEngine;
	typedef boost::shared_ptr<PythonEngine> PythonEnginePtr;
	

    //! A scripting module using Python
    class MODULE_API PythonScriptModule : public Foundation::ModuleInterfaceImpl
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
        virtual void Update(Core::f64 frametime);

		//handling events
		virtual bool HandleEvent(
            Core::event_category_id_t category_id,
            Core::event_id_t event_id, 
            Foundation::EventDataInterface* data);

		//! callback for console command
        Console::CommandResult ConsoleRunString(const Core::StringVector &params);
        Console::CommandResult ConsoleRunFile(const Core::StringVector &params);
        Console::CommandResult ConsoleReset(const Core::StringVector &params);
        
        MODULE_LOGGING_FUNCTIONS

        //! returns name of this module. Needed for logging.
        static const std::string &NameStatic() { return Foundation::Module::NameFromType(type_static_); }
        static const Foundation::Module::Type type_static_ = Foundation::Module::MT_PythonScript;

		static Foundation::Framework *GetFramework() { return PythonScript::staticframework; } //trying to expose to a non-static func, Entity getattro
		static Foundation::ScriptEventInterface* engineAccess;

		
	private:
        PythonEnginePtr engine_;
		
		//basic feats
		void PythonScriptModule::RunString(const char* codestr);
		void PythonScriptModule::RunFile(const std::string &modulename);
		void PythonScriptModule::Reset();

		// Category id for incoming messages.
		Core::event_category_id_t inboundCategoryID_;

		// first stab at having a py defined event handler
		// now just one - eventually could have a list/dict of these?
		/*
		PyObject *pName, *pModule, *pDict, *pFunc;
	    PyObject *pArgs, *pValue;*/
		Foundation::ScriptObject* chathandler;
	};

	static void initpymod();
}

#endif
