#ifndef incl_Comm_CommunicationModule_h
#define incl_Comm_CommunicationModule_h

#include <StableHeaders.h>
#include <ModuleInterface.h>
#include <QObject>
#include "../ReXLogic/RexLogicModule.h"

#include "CommunicationModuleApi.h"
#include "CommunicationService.h"
#include "CommunicationUI/QtGUI.h"
#include "CommunicationUI/OpenSimChat.h"
#include "Test.h"

namespace ProtocolUtilities
{
    class ProtocolModuleInterface;
}

namespace Communication
{
	/**
	 *  Communication module
	 *
	 *  Provides:
	 *  - CommunicationService class which implements Communication::CommunicationServiceInterface
     *  - Tests available from debug console (command "comm test" to see all available tests)
	 *
     *  Enables IM GUI if jabber protocol is usable
     *  Enables world chat GUI when connected to opensim based world server. 
     *
	 */
	class COMMS_MODULE_API CommunicationModule : public QObject, public Foundation::ModuleInterfaceImpl
	{
        Q_OBJECT
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

	    bool HandleEvent(Core::event_category_id_t category_id, Core::event_id_t event_id, Foundation::EventDataInterface* data);

		MODULE_LOGGING_FUNCTIONS
		//! returns name of this module. Needed for logging.
		static const std::string &NameStatic() { return Foundation::Module::NameFromType(type_static_); }

	protected:
		// Run given test
		Console::CommandResult Test(const Core::StringVector &params);


		CommunicationUI::QtGUI* qt_ui_;
		CommunicationUI::OpenSimChat* opensim_ui_;
		CommunicationServiceInterface* communication_service_;
		CommunicationTest::Test* test_;

		// Event category IDs
		Core::event_category_id_t event_category_networkstate_;
        Core::event_category_id_t event_category_framework_;
    private slots:
        void OnNewProtocol(QString &protocol);
        void OnProtocolSupportEnded(QString &protocol);
        void OnConnectionOpened(Communication::ConnectionInterface* connection);
        void OnConnectionClosed(Communication::ConnectionInterface* connection);
	};
}

#endif // incl_Comm_CommunicationModule_h
