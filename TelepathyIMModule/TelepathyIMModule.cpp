// For conditions of distribution and use, see copyright notice in license.txt

#include "StableHeaders.h"
#include "DebugOperatorNew.h"
#include "TelepathyIMModule.h"
#include "CommunicationService.h"
#include "CommunicationUI/MasterWidget.h"
#include "CommunicationUI/OpenSimChatWidget.h"
#include "Test.h"
//#include "OpensimIM/ConnectionProvider.h"
//#include "OpensimIM/ChatController.h"
#include "TelepathyIM/ConnectionProvider.h"
#include "EventManager.h"
#include "ModuleManager.h"
#include "WorldStream.h"
#include "Interfaces/ProtocolModuleInterface.h"

#include "RexLogicModule.h"
#include "UiModule.h"
#include "Inworld/InworldSceneController.h"
#include "UiProxyWidget.h"

#include "MemoryLeakCheck.h"

namespace Communication
{
    std::string TelepathyIMModule::type_name_static_ = "TelepathyIM";

    TelepathyIMModule::TelepathyIMModule()
        : ModuleInterface(type_name_static_),
          im_ui_(0),
          im_ui_proxy_widget_(0),
          communication_service_(0),
          test_(0),
          event_category_networkstate_(0),
          event_category_framework_(0),
          os_chat_controller_(0)
    {
    }

    TelepathyIMModule::~TelepathyIMModule()
    {
    }

    void TelepathyIMModule::Load()
    {
    }

    void TelepathyIMModule::Unload()
    {
    }

    void TelepathyIMModule::Initialize() 
    {
        event_category_framework_ = framework_->GetEventManager()->QueryEventCategory("Framework");
    }

    void TelepathyIMModule::PostInitialize()
    {
        Foundation::EventManagerPtr event_manager = framework_->GetEventManager(); 
        CommunicationService::CreateInstance(framework_);
        communication_service_ = CommunicationService::GetInstance();
        if (communication_service_ == 0)
        {
            LogError("Cannot initialize CommunicationService object");
            return;
        }
        
        connect(communication_service_, SIGNAL( NewProtocolSupported(QString &) ), 
                SLOT( OnNewProtocol(QString &) ));
        connect(communication_service_, SIGNAL( ProtocolSupportEnded(QString &) ), 
                SLOT( OnProtocolSupportEnded(QString &) ));
        connect(communication_service_, SIGNAL( ConnectionOpened(Communication::ConnectionInterface*) ),
                SLOT( OnConnectionOpened(Communication::ConnectionInterface*) ));
        connect(communication_service_, SIGNAL( ConnectionClosed(Communication::ConnectionInterface*) ),
                SLOT( OnConnectionClosed(Communication::ConnectionInterface*) ));

        //OpensimIM::ConnectionProvider* opensim = new OpensimIM::ConnectionProvider(framework_);
        //communication_service_->RegisterConnectionProvider(opensim);

        TelepathyIM::ConnectionProvider* telepathy = new TelepathyIM::ConnectionProvider(framework_);
        communication_service_->RegisterConnectionProvider(telepathy);

        test_ = new CommunicationTest::Test(framework_);

        QStringList protocols = communication_service_->GetSupportedProtocols();
        if (protocols.size() == 0)
            LogInfo("No IM protocols supported");
        else
        {
            for (QStringList::iterator i = protocols.begin(); i != protocols.end(); ++i)
            {
                QString message = "IM protocol support for: ";
                message.append(*i);
                LogInfo( message.toStdString() );
            }
        }
    }

    void TelepathyIMModule::Uninitialize()
    {
//        SAFE_DELETE(im_ui_);
//        SAFE_DELETE(os_chat_controller_);
        SAFE_DELETE(test_);

        CommunicationService::CleanUp();
        communication_service_ = NULL;
    }

    void TelepathyIMModule::Update(f64 frametime)
    {

    }

    bool TelepathyIMModule::HandleEvent(event_category_id_t category_id, event_id_t event_id, Foundation::EventDataInterface* data)
    {
        event_category_networkstate_ = framework_->GetEventManager()->QueryEventCategory("NetworkState");

        if (category_id == event_category_networkstate_)
        {
            if (event_id == ProtocolUtilities::Events::EVENT_SERVER_CONNECTED)
            {
                //! Remove RexLogicModule dependency. Get WorldStream from WORLD_STREAM_READY event.
                boost::weak_ptr<ProtocolUtilities::ProtocolModuleInterface> current_protocol_module =
                    framework_->GetModuleManager()->GetModule<RexLogic::RexLogicModule>().lock()->GetServerConnection()->GetCurrentProtocolModuleWeakPointer();
                if (current_protocol_module.lock().get())
                {
                    ProtocolUtilities::ClientParameters client_params = current_protocol_module.lock()->GetClientParameters();
//                    os_chat_controller_ = new OpensimIM::ChatController(client_params);
                }
            } 
            else if (event_id == ProtocolUtilities::Events::EVENT_SERVER_DISCONNECTED || event_id == ProtocolUtilities::Events::EVENT_CONNECTION_FAILED)
            {
//                SAFE_DELETE(os_chat_controller_);
            }
        }

        if (communication_service_)
            return static_cast<CommunicationService*>( communication_service_ )->HandleEvent(category_id, event_id, data);
        return false;
    }

    void TelepathyIMModule::OnNewProtocol(QString &protocol)
    {
        if (protocol == "jabber")
        {
            im_ui_ = new CommunicationUI::MasterWidget(framework_);
            AddWidgetToUi("IM");
        }
    }

    void TelepathyIMModule::OnProtocolSupportEnded(QString &protocol)
    {
        if (protocol == "jabber")
        {
            SAFE_DELETE(im_ui_);
            return;
        }
    }

    void TelepathyIMModule::OnConnectionOpened(Communication::ConnectionInterface* connection)
    {
    }

    void TelepathyIMModule::OnConnectionClosed(Communication::ConnectionInterface* connection)
    {
    }

    void TelepathyIMModule::AddWidgetToUi(const QString &name)
    {
        UiServices::UiModule *ui_module = framework_->GetModule<UiServices::UiModule>();
        if (ui_module)
        {
            if (name == "IM")
            {
                im_ui_->setWindowTitle(name);
                im_ui_proxy_widget_ = ui_module->GetInworldSceneController()->AddWidgetToScene(im_ui_);
                if (im_ui_proxy_widget_)
                    ui_module->GetInworldSceneController()->SetImWidget(im_ui_proxy_widget_);
            }
        }
    }

    void TelepathyIMModule::RemoveProxyWidgetFromUi(UiProxyWidget *proxy_widget)
    {
        UiServices::UiModule *ui_module = framework_->GetModule<UiServices::UiModule>();
        if (ui_module)
            ui_module->GetInworldSceneController()->RemoveProxyWidgetFromScene(proxy_widget);
    }
}

extern "C" void POCO_LIBRARY_API SetProfiler(Foundation::Profiler *profiler);
void SetProfiler(Foundation::Profiler *profiler)
{
    Foundation::ProfilerSection::SetProfiler(profiler);
}

using namespace Communication;

POCO_BEGIN_MANIFEST(Foundation::ModuleInterface)
POCO_EXPORT_CLASS(TelepathyIMModule)
POCO_END_MANIFEST
