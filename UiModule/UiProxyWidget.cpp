// For conditions of distribution and use, see copyright notice in license.txt

#include "StableHeaders.h"
#include "UiProxyWidget.h"

namespace UiServices
{
    UiProxyWidget::UiProxyWidget(QWidget *widget, const UiWidgetProperties &in_widget_properties)
        : QGraphicsProxyWidget(0, in_widget_properties.getWindowStyle()),
          widget_properties_(in_widget_properties),
          show_timeline_(new QTimeLine(300, this)),
          show_animation_(new QGraphicsItemAnimation(this))
    {
        widget->setWindowFlags(widget_properties_.getWindowStyle());
        widget->setWindowTitle(widget_properties_.getWidgetName());

        setWidget(widget);
        setPos(widget_properties_.getPosition().x(), widget_properties_.getPosition().y());
        setGeometry(QRectF(widget_properties_.getPosition(), QSizeF(widget_properties_.getSize())));

        InitAnimations();
    }

    UiProxyWidget::~UiProxyWidget()
    {
        SAFE_DELETE(show_timeline_);
        SAFE_DELETE(show_animation_);
    }

    void UiProxyWidget::InitAnimations()
    {
        if (!widget_properties_.isFullscreen())
        {
            QGraphicsDropShadowEffect *shadow_effect = new QGraphicsDropShadowEffect(this);
            shadow_effect->setBlurRadius(3);
            shadow_effect->setOffset(3.0, 3.0);
            setGraphicsEffect(shadow_effect);
        }

        if (widget_properties_.getWidgetName() != "Login loader") // fix
            QObject::connect(show_timeline_, SIGNAL( valueChanged(qreal) ), SLOT( AnimationStep(qreal) ));
    }

    void UiProxyWidget::AnimationStep(qreal step)
    {
        setOpacity(step);
    }

    void UiProxyWidget::showEvent(QShowEvent *show_event)
    {
        emit Visible(true);
        QGraphicsProxyWidget::showEvent(show_event);
	    show_timeline_->start();
    }

    void UiProxyWidget::hideEvent(QHideEvent *hide_event)
    {
        emit Visible(false);
        QGraphicsProxyWidget::hideEvent(hide_event);
        if (widget_properties_.getWidgetName() != "Login loader") // fix
            setOpacity(0.0);
    }

    UiWidgetProperties UiProxyWidget::getWidgetProperties()
    {
        return widget_properties_;
    }
}
