//$ HEADER_MOD_FILE $
// For conditions of distribution and use, see copyright notice in license.txt

#include "StableHeaders.h"
#include "DebugOperatorNew.h"

#include "UiModule.h"
//#include "UiSettingsService.h"
#include "UiDarkBlueStyle.h"
#include "UiStateMachine.h"
#include "Input.h"

#include "Inworld/InworldSceneController.h"
#include "Inworld/ControlPanelManager.h"
#include "Inworld/NotificationManager.h"
#include "UiProxyWidget.h"
#include "Inworld/Notifications/MessageNotification.h"
#include "Inworld/Notifications/InputNotification.h"
#include "Inworld/Notifications/QuestionNotification.h"
#include "Inworld/Notifications/ProgressNotification.h"

#include "Outworld/ExternalPanelManager.h"
#include "Outworld/ExternalMenuManager.h"
#include "Outworld/ExternalToolBarManager.h"
#include "Outworld/StaticToolBar.h"
#include "Outworld/ViewManager.h"

#include "Common/UiAction.h"
#include "UiSceneService.h"
#include "NaaliUi.h"
#include "NaaliGraphicsView.h"

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
#include "EC_Placeable.h"
#include "EC_Mesh.h"
#include "Renderer.h"
#include "Entity.h"
#include "AssetAPI.h"
#include "QtUiAsset.h"
#include "GenericAssetFactory.h"

#include <Ogre.h>

#include <QApplication>
#include <QFontDatabase>
#include <QDir>
#include <QMainWindow>
#include "MemoryLeakCheck.h"

namespace UiServices
{
    std::string UiModule::type_name_static_ = "UI";

    UiModule::UiModule() :
        IModule(type_name_static_),
        ui_state_machine_(0),
        inworld_scene_controller_(0),
		qWin_(0),
		external_menu_manager_(0),
		external_panel_manager_(0),
		external_toolbar_manager_(0),
		staticToolBar_(0),
        inworld_notification_manager_(0),
		postInitialize_(false)
    {
    }

    UiModule::~UiModule()
    {
        SAFE_DELETE(ui_state_machine_);
        SAFE_DELETE(inworld_scene_controller_);
        SAFE_DELETE(inworld_notification_manager_);
    }

    void UiModule::Load()
    {
		//QApplication::setStyle(new UiDarkBlueStyle());
        //QFontDatabase::addApplicationFont("./media/fonts/FACB.TTF");
        //QFontDatabase::addApplicationFont("./media/fonts/FACBK.TTF");
        event_query_categories_ << "Framework" << "Scene" << "Input";
    }

    void UiModule::Unload()
    {
        ///\todo Just remove the fonts added by UiModule.
        QFontDatabase::removeAllApplicationFonts();
    }

    void UiModule::Initialize()
    {
		framework_->Asset()->RegisterAssetTypeFactory(AssetTypeFactoryPtr(new GenericAssetFactory<QtUiAsset>("QtUiFile")));

		if (GetFramework()->IsHeadless())
			return;

        ui_view_ = GetFramework()->Ui()->GraphicsView();
        if (ui_view_)
        {
            ui_state_machine_ = new CoreUi::UiStateMachine(ui_view_, this);
            ui_state_machine_->RegisterMainScene("Inworld", ui_view_->scene());
            LogDebug("State Machine STARTED");

            inworld_scene_controller_ = new InworldSceneController(GetFramework(), ui_view_);
            LogDebug("Scene Manager service READY");

            inworld_notification_manager_ = new NotificationManager(inworld_scene_controller_);
            /*connect(ui_state_machine_, SIGNAL(SceneAboutToChange(const QString&, const QString&)), 
                    inworld_notification_manager_, SLOT(SceneAboutToChange(const QString&, const QString&)));*/
			connect(ui_state_machine_, SIGNAL(SceneChangedFromMain()), inworld_notification_manager_, SLOT(SceneAboutToChange()));
            LogDebug("Notification Manager service READY");

            // Register UI service
            ui_scene_service_ = UiSceneServicePtr(new UiSceneService(this));
            framework_->GetServiceManager()->RegisterService(Service::ST_Gui, ui_scene_service_);
            connect(ui_scene_service_.get(), SIGNAL(TransferRequest(const QString&, QGraphicsProxyWidget*)),
                    inworld_scene_controller_, SLOT(HandleWidgetTransfer(const QString&, QGraphicsProxyWidget*)));

            framework_->RegisterDynamicObject("uiservice", ui_scene_service_.get());
        }
        else
            LogWarning("Could not acquire QGraphicsView shared pointer from framework, UiServices are disabled");

		//External Ui
		qWin_ = dynamic_cast<QMainWindow*>(framework_->Ui()->MainWindow());
        if (qWin_)
        {
			//qWin_->setObjectName("Naali MainWindow");
		   //Create MenuManager and PanelManager and ToolBarManager
           external_menu_manager_ = new ExternalMenuManager(qWin_->menuBar(), this);
		   //connect(ui_state_machine_, SIGNAL(SceneChangedFromMain()), external_menu_manager_, SLOT(DisableMenus()));
		   //connect(ui_state_machine_, SIGNAL(SceneChangedToMain()), external_menu_manager_, SLOT(EnableMenus()));
		   external_panel_manager_ = new ExternalPanelManager(qWin_, this);
		   connect(ui_state_machine_, SIGNAL(SceneChangedFromMain()), external_panel_manager_, SLOT(DisableDockWidgets()));
		   connect(ui_state_machine_, SIGNAL(SceneChangedToMain()), external_panel_manager_, SLOT(EnableDockWidgets()));
		   external_toolbar_manager_ = new ExternalToolBarManager(qWin_, this);
		   connect(ui_state_machine_, SIGNAL(SceneChangedFromMain()), external_toolbar_manager_, SLOT(DisableToolBars()));
		   connect(ui_state_machine_, SIGNAL(SceneChangedToMain()), external_toolbar_manager_, SLOT(EnableToolBars()));
		   
		   //Configure Static Stuff of the main window
		   //createStaticContent();
        }
        else
			LogWarning("Could not acquire QMainWindow!");
    }


