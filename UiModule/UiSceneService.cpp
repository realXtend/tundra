//$ HEADER_MOD_FILE $
/**
 *  For conditions of distribution and use, see copyright notice in license.txt
 *
 *  @file   UiSceneService.cpp
 *  @brief  Implements UiServiceInterface and provides means of adding widgets to the 
 *          in-world scene and managing different UI scenes.
 *          Basically this class is just a wrapper around InworldSceneController
 *          and UiStateMachine.
 */

#include "StableHeaders.h"
#include "UiSceneService.h"
#include "UiModule.h"
#include "Inworld/InworldSceneController.h"
#include "UiStateMachine.h"
#include "UiProxyWidget.h"
#include "Inworld/Menus/MenuManager.h"
#include "Inworld/NotificationManager.h"

#include <QUiLoader>
#include <QSettings>

#include "Inworld/ControlPanel/SettingsWidget.h"

namespace UiServices
{
    UiSceneService::UiSceneService(UiModule *owner) : owner_(owner), moveable_widgets_(new QList<QString>())
    {
        connect(owner_->GetUiStateMachine(), SIGNAL(SceneChanged(const QString&, const QString&)),
                this, SIGNAL(SceneChanged(const QString&, const QString&)));
        connect(owner_->GetUiStateMachine(), SIGNAL(SceneChangeComplete()),
                this, SLOT(TranferWidgets()));

        connect(owner_->GetNotificationManager(), SIGNAL(ShowNotificationCalled(const QString&)), this, SIGNAL(Notification(const QString&)));

		//connect(owner_->GetUiStateMachine(), SIGNAL(SceneChanged(const QString&, const QString&)),this,SLOT(HandleTransferToBuild(const QString&, const QString&)));
		//Settings Panel, List of panels send when we enter the world
		//connect(owner_->GetUiStateMachine(), SIGNAL(SceneChanged(const QString&, const QString&)), this,SLOT(SetPanelsList(const QString&, const QString&)));
    }

    UiSceneService::~UiSceneService()
    {
    }

    UiProxyWidget *UiSceneService::AddWidgetToScene(QWidget *widget, bool moveable , bool outside, Qt::WindowFlags flags)
    {
		bool bydefault=false;
		//uiExternal= owner_->GetFramework()->GetService<Foundation::UiExternalServiceInterface>();
		QWidget* qdock= new QDockWidget(widget->windowTitle());
		qdock->setObjectName(widget->windowTitle());
		qdock->setWindowTitle(widget->windowTitle());		
		widget->setObjectName(widget->windowTitle());
		UiProxyWidget *proxy;

		QString test = widget->windowTitle(); 

		//First of all, we check if it is moveable and if it has been placed in settings file..
		if (moveable){
			//save value
			moveable_widgets_->append(widget->windowTitle());
			//then check if we have settings..
			QSettings settings("Naali UIExternal2", "UiExternal Settings");
			QString pos = "vacio";
			if (widget->windowTitle() != "")
				pos = settings.value(widget->windowTitle(), QString("vacio")).toString();

			if (pos == "outside"){
				proxy = new UiProxyWidget(widget, flags);
				proxy->setWidget(0);
				qdock = owner_->GetExternalPanelManager()->AddExternalPanel(widget,widget->windowTitle());
			} else if(pos == "inside")
				proxy= owner_->GetInworldSceneController()->AddWidgetToScene(widget, flags);
			else 
				bydefault = true;
		}else
			bydefault = true;

		//We do by default behaviour when seetings var doesn't exist or if it is not moveable
		if (bydefault) {
			if(outside){	
				proxy = new UiProxyWidget(widget, flags);
				proxy->setWidget(0);
				qdock = owner_->GetExternalPanelManager()->AddExternalPanel(widget,widget->windowTitle()); 
			}else
				proxy= owner_->GetInworldSceneController()->AddWidgetToScene(widget, flags);
		}
		//Save the pair
		proxy_dock_list[widget->windowTitle()]=proxyDock(proxy,dynamic_cast<QDockWidget*>(qdock));
		return proxy;
    }

    bool UiSceneService::AddWidgetToScene(UiProxyWidget *widget)
    {
		//DEFAULT OUTSIDE = TRUE & MOVEABLE = TRUE. IMPORTANT THING TO HAVE IN MIND!!		
		QWidget* qdock= new QDockWidget(widget->windowTitle());
		qdock->setObjectName(widget->windowTitle());
		qdock->setWindowTitle(widget->windowTitle());
		widget->setObjectName(widget->windowTitle());
		QWidget* wid = widget->widget();

		QSettings settings("Naali UIExternal2", "UiExternal Settings");
		QString pos = settings.value(widget->windowTitle(), QString("vacio")).toString();
		if (pos != "inside")
        {
			widget->setWidget(0);
			qdock = owner_->GetExternalPanelManager()->AddExternalPanel(wid,widget->windowTitle());
		}
        else
			owner_->GetInworldSceneController()->AddProxyWidget(widget);
		//Save the pair
		proxy_dock_list[widget->windowTitle()]=proxyDock(widget,dynamic_cast<QDockWidget*>(qdock));
		//If moveable, register it as moveable
		moveable_widgets_->append(wid->windowTitle());
		return true;
    }

