#ifndef incl_QtScriptModule_h
#define incl_QtScriptModule_h

#include "Foundation.h"
#include "StableHeaders.h"
#include "ModuleInterface.h"
#include "ComponentRegistrarInterface.h"
#include <QtScript>

namespace RexQtScript
{
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
		virtual void Update(Core::f64 frametime);

		MODULE_LOGGING_FUNCTIONS;

        //! returns name of this module. Needed for logging.
        static const std::string &NameStatic() { return Foundation::Module::NameFromType(type_static_); }
		static const Foundation::Module::Type type_static_ = Foundation::Module::MT_QtScript;

	private:
		QScriptEngine engine;
	};
}

#endif
