/**
 *  For conditions of distribution and use, see copyright notice in license.txt
 *
 *  @file   UiSceneService.cpp
 *  @brief  
 */

#include "StableHeaders.h"
#include "UiSceneService.h"
#include "UiModule.h"
#include "Inworld/InworldSceneController.h"
#include "UiStateMachine.h"

namespace UiServices
{
    UiSceneService::UiSceneService(UiModule *owner) :
        owner_(owner)
    {
    }

    UiSceneService::~UiSceneService()
    {
    }

    QGraphicsProxyWidget *UiSceneService::AddWidgetToScene(QWidget *widget)
    {
        //return owner_->GetInworldSceneController()->AddWidgetToScene(widget);
        return 0;
    }

    void UiSceneService::AddWidgetToScene(QGraphicsProxyWidget *widget)
    {
        //owner_->GetInworldSceneController()->AddProxyWidget(widget);
    }

    void UiSceneService::RemoveWidgetFromScene(QWidget *widget)
    {
        owner_->GetInworldSceneController()->RemoveProxyWidgetFromScene(widget);
    }

    void UiSceneService::RemoveWidgetFromScene(QGraphicsProxyWidget *widget)
    {
        //owner_->GetInworldSceneController()->RemoveProxyWidgetFromScene(widget);
    }

    void UiSceneService::ShowWidget(QWidget *widget) const
    {
        owner_->GetInworldSceneController()->ShowProxyForWidget(widget);
    }

    void UiSceneService::ShowWidget(QGraphicsProxyWidget *widget) const
    {
        //owner_->GetInworldSceneController()->ShowProxyForWidget(widget);
    }

    void UiSceneService::HideWidget(QWidget *widget) const
    {
        owner_->GetInworldSceneController()->HideProxyForWidget(widget);
    }

    void UiSceneService::HideWidget(QGraphicsProxyWidget *widget) const
    {
        //owner_->GetInworldSceneController()->HideProxyForWidget(widget);
    }

    void UiSceneService::BringWidgetToFront(QWidget *widget) const
    {
        owner_->GetInworldSceneController()->BringProxyToFront(widget);
    }

    void UiSceneService::BringWidgetToFront(QGraphicsProxyWidget *widget) const
    {
        //owner_->GetInworldSceneController()->BringProxyToFront(widget);
    }

    const QGraphicsScene *UiSceneService::GetScene(const QString &name) const
    {
        owner_->GetUiStateMachine()->GetScene(name);
        return 0;
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

