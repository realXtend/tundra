// For conditions of distribution and use, see copyright notice in license.txt

#include "StableHeaders.h"
#include "DebugOperatorNew.h"

#include "UiModule.h"
#include "UiProxyStyle.h"
#include "UiStateMachine.h"
#include "ServiceGetter.h"
#include "Ether/EtherLogic.h"
#include "Ether/EtherLoginNotifier.h"
#include "Ether/View/EtherScene.h"
#include "Inworld/InworldSceneController.h"
#include "Inworld/ControlPanelManager.h"
#include "Inworld/NotificationManager.h"
#include "Inworld/View/UiProxyWidget.h"
#include "Inworld/View/UiWidgetProperties.h"
#include "Inworld/Console/UiConsoleManager.h"
#include "Inworld/Notifications/MessageNotification.h"
#include "Inworld/Notifications/InputNotification.h"
#include "Inworld/Notifications/QuestionNotification.h"
#include "Inworld/Notifications/ProgressNotification.h"
#include "Common/UiAction.h"

#include "EventManager.h"
#include "ConfigurationManager.h"
#include "Framework.h"
#include "WorldStream.h"
#include "NetworkEvents.h"
#include "SceneEvents.h"
#include "ConsoleEvents.h"
#include "InputEvents.h"

#include <QApplication>
#include <QDir>

#include "MemoryLeakCheck.h"

namespace UiServices
{
    UiModule::UiModule() 
        : Foundation::ModuleInterfaceImpl(Foundation::Module::MT_UiServices),
          event_query_categories_(QStringList()),
          ui_state_machine_(0),
          service_getter_(0),
          inworld_scene_controller_(0),
          inworld_notification_manager_(0),
          ui_console_manager_(0),
          ether_logic_(0)
    {
    }

    UiModule::~UiModule()
    {
        SAFE_DELETE(ui_state_machine_);
        SAFE_DELETE(service_getter_);
        SAFE_DELETE(inworld_scene_controller_);
        SAFE_DELETE(inworld_notification_manager_);
        SAFE_DELETE(ui_console_manager_);
        SAFE_DELETE(ether_logic_);
    }

    /*************** ModuleInterfaceImpl ***************/

    void UiModule::Load()
    {
        // Application take ownership of the new UiProxyStyle
        QApplication::setStyle(new UiProxyStyle());
        event_query_categories_ << "Framework" << "Scene"  << "Console" << "Input";
    }

    void UiModule::Unload()
    {
    }

    void UiModule::Initialize()
    {
        ui_view_ = framework_->GetUIView();
        if (ui_view_)
        {
            ui_state_machine_ = new CoreUi::UiStateMachine(ui_view_, this);
            ui_state_machine_->RegisterScene("Inworld", ui_view_->scene());
            UiAction *ether_action = new UiAction(ui_state_machine_);
            QObject::connect(ether_action, SIGNAL(triggered()), ui_state_machine_, SLOT(SwitchToEtherScene()));
            LogDebug("State Machine STARTED");

            inworld_scene_controller_ = new InworldSceneController(GetFramework(), ui_view_);
            inworld_scene_controller_->GetControlPanelManager()->SetHandler(UiDefines::Ether, ether_action);
            LogDebug("Scene Manager service READY");

            inworld_notification_manager_ = new NotificationManager(inworld_scene_controller_);
            LogDebug("Notification Manager service READY");

            ui_console_manager_ = new CoreUi::UiConsoleManager(GetFramework(), ui_view_);
            LogDebug("Console UI READY");

            service_getter_ = new CoreUi::ServiceGetter(GetFramework());
            inworld_scene_controller_->GetControlPanelManager()->SetServiceGetter(service_getter_);
            ui_state_machine_->SetServiceGetter(service_getter_);
            LogDebug("Service getter READY");

        }
        else
            LogWarning("Could not acquire QGraphicsView shared pointer from framework, UiServices are disabled");
    }

    void UiModule::PostInitialize()
    {
        SubscribeToEventCategories();
        ui_console_manager_->SendInitializationReadyEvent();
        ether_logic_ = new Ether::Logic::EtherLogic(GetFramework(), ui_view_);
        ui_state_machine_->RegisterScene("Ether", ether_logic_->GetScene());
        ether_logic_->Start();
        ui_state_machine_->SwitchToEtherScene();
        LogDebug("Ether Logic STARTED");
    }

    void UiModule::Uninitialize()
    {
    }

    void UiModule::Update(f64 frametime)
    {
    }

