/**
    For conditions of distribution and use, see copyright notice in LICENSE

    @file   UiProxyWidget.cpp
    @brief  Represents a UI widget created by embedding QWidget to the same canvas as the in-world 3D scene. */

#include "StableHeaders.h"
#include "DebugOperatorNew.h"

#include "UiProxyWidget.h"

#include <QWidget>
#include <QApplication>

#include "MemoryLeakCheck.h"

UiProxyWidget::UiProxyWidget(QWidget *widget, Qt::WindowFlags flags):
    QGraphicsProxyWidget(0, flags)
{
    QString name = "UiProxyWidget";
    if (widget && !widget->objectName().isEmpty())
        name.append(":" + widget->objectName());
    else if (widget && widget->windowTitle().isEmpty())
        name.append(":" + widget->windowTitle());
    setObjectName(name);

    // Embed widget to this proxy widget.
    setWidget(widget);
    if (widget)
        widget->installEventFilter(this);
}

UiProxyWidget::~UiProxyWidget()
{
}

void UiProxyWidget::BringToFront()
{
    emit BringProxyToFrontRequest(this);
    if (!isVisible())
        show();
}

bool UiProxyWidget::eventFilter(QObject *obj, QEvent *e)
{
    if (obj == widget() && e->type() == QEvent::LanguageChange)
    {
        // Temporarily disable event filter so that we don't get infinite loop.
        obj->removeEventFilter(this);
        QApplication::sendEvent(obj, e);
        obj->installEventFilter(this);
        QString newTitle = widget()->windowTitle();
        if (newTitle.length())
            setWindowTitle(newTitle);
        return true;
    }

    return QGraphicsProxyWidget::eventFilter(obj, e);
}

void UiProxyWidget::showEvent(QShowEvent *e)
{
    QGraphicsProxyWidget::showEvent(e);
    emit Visible(true);
    emit BringProxyToFrontRequest(this);
}

void UiProxyWidget::hideEvent(QHideEvent *e)
{
    QGraphicsProxyWidget::hideEvent(e);
    emit Visible(false);
}

void UiProxyWidget::closeEvent(QCloseEvent *e)
{
    QGraphicsProxyWidget::closeEvent(e);
    emit Closed();
}
