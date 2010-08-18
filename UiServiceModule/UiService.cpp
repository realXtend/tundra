/**
 *  For conditions of distribution and use, see copyright notice in license.txt
 *
 *  @file   UiService.cpp
 *  @brief  Light-weight UI service. Implements UiServiceInterface and provides 
 *          means of embedding Qt widgets to the same scene/canvas as the 3D in-world
 *          view. Uses only one UI scene for everything.
 */

#include "StableHeaders.h"
#include "DebugOperatorNew.h"

#include "UiService.h"
#include "UiProxyWidget.h"

#include "MemoryLeakCheck.h"

UiService::UiService(QGraphicsView *view) : view_(view), scene_(view->scene())
{
    assert(view_);
    assert(scene_);
}

UiService::~UiService()
{
}

UiProxyWidget *UiService::AddWidgetToScene(QWidget *widget, Qt::WindowFlags flags)
{
        /*  QGraphicsProxyWidget maintains symmetry for the following states:
         *  state, enabled, visible, geometry, layoutDirection, style, palette,
         *  font, cursor, sizeHint, getContentsMargins and windowTitle
         */

        UiProxyWidget *proxy = new UiProxyWidget(widget, flags);
        AddWidgetToScene(proxy);

        // If the widget has WA_DeleteOnClose on, connect its proxy's visibleChanged()
        // signal to a slot which handles the deletion. This must be done because closing
        // proxy window in our system doesn't yield closeEvent, but hideEvent instead.
        if (widget->testAttribute(Qt::WA_DeleteOnClose))
            connect(proxy, SIGNAL(visibleChanged()), SLOT(DeleteCallingWidgetOnClose()));

        return proxy;
}

void UiService::AddWidgetToScene(UiProxyWidget *widget)
{
    if (widgets_.contains(widget))
        return;

    widgets_.append(widget);

    if (widget->isVisible())
        widget->hide();

    // If no position has been set for widget, use default one so that the window's title
    // bar - or any other critical part, doesn't go outside the view.
    if (widget->pos() == QPointF())
        widget->setPos(10.0, 200.0);

    scene_->addItem(widget);
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
    scene_->removeItem(widget->graphicsProxyWidget());
    widgets_.removeOne(widget->graphicsProxyWidget());
}

void UiService::RemoveWidgetFromScene(QGraphicsProxyWidget *widget)
{
    scene_->removeItem(widget);
    widgets_.removeOne(widget);
}

void UiService::ShowWidget(QWidget *widget) const
{
    widget->graphicsProxyWidget()->show();
}

void UiService::HideWidget(QWidget *widget) const
{
    widget->graphicsProxyWidget()->hide();
}

void UiService::BringWidgetToFront(QWidget *widget) const
{
    ShowWidget(widget);
    scene_->setActiveWindow(widget->graphicsProxyWidget());
    scene_->setFocusItem(widget->graphicsProxyWidget(), Qt::ActiveWindowFocusReason);
}

void UiService::BringWidgetToFront(QGraphicsProxyWidget *widget) const
{
    scene_->setActiveWindow(widget);
    scene_->setFocusItem(widget, Qt::ActiveWindowFocusReason);
}

void UiService::DeleteCallingWidgetOnClose()
{
    QGraphicsProxyWidget *proxy = dynamic_cast<QGraphicsProxyWidget *>(sender());
    if (proxy && !proxy->isVisible())
        proxy->deleteLater();
}
