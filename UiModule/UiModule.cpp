// For conditions of distribution and use, see copyright notice in license.txt

#include "StableHeaders.h"
#include "DebugOperatorNew.h"

#include "UiModule.h"
#include "UiSettingsService.h"
//#include "UiProxyStyle.h"
#include "UiDarkBlueStyle.h"
#include "UiStateMachine.h"
#include "ServiceGetter.h"
#include "InputServiceInterface.h"

#include "Ether/EtherLogic.h"
#include "Ether/View/EtherScene.h"
#include "Inworld/InworldSceneController.h"
#include "Inworld/ControlPanelManager.h"
#include "Inworld/NotificationManager.h"
#include "UiProxyWidget.h"
#include "UiWidgetProperties.h"
#include "Inworld/Notifications/MessageNotification.h"
#include "Inworld/Notifications/InputNotification.h"
#include "Inworld/Notifications/QuestionNotification.h"
#include "Inworld/Notifications/ProgressNotification.h"
#include "Common/UiAction.h"
#include "UiSceneService.h"

#include "EventManager.h"
#include "ServiceManager.h"
#include "ConfigurationManager.h"
#include "Framework.h"
#include "WorldStream.h"
#include "NetworkEvents.h"
#include "SceneEvents.h"
#include "InputEvents.h"
#include "UiServiceInterface.h"

#include <QApplication>
#include <QFontDatabase>
#include <QDir>

#include "MemoryLeakCheck.h"

namespace UiServices
{
    std::string UiModule::type_name_static_ = "UI";

    UiModule::UiModule() :
        Foundation::ModuleInterface(type_name_static_),
        event_query_categories_(QStringList()),
        ui_state_machine_(0),
        service_getter_(0),
        inworld_scene_controller_(0),
        inworld_notification_manager_(0),
        ether_logic_(0)
    {
    }

    UiModule::~UiModule()
    {
        SAFE_DELETE(ui_state_machine_);
        SAFE_DELETE(service_getter_);
        SAFE_DELETE(inworld_scene_controller_);
        SAFE_DELETE(inworld_notification_manager_);
        SAFE_DELETE(ether_logic_);
    }

    /*************** ModuleInterfaceImpl ***************/

    void UiModule::Load()
    {
        //QApplication::setStyle(new UiProxyStyle());
        // Application take ownership of the new UiDarkBlueStyle
        QApplication::setStyle(new UiDarkBlueStyle());
        QFontDatabase::addApplicationFont("./media/fonts/FACB.TTF");
        QFontDatabase::addApplicationFont("./media/fonts/FACBK.TTF");

        event_query_categories_ << "Framework" << "Scene" << "Input";
    }

    void UiModule::Unload()
    {
    }

    void UiModule::Initialize()
    {
        ui_view_ = GetFramework()->GetUIView();
        if (ui_view_)
        {
            ui_state_machine_ = new CoreUi::UiStateMachine(ui_view_, this);
            ui_state_machine_->RegisterScene("Inworld", ui_view_->scene());
            UiAction *ether_action = new UiAction(ui_state_machine_);
            UiAction *build_action = new UiAction(ui_state_machine_);
            QObject::connect(ether_action, SIGNAL(triggered()), ui_state_machine_, SLOT(SwitchToEtherScene()));
            QObject::connect(build_action, SIGNAL(triggered()), ui_state_machine_, SLOT(SwitchToBuildScene()));
            LogDebug("State Machine STARTED");

            inworld_scene_controller_ = new InworldSceneController(GetFramework(), ui_view_);
            inworld_scene_controller_->GetControlPanelManager()->SetHandler(Ether, ether_action);
            inworld_scene_controller_->GetControlPanelManager()->SetHandler(Build, build_action);
            LogDebug("Scene Manager service READY");

            inworld_notification_manager_ = new NotificationManager(inworld_scene_controller_);
            LogDebug("Notification Manager service READY");

            service_getter_ = new CoreUi::ServiceGetter(GetFramework());
            inworld_scene_controller_->GetControlPanelManager()->SetServiceGetter(service_getter_);
            ui_state_machine_->SetServiceGetter(service_getter_);
            LogDebug("Service getter READY");

            ui_settings_service_ = UiSettingsPtr(new UiSettingsService(inworld_scene_controller_->GetControlPanelManager()));
            GetFramework()->GetServiceManager()->RegisterService(Foundation::Service::ST_UiSettings, ui_settings_service_);
            LogDebug("UI Settings Service registered and READY");

            // Register UI service.
            ui_scene_service_ = UiSceneServicePtr(new UiSceneService(this));
            framework_->GetServiceManager()->RegisterService(Foundation::Service::ST_Gui, ui_scene_service_);
        }
        else
            LogWarning("Could not acquire QGraphicsView shared pointer from framework, UiServices are disabled");
    }

    void UiModule::PostInitialize()
    {
        SubscribeToEventCategories();
        ether_logic_ = new Ether::Logic::EtherLogic(GetFramework(), ui_view_);
        ui_state_machine_->RegisterScene("Ether", ether_logic_->GetScene());
        ether_logic_->Start();
        ui_state_machine_->SwitchToEtherScene();
        LogDebug("Ether Logic STARTED");

        input = framework_->Input().RegisterInputContext("EtherInput", 90);
        input->SetTakeKeyboardEventsOverQt(true);
        connect(input.get(), SIGNAL(KeyPressed(KeyEvent *)), this, SLOT(OnKeyPressed(KeyEvent *)));
    }

    void UiModule::Uninitialize()
    {
        framework_->GetServiceManager()->UnregisterService(ui_scene_service_);
        ui_scene_service_.reset();
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
                    PublishConnectionState(Failed);
                    break;
                case ProtocolUtilities::Events::EVENT_SERVER_DISCONNECTED:
                    PublishConnectionState(Disconnected);
                    break;
                case ProtocolUtilities::Events::EVENT_USER_KICKED_OUT:
                    PublishConnectionState(Disconnected);
                    break;
                case ProtocolUtilities::Events::EVENT_SERVER_CONNECTED:
                    // Udp connection has been established, we are still loading object so lets not change UI layer yet
                    // to connected state. See Scene categorys EVENT_CONTROLLABLE_ENTITY case for real UI switch.
                    break;
                default:
                    break;
            }
        }
        else if (category == "Scene")
        {
            switch (event_id)
            {
                case Scene::Events::EVENT_CONTROLLABLE_ENTITY:
                    PublishConnectionState(Connected);
                    break;
                default:
                    break;
            }
        }

        return false;
    }

    void UiModule::OnKeyPressed(KeyEvent *key)
    {
        // We only act on key presses that are not repeats.
        if (key->eventType != KeyEvent::KeyPressed || key->keyPressCount > 1)
            return;

        InputServiceInterface &inputService = framework_->Input();

        const QKeySequence toggleEther =   inputService.KeyBinding("Ether.ToggleEther", Qt::Key_Escape);
        const QKeySequence toggleWorldChat =  inputService.KeyBinding("Ether.ToggleWorldChat", Qt::Key_F2);

        if (key->keyCode == toggleEther)
            ui_state_machine_->ToggleEther();

        if (key->keyCode == toggleWorldChat)
            inworld_scene_controller_->SetFocusToChat();
    }

    void UiModule::PublishConnectionState(UiServices::ConnectionState connection_state)
    {
        switch (connection_state)
        {
            case Connected:
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
            case Disconnected:
            {
                inworld_notification_manager_->SetConnectionState(connection_state);
                ether_logic_->SetConnectionState(connection_state);
                ui_state_machine_->SetConnectionState(connection_state);
                break;
            }
            case Failed:
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

