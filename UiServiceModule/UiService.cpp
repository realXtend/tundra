/**
 *  For conditions of distribution and use, see copyright notice in license.txt
 *
 *  @file   UiService.cpp
 *  @brief  Implements UiServiceInterface and provides means of adding widgets to the 
 */

#include "StableHeaders.h"
#include "UiService.h"
#include "UiProxyWidget.h"

UiService::UiService(QGraphicsView *view) : view_(view), scene_(view->scene())
{
}

UiService::~UiService()
{
}

UiProxyWidget *UiService::AddWidgetToScene(QWidget *widget, Qt::WindowFlags flags)
{
    return 0;
}

void UiService::AddWidgetToScene(UiProxyWidget *widget)
{
}

void UiService::AddWidgetToMenu(QWidget *widget)
{
}

void UiService::AddWidgetToMenu(QWidget *widget, const QString &entry, const QString &menu, const QString &icon)
{
}

void UiService::AddWidgetToMenu(UiProxyWidget *widget, const QString &entry, const QString &menu, const QString &icon)
{
}

void UiService::RemoveWidgetFromScene(QWidget *widget)
{
}

void UiService::RemoveWidgetFromScene(QGraphicsProxyWidget *widget)
{
}

void UiService::ShowWidget(QWidget *widget) const
{
}

void UiService::HideWidget(QWidget *widget) const
{
}

void UiService::BringWidgetToFront(QWidget *widget) const
{
}

void UiService::BringWidgetToFront(QGraphicsProxyWidget *widget) const
{
}

