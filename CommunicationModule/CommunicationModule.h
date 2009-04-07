#ifndef incl_CommunicationModule_h
#define incl_CommunicationModule_h

#include "ModuleInterface.h"
#include "CommunicationManagerServiceInterface.h"
#include "TelepathyCommunicationManager.h"

namespace Communication
{
	//MODULE_API 
	class MODULE_API CommunicationModule : public Foundation::ModuleInterfaceImpl
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



		//// Communications API
		//
		//// For receiving comm events
	 //   void AddListener(ICommunicationListener *listener);
	 //   void RemoveListener(ICommunicationListener *listener);

		//// Init comm events
		//void Connect();
		//void Disconnect();


	private:
		Foundation::Comms::CommunicationManagerPtr communication_manager_;
		CommunicationServicePtr communication_manager;

	};
}

#endif