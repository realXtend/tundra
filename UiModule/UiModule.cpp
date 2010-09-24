// For conditions of distribution and use, see copyright notice in license.txt

#include "StableHeaders.h"
#include "DebugOperatorNew.h"

#include "UiModule.h"
#include "UiSettingsService.h"
#include "UiDarkBlueStyle.h"
#include "UiStateMachine.h"
#include "ServiceGetter.h"
#include "InputServiceInterface.h"

#include "Ether/EtherLogic.h"
#include "Ether/EtherSceneController.h"
#include "Ether/View/EtherScene.h"
#include "Inworld/InworldSceneController.h"
#include "Inworld/ControlPanelManager.h"
#include "Inworld/NotificationManager.h"
#include "UiProxyWidget.h"
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
#include "WorldLogicInterface.h"
#include "EC_OgrePlaceable.h"
#include "EC_OgreMesh.h"
#include "Renderer.h"
#include "Entity.h"

#include <Ogre.h>

#include <QApplication>
#include <QFontDatabase>
#include <QDir>

#include "MemoryLeakCheck.h"

namespace UiServices
{
    std::string UiModule::type_name_static_ = "UI";

    UiModule::UiModule() :
        IModule(type_name_static_),
        ui_state_machine_(0),
        service_getter_(0),
        inworld_scene_controller_(0),
        inworld_notification_manager_(0),
        ether_logic_(0),
        welcome_message_(0)
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

    void UiModule::Load()
    {
        //QApplication::setStyle(new UiProxyStyle());
        // QApplication take ownership of the new UiDarkBlueStyle
        ///\todo UiDarkBlueStyle seems to be causing many memory leaks.
        /// Maybe it's not deleted properly by the QApplication?
        QApplication::setStyle(new UiDarkBlueStyle());
        QFontDatabase::addApplicationFont("./media/fonts/FACB.TTF");
        QFontDatabase::addApplicationFont("./media/fonts/FACBK.TTF");

        event_query_categories_ << "Framework" << "Scene" << "Input";
    }

