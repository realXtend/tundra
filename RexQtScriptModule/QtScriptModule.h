#ifndef incl_RexQtScriptModule_h
#define incl_RexQtScriptModule_h

#include "Foundation.h"
#include "StableHeaders.h"
#include "ModuleInterface.h"
#include "ModuleLoggingFunctions.h"
#include "ComponentRegistrarInterface.h"
#include <QtScript>

namespace RexQtScript
{
     //hack to have a ref to framework so can get the module in api funcs
    static Foundation::Framework *staticframework;

    class MODULE_API RexQtScriptModule : public Foundation::ModuleInterfaceImpl
	{
    public:
		RexQtScriptModule();
		virtual ~RexQtScriptModule();

		//the module interface	        
		virtual void Load();
		virtual void Unload();
		virtual void Initialize();
		virtual void PostInitialize();
		virtual void Uninitialize();
		virtual void Update(f64 frametime);

		MODULE_LOGGING_FUNCTIONS

        //! returns name of this module. Needed for logging.
        static const std::string &NameStatic() { return Foundation::Module::NameFromType(type_static_); }
		static const Foundation::Module::Type type_static_ = Foundation::Module::MT_QtScript;

		//QScriptValue test(QScriptContext *context, QScriptEngine *engine);

	private:
		QScriptEngine engine;
	};

	//api stuff
	//QScriptValue LoadUI(QScriptContext *context, QScriptEngine *engine);
	QScriptValue Print(QScriptContext *context, QScriptEngine *engine);
}

#endif
