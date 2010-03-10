// For conditions of distribution and use, see copyright notice in license.txt

#include "StableHeaders.h"
#include "DebugOperatorNew.h"
#include "CommunicationModule.h"
#include "CommunicationService.h"
#include "CommunicationUI/MasterWidget.h"
#include "CommunicationUI/OpenSimChatWidget.h"
#include "Test.h"
#include "OpensimIM/ConnectionProvider.h"
#include "OpensimIM/ChatController.h"
#include "TelepathyIM/ConnectionProvider.h"
#include "EventManager.h"
#include "ModuleManager.h"
#include "WorldStream.h"
#include "Interfaces/ProtocolModuleInterface.h"

#include <RexLogicModule.h>
#include <UiModule.h>
#include <Inworld/InworldSceneController.h>
#include <Inworld/View/UiProxyWidget.h>

#include "MemoryLeakCheck.h"

namespace Communication
{
    CommunicationModule::CommunicationModule()
        : ModuleInterfaceImpl(type_static_),
          im_ui_(0),
          im_ui_proxy_widget_(0),
          opensim_chat_ui_(0),
          opensim_chat_proxy_widget_(0),
          communication_service_(0),
          test_(0),
          event_category_networkstate_(0),
          event_category_framework_(0)
    {
    }

    CommunicationModule::~CommunicationModule()
    {
    }

    void CommunicationModule::Load()
    {
    }

    void CommunicationModule::Unload()
    {
    }

    void CommunicationModule::Initialize() 
    {
        event_category_framework_ = framework_->GetEventManager()->QueryEventCategory("Framework");
    }