    void UiSceneService::AddWidgetToMenu(QWidget *widget)
    {
		//Save initial values
		if (!panels_menus_list_.contains(widget->windowTitle()))
			panels_menus_list_[widget->windowTitle()]=menusPair("", "");

		QDockWidget* qdock=proxy_dock_list[widget->windowTitle()].second;
		//If is outside is because it has been put outside, and also the menu!
		if(qdock->widget())
			owner_->GetExternalMenuManager()->AddExternalMenuPanel(qdock,widget->windowTitle(),"Panels", moveable_widgets_->contains(widget->windowTitle()));
        else
		    owner_->GetInworldSceneController()->AddWidgetToMenu(widget, widget->windowTitle(), "", "");
    }

    void UiSceneService::AddWidgetToMenu(QWidget *widget, const QString &entry, const QString &menu, const QString &icon)
    {
		//Save initial values
		if (!panels_menus_list_.contains(widget->windowTitle()))
			panels_menus_list_[widget->windowTitle()]=menusPair(menu, icon);

		QDockWidget* qdock = proxy_dock_list[widget->windowTitle()].second;
        if (qdock && qdock->widget())
           owner_->GetExternalMenuManager()->AddExternalMenuPanel(qdock,entry,menu, moveable_widgets_->contains(widget->windowTitle()));
        else
            owner_->GetInworldSceneController()->AddWidgetToMenu(widget, entry, menu, icon);
    }

    void UiSceneService::AddWidgetToMenu(UiProxyWidget *widget, const QString &entry, const QString &menu, const QString &icon)
    {
		//Save initial values
		if (!panels_menus_list_.contains(widget->windowTitle()))
			panels_menus_list_[widget->windowTitle()]=menusPair(menu, icon);

	    QDockWidget* qdock = proxy_dock_list[widget->windowTitle()].second; 
        if (qdock->widget())
           owner_->GetExternalMenuManager()->AddExternalMenuPanel(qdock,entry,menu, moveable_widgets_->contains(widget->windowTitle()));
        else
            owner_->GetInworldSceneController()->AddWidgetToMenu(widget->widget(), entry, menu, icon);
    }

    void UiSceneService::RemoveWidgetFromMenu(QWidget *widget)
    {
		QDockWidget* qdock=proxy_dock_list[widget->windowTitle()].second;
		owner_->GetExternalMenuManager()->RemoveExternalMenuPanel(qdock->widget());
		owner_->GetInworldSceneController()->RemoveWidgetFromMenu(widget);
    }

    void UiSceneService::RemoveWidgetFromMenu(QGraphicsProxyWidget *widget)
    {
        owner_->GetInworldSceneController()->RemoveWidgetFromMenu(widget->widget());
		QDockWidget* qdock=proxy_dock_list[widget->windowTitle()].second;
		owner_->GetExternalMenuManager()->RemoveExternalMenuPanel(qdock->widget());

    }

    void UiSceneService::RemoveWidgetFromScene(QWidget *widget)
    {
		if(proxy_dock_list[widget->windowTitle()].second->widget())
			owner_->GetExternalPanelManager()->RemoveExternalPanel(widget->parentWidget());
		else
			owner_->GetInworldSceneController()->RemoveProxyWidgetFromScene(widget);
    }

    void UiSceneService::RemoveWidgetFromScene(QGraphicsProxyWidget *widget)
    {
		if(widget->widget())
			owner_->GetInworldSceneController()->RemoveProxyWidgetFromScene(widget);
		else
		{
			QDockWidget* qdock=proxy_dock_list[widget->windowTitle()].second;
			owner_->GetExternalPanelManager()->RemoveExternalPanel(qdock);
		}
    }

    void UiSceneService::ShowWidget(QWidget *widget) const
    {
		if(proxy_dock_list[widget->windowTitle()].second->widget())
			owner_->GetExternalPanelManager()->ShowWidget(widget);
		else
			owner_->GetInworldSceneController()->ShowProxyForWidget(widget);
    }

    void UiSceneService::HideWidget(QWidget *widget) const
    {
		if(proxy_dock_list[widget->windowTitle()].second->widget())
			owner_->GetExternalPanelManager()->HideWidget(widget);
		else
			owner_->GetInworldSceneController()->HideProxyForWidget(widget);
    }

    void UiSceneService::BringWidgetToFront(QWidget *widget) const
    {
		if(proxy_dock_list[widget->windowTitle()].second->widget())
			owner_->GetExternalPanelManager()->ShowWidget(widget);
		else
			owner_->GetInworldSceneController()->BringProxyToFront(widget);
    }

	void UiSceneService::BringWidgetToFront(QString widget)
    {
		//To show the widget, it has to exist
		if (proxy_dock_list.contains(widget)){
			proxyDock pair = proxy_dock_list.value(widget);
			QDockWidget* qdock=pair.second;
			UiProxyWidget* proxy=pair.first;
			if(qdock->widget())
				BringWidgetToFront(qdock->widget());
			else
				BringWidgetToFront(proxy->widget());
		}
    }

