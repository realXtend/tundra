// For conditions of distribution and use, see copyright notice in license.txt

#include "StableHeaders.h"
#include "UiProxyWidget.h"

namespace UiServices
{
    UiProxyWidget::UiProxyWidget(QWidget *widget, const UiWidgetProperties &in_widget_properties)
        : QGraphicsProxyWidget(0, in_widget_properties.getWindowStyle()),
          widget_properties_(in_widget_properties),
          show_timeline_(new QTimeLine(1300, this)),
          show_animation_(new QGraphicsItemAnimation(this))
    {
        widget->setWindowFlags(widget_properties_.getWindowStyle());
        widget->setWindowTitle(widget_properties_.getWidgetName());

        setWidget(widget);
        setPos(widget_properties_.getPosition().x(), widget_properties_.getPosition().y());
        setGeometry(QRectF(widget_properties_.getPosition(), QSizeF(widget_properties_.getSize())));
       
        //InitAnimations();
    }

    UiProxyWidget::~UiProxyWidget()
    {
        SAFE_DELETE(show_timeline_);
        SAFE_DELETE(show_animation_);
    }

    void UiProxyWidget::InitAnimations()
    {
        setTransform(QTransform().scale(0,0));
        show_timeline_->setFrameRange(0, 100);
        show_timeline_->setUpdateInterval(40);
        show_timeline_->setCurveShape(QTimeLine::EaseOutCurve);
        QObject::connect(show_timeline_, SIGNAL( frameChanged(int) ), SLOT( AnimationStep(int) ));
    }

    void UiProxyWidget::AnimationStep(int step)
    {
        QTransform transformation;
        if (widget_properties_.getAnimationType() == UiServices::SlideFromTop)
            transformation = QTransform().translate(0.0, size().width()*(-0.01*(100-step)));
        else if (widget_properties_.getAnimationType() == UiServices::SlideFromBottom)
            transformation = QTransform().translate(0.0, size().width()*(0.01*(100-step)));
        setTransform(transformation);
    }

    void UiProxyWidget::showEvent(QShowEvent *show_event)
    {
        emit Visible(true);
        QGraphicsProxyWidget::showEvent(show_event);
	    //show_timeline_->start();
    }  

    void UiProxyWidget::hideEvent(QHideEvent *hide_event)
    {
        emit Visible(false);
        QGraphicsProxyWidget::hideEvent(hide_event);
    }

    UiWidgetProperties UiProxyWidget::getWidgetProperties()
    {
        return widget_properties_;
    }

}
