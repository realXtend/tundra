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
//$ BEGIN_MOD $
		connect(owner_->GetUiStateMachine(), SIGNAL(SceneChanged(const QString&, const QString&)),
			this,SLOT(HandleTransferToBuild(const QString&, const QString&)));
//$ END_MOD $
    }

    UiSceneService::~UiSceneService()
    {
    }

    UiProxyWidget *UiSceneService::AddWidgetToScene(QWidget *widget, bool moveable , bool outside, Qt::WindowFlags flags)
    {

		uiExternal= owner_->GetFramework()->GetService<Foundation::UiExternalServiceInterface>();
		QWidget* qdock= new QDockWidget(widget->windowTitle());
		UiProxyWidget *proxy;
		//If is outside and external service available, put it in a dockwidget
		if(uiExternal && outside){	
			proxy = new UiProxyWidget(widget, flags);
			proxy->setWidget(0);
			qdock = uiExternal->AddExternalPanel(widget,widget->windowTitle());
		}else{
			//proxy->setWidget(0);
			proxy= owner_->GetInworldSceneController()->AddWidgetToScene(widget, flags);
		}
		//Save the pair
		proxy_dock_list[widget->windowTitle()]=proxyDock(proxy,dynamic_cast<QDockWidget*>(qdock));
		//If moveable, register it as moveable
		if (moveable)
			moveable_widgets_->append(widget->windowTitle());

		return proxy;

    }

    bool UiSceneService::AddWidgetToScene(UiProxyWidget *widget)
    {
		//Not moveable!!
		uiExternal= owner_->GetFramework()->GetService<Foundation::UiExternalServiceInterface>();
		widget->widget()->setWindowTitle(widget->windowTitle());
		QWidget* qdock= new QDockWidget(widget->windowTitle());
		QWidget* wid = widget->widget();
		if(uiExternal){
			widget->setWidget(0);
			qdock = uiExternal->AddExternalPanel(wid,widget->windowTitle());
		}else
			owner_->GetInworldSceneController()->AddProxyWidget(widget);

		proxy_dock_list[widget->windowTitle()]=proxyDock(widget,dynamic_cast<QDockWidget*>(qdock));

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
		else {
			owner_->GetInworldSceneController()->AddWidgetToMenu(widget, widget->windowTitle(), "", "");
			//If it's inside and is moveable, we put also the menu outside..
			if (moveable_widgets_->contains(widget->windowTitle()) && uiExternal)
				uiExternal->AddExternalMenuPanel(qdock,widget->windowTitle(),"Panels");
		}
    }

    void UiSceneService::AddWidgetToMenu(QWidget *widget, const QString &entry, const QString &menu, const QString &icon)
    {
		//Save initial values
		if (!panels_menus_list_.contains(widget->windowTitle()))
			panels_menus_list_[widget->windowTitle()]=menusPair(menu, icon);

		QDockWidget* qdock=proxy_dock_list[widget->windowTitle()].second;
		if(qdock->widget() && uiExternal)
			if (moveable_widgets_->contains(widget->windowTitle()) && uiExternal)
				uiExternal->AddExternalMenuPanel(qdock,widget->windowTitle(),"Panels", true);
			else
				uiExternal->AddExternalMenuPanel(qdock,widget->windowTitle(),"Panels", false); 
		else
			owner_->GetInworldSceneController()->AddWidgetToMenu(widget, entry, menu, icon);
			//If it's inside and is moveable, we put also the menu outside..
			if (moveable_widgets_->contains(widget->windowTitle()) && uiExternal)
				uiExternal->AddExternalMenuPanel(qdock,widget->windowTitle(),"Panels", true);
    }

    void UiSceneService::AddWidgetToMenu(UiProxyWidget *widget, const QString &entry, const QString &menu, const QString &icon)
    {
		//Save initial values
		if (!panels_menus_list_.contains(widget->windowTitle()))
			panels_menus_list_[widget->windowTitle()]=menusPair(menu, icon);

		if(widget->widget()){
			owner_->GetInworldSceneController()->AddWidgetToMenu(widget, entry, menu, icon);
			//If it's inside and is moveable, we put also the menu outside..
			if (moveable_widgets_->contains(widget->windowTitle()) && uiExternal){
				QDockWidget* qdock=proxy_dock_list[widget->windowTitle()].second;
				uiExternal->AddExternalMenuPanel(qdock,widget->windowTitle(),"Panels", true);
			}
		}
		else{
			QDockWidget* qdock=proxy_dock_list[widget->windowTitle()].second;
			uiExternal->AddExternalMenuPanel(qdock,widget->windowTitle(),"Panels", moveable_widgets_->contains(widget->windowTitle()));
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
			uiExternal->ShowWidget(widget->parentWidget());
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
			uiExternal->ShowWidget(widget->parentWidget());
		else
			owner_->GetInworldSceneController()->BringProxyToFront(widget);
    }

	//Begin
	void UiSceneService::BringWidgetToFront(QString widget)
    {
		//To show the widget, it has to exist
		if (proxy_dock_list.contains(widget)){
			proxyDock pair = proxy_dock_list.value(widget);
			QDockWidget* qdock=pair.second;
			UiProxyWidget* proxy=dynamic_cast<UiProxyWidget*>(pair.first);
			QWidget* widget;
			if(qdock->widget())
				BringWidgetToFront(qdock->widget());
			else
				BringWidgetToFront(proxy->widget());
		}
    }
	//End

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
	bool UiSceneService::TransferWidgetInOut(QString widgetToChange)
	{
		//To change the widget, it has to exist and it has to be registered as moveable
		if (proxy_dock_list.contains(widgetToChange) && moveable_widgets_->contains(widgetToChange)){
			proxyDock pair = proxy_dock_list.value(widgetToChange);
			QDockWidget* qdock=pair.second;
			UiProxyWidget* proxy=dynamic_cast<UiProxyWidget*>(pair.first);
			QWidget* widget;
			if(qdock->widget()){
				qdock->hide();
				widget=qdock->widget();
				widget->adjustSize();
				//We are not going to Remove the menu of the widget..
				//uiExternal->RemoveExternalMenuPanel(widget);
				//If we remove Panel we have to Add it later
				uiExternal->RemoveExternalPanel(widget);
				widget->setParent(0);
				proxy->setWidget(widget);
				if (owner_->GetInworldSceneController()->AddProxyWidget(proxy)){
					//We use initial values.. owner_->GetInworldSceneController()->AddWidgetToMenu(proxy,proxy->windowTitle(),"Panels","./data/ui/images/menus/edbutton_ENVED_normal");
					if (panels_menus_list_.contains(widget->windowTitle())){
						menusPair par = panels_menus_list_.value(widget->windowTitle());
						owner_->GetInworldSceneController()->AddWidgetToMenu(proxy, proxy->windowTitle(), par.first, par.second);
					}
					else
						owner_->GetInworldSceneController()->AddWidgetToMenu(proxy,proxy->windowTitle(),"Panels","./data/ui/images/menus/edbutton_ENVED_normal");
					owner_->GetInworldSceneController()->ShowProxyForWidget(widget);
					return true;
				}
			}else{
				proxy->hide();
				widget=proxy->widget();
				widget->adjustSize();
				owner_->GetInworldSceneController()->RemoveProxyWidgetFromScene(proxy);
				owner_->GetInworldSceneController()->RemoveWidgetFromMenu(proxy);
				proxy->setWidget(0);
				qdock->setWidget(widget);
				//Add Panel..
				if (uiExternal->AddExternalPanel(qdock)){
					uiExternal->ShowWidget(widget);
					uiExternal->AddExternalMenuPanel(qdock,widget->windowTitle(),"Panels");
					return true;
				}
			}			
		}
		return false;
	}

	void UiSceneService::TransferWidgetOut(QString widgetToChange,bool out)
	{
		if (proxy_dock_list.contains(widgetToChange) && uiExternal){
			proxyDock pair = proxy_dock_list.value(widgetToChange);
			QDockWidget* qdock=pair.second;
			UiProxyWidget* proxy=dynamic_cast<UiProxyWidget*>(pair.first);
			QWidget* widget;
			if(!out){
				if(qdock->widget()){
					qdock->hide();
					widget=qdock->widget();
					uiExternal->RemoveExternalPanel(widget);
					widget->setParent(0);
					proxy->setWidget(widget);
					owner_->GetInworldSceneController()->AddProxyWidget(proxy);
					//owner_->GetInworldSceneController()->AddWidgetToMenu(proxy,proxy->windowTitle(),"Panels","./data/ui/images/menus/edbutton_ENVED_normal");
				}
			}else{
				if(!qdock->widget()){
					proxy->hide();
					widget=proxy->widget();
					owner_->GetInworldSceneController()->RemoveProxyWidgetFromScene(proxy);
					owner_->GetInworldSceneController()->RemoveWidgetFromMenu(proxy);
					proxy->setWidget(0);
					qdock->setWidget(widget);
					//Add Panel..
					if (uiExternal->AddExternalPanel(qdock)){
						uiExternal->AddExternalMenuPanel(qdock,widget->windowTitle(),"Panels");
					}
				}
			}
		}
	}

	void UiSceneService::HandleTransferToBuild(const QString& old_name, const QString& new_name)
	{
		if(new_name=="WorldBuilding"){
			QList<QString> panels;
			panels << "Entity-component Editor";
			panels << "Inventory";
			foreach(QString s, panels){
				proxyDock pair = proxy_dock_list.value(s);
				QDockWidget* qdock=pair.second;
				UiProxyWidget* proxy=dynamic_cast<UiProxyWidget*>(pair.first);
				QWidget* widget;
				if(qdock->widget()){
					qdock->hide();
					widget=qdock->widget();
					//We are not going to Remove the menu of the widget..
					//uiExternal->RemoveExternalMenuPanel(widget);
					//If we remove Panel we have to Add it later
					uiExternal->RemoveExternalPanel(widget);
					widget->setParent(0);
					proxy->setWidget(widget);
					if (owner_->GetInworldSceneController()->AddProxyWidget(proxy)){
						owner_->GetInworldSceneController()->AddWidgetToMenu(proxy,proxy->windowTitle(),"Panels","./data/ui/images/menus/edbutton_ENVED_normal");
						//owner_->GetInworldSceneController()->ShowProxyForWidget(widget);
					}
				}
			}
		}
	}
//$ END_MOD $
}