    void UiSceneService::BringWidgetToFront(QGraphicsProxyWidget *widget) const
    {
		if(widget->widget())
			owner_->GetInworldSceneController()->BringProxyToFront(widget);
		else
			owner_->GetExternalPanelManager()->ShowWidget(proxy_dock_list[widget->windowTitle()].second->widget());

    }
//$ END_MOD $
    bool UiSceneService::AddSettingsWidget(QWidget *widget, const QString &name) const
    {
        return owner_->GetInworldSceneController()->AddSettingsWidget(widget, name);
    }

    QGraphicsScene *UiSceneService::GetScene(const QString &name) const
    {
        return owner_->GetUiStateMachine()->GetScene(name);
    }

    void UiSceneService::RegisterScene(const QString &name, QGraphicsScene *scene)
    {
        owner_->GetUiStateMachine()->RegisterScene(name, scene);
    }

    bool UiSceneService::UnregisterScene(const QString &name)
    {
        return owner_->GetUiStateMachine()->UnregisterScene(name);
    }

    bool UiSceneService::SwitchToScene(const QString &name)
    {
       return owner_->GetUiStateMachine()->SwitchToScene(name);
    }

    void UiSceneService::RegisterUniversalWidget(const QString &name, QGraphicsProxyWidget *widget)
    {
        return owner_->GetUiStateMachine()->RegisterUniversalWidget(name, widget);
    }

    void UiSceneService::ShowNotification(CoreUi::NotificationBaseWidget *notification_widget)
    {
        owner_->GetNotificationManager()->ShowNotification(notification_widget);
    }

    QWidget *UiSceneService::LoadFromFile(const QString &file_path, bool add_to_scene, QWidget *parent)
    {
        QWidget *widget = 0;
        QUiLoader loader;
        QFile file(file_path); 
        file.open(QFile::ReadOnly);
        widget = loader.load(&file, parent);
        if(add_to_scene && widget)
            AddWidgetToScene(widget);
        return widget;
    }

    void UiSceneService::TranferWidgets()
    {
        CoreUi::UniversalWidgetMap universal_widgets = owner_->GetUiStateMachine()->GetUniversalWidgets();
        foreach(QString widget_name, universal_widgets.keys())
        {
            QGraphicsProxyWidget *widget = universal_widgets[widget_name];
            if (!widget)
                continue;
            emit TransferRequest(widget_name, widget);
        }
    }
//$ BEGIN_MOD $
	bool UiSceneService::AddExternalMenuAction(QAction *action, const QString &name, const QString &menu, const QString &icon){
		return owner_->GetExternalMenuManager()->AddExternalMenuAction(action,name,menu,icon);
	}

	bool UiSceneService::AddExternalToolbar(QToolBar *toolbar, const QString &name){
		return owner_->GetExternalToolBarManager()->AddExternalToolbar(toolbar, name);
    }

	bool UiSceneService::RemoveExternalToolbar(QString name){
		return owner_->GetExternalToolBarManager()->RemoveExternalToolbar(name);
    }

	bool UiSceneService::ShowExternalToolbar(QString name){
		return owner_->GetExternalToolBarManager()->ShowExternalToolbar(name);
    }

	bool UiSceneService::HideExternalToolbar(QString name){
		return owner_->GetExternalToolBarManager()->HideExternalToolbar(name);
    }

	bool UiSceneService::EnableExternalToolbar(QString name){
		return owner_->GetExternalToolBarManager()->EnableExternalToolbar(name);
    }

	bool UiSceneService::DisableExternalToolbar(QString name){
		return owner_->GetExternalToolBarManager()->DisableExternalToolbar(name);
    }

	QToolBar* UiSceneService::GetExternalToolbar(QString name){
		return owner_->GetExternalToolBarManager()->GetExternalToolbar(name);
    }

	//TO MANAGE MENU SETTINGS
	void UiSceneService::CreateSettingsPanel(){
		CoreUi::SettingsWidget *settings_widget_ = dynamic_cast<CoreUi::SettingsWidget *>(owner_->GetInworldSceneController()->GetSettingsObject());
		QWidget *internal_wid = settings_widget_->GetInternalWidget();//
		if (internal_wid)
			AddWidgetToScene(internal_wid, true, true);
	}

	bool UiSceneService::IsMenuInside(QString name){
		if (proxy_dock_list.contains(name)){
			proxyDock pair = proxy_dock_list.value(name);
			QDockWidget* qdock=pair.second;
			UiProxyWidget* proxy=dynamic_cast<UiProxyWidget*>(pair.first);
			if(qdock->widget())
				//Is outside
				return false;
			else
				return true;
		}else
			//Some widget name that doesnt exist
			return false;
	}

	bool UiSceneService::IsMenuMoveable(QString name){
		if (moveable_widgets_->contains(name))
			return true;
		else
			return false;
	}
//$ END_MOD $
}