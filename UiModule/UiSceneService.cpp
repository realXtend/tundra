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
    UiSceneService::UiSceneService(UiModule *owner) : owner_(owner)
    {
        connect(owner_->GetUiStateMachine(), SIGNAL(SceneChanged(const QString&, const QString&)),
                this, SIGNAL(SceneChanged(const QString&, const QString&)));
        connect(owner_->GetUiStateMachine(), SIGNAL(SceneChangeComplete()),
                this, SLOT(TranferWidgets()));

        connect(owner_->GetNotificationManager(), SIGNAL(ShowNotificationCalled(const QString&)), this, SIGNAL(Notification(const QString&)));
    }

    UiSceneService::~UiSceneService()
    {
    }

    UiProxyWidget *UiSceneService::AddWidgetToScene(QWidget *widget, Qt::WindowFlags flags)
    {
        return owner_->GetInworldSceneController()->AddWidgetToScene(widget, flags);
    }

    bool UiSceneService::AddWidgetToScene(UiProxyWidget *widget)
    {
        return owner_->GetInworldSceneController()->AddProxyWidget(widget);
    }

    void UiSceneService::AddWidgetToMenu(QWidget *widget)
    {
        owner_->GetInworldSceneController()->AddWidgetToMenu(widget, widget->windowTitle(), "", "");
    }

    void UiSceneService::AddWidgetToMenu(QWidget *widget, const QString &entry, const QString &menu, const QString &icon)
    {
        owner_->GetInworldSceneController()->AddWidgetToMenu(widget, entry, menu, icon);
    }

    void UiSceneService::AddWidgetToMenu(UiProxyWidget *widget, const QString &entry, const QString &menu, const QString &icon)
    {
        owner_->GetInworldSceneController()->AddWidgetToMenu(widget, entry, menu, icon);
    }

    void UiSceneService::RemoveWidgetFromMenu(QWidget *widget)
    {
        owner_->GetInworldSceneController()->RemoveWidgetFromMenu(widget->graphicsProxyWidget());
    }

    void UiSceneService::RemoveWidgetFromMenu(QGraphicsProxyWidget *widget)
    {
        owner_->GetInworldSceneController()->RemoveWidgetFromMenu(widget);
    }

    void UiSceneService::RemoveWidgetFromScene(QWidget *widget)
    {
        owner_->GetInworldSceneController()->RemoveProxyWidgetFromScene(widget);
    }

    void UiSceneService::RemoveWidgetFromScene(QGraphicsProxyWidget *widget)
    {
        owner_->GetInworldSceneController()->RemoveProxyWidgetFromScene(widget);
    }

    void UiSceneService::ShowWidget(QWidget *widget) const
    {
        owner_->GetInworldSceneController()->ShowProxyForWidget(widget);
    }

    void UiSceneService::HideWidget(QWidget *widget) const
    {
        owner_->GetInworldSceneController()->HideProxyForWidget(widget);
    }

    void UiSceneService::BringWidgetToFront(QWidget *widget) const
    {
        owner_->GetInworldSceneController()->BringProxyToFront(widget);
    }

    void UiSceneService::BringWidgetToFront(QGraphicsProxyWidget *widget) const
    {
        owner_->GetInworldSceneController()->BringProxyToFront(widget);
    }

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
}

