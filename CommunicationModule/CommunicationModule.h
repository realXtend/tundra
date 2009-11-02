#ifndef incl_Comm_CommunicationModule_h
#define incl_Comm_CommunicationModule_h

#include "Foundation.h"
#include "ModuleInterface.h"
#include "OpenSimProtocolModule.h"

#include "CommunicationModuleApi.h"
#include "CommunicationManager.h"
#include "CommunicationService.h"
#include "ConsoleUI.h"
#include "CommunicationUI/QtGUI.h"
#include "CommunicationUI/OpenSimChat.h"
#include "Test.h"

// Did not compile without this, dunno why?
namespace CommunicationUI
{
	class OpenSimChat;	
}

namespace Communication
{
	/**
	 *  Communication module
	 *
	 *  Provides:
	 *  - CommunicationService class 
	 *
	 */
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

		static const Foundation::Module::Type type_static_ = Foundation::Module::MT_Communication;

	    bool CommunicationModule::HandleEvent(Core::event_category_id_t category_id, Core::event_id_t event_id, Foundation::EventDataInterface* data);

		MODULE_LOGGING_FUNCTIONS
		//! returns name of this module. Needed for logging.
		static const std::string &NameStatic() { return Foundation::Module::NameFromType(type_static_); }

	protected:
		// Run given test
		Console::CommandResult Test(const Core::StringVector &params);

		TpQt4Communication::CommunicationManager* communication_manager_;
		CommunicationUI::ConsoleUI* console_ui_;
		CommunicationUI::QtGUI* qt_ui_;
		CommunicationUI::OpenSimChat* opensim_ui_;
		CommunicationServiceInterface* communication_service_;
		CommunicationTest::Test* test_;

		// Needed for catch 'connected to server' event
		Core::event_category_id_t event_category_networkstate_;
	};
}

#endif // incl_Comm_CommunicationModule_h
