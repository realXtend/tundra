#ifndef incl_Comm_CommunicationModule_h
#define incl_Comm_CommunicationModule_h

#include <StableHeaders.h>
#include <ModuleInterface.h>
#include <RexLogicModule.h>

#include "CommunicationModuleApi.h"
#include "CommunicationService.h"
#include "Test.h"

#include "CommunicationUI/MasterWidget.h"
#include "CommunicationUI/OpenSimChatWidget.h"

#include <QObject>

namespace ProtocolUtilities
{
    class ProtocolModuleInterface;
}

namespace UiServices
{
    class UiProxyWidget;
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
        //! Logging
		MODULE_LOGGING_FUNCTIONS
		static const std::string &NameStatic() { return Foundation::Module::NameFromType(type_static_); } //! returns name of this module. Needed for logging.
        static const Foundation::Module::Type type_static_ = Foundation::Module::MT_Communication;

		CommunicationModule(void);
		virtual ~CommunicationModule(void);

		void Load();
		void Unload();
		void Initialize();
		void PostInitialize();
		void Uninitialize();

		void Update(f64 frametime);
	    bool HandleEvent(event_category_id_t category_id, event_id_t event_id, Foundation::EventDataInterface* data);

	protected:
		// Run given test
		Console::CommandResult Test(const StringVector &params);

        CommunicationUI::MasterWidget* im_ui_;
        CommunicationUI::OpenSimChatWidget* opensim_chat_ui_;

        UiServices::UiProxyWidget *im_ui_proxy_widget_;
        UiServices::UiProxyWidget *opensim_chat_proxy_widget_;

		CommunicationServiceInterface* communication_service_;
		CommunicationTest::Test* test_;

		// Event category IDs
		event_category_id_t event_category_networkstate_;
        event_category_id_t event_category_framework_;

    private:
        void AddWidgetToUi(const QString &name);
        void RemoveProxyWidgetFromUi(UiServices::UiProxyWidget *proxy_widget);

    private slots:
        void OnNewProtocol(QString &protocol);
        void OnProtocolSupportEnded(QString &protocol);
        void OnConnectionOpened(Communication::ConnectionInterface* connection);
        void OnConnectionClosed(Communication::ConnectionInterface* connection);
	};
}

#endif // incl_Comm_CommunicationModule_h
