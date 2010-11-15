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

namespace UiServices
{
//$ BEGIN_MOD $
    UiSceneService::UiSceneService(UiModule *owner) : owner_(owner), moveable_widgets_(new QList<QString>())
    {
        connect(owner_->GetUiStateMachine(), SIGNAL(SceneChanged(const QString&, const QString&)),
                this, SIGNAL(SceneChanged(const QString&, const QString&)));
        connect(owner_->GetUiStateMachine(), SIGNAL(SceneChangeComplete()),
                this, SLOT(TranferWidgets()));

        connect(owner_->GetNotificationManager(), SIGNAL(ShowNotificationCalled(const QString&)), this, SIGNAL(Notification(const QString&)));

		connect(owner_->GetUiStateMachine(), SIGNAL(SceneChanged(const QString&, const QString&)),
				this,SLOT(HandleTransferToBuild(const QString&, const QString&)));
		//Settings Panel, List of panels send when we enter the world
		connect(owner_->GetUiStateMachine(), SIGNAL(SceneChanged(const QString&, const QString&)),
			this,SLOT(SetPanelsList(const QString&, const QString&)));
//$ END_MOD $
    }

    UiSceneService::~UiSceneService()
    {
    }
//$ BEGIN_MOD $
    UiProxyWidget *UiSceneService::AddWidgetToScene(QWidget *widget, bool moveable , bool outside, Qt::WindowFlags flags)
    {
		bool bydefault=false;
		uiExternal= owner_->GetFramework()->GetService<Foundation::UiExternalServiceInterface>();
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
			QSettings settings("Naali UIExternal", "UiExternal Settings");
			QString pos = settings.value(widget->windowTitle(), QString("vacio")).toString();
			if (pos == "outside" && uiExternal){
				proxy = new UiProxyWidget(widget, flags);
				proxy->setWidget(0);
				qdock = uiExternal->AddExternalPanel(widget,widget->windowTitle());
			} else if(pos == "inside")
				proxy= owner_->GetInworldSceneController()->AddWidgetToScene(widget, flags);
			else 
				bydefault = true;
		}else
			bydefault = true;

