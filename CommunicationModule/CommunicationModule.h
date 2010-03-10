// For conditions of distribution and use, see copyright notice in license.txt

#ifndef incl_Comm_CommunicationModule_h
#define incl_Comm_CommunicationModule_h

#include "ModuleInterface.h"
#include "ModuleLoggingFunctions.h"

#include "CommunicationModuleApi.h"

#include <QObject>

namespace ProtocolUtilities
{
    class ProtocolModuleInterface;
}

namespace UiServices
{
    class UiProxyWidget;
}

namespace CommunicationUI
{
    class MasterWidget;
    class OpenSimChatWidget;
}

namespace Communication
{
    class CommunicationServiceInterface;
    class ConnectionInterface;
}

namespace OpensimIM
{
    class ChatController;
}

namespace CommunicationTest
{
    class Test;
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
        CommunicationModule();
        virtual ~CommunicationModule();

        void Load();
        void Unload();
        void Initialize();
        void PostInitialize();
        void Uninitialize();

        void Update(f64 frametime);
        bool HandleEvent(event_category_id_t category_id, event_id_t event_id, Foundation::EventDataInterface* data);

        //! Logging
        MODULE_LOGGING_FUNCTIONS
        static const std::string &NameStatic() { return Foundation::Module::NameFromType(type_static_); } //! returns name of this module. Needed for logging.
        static const Foundation::Module::Type type_static_ = Foundation::Module::MT_Communication;

    protected:
        // Run given test
        Console::CommandResult Test(const StringVector &params);

        //! Ui related widgets/controllers
        CommunicationUI::MasterWidget* im_ui_;
        UiServices::UiProxyWidget *im_ui_proxy_widget_;
        OpensimIM::ChatController *os_chat_controller_;

        CommunicationServiceInterface* communication_service_;
        CommunicationTest::Test* test_;

        // Event category IDs
        event_category_id_t event_category_networkstate_;
        event_category_id_t event_category_framework_;

    private:
        void AddWidgetToUi(const QString &name);
        void UpdateChatControllerToUiModule();
        void RemoveProxyWidgetFromUi(UiServices::UiProxyWidget *proxy_widget);

    private slots:
        void OnNewProtocol(QString &protocol);
        void OnProtocolSupportEnded(QString &protocol);
        void OnConnectionOpened(Communication::ConnectionInterface* connection);
        void OnConnectionClosed(Communication::ConnectionInterface* connection);
    };
}

#endif // incl_Comm_CommunicationModule_h
