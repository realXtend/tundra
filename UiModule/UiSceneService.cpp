/**
 *  For conditions of distribution and use, see copyright notice in license.txt
 *
 *  @file   UiSceneService.cpp
 *  @brief  
 */

#include "StableHeaders.h"
#include "UiSceneService.h"

namespace UiServices
{
    UiSceneService::UiSceneService()
    {
    }

    UiSceneService::~UiSceneService()
    {
    }

    QGraphicsProxyWidget *UiSceneService::AddWidgetToScene(QWidget *widget)
    {
        return &QGraphicsProxyWidget();
    }

    void UiSceneService::AddWidgetToScene(QGraphicsProxyWidget *widget)
    {
    }

    void UiSceneService::RemoveWidgetFromScene(QWidget *widget)
    {
    }

    void UiSceneService::RemoveWidgetFromScene(QGraphicsProxyWidget *widget)
    {
    }

    void UiSceneService::ShowWidget(QWidget *widget) const
    {
    }

    void UiSceneService::ShowWidget(QGraphicsProxyWidget *widget) const
    {
    }

    void UiSceneService::HideWidget(QWidget *widget) const
    {
    }

    void UiSceneService::HideWidget(QGraphicsProxyWidget *widget) const
    {
    }

    void UiSceneService::BringWidgetToFront(QWidget *widget) const
    {
    }

    void UiSceneService::BringWidgetToFront(QGraphicsProxyWidget *widget) const
    {
    }

    QGraphicsScene *UiSceneService::GetScene(const QString &name) const
    {
        return &QGraphicsScene();
    }

    void UiSceneService::RegisterScene(const QString &name, QGraphicsScene *scene)
    {
    }

    bool UiSceneService::DeleteScene(const QString &name)
    {
        return false;
    }

    bool UiSceneService::SwitchToScene(const QString &name)
    {
        return false;
    }
}