		//We do by default behaviour when seetings var doesn't exist or if it is not moveable
		if (bydefault) {
			if(uiExternal && outside){	
				proxy = new UiProxyWidget(widget, flags);
				proxy->setWidget(0);
				qdock = uiExternal->AddExternalPanel(widget,widget->windowTitle()); 
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
		uiExternal= owner_->GetFramework()->GetService<Foundation::UiExternalServiceInterface>();
		QWidget* qdock= new QDockWidget(widget->windowTitle());
		qdock->setObjectName(widget->windowTitle());
		qdock->setWindowTitle(widget->windowTitle());
		widget->setObjectName(widget->windowTitle());
		QWidget* wid = widget->widget();

		QSettings settings("Naali UIExternal", "UiExternal Settings");
		QString pos = settings.value(widget->windowTitle(), QString("vacio")).toString();
		if (pos != "inside" && uiExternal){
			widget->setWidget(0);
			qdock = uiExternal->AddExternalPanel(wid,widget->windowTitle());
		} else
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
		if(qdock->widget() && uiExternal)
			uiExternal->AddExternalMenuPanel(qdock,widget->windowTitle(),"Panels", moveable_widgets_->contains(widget->windowTitle()));
		else
			owner_->GetInworldSceneController()->AddWidgetToMenu(widget, widget->windowTitle(), "", "");
    }

    void UiSceneService::AddWidgetToMenu(QWidget *widget, const QString &entry, const QString &menu, const QString &icon)
    {
		//Save initial values
		if (!panels_menus_list_.contains(widget->windowTitle()))
			panels_menus_list_[widget->windowTitle()]=menusPair(menu, icon);

		QDockWidget* qdock=proxy_dock_list[widget->windowTitle()].second;	
		if(qdock->widget() && uiExternal)
			if (moveable_widgets_->contains(widget->windowTitle()) && uiExternal)
				uiExternal->AddExternalMenuPanel(qdock,entry,menu, true);
			else
				uiExternal->AddExternalMenuPanel(qdock,entry,menu, false); 
		else
			owner_->GetInworldSceneController()->AddWidgetToMenu(widget, entry, menu, icon);
    }

    void UiSceneService::AddWidgetToMenu(UiProxyWidget *widget, const QString &entry, const QString &menu, const QString &icon)
    {
		//Save initial values
		if (!panels_menus_list_.contains(widget->windowTitle()))
			panels_menus_list_[widget->windowTitle()]=menusPair(menu, icon);

		if(widget->widget())
			owner_->GetInworldSceneController()->AddWidgetToMenu(widget, entry, menu, icon);
		else{
			QDockWidget* qdock=proxy_dock_list[widget->windowTitle()].second; 
			uiExternal->AddExternalMenuPanel(qdock,entry,menu, moveable_widgets_->contains(widget->windowTitle()));
		}
    }

    void UiSceneService::RemoveWidgetFromMenu(QWidget *widget)
    {
		uiExternal = owner_->GetFramework()->GetService<Foundation::UiExternalServiceInterface>();
		QDockWidget* qdock=proxy_dock_list[widget->windowTitle()].second;
		if(qdock->widget())
			//To check if available
			if (uiExternal)
				uiExternal->RemoveExternalMenuPanel(qdock->widget());
		else
			owner_->GetInworldSceneController()->RemoveWidgetFromMenu(widget->graphicsProxyWidget());
    }

    void UiSceneService::RemoveWidgetFromMenu(QGraphicsProxyWidget *widget)
    {
		uiExternal = owner_->GetFramework()->GetService<Foundation::UiExternalServiceInterface>();
		if(widget->widget())
			owner_->GetInworldSceneController()->RemoveWidgetFromMenu(widget);
		else{
			//To check if available
			if (uiExternal) {
				QDockWidget* qdock=proxy_dock_list[widget->windowTitle()].second;
				uiExternal->RemoveExternalMenuPanel(qdock->widget());
			}
		}
    }

    void UiSceneService::RemoveWidgetFromScene(QWidget *widget)
    {
		uiExternal = owner_->GetFramework()->GetService<Foundation::UiExternalServiceInterface>();
		if(proxy_dock_list[widget->windowTitle()].second->widget())
			if (uiExternal)
				uiExternal->RemoveExternalPanel(widget->parentWidget());
		else
			owner_->GetInworldSceneController()->RemoveProxyWidgetFromScene(widget);
    }

    void UiSceneService::RemoveWidgetFromScene(QGraphicsProxyWidget *widget)
    {
		uiExternal = owner_->GetFramework()->GetService<Foundation::UiExternalServiceInterface>();
		if(widget->widget())
			owner_->GetInworldSceneController()->RemoveProxyWidgetFromScene(widget);
		else{
			if(widget->widget()){
				QDockWidget* qdock=proxy_dock_list[widget->windowTitle()].second;
				uiExternal->RemoveExternalPanel(qdock);
			}
		}
    }

    void UiSceneService::ShowWidget(QWidget *widget) const
    {
		if(proxy_dock_list[widget->windowTitle()].second->widget())
			uiExternal->ShowWidget(widget);
		else
			owner_->GetInworldSceneController()->ShowProxyForWidget(widget);
    }

    void UiSceneService::HideWidget(QWidget *widget) const
    {
		if(proxy_dock_list[widget->windowTitle()].second->widget())
			uiExternal->HideWidget(widget);
		else
			owner_->GetInworldSceneController()->HideProxyForWidget(widget);
    }

    void UiSceneService::BringWidgetToFront(QWidget *widget) const
    {
		if(proxy_dock_list[widget->windowTitle()].second->widget())
			uiExternal->ShowWidget(widget);
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
			uiExternal->ShowWidget(proxy_dock_list[widget->windowTitle()].second->widget());

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
		uiExternal = owner_->GetFramework()->GetService<Foundation::UiExternalServiceInterface>();
		if (uiExternal)
			return uiExternal->AddExternalMenuAction(action,name,menu,icon);
		else
			return false;
	}

	void UiSceneService::TransferWidgetOut(QString widgetToChange,bool out)
	{
		if (proxy_dock_list.contains(widgetToChange) && uiExternal){
			//Get info from the list
			proxyDock pair = proxy_dock_list.value(widgetToChange);
			QDockWidget* qdock=pair.second;
			UiProxyWidget* proxy=dynamic_cast<UiProxyWidget*>(pair.first);
			QWidget* widget;
			if(!out){
				//We have to put it inside
				if(qdock->widget()){
					//It is outside
					qdock->hide();
					widget=qdock->widget();
					widget->adjustSize();
					//Remove Panel and Menu from External
					uiExternal->RemoveExternalPanel(widget);
					uiExternal->RemoveExternalMenuPanel(widget);
					//Put Panel inside Proxy, add to scene and add internal menu entry with icon if available
					widget->setParent(0);
					proxy->setWidget(widget);
					if (owner_->GetInworldSceneController()->AddProxyWidget(proxy)){
						owner_->GetInworldSceneController()->BringProxyToFront(widget);
						if (panels_menus_list_.contains(widget->windowTitle())){
							menusPair par = panels_menus_list_.value(widget->windowTitle());
							owner_->GetInworldSceneController()->AddWidgetToMenu(proxy, proxy->windowTitle(), par.first, par.second);
						}
						else
							owner_->GetInworldSceneController()->AddWidgetToMenu(proxy,proxy->windowTitle(),"Panels","./data/ui/images/menus/edbutton_ENVED_normal");
					}
				}
				else {
					//If is inside, just show it
					owner_->GetInworldSceneController()->BringProxyToFront(proxy);
				}
			}else{
				//We have to put it outside
				if(!qdock->widget()){
					//If is inside..
					proxy->hide();
					widget=proxy->widget();
					//Remove panel and menu entry
					owner_->GetInworldSceneController()->RemoveProxyWidgetFromScene(proxy);
					owner_->GetInworldSceneController()->RemoveWidgetFromMenu(proxy);
					proxy->setWidget(0);
					qdock->setWidget(widget);
					//Put external panel and menu entry
					if (uiExternal->AddExternalPanel(qdock)){
						uiExternal->ShowWidget(widget);
						if (panels_menus_list_.contains(widget->windowTitle())){
							menusPair par = panels_menus_list_.value(widget->windowTitle());
							uiExternal->AddExternalMenuPanel(qdock,widget->windowTitle(),par.first);
						} else
							uiExternal->AddExternalMenuPanel(qdock,widget->windowTitle(),"panels");
					}
				}
				else {
					//Is outside, just show it
					uiExternal->ShowWidget(qdock->widget());
				}
			}
		}
	}

	void UiSceneService::AddPanelToEditMode(QWidget* widget){
		uiExternal= owner_->GetFramework()->GetService<Foundation::UiExternalServiceInterface>();
		if(uiExternal)
			uiExternal->AddPanelToEditMode(widget);
	}

	void UiSceneService::HandleTransferToBuild(const QString& old_name, const QString& new_name)
	{
		uiExternal= owner_->GetFramework()->GetService<Foundation::UiExternalServiceInterface>();
		if(uiExternal){
			QList<QString> panels;
			panels << "Entity-component Editor";
			panels << "Inventory";
			panels << "Environment Editor";

			if(new_name=="WorldBuilding"){
				foreach(QString s, panels){
					//Get info from the list
					proxyDock pair = proxy_dock_list.value(s);
					QDockWidget* qdock=pair.second;
					UiProxyWidget* proxy=dynamic_cast<UiProxyWidget*>(pair.first);
					QWidget* widget;

					//We have to put it inside
					if(qdock->widget()){
						//It is outside
						qdock->hide();
						widget=qdock->widget();
						widget->adjustSize();
						//Remove Panel and Menu from External
						uiExternal->RemoveExternalPanel(widget);
						uiExternal->RemoveExternalMenuPanel(widget);
						//Put Panel inside Proxy, add to scene and add internal menu entry with icon if available
						widget->setParent(0);
						proxy->setWidget(widget);
						if (owner_->GetInworldSceneController()->AddProxyWidget(proxy)){
							if (panels_menus_list_.contains(widget->windowTitle())){
								menusPair par = panels_menus_list_.value(widget->windowTitle());
								owner_->GetInworldSceneController()->AddWidgetToMenu(proxy, proxy->windowTitle(), par.first, par.second);
							}
							else
								owner_->GetInworldSceneController()->AddWidgetToMenu(proxy,proxy->windowTitle(),"Panels","./data/ui/images/menus/edbutton_ENVED_normal");
						}
					}
				}
			}else if(new_name=="Inworld" && old_name=="WorldBuilding")
			{
				//begin_mod
				foreach(QString s, panels){
					QSettings settings("Naali UIExternal", "UiExternal Settings");
					QString pos = settings.value(s, QString("vacio")).toString();
					if (pos == "outside" && uiExternal){
						proxyDock pair = proxy_dock_list.value(s);
						QDockWidget* qdock=pair.second;
						UiProxyWidget* proxy=dynamic_cast<UiProxyWidget*>(pair.first);
						QWidget* widget;

						proxy->hide();
						widget=proxy->widget();
						//Remove panel and menu entry
						owner_->GetInworldSceneController()->RemoveProxyWidgetFromScene(proxy);
						owner_->GetInworldSceneController()->RemoveWidgetFromMenu(proxy);
						proxy->setWidget(0);
						qdock->setWidget(widget);
						//Put external panel and menu entry
						if (uiExternal->AddExternalPanel(qdock)){
							if (panels_menus_list_.contains(widget->windowTitle())){
								menusPair par = panels_menus_list_.value(widget->windowTitle());
								uiExternal->AddExternalMenuPanel(qdock,widget->windowTitle(),par.first);
							} else
								uiExternal->AddExternalMenuPanel(qdock,widget->windowTitle(),"panels");
						}
					} //else{
						//TransferWidgetOut(s,false);
					//}
					/*Hide panels
					proxyDock pair = proxy_dock_list.value(s);
					QDockWidget* qdock=pair.second;
					UiProxyWidget* proxy=dynamic_cast<UiProxyWidget*>(pair.first);
					if(qdock->widget())
						HideWidget(qdock->widget());
					else
						HideWidget(proxy->widget());*/
				//end_mod
				}
			}
		}
	}

	bool UiSceneService::AddExternalToolbar(QToolBar *toolbar, const QString &name){
		uiExternal= owner_->GetFramework()->GetService<Foundation::UiExternalServiceInterface>();
		if (uiExternal)
			return uiExternal->AddExternalToolbar(toolbar, name);
		else
			return false;
    }

	bool UiSceneService::RemoveExternalToolbar(QString name){
		uiExternal = owner_->GetFramework()->GetService<Foundation::UiExternalServiceInterface>();
		if (uiExternal)
			return uiExternal->RemoveExternalToolbar(name);
		else
			return false;
    }

	bool UiSceneService::ShowExternalToolbar(QString name){
		if (uiExternal)
			return uiExternal->ShowExternalToolbar(name);
		else
			return false;
    }

	bool UiSceneService::HideExternalToolbar(QString name){
		if (uiExternal)
			return uiExternal->HideExternalToolbar(name);
		else
			return false;
    }

	bool UiSceneService::EnableExternalToolbar(QString name){
		if (uiExternal)
			return uiExternal->EnableExternalToolbar(name);
		else
			return false;
    }

	bool UiSceneService::DisableExternalToolbar(QString name){
		if (uiExternal)
			return uiExternal->DisableExternalToolbar(name);
		else
			return false;
    }

	QToolBar* UiSceneService::GetExternalToolbar(QString name){
		uiExternal= owner_->GetFramework()->GetService<Foundation::UiExternalServiceInterface>();
		if (uiExternal)
			return uiExternal->GetExternalToolbar(name);
		else
			return false;
    }

	//TO MANAGE MENU SETTINGS
	void UiSceneService::CreateSettingsPanel(){
		//Only if uiexternal available
		if (!uiExternal)
			return;
		// Initialize post-process dialog.
		settings_panel_ = new MenuSettingsWidget(this);
		settings_panel_->setWindowTitle("Menu Config");
        // Add to scene.
        AddWidgetToScene(settings_panel_, true, true);
		AddWidgetToMenu(settings_panel_, QObject::tr("Menu Config"), QObject::tr("Settings"),  "./data/ui/images/menus/edbutton_POSTPR_normal.png");	
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

	void UiSceneService::SetPanelsList(const QString& old_name, const QString& new_name) {
		if (settings_panel_ && old_name == "Ether")
			settings_panel_->SetPanelsList(panels_menus_list_.keys());
	}

//$ END_MOD $
}