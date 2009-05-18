#ifndef incl_CommunicationModule_h
#define incl_CommunicationModule_h

#include "Foundation.h"
#include "ModuleInterface.h"

#include "CommunicationModuleApi.h"

/**
 *  Communication module
 *
 *  Creates CommunicationManager object to offer CommunicationServiceInterfce 
 */
namespace Communication
{
    class CommunicationManager;
	typedef boost::shared_ptr<CommunicationManager> CommunicationManagerPtr;    
    
	class COMMS_MODULE_API CommunicationModule : public Foundation::ModuleInterfaceImpl
	{
	public:
		CommunicationModule(void);
		virtual ~CommunicationModule(void);

		void Load();
		void Unload();
		void Initialize();
		void PostInitialize();
		void Uninitialize();

		void Update(Core::f64 frametime);

		MODULE_LOGGING_FUNCTIONS

		//! returns name of this module. Needed for logging.
		static const std::string &NameStatic() { return Foundation::Module::NameFromType(type_static_); }
		static const Foundation::Module::Type type_static_ = Foundation::Module::MT_Communication;

	private:
		CommunicationManagerPtr communication_manager_;
	};
}

#endif
