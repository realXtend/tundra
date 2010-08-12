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
#include "Inworld/View/UiProxyWidget.h"
#include "Inworld/Menus/MenuManager.h"

namespace UiServices
{
    UiSceneService::UiSceneService(UiModule *owner) : owner_(owner)
    {
    }

    UiSceneService::~UiSceneService()
    {
    }

    QGraphicsProxyWidget *UiSceneService::AddWidgetToScene(QWidget *widget)
    {
        return owner_->GetInworldSceneController()->AddWidgetToScene(widget);
    }

    void UiSceneService::AddWidgetToScene(QGraphicsProxyWidget *widget)
    {
        owner_->GetInworldSceneController()->AddProxyWidget(widget);
    }

    QGraphicsProxyWidget *UiSceneService::AddWidgetToScene(QWidget *widget, const UiWidgetProperties &properties)
    {
        return dynamic_cast<QGraphicsProxyWidget *>(owner_->GetInworldSceneController()->AddWidgetToScene(widget, properties));
    }

    QGraphicsProxyWidget *UiSceneService::AddWidgetToScene(QWidget *widget, UiServices::WidgetType type)
    {
        // Create widget properties
        UiWidgetProperties props(widget->windowTitle(), type);
        props.SetIcon("./data/ui/images/menus/edbutton_MATWIZ_normal.png");

        // Create proxy and add it to the scene.
        UiProxyWidget *proxy = new UiProxyWidget(widget, props);
        owner_->GetInworldSceneController()->AddWidgetToScene(proxy);
        return dynamic_cast<QGraphicsProxyWidget *>(proxy);
    }

    void UiSceneService::AddWidgetToMenu(QWidget *widget, const UiWidgetProperties &properties)
    {
        owner_->GetInworldSceneController()->menu_manager_->AddMenuItem(widget->graphicsProxyWidget(), properties);
    }

    void UiSceneService::AddWidgetToMenu(QWidget *widget, const QString &entry, const QString &menu)
    {
        owner_->GetInworldSceneController()->menu_manager_->AddMenuItem(entry, menu, widget->graphicsProxyWidget());
    }

    void UiSceneService::AddWidgetToMenu(QGraphicsProxyWidget *widget, const QString &entry, const QString &menu)
    {
        owner_->GetInworldSceneController()->menu_manager_->AddMenuItem(entry, menu, widget);
//        owner_->GetInworldSceneController()->GetControlPanelManager()->GetPersonalWidget()->SetInventoryWidget(uiproxy);
//        owner_->GetInworldSceneController->GetPersonalWidget()->SetAvatarWidget(uiproxy);
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

    const QGraphicsScene *UiSceneService::GetScene(const QString &name) const
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
}