    void CommunicationModule::PostInitialize()
    {
        Foundation::EventManagerPtr event_manager = framework_->GetEventManager(); 
        CommunicationService::CreateInstance(framework_);
        communication_service_ = CommunicationService::GetInstance();
        if (communication_service_ == 0)
        {
            LogError("Cannot initialize CommunicationService object");
            return;
        }
        
        connect(communication_service_, SIGNAL( NewProtocolSupported(QString &) ), SLOT( OnNewProtocol(QString &) ));
        connect(communication_service_, SIGNAL( ProtocolSupportEnded(QString &) ), SLOT( OnProtocolSupportEnded(QString &) ));
        QObject::connect(communication_service_, SIGNAL( ConnectionOpened(Communication::ConnectionInterface*) ),
            this, SLOT( OnConnectionOpened(Communication::ConnectionInterface*) ));
        QObject::connect(communication_service_, SIGNAL( ConnectionClosed(Communication::ConnectionInterface*) ),
            this, SLOT( OnConnectionClosed(Communication::ConnectionInterface*) ));

        OpensimIM::ConnectionProvider* opensim = new OpensimIM::ConnectionProvider(framework_);
        communication_service_->RegisterConnectionProvider(opensim);

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

    void CommunicationModule::Uninitialize()
    {
        SAFE_DELETE(im_ui_);
        SAFE_DELETE(opensim_chat_ui_);
        SAFE_DELETE(os_chat_controller_);
        SAFE_DELETE(test_);

        CommunicationService::CleanUp();
        communication_service_ = NULL;
    }

    void CommunicationModule::Update(f64 frametime)
    {
    }

    bool CommunicationModule::HandleEvent(event_category_id_t category_id, event_id_t event_id, Foundation::EventDataInterface* data)
    {
        event_category_networkstate_ = framework_->GetEventManager()->QueryEventCategory("NetworkState");

        if (category_id == event_category_networkstate_)
        {
            if (event_id == ProtocolUtilities::Events::EVENT_SERVER_CONNECTED)
            {
                boost::weak_ptr<ProtocolUtilities::ProtocolModuleInterface> current_protocol_module = framework_->GetModuleManager()->GetModule<RexLogic::RexLogicModule>(Foundation::Module::MT_WorldLogic).lock().get()->GetServerConnection()->GetCurrentProtocolModuleWeakPointer();
                if (current_protocol_module.lock().get())
                {
                    ProtocolUtilities::ClientParameters client_params = current_protocol_module.lock()->GetClientParameters();
                    os_chat_controller_ = new OpensimIM::ChatController(client_params);
                    UpdateChatControllerToUiModule();

                    /* OLD OS UI, REMOVE THIS PROPERLY

                    //! TODO: Currently we can have only one world_chat ui but this
                    //!       should be changed to 1:1 relation between "WorldChatWidget" - "WorldConnectionWidget"
                    SAFE_DELETE(opensim_chat_ui_);
                    opensim_chat_ui_ = new CommunicationUI::OpenSimChatWidget(client_params);
                    AddWidgetToUi("World Chat");

                    */

                }
            } 
            else if (event_id == ProtocolUtilities::Events::EVENT_SERVER_DISCONNECTED || event_id == ProtocolUtilities::Events::EVENT_CONNECTION_FAILED)
            {
                SAFE_DELETE(os_chat_controller_);

                /* OLD OS UI, REMOVE THIS PROPERLY

                if (opensim_chat_ui_ && opensim_chat_proxy_widget_)
                {
                    RemoveProxyWidgetFromUi(opensim_chat_proxy_widget_);
                    opensim_chat_proxy_widget_ = 0;
                    SAFE_DELETE(opensim_chat_ui_);
                }

                */
            }
        }

        if (communication_service_)
            return dynamic_cast<CommunicationService*>( communication_service_ )->HandleEvent(category_id, event_id, data);
        return false;
    }

    void CommunicationModule::OnNewProtocol(QString &protocol)
    {
        if (protocol.compare("jabber") == 0)
        {
            if (!im_ui_)
            {
                im_ui_ = new CommunicationUI::MasterWidget(framework_);
                AddWidgetToUi("IM");
            }
            return;
        }
    }

    void CommunicationModule::OnProtocolSupportEnded(QString &protocol)
    {
        if (protocol.compare("jabber") == 0)
        {
            SAFE_DELETE(im_ui_);
            return;
        }
    }

    void CommunicationModule::OnConnectionOpened(Communication::ConnectionInterface* connection)
    {
    }

    void CommunicationModule::OnConnectionClosed(Communication::ConnectionInterface* connection)
    {
    }

    void CommunicationModule::AddWidgetToUi(const QString &name)
    {
        boost::shared_ptr<UiServices::UiModule> ui_module = framework_->GetModuleManager()->GetModule<UiServices::UiModule>(Foundation::Module::MT_UiServices).lock();
        if (ui_module.get())
        {
            if (name == "World Chat")
            {
                UiServices::UiWidgetProperties widget_properties(name, UiServices::ModuleWidget);
                opensim_chat_proxy_widget_ = ui_module->GetInworldSceneController()->AddWidgetToScene(opensim_chat_ui_, widget_properties);
            }
            else if (name == "IM")
            {
                UiServices::UiWidgetProperties widget_properties(name, UiServices::ModuleWidget);
                im_ui_proxy_widget_ = ui_module->GetInworldSceneController()->AddWidgetToScene(im_ui_, widget_properties);
            }
        }
    }

    void CommunicationModule::UpdateChatControllerToUiModule()
    {
        boost::shared_ptr<UiServices::UiModule> ui_module = framework_->GetModuleManager()->GetModule<UiServices::UiModule>(Foundation::Module::MT_UiServices).lock();
        if (ui_module.get())
            ui_module->GetInworldSceneController()->SetWorldChatController(os_chat_controller_);
    }

    void CommunicationModule::RemoveProxyWidgetFromUi(UiServices::UiProxyWidget *proxy_widget)
    {
        boost::shared_ptr<UiServices::UiModule> ui_module = framework_->GetModuleManager()->GetModule<UiServices::UiModule>(Foundation::Module::MT_UiServices).lock();
        if (ui_module.get())
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
POCO_EXPORT_CLASS(CommunicationModule)
POCO_END_MANIFEST
