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
    UiSceneService::UiSceneService(UiModule *owner) : owner_(owner)
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

    UiProxyWidget *UiSceneService::AddWidgetToScene(QWidget *widget, Qt::WindowFlags flags)
    {
		uiExternal= owner_->GetFramework()->GetService<Foundation::UiExternalServiceInterface>();
		QWidget* qdock= new QDockWidget(widget->windowTitle());
		UiProxyWidget *proxy = new UiProxyWidget(widget, flags);

		if(uiExternal){
			proxy->setWidget(0);
			qdock = uiExternal->AddExternalPanel(widget,widget->windowTitle());
		}else
			proxy= owner_->GetInworldSceneController()->AddWidgetToScene(widget, flags);

		proxy_dock_list[widget->windowTitle()]=proxyDock(proxy,dynamic_cast<QDockWidget*>(qdock));

		return proxy;

    }

    bool UiSceneService::AddWidgetToScene(UiProxyWidget *widget)
    {
        return owner_->GetInworldSceneController()->AddProxyWidget(widget);
    }

    void UiSceneService::AddWidgetToMenu(QWidget *widget)
    {
		QDockWidget* qdock=proxy_dock_list[widget->windowTitle()].second;
		if(qdock->widget())
			uiExternal->AddExternalMenuPanel(qdock,widget->windowTitle(),"Panels");
		else
			owner_->GetInworldSceneController()->AddWidgetToMenu(widget, widget->windowTitle(), "", "");
    }

    void UiSceneService::AddWidgetToMenu(QWidget *widget, const QString &entry, const QString &menu, const QString &icon)
    {
		QDockWidget* qdock=proxy_dock_list[widget->windowTitle()].second;
		if(qdock->widget())
			uiExternal->AddExternalMenuPanel(qdock,widget->windowTitle(),"Panels");
		else
			owner_->GetInworldSceneController()->AddWidgetToMenu(widget, entry, menu, icon);
    }

    void UiSceneService::AddWidgetToMenu(UiProxyWidget *widget, const QString &entry, const QString &menu, const QString &icon)
    {
		if(widget->widget())
			owner_->GetInworldSceneController()->AddWidgetToMenu(widget, entry, menu, icon);
		else{
			QDockWidget* qdock=proxy_dock_list[widget->windowTitle()].second;
			uiExternal->AddExternalMenuPanel(qdock,widget->windowTitle(),"Panels");
		}
    }

    void UiSceneService::RemoveWidgetFromMenu(QWidget *widget)
    {
		QDockWidget* qdock=proxy_dock_list[widget->windowTitle()].second;
		if(qdock->widget())
			uiExternal->RemoveExternalMenuPanel(qdock->widget());
		else
			owner_->GetInworldSceneController()->RemoveWidgetFromMenu(widget->graphicsProxyWidget());
    }

    void UiSceneService::RemoveWidgetFromMenu(QGraphicsProxyWidget *widget)
    {
		if(widget->widget())
			owner_->GetInworldSceneController()->RemoveWidgetFromMenu(widget);
		else{
			QDockWidget* qdock=proxy_dock_list[widget->windowTitle()].second;
			uiExternal->RemoveExternalMenuPanel(qdock->widget());
		}
    }

    void UiSceneService::RemoveWidgetFromScene(QWidget *widget)
    {
		if(proxy_dock_list[widget->windowTitle()].second->widget())
			uiExternal->RemoveExternalPanel(widget->parentWidget());
		else
			owner_->GetInworldSceneController()->RemoveProxyWidgetFromScene(widget);
    }

    void UiSceneService::RemoveWidgetFromScene(QGraphicsProxyWidget *widget)
    {
		if(widget->widget())
			owner_->GetInworldSceneController()->RemoveProxyWidgetFromScene(widget);
		else{
			QDockWidget* qdock=proxy_dock_list[widget->windowTitle()].second;
			uiExternal->RemoveExternalPanel(qdock);
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
			uiExternal->HideWidget(widget->parentWidget());
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
	void UiSceneService::TransferWidgetInOut(QString widgetToChange)
	{
		if (proxy_dock_list.contains(widgetToChange)){
			proxyDock pair = proxy_dock_list.value(widgetToChange);
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
					owner_->GetInworldSceneController()->ShowProxyForWidget(widget);
				}
			}else{
				proxy->hide();
				widget=proxy->widget();
				owner_->GetInworldSceneController()->RemoveProxyWidgetFromScene(proxy);
				owner_->GetInworldSceneController()->RemoveWidgetFromMenu(proxy);
				proxy->setWidget(0);
				qdock->setWidget(widget);
				//Add Panel..
				if (uiExternal->AddExternalPanel(qdock)){
					uiExternal->ShowWidget(widget);
					uiExternal->AddExternalMenuPanel(qdock,widget->windowTitle(),"Panels");
				}
			}
		}
	}

	void UiSceneService::HandleTransferToBuild(const QString& old_name, const QString& new_name)
	{
		if(new_name=="WorldBuilding")
			foreach(proxyDock pair,proxy_dock_list){
				QDockWidget* qdock=pair.second;
				UiProxyWidget* proxy=dynamic_cast<UiProxyWidget*>(pair.first);
				QWidget* widget;
				if(qdock->widget()){
					if(qdock->widget()->windowTitle()=="Entity-component Editor" || qdock->widget()->windowTitle()=="Inventory") {
						qdock->hide();
						widget=qdock->widget();
						uiExternal->RemoveExternalMenuPanel(widget);
						uiExternal->RemoveExternalPanel(widget);
						widget->setParent(0);
						proxy->setWidget(widget);
						if (owner_->GetInworldSceneController()->AddProxyWidget(proxy)){
							owner_->GetInworldSceneController()->AddWidgetToMenu(proxy,proxy->windowTitle(),"Panels","./data/ui/images/menus/edbutton_ENVED_normal");
							owner_->GetInworldSceneController()->BringProxyToFront(proxy);
						}
					}
				}
			}
	}
//$ END_MOD $
}

