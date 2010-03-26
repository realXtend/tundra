// For conditions of distribution and use, see copyright notice in license.txt

#include "StableHeaders.h"
#include "BackdropWidget.h"

namespace CoreUi
{
    BackdropWidget::BackdropWidget() :
        QGraphicsProxyWidget(0, Qt::Widget),
        internal_widget_(new QWidget())
    {
        setupUi(internal_widget_);
        setWidget(internal_widget_);
    }

    void BackdropWidget::SetContentWidth(qreal width)
    {
        centerWidget->setMinimumWidth(width);
    }

    void BackdropWidget::SetWidgetWidth(qreal width)
    {
        setMinimumWidth(width);
    }

    qreal BackdropWidget::GetContentWidth()
    {
        return centerWidget->width();
    }

    qreal BackdropWidget::GetContentHeight()
    {
        return centerWidget->height();
    }

    qreal BackdropWidget::GetWidgetWidth()
    {
        return size().width();
    }
}