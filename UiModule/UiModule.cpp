//$ HEADER_MOD_FILE $
// For conditions of distribution and use, see copyright notice in license.txt

#include "StableHeaders.h"
#include "DebugOperatorNew.h"

#include "UiModule.h"
#include "UiDarkBlueStyle.h"
#include "InputAPI.h"

#include "Inworld/InworldSceneController.h"
#include "UiProxyWidget.h"

#include "Inworld/ControlPanel/ChangeThemeWidget.h"

#include "Outworld/ExternalPanelManager.h"
#include "Outworld/ViewManager.h"

#include "UiAPI.h"
#include "UiGraphicsView.h"
#include "UiMainWindow.h"

#include "EventManager.h"
#include "ConfigurationManager.h"
#include "Framework.h"
#include "WorldStream.h"
#include "NetworkEvents.h"
#include "SceneEvents.h"
#include "InputEvents.h"
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
        inworld_scene_controller_(0),
		qWin_(0),
		external_panel_manager_(0),
        external_widgets_(),
		win_restored_(false),
        win_uninitialized_(false),
        changetheme_widget_(0)
    {
    }

    UiModule::~UiModule()
    {
        SAFE_DELETE(inworld_scene_controller_);
    }

    void UiModule::Load()
    {
        event_query_categories_ << "Framework" << "Scene" << "Input";
    }

    void UiModule::Unload()
    {
        ///\todo Just remove the fonts added by UiModule.
        QFontDatabase::removeAllApplicationFonts();
    }

    void UiModule::Initialize()
    {
		if (GetFramework()->IsHeadless())
			return;

        ui_view_ = GetFramework()->Ui()->GraphicsView();
        if (ui_view_)
        {
            inworld_scene_controller_ = new InworldSceneController(GetFramework(), ui_view_);
            LogDebug("Scene Manager service READY");
        }
        else
            LogWarning("Could not acquire QGraphicsView shared pointer from framework, UiServices are disabled");

		//External Ui
		qWin_ = dynamic_cast<UiMainWindow*>(framework_->Ui()->MainWindow());
        if (qWin_)
            external_panel_manager_ = new ExternalPanelManager(qWin_, this);
        else
			LogWarning("Could not acquire QMainWindow!");

        //Listen to UiAPI
        connect(framework_->Ui(), SIGNAL(CustomizeAddWidgetToWindow(UiWidget *)), SLOT(AddWidgetToWindow(UiWidget *)));
        connect(framework_->Ui(), SIGNAL(CustomizeRemoveWidgetFromWindow(UiWidget *)), SLOT(RemoveWidgetFromScene(UiWidget *)));
    }


    void UiModule::PostInitialize()
    {
		if (GetFramework()->IsHeadless())
			return;

		if (!framework_->IsEditionless())
			viewManager_=new ViewManager(this);
        
        // Adding change theme tab
        changetheme_widget_ = new CoreUi::ChangeThemeWidget(framework_);
    }

	void UiModule::RestoreMainWindow()
	{
		bool window_fullscreen = true;
		//Restore values
		if (framework_->IsEditionless()) {
			QSettings settings(QSettings::IniFormat, QSettings::UserScope, APPLICATION_NAME, "configuration/UiPlayerSettings");
			if (!settings.contains("win_state") && !framework_->IsEditionless()){
				//Set default settings
				QSettings default_settings("data/uidefault.ini", QSettings::IniFormat);
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
				QSettings default_settings("data/uidefault.ini", QSettings::IniFormat);
				qWin_->restoreState(default_settings.value("win_state", QByteArray()).toByteArray());
				//First time, show it maximized
				qWin_->showMaximized();
			} 
			else if (settings.contains("win_state"))
				qWin_->restoreState(settings.value("win_state", QByteArray()).toByteArray());
			window_fullscreen = settings.value("win_fullscreen", false).toBool();
		}
		if (window_fullscreen)
			qWin_->showFullScreen();
	}

    void UiModule::Uninitialize()
    {
        win_uninitialized_ = true;

		if (GetFramework()->IsHeadless())
			return;

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
					settings.setValue("win_pos", qWin_->pos());
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
					settings.setValue("win_pos", qWin_->pos());
				}
				settings.setValue("win_fullscreen", qWin_->isFullScreen());
			}
		}

		viewManager_->DeleteView("Previous");
    }

    void UiModule::Update(f64 frametime)
    {
		//Notify that the restore of the main window has been done
		if (!win_restored_ && !framework_->IsEditionless() && qWin_){
			win_restored_ = true;
			RestoreMainWindow();
		}
    }
	
    bool UiModule::HandleEvent(event_category_id_t category_id, event_id_t event_id, IEventData* data)
    {
        return false;
    }

    void UiModule::OnKeyPressed(KeyEvent *key)
    {
    }


    void UiModule::AddWidgetToWindow(UiWidget *widget)
    {
		QString pos = "vacio";
		if (widget->windowTitle() == "")
            return;

        if(widget->property("dockable") == false)
            return;

		if (!external_widgets_.contains(widget))
        {
            QDockWidget* dock_widget = external_panel_manager_->AddExternalPanel(widget,widget->windowTitle());
            if (dock_widget)
                external_widgets_[widget] = dock_widget;
        }
    }

    void UiModule::RemoveWidgetFromScene(UiWidget *widget)
    {
		if (external_widgets_.contains(widget)) {
			external_panel_manager_->RemoveExternalPanel(external_widgets_[widget]);
			external_widgets_.remove(widget);
        }
    }

    void UiModule::AddAnchoredWidgetToScene(QWidget *widget, Qt::Corner corner, Qt::Orientation orientation, int priority, bool persistence)
    {
		inworld_scene_controller_->AddAnchoredWidgetToScene(widget, corner, orientation, priority, persistence);
	}

    void UiModule::RemoveAnchoredWidgetFromScene(QWidget *widget)
    {
        inworld_scene_controller_->RemoveAnchoredWidgetFromScene(widget);
    }

    QWidget *UiModule::GetThemeSettingsWidget()
    {
        if (changetheme_widget_)
            return dynamic_cast<QWidget *>(changetheme_widget_);
        else
            return 0;
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

