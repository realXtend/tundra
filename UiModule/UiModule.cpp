// For conditions of distribution and use, see copyright notice in license.txt

#include "StableHeaders.h"
#include "DebugOperatorNew.h"
#include "UiModule.h"
#include "UiProxyStyle.h"
#include "UiStateMachine.h"

#include "EventManager.h"
#include "ConfigurationManager.h"
#include "Framework.h"

#include "Inworld/InworldSceneController.h"
#include "Inworld/NotificationManager.h"
#include "Inworld/View/UiProxyWidget.h"
#include "Inworld/View/UiWidgetProperties.h"
#include "Inworld/Console/UiConsoleManager.h"

#include "Ether/EtherLogic.h"
#include "Ether/View/EtherScene.h"

#include "NetworkEvents.h"
#include "SceneEvents.h"
#include "ConsoleEvents.h"

#include <QApplication>
#include <QDir>

#include "MemoryLeakCheck.h"

namespace UiServices
{
    UiModule::UiModule() 
        : Foundation::ModuleInterfaceImpl(Foundation::Module::MT_UiServices),
          event_query_categories_(QStringList()),
          inworld_scene_controller_(0),
          inworld_notification_manager_(0)
    {
    }

    UiModule::~UiModule()
    {
        SAFE_DELETE(ui_state_machine_);
        SAFE_DELETE(inworld_scene_controller_);
        SAFE_DELETE(inworld_notification_manager_);
        SAFE_DELETE(ui_console_manager_);
        SAFE_DELETE(ether_logic_);
    }

    /*************** ModuleInterfaceImpl ***************/

    void UiModule::Load()
    {
        QApplication::setStyle(new UiProxyStyle());
        event_query_categories_ << "Framework" << "Scene"  << "Console";
    }

    void UiModule::Unload()
    {
    }

    void UiModule::Initialize()
    {
        ui_view_ = framework_->GetUIView();
        if (ui_view_)
        {
            LogDebug("Acquired Ogre QGraphicsView shared pointer from framework");

            ui_state_machine_ = new UiStateMachine(ui_view_);
            ui_state_machine_->RegisterScene("Inworld", ui_view_->scene());
            LogDebug("State Machine STARTED");

            inworld_scene_controller_ = new InworldSceneController(GetFramework(), ui_view_);
            LogDebug("Scene Manager service READY");

            inworld_notification_manager_ = new NotificationManager(GetFramework(), ui_view_);
            LogDebug("Notification Manager service READY");

            ui_console_manager_ = new CoreUi::UiConsoleManager(GetFramework(), ui_view_);
            LogDebug("Console UI READY");
        }
        else
            LogWarning("Could not accuire QGraphicsView shared pointer from framework, UiServices are disabled");
    }

    void UiModule::PostInitialize()
    {
        SubscribeToEventCategories();
        ui_console_manager_->SendInitializationReadyEvent();

        ether_logic_ = new Ether::Logic::EtherLogic(GetFramework(), ui_view_);
        ui_state_machine_->RegisterScene("Ether", ether_logic_->GetScene());
        ether_logic_->Start();
        ui_state_machine_->SwitchToEtherScene(); // comment to set classic login as default on startup
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
                    // Do this only once
                    if (!event_query_categories_.contains("NetworkState"))
                        event_query_categories_ << "NetworkState";
                    SubscribeToEventCategories();
                    break;
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
                    PublishConnectionState(Failed);
                    break;
                }
                case ProtocolUtilities::Events::EVENT_SERVER_DISCONNECTED:
                {
                    PublishConnectionState(Disconnected);
                    break;
                }
                case ProtocolUtilities::Events::EVENT_SERVER_CONNECTED:
                {
                    ProtocolUtilities::AuthenticationEventData *auth_data = dynamic_cast<ProtocolUtilities::AuthenticationEventData *>(data);
                    if (auth_data)
                    {
                        current_avatar_ = QString::fromStdString(auth_data->identityUrl);
                        current_server_ = QString::fromStdString(auth_data->hostUrl);
                    }
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
                ui_console_manager_->ToggleConsole();
                break;
            case Console::Events::EVENT_CONSOLE_PRINT_LINE:
                Console::ConsoleEventData* console_data=dynamic_cast<Console::ConsoleEventData*>(data);
                ui_console_manager_->QueuePrintRequest(QString(console_data->message.c_str()));
                break;
            }
        }
        else if (category == "Scene")
        {
            switch (event_id)
            {
                case Scene::Events::EVENT_CONTROLLABLE_ENTITY:
                {
                    PublishConnectionState(Connected);
                    QString welcome_message;
                    if (!current_avatar_.isEmpty())
                        welcome_message = current_avatar_ + " welcome to " + current_server_;
                    else
                        welcome_message = "Welcome to " + current_server_;
                    inworld_notification_manager_->ShowInformationString(welcome_message, 10000);
                    break;
                }
                default:
                    break;
            }
        }

        return false;
    }

    void UiModule::PublishConnectionState(ConnectionState connection_state)
    {
        switch (connection_state)
        {
            case Connected:
                ui_state_machine_->SetConnectionState(connection_state);
                ether_logic_->SetConnectionState(connection_state);
                break;

            case Disconnected:
                ether_logic_->SetConnectionState(connection_state);
                ui_state_machine_->SetConnectionState(connection_state);
                break;

            case Failed:
                ether_logic_->SetConnectionState(connection_state);
                break;

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

    QObject *UiModule::GetEtherLoginNotifier() 
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

