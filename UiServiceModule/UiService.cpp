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

#include <QUiLoader>

UiService::UiService(QGraphicsView *view) : view_(view), scene_(view->scene())
{
    assert(view_);
    assert(scene_);

    connect(scene_, SIGNAL(sceneRectChanged(const QRectF &)), SLOT(SceneRectChanged(const QRectF &)));
}

UiService::~UiService()
{
}

UiProxyWidget *UiService::AddWidgetToScene(QWidget *widget, Qt::WindowFlags flags)
{
    if (!widget)
        return 0;

    /*  QGraphicsProxyWidget maintains symmetry for the following states:
     *  state, enabled, visible, geometry, layoutDirection, style, palette,
     *  font, cursor, sizeHint, getContentsMargins and windowTitle
     */

    UiProxyWidget *proxy = new UiProxyWidget(widget, flags);
    assert(proxy->widget());

    // Synchorize windowState flags
    proxy->widget()->setWindowState(widget->windowState());

    AddWidgetToScene(proxy);

    // If the widget has WA_DeleteOnClose on, connect its proxy's visibleChanged()
    // signal to a slot which handles the deletion. This must be done because closing
    // proxy window in our system doesn't yield closeEvent, but hideEvent instead.
    if (widget->testAttribute(Qt::WA_DeleteOnClose))
        connect(proxy, SIGNAL(visibleChanged()), SLOT(DeleteCallingWidgetOnClose()));

    return proxy;
}

bool UiService::AddWidgetToScene(UiProxyWidget *widget)
{
    if (!widget)
        return false;

    if (widgets_.contains(widget))
        return false;

    widgets_.append(widget);

    if (widget->isVisible())
        widget->hide();

    // If no position has been set for Qt::Dialog widget, use default one so that the window's title
    // bar - or any other critical part, doesn't go outside the view.
    if ((widget->windowFlags() & Qt::Dialog) && widget->pos() == QPointF())
        widget->setPos(10.0, 200.0);

    // Resize full screen widgets to fit the scene rect.
    if (widget->widget()->windowState() & Qt::WindowFullScreen)
    {
        fullScreenWidgets_ << widget;
        widget->setGeometry(scene_->sceneRect().toRect());
    }

    scene_->addItem(widget);
    return true;
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
    if (!widget)
        return;

    if (scene_)
        scene_->removeItem(widget->graphicsProxyWidget());
    widgets_.removeOne(widget->graphicsProxyWidget());
    fullScreenWidgets_.removeOne(widget->graphicsProxyWidget());
}

void UiService::RemoveWidgetFromScene(QGraphicsProxyWidget *widget)
{
    if (!widget)
        return;

    if (scene_)
        scene_->removeItem(widget);
    widgets_.removeOne(widget);
    fullScreenWidgets_.removeOne(widget);
}

QWidget *UiService::LoadFromFile(const QString &file_path, bool add_to_scene, QWidget *parent)
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
    
void UiService::RemoveWidgetFromMenu(QWidget *widget)
{
}

void UiService::RemoveWidgetFromMenu(QGraphicsProxyWidget *widget)
{
}

void UiService::ShowWidget(QWidget *widget) const
{
    if (!widget)
        return;

    if (widget->graphicsProxyWidget())
        widget->graphicsProxyWidget()->show();
    else
        widget->show();
}

void UiService::HideWidget(QWidget *widget) const
{
    if (!widget)
        return;

    if (widget->graphicsProxyWidget())
        widget->graphicsProxyWidget()->hide();
    else
        widget->hide();
}

void UiService::BringWidgetToFront(QWidget *widget) const
{
    if (!widget)
        return;

    ShowWidget(widget);
    scene_->setActiveWindow(widget->graphicsProxyWidget());
    scene_->setFocusItem(widget->graphicsProxyWidget(), Qt::ActiveWindowFocusReason);
}

void UiService::BringWidgetToFront(QGraphicsProxyWidget *widget) const
{
    if (!widget)
        return;

    scene_->setActiveWindow(widget);
    scene_->setFocusItem(widget, Qt::ActiveWindowFocusReason);
}

void UiService::SceneRectChanged(const QRectF &rect)
{
    foreach(QGraphicsProxyWidget *widget, fullScreenWidgets_)
        widget->setGeometry(rect);
}

void UiService::DeleteCallingWidgetOnClose()
{
    QGraphicsProxyWidget *proxy = dynamic_cast<QGraphicsProxyWidget *>(sender());
    if (proxy && !proxy->isVisible())
        proxy->deleteLater();
}