    bool UiModule::HandleEvent(event_category_id_t category_id, event_id_t event_id, Foundation::EventDataInterface* data)
    {
        PROFILE(UiModule_HandleEvent);

        QString category = service_category_identifiers_.keys().value(service_category_identifiers_.values().indexOf(category_id));
        if (category == "Framework")
        {
            switch (event_id)
            {
                case Foundation::NETWORKING_REGISTERED:
                {
                    if (!event_query_categories_.contains("NetworkState"))
                        event_query_categories_ << "NetworkState";
                    SubscribeToEventCategories();
                    break;
                }
                case Foundation::WORLD_STREAM_READY:
                {
                    ProtocolUtilities::WorldStreamReadyEvent *event_data = dynamic_cast<ProtocolUtilities::WorldStreamReadyEvent *>(data);
                    if (event_data)
                        current_world_stream_ = event_data->WorldStream;
                    break;
                }
                default:
                    break;
            }
        }
        else if (category == "NetworkState")
        {
            switch (event_id)
            {
                case ProtocolUtilities::Events::EVENT_CONNECTION_FAILED:
                {
                    PublishConnectionState(UiDefines::Failed);
                    break;
                }
                case ProtocolUtilities::Events::EVENT_SERVER_DISCONNECTED:
                {
                    PublishConnectionState(UiDefines::Disconnected);
                    break;
                }
                case ProtocolUtilities::Events::EVENT_SERVER_CONNECTED:
                {
                    // Udp connection has been established, we are still loading object so lets not change UI layer yet
                    // to connected state. See Scene categorys EVENT_CONTROLLABLE_ENTITY case for real UI switch.

                    // School project
                    // hack for teleport, parse the base robust address from here for next login
                    ProtocolUtilities::AuthenticationEventData *auth_data = dynamic_cast<ProtocolUtilities::AuthenticationEventData*>(data);
                    if (auth_data)
                    {
                        //auth_data->webdav_host
                    }
                    break;
                }
                case ProtocolUtilities::Events::EVENT_LOGIN_INFO_AQQUIRED:
                {
                    ProtocolUtilities::LoginDataEvent *login_data = dynamic_cast<ProtocolUtilities::LoginDataEvent*>(data);
                    Ether::Logic::EtherLoginNotifier *notifier = dynamic_cast<Ether::Logic::EtherLoginNotifier*>(ether_logic_->GetLoginNotifier());
                    if (notifier && login_data)
                        notifier->SetLoginData(login_data->base_address_.c_str(), login_data->port_);
                    break;
                }

                default:
                    break;
            }
        }
        else if (category == "Console")
        {
            switch (event_id)
            {
                case Console::Events::EVENT_CONSOLE_TOGGLE:
                {
                    ui_console_manager_->ToggleConsole();
                    break;
                }
                case Console::Events::EVENT_CONSOLE_PRINT_LINE:
                {
                    Console::ConsoleEventData *console_data = dynamic_cast<Console::ConsoleEventData*>(data);
                    ui_console_manager_->QueuePrintRequest(QString(console_data->message.c_str()));
                    break;
                }
                default:
                    break;
            }
        }
        else if (category == "Scene")
        {
            switch (event_id)
            {
                case Scene::Events::EVENT_CONTROLLABLE_ENTITY:
                {
                    PublishConnectionState(UiDefines::Connected);
                    break;
                }
                default:
                    break;
            }
        }
        else if (category == "Input")
        {
            switch (event_id)
            {
                case Input::Events::NAALI_BINDINGS_CHANGED:
                {
                    Input::Events::BindingsData *bindings_data = dynamic_cast<Input::Events::BindingsData*>(data);
                    if (bindings_data)
                        service_getter_->PublishChangedBindings(bindings_data->bindings);
                    break;
                }
                case Input::Events::NAALI_TOGGLE_ETHER:
                {
                    ui_state_machine_->ToggleEther();
                    break;
                }
                case Input::Events::NAALI_TOGGLE_WORLDCHAT:
                {
                    inworld_scene_controller_->SetFocusToChat();
                    break;
                }
            }
        }

        return false;
    }

    void UiModule::PublishConnectionState(UiDefines::ConnectionState connection_state)
    {
        switch (connection_state)
        {
            case UiDefines::Connected:
            {
                ui_state_machine_->SetConnectionState(connection_state);
                ether_logic_->SetConnectionState(connection_state);
                inworld_notification_manager_->SetConnectionState(connection_state);

                // Send welcome message to notification manager
                if (current_world_stream_.get())
                {
                    QString sim = QString::fromStdString(current_world_stream_->GetSimName());
                    QString username = QString::fromStdString(current_world_stream_->GetUsername());
                    if (!sim.isEmpty())
                        GetNotificationManager()->ShowNotification(new MessageNotification("Welcome to " + sim + " " + username, 10000));
                }
                break;
            }
            case UiDefines::Disconnected:
            {
                inworld_notification_manager_->SetConnectionState(connection_state);
                ether_logic_->SetConnectionState(connection_state);
                ui_state_machine_->SetConnectionState(connection_state);
                break;
            }
            case UiDefines::Failed:
            {
                ether_logic_->SetConnectionState(connection_state);
                break;
            }
            default:
                return;
        }
    }

    void UiModule::SubscribeToEventCategories()
    {
        service_category_identifiers_.clear();
        foreach (QString category, event_query_categories_)
            service_category_identifiers_[category] = framework_->GetEventManager()->QueryEventCategory(category.toStdString());
    }

    QObject *UiModule::GetEtherLoginNotifier() const
    { 
        return ether_logic_->GetLoginNotifier();
    }

    QPair<QString, QString> UiModule::GetScreenshotPaths()
    {
        return ether_logic_->GetLastLoginScreenshotData(framework_->GetConfigManager()->GetPath());
    }

}

/************** Poco Module Loading System **************/

extern "C" void POCO_LIBRARY_API SetProfiler(Foundation::Profiler *profiler);
void SetProfiler(Foundation::Profiler *profiler)
{
    Foundation::ProfilerSection::SetProfiler(profiler);
}

using namespace UiServices;

POCO_BEGIN_MANIFEST(Foundation::ModuleInterface)
   POCO_EXPORT_CLASS(UiModule)
POCO_END_MANIFEST

