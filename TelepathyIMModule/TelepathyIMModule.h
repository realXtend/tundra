// For conditions of distribution and use, see copyright notice in license.txt

#ifndef incl_TelepathyIMModule_TelepathyIMModule_h
#define incl_TelepathyIMModule_TelepathyIMModule_h

#include "ModuleInterface.h"
#include "ModuleLoggingFunctions.h"

#include "TelepathyIMModuleApi.h"
#include "CommunicationsService.h"

#include <QObject>

namespace ProtocolUtilities
{
    class ProtocolModuleInterface;
}

class UiProxyWidget;

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

/// @todo: Rename to TelepathyIM
namespace Communication
{
    /**
     *  NOTE *********************************************
     *
     *  >> Telepathy funtionality will be moved modified to implement CommunicationsService::IM
     *  >> Opensim functionality will be moved to RexLogic module
     *
     *  Provides:
     *  - CommunicationService class which implements Communication::CommunicationServiceInterface
     *  - Tests available from debug console (command "comm test" to see all available tests)
     *
     *  Enables IM GUI if jabber protocol is usable
     *
     */
    class TELEPATHY_IM_MODULE_API TelepathyIMModule : public QObject, public Foundation::ModuleInterface
    {
        Q_OBJECT

    public:
        TelepathyIMModule();
        virtual ~TelepathyIMModule();

        void Load();
        void Unload();
        void Initialize();
        void PostInitialize();
        void Uninitialize();

        void Update(f64 frametime);
        bool HandleEvent(event_category_id_t category_id, event_id_t event_id, Foundation::EventDataInterface* data);

        //! Logging
        MODULE_LOGGING_FUNCTIONS
        static const std::string &NameStatic() { return type_name_static_; }

    protected:
        //! Run given test
        Console::CommandResult Test(const StringVector &params);
        //! Ui related widgets/controllers
        CommunicationUI::MasterWidget* im_ui_;
        UiProxyWidget *im_ui_proxy_widget_;
        OpensimIM::ChatController *os_chat_controller_;

        CommunicationServiceInterface* communication_service_;
        CommunicationTest::Test* test_;

        // Event category IDs
        event_category_id_t event_category_networkstate_;
        event_category_id_t event_category_framework_;
        typedef boost::shared_ptr<Communications::ServiceInterface> CommunicationsServicePtr;
        CommunicationsServicePtr communications_service_;

    private:
        void AddWidgetToUi(const QString &name);
        void RemoveProxyWidgetFromUi(UiProxyWidget *proxy_widget);
        //! Type name of the module.
        static std::string type_name_static_;

    private slots:
        void OnNewProtocol(QString &protocol);
        void OnProtocolSupportEnded(QString &protocol);
        void OnConnectionOpened(Communication::ConnectionInterface* connection);
        void OnConnectionClosed(Communication::ConnectionInterface* connection);
    };
}

#endif // incl_TelepathyIMModule_TelepathyIMModule_h