    void UiModule::PostInitialize()
    {
		if (ui_scene_service_)
		{
			SubscribeToEventCategories();
			ui_scene_service_->CreateSettingsPanel();

			bool window_fullscreen = true;
			//Restore values
			if (framework_->IsEditionless()) {
				QSettings settings(QSettings::IniFormat, QSettings::UserScope, APPLICATION_NAME, "configuration/UiPlayerSettings");
				if (!settings.contains("win_state") && !framework_->IsEditionless()){
					//Set default settings
					QSettings default_settings("data/uiexternaldefault.ini", QSettings::IniFormat);
					qWin_->restoreState(default_settings.value("win_state", QByteArray()).toByteArray());		
				} 
				else if (settings.contains("win_state"))
					qWin_->restoreState(settings.value("win_state", QByteArray()).toByteArray());
				window_fullscreen = settings.value("win_fullscreen", false).toBool();
			}
			else
			{
				QSettings settings(QSettings::IniFormat, QSettings::UserScope, APPLICATION_NAME, "configuration/UiSettings");
				if (!settings.contains("win_state") && !framework_->IsEditionless()){
					//Set default settings
					QSettings default_settings("data/uiexternaldefault.ini", QSettings::IniFormat);
					qWin_->restoreState(default_settings.value("win_state", QByteArray()).toByteArray());		
				} 
				else if (settings.contains("win_state"))
					qWin_->restoreState(settings.value("win_state", QByteArray()).toByteArray());
				window_fullscreen = settings.value("win_fullscreen", false).toBool();
			}
			if (window_fullscreen)
				qWin_->showFullScreen();

			//Create the view manager
			if (!framework_->IsEditionless())
				viewManager_=new ViewManager(this,ui_scene_service_.get());

			//Notify that the restore of the main window has been done
			postInitialize_ = true;
		}
    }

    void UiModule::Uninitialize()
    {
		//Save state of the MainWindow
		if (qWin_)
		{
			QSettings settings;
			if (framework_->IsEditionless())
			{
				QSettings settings(QSettings::IniFormat, QSettings::UserScope, APPLICATION_NAME, "configuration/UiPlayerSettings");
				settings.setValue("win_state", qWin_->saveState());
				if (!qWin_->isFullScreen())
				{
					settings.setValue("win_width", qWin_->width());
					settings.setValue("win_height", qWin_->height());
				}
				settings.setValue("win_fullscreen", qWin_->isFullScreen());
			}
			else
			{
				QSettings settings(QSettings::IniFormat, QSettings::UserScope, APPLICATION_NAME, "configuration/UiSettings");
				settings.setValue("win_state", qWin_->saveState());
				if (!qWin_->isFullScreen())
				{
					settings.setValue("win_width", qWin_->width());
					settings.setValue("win_height", qWin_->height());
				}
				settings.setValue("win_fullscreen", qWin_->isFullScreen());
			}
		}

		if (ui_scene_service_)
		{
			framework_->GetServiceManager()->UnregisterService(ui_scene_service_);
			ui_scene_service_.reset();
		}
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
                    ConnectionFailedEvent *in_data = static_cast<ConnectionFailedEvent *>(data);
                    if (in_data)
                        PublishConnectionState(Failed, in_data->message);
                    else
                        PublishConnectionState(Failed, "Unknown connection error");
                    break;
                }
                case Events::EVENT_SERVER_DISCONNECTED:
                    PublishConnectionState(Disconnected);
                    break;
                case Events::EVENT_USER_KICKED_OUT:
                    PublishConnectionState(Disconnected);
                    break;
                case Events::EVENT_SERVER_CONNECTED:
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
    }

    void UiModule::PublishConnectionState(UiServices::ConnectionState connection_state, const QString &message)
    {
        switch (connection_state)
        {
            case Connected:
            {
                ui_state_machine_->SetConnectionState(connection_state);
                break;
            }
            case Disconnected:
            {
                inworld_notification_manager_->ClearHistory();
                ui_state_machine_->SetConnectionState(connection_state);
                break;
            }
            case Failed:
            {
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