    void UiModule::Unload()
    {
        ///\todo Just remove the fonts added by UiModule.
        QFontDatabase::removeAllApplicationFonts();
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
            UiAction *avatar_action = new UiAction(ui_state_machine_);
            connect(ether_action, SIGNAL(triggered()), ui_state_machine_, SLOT(SwitchToEtherScene()));
            connect(build_action, SIGNAL(triggered()), ui_state_machine_, SLOT(SwitchToBuildScene()));
            connect(avatar_action, SIGNAL(triggered()), ui_state_machine_, SLOT(SwitchToAvatarScene()));
            LogDebug("State Machine STARTED");

            inworld_scene_controller_ = new InworldSceneController(GetFramework(), ui_view_);
            inworld_scene_controller_->GetControlPanelManager()->SetHandler(Ether, ether_action);
            inworld_scene_controller_->GetControlPanelManager()->SetHandler(Build, build_action);
            inworld_scene_controller_->GetControlPanelManager()->SetHandler(Avatar, avatar_action);
            LogDebug("Scene Manager service READY");

            inworld_notification_manager_ = new NotificationManager(inworld_scene_controller_);
            connect(ui_state_machine_, SIGNAL(SceneAboutToChange(const QString&, const QString&)), 
                    inworld_notification_manager_, SLOT(SceneAboutToChange(const QString&, const QString&)));
            LogDebug("Notification Manager service READY");

            service_getter_ = new CoreUi::ServiceGetter(GetFramework());
            inworld_scene_controller_->GetControlPanelManager()->SetServiceGetter(service_getter_);
            ui_state_machine_->SetServiceGetter(service_getter_);
            LogDebug("Service getter READY");

            // Register settings service
            ui_settings_service_ = UiSettingsPtr(new UiSettingsService(inworld_scene_controller_->GetControlPanelManager()));
            GetFramework()->GetServiceManager()->RegisterService(Foundation::Service::ST_UiSettings, ui_settings_service_);
            LogDebug("UI Settings Service registered and READY");

            // Register UI service
            ui_scene_service_ = UiSceneServicePtr(new UiSceneService(this));
            framework_->GetServiceManager()->RegisterService(Foundation::Service::ST_Gui, ui_scene_service_);
            connect(ui_scene_service_.get(), SIGNAL(TransferRequest(const QString&, QGraphicsProxyWidget*)),
                    inworld_scene_controller_, SLOT(HandleWidgetTransfer(const QString&, QGraphicsProxyWidget*)));
        }
        else
            LogWarning("Could not acquire QGraphicsView shared pointer from framework, UiServices are disabled");
    }

    void UiModule::PostInitialize()
    {
        SubscribeToEventCategories();

        // Start ether logic and register to scene service
        ether_logic_ = new Ether::Logic::EtherLogic(GetFramework(), ui_view_);
        ui_state_machine_->RegisterScene("Ether", ether_logic_->GetScene());
        ether_logic_->Start();
        // Switch ether scene active on startup
        ui_state_machine_->SwitchToEtherScene();
        // Connect the switch signal to needed places
        connect(ui_state_machine_, SIGNAL(SceneChanged(const QString&, const QString&)), 
                ether_logic_->GetQObjSceneController(), SLOT(UiServiceSceneChanged(const QString&, const QString&)));
        connect(ui_state_machine_, SIGNAL(SceneChanged(const QString&, const QString&)), 
                SLOT(OnSceneChanged(const QString&, const QString&)));
        LogDebug("Ether Logic STARTED");

        input = framework_->Input()->RegisterInputContext("EtherInput", 90);
        input->SetTakeKeyboardEventsOverQt(true);
        connect(input.get(), SIGNAL(KeyPressed(KeyEvent *)), this, SLOT(OnKeyPressed(KeyEvent *)));

        Foundation::WorldLogicInterface *worldLogic = framework_->GetService<Foundation::WorldLogicInterface>();
        if (worldLogic)
            connect(worldLogic, SIGNAL(AboutToDeleteWorld()), SLOT(TakeEtherScreenshots()));
        else
            LogWarning("Could not get world logic service.");
    }

    void UiModule::Uninitialize()
    {
        framework_->GetServiceManager()->UnregisterService(ui_scene_service_);
        ui_scene_service_.reset();
    }

    void UiModule::Update(f64 frametime)
    {
    }

    bool UiModule::HandleEvent(event_category_id_t category_id, event_id_t event_id, IEventData* data)
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
            using namespace ProtocolUtilities;
            switch (event_id)
            {
                case Events::EVENT_CONNECTION_FAILED:
                {
                    ConnectionFailedEvent *event = static_cast<ConnectionFailedEvent *>(data);
                    PublishConnectionState(Failed, event->message);
                    break;
                }
                case Events::EVENT_SERVER_DISCONNECTED:
                    PublishConnectionState(Disconnected);
                    break;
                case Events::EVENT_USER_KICKED_OUT:
                    PublishConnectionState(Disconnected);
                    break;
                case Events::EVENT_SERVER_CONNECTED:
                    // Udp connection has been established, we are still loading object so lets not change UI layer yet
                    // to connected state. See Scene categorys EVENT_CONTROLLABLE_ENTITY case for real UI switch.
                    ether_logic_->GetSceneController()->ShowStatusInformation("Connected, loading world content...", 60000);
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

        InputServiceInterface &inputService = *framework_->Input();

        const QKeySequence toggleEther = inputService.KeyBinding("Ether.ToggleEther", Qt::Key_Escape);
        const QKeySequence toggleWorldChat = inputService.KeyBinding("Ether.ToggleWorldChat", Qt::Key_F2);

        if (key->keyCode == toggleEther)
            ui_state_machine_->ToggleEther();

        if (key->keyCode == toggleWorldChat)
            inworld_scene_controller_->SetFocusToChat();
    }

    void UiModule::OnSceneChanged(const QString &old_name, const QString &new_name)
    {
        if (!welcome_message_)
            return;
        if (old_name.toLower() == "ether" && new_name.toLower() == "inworld")
        {
            // A bit of a hack to add the notification only when inworld scene is active,
            // if we dont do this the notification widget does not respond to mouse clicks
            // and wont hide when the timer runs out
            GetNotificationManager()->ShowNotification(welcome_message_);
            welcome_message_ = 0;
        }
    }

    void UiModule::PublishConnectionState(UiServices::ConnectionState connection_state, const QString &message)
    {
        switch (connection_state)
        {
            case Connected:
            {
                ui_state_machine_->SetConnectionState(connection_state);
                ether_logic_->SetConnectionState(connection_state);

                // Send welcome message to notification manager
                if (current_world_stream_.get())
                {
                    QString sim = QString::fromStdString(current_world_stream_->GetSimName());
                    QString username = QString::fromStdString(current_world_stream_->GetUsername());
                    if (!sim.isEmpty())
                        welcome_message_ = new MessageNotification("Welcome to " + sim + " " + username, 10000);
                    else
                        welcome_message_ = 0;
                }
                break;
            }
            case Disconnected:
            {
                inworld_notification_manager_->ClearHistory();
                ether_logic_->SetConnectionState(connection_state);
                ui_state_machine_->SetConnectionState(connection_state);
                break;
            }
            case Failed:
            {
                ether_logic_->SetConnectionState(connection_state, message);
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

    Ether::Logic::EtherLoginNotifier *UiModule::GetEtherLoginNotifier() const
    {
        return ether_logic_->GetLoginNotifier();
    }

    void UiModule::TakeEtherScreenshots()
    {
        Foundation::WorldLogicInterface *worldLogic = framework_->GetService<Foundation::WorldLogicInterface>();
        if (!worldLogic)
            return;

        Scene::EntityPtr avatar_entity = worldLogic->GetUserAvatarEntity();
        if (!avatar_entity)
            return;

        OgreRenderer::EC_OgrePlaceable *ec_placeable = avatar_entity->GetComponent<OgreRenderer::EC_OgrePlaceable>().get();
        OgreRenderer::EC_OgreMesh *ec_mesh = avatar_entity->GetComponent<OgreRenderer::EC_OgreMesh>().get();

        if (!ec_placeable || !ec_mesh || !avatar_entity->HasComponent("EC_AvatarAppearance"))
            return;
        if (!ec_mesh->GetEntity())
            return;

        // Head bone pos setup
        Vector3df avatar_position = ec_placeable->GetPosition();
        Quaternion avatar_orientation = ec_placeable->GetOrientation();
        Ogre::SkeletonInstance* skel = ec_mesh->GetEntity()->getSkeleton();
        float adjustheight = ec_mesh->GetAdjustPosition().z;
        Vector3df avatar_head_position;

        QString view_bone_name = worldLogic->GetAvatarAppearanceProperty("headbone");
        if (!view_bone_name.isEmpty() && skel && skel->hasBone(view_bone_name.toStdString()))
        {
            adjustheight += 0.15f;
            Ogre::Bone* bone = skel->getBone(view_bone_name.toStdString());
            Ogre::Vector3 headpos = bone->_getDerivedPosition();
            Vector3df ourheadpos(-headpos.z + 0.5f, -headpos.x, headpos.y + adjustheight);
            avatar_head_position = avatar_position + (avatar_orientation * ourheadpos);
        }
        else
        {
            // Fallback: will get screwed up shot but not finding the headbone should not happen, ever
            avatar_head_position = ec_placeable->GetPosition();
        }

        // Get paths where to store the screenshots and pass to renderer for screenshots.
        QPair<QString, QString> paths = ether_logic_->GetLastLoginScreenshotData(framework_->GetConfigManager()->GetPath());
        boost::shared_ptr<Foundation::RenderServiceInterface> render_service = 
            framework_->GetServiceManager()->GetService<Foundation::RenderServiceInterface>(Foundation::Service::ST_Renderer).lock();

        if (render_service && !paths.first.isEmpty() && !paths.second.isEmpty())
        {
            QPixmap render_result;
            render_result = render_service->RenderImage();
            render_result.save(paths.first);
            render_result = render_service->RenderAvatar(avatar_head_position, avatar_orientation);
            render_result.save(paths.second);
        }
    }
}

/************** Poco Module Loading System **************/

extern "C" void POCO_LIBRARY_API SetProfiler(Foundation::Profiler *profiler);
void SetProfiler(Foundation::Profiler *profiler)
{
    Foundation::ProfilerSection::SetProfiler(profiler);
}

using namespace UiServices;

POCO_BEGIN_MANIFEST(IModule)
   POCO_EXPORT_CLASS(UiModule)
POCO_END_MANIFEST

