// For conditions of distribution and use, see copyright notice in license.txt

#include "StableHeaders.h"
#include "NotificationBaseWidget.h"

#include <QTimer>
#include <QPropertyAnimation>
#include <QGraphicsDropShadowEffect>

namespace CoreUi
{
    NotificationBaseWidget::NotificationBaseWidget(int hide_in_msec) :
        QGraphicsProxyWidget(0, Qt::Widget),
        internal_widget_(new QWidget()),
        fade_animation_(new QPropertyAnimation(this)),
        progress_animation_(new QPropertyAnimation(this)),
        move_animation_(new QPropertyAnimation(this)),
        timestamp_(QDateTime::currentDateTime()),
        hide_in_msec_(hide_in_msec)
    {
        InitSelf();
    }

    // Private

    void NotificationBaseWidget::InitSelf()
    {
        // Ui
        setupUi(internal_widget_);
        setWidget(internal_widget_);

        // Animations
        fade_animation_->setTargetObject(this);
        fade_animation_->setPropertyName("opacity");
        fade_animation_->setDuration(200);
        fade_animation_->setEasingCurve(QEasingCurve::InOutSine);
        fade_animation_->setStartValue(1);
        fade_animation_->setEndValue(0);

        move_animation_->setTargetObject(this);
        move_animation_->setPropertyName("pos");
        move_animation_->setDuration(200);
        move_animation_->setEasingCurve(QEasingCurve::InOutSine);

        progress_animation_->setTargetObject(timeoutProgressBar);
        progress_animation_->setPropertyName("value");
        progress_animation_->setDuration(hide_in_msec_);
        progress_animation_->setEasingCurve(QEasingCurve::Linear);
        progress_animation_->setStartValue(100);
        progress_animation_->setEndValue(0);

        // Shadow effect
        QGraphicsDropShadowEffect *shadow_effect = new QGraphicsDropShadowEffect(this);
        shadow_effect->setOffset(-3,3);
        shadow_effect->setBlurRadius(10);
        setGraphicsEffect(shadow_effect);

        // Connect signals
        connect(closePushButton, SIGNAL(clicked()), SLOT(TimedOut()));        
        connect(progress_animation_, SIGNAL(finished()), SLOT(TimedOut()));
        connect(fade_animation_, SIGNAL(finished()), SLOT(WidgetHidden()));
    }

    void NotificationBaseWidget::TimedOut()
    {
        fade_animation_->start(QAbstractAnimation::DeleteWhenStopped);
    }

    void NotificationBaseWidget::WidgetHidden()
    {
        emit Completed(this);
    }

    // Protected

    void NotificationBaseWidget::SetCentralWidget(QWidget *widget)
    {
        mainLayout->insertWidget(0, widget);
    }

    void NotificationBaseWidget::hoverEnterEvent(QGraphicsSceneHoverEvent *hover_enter_event)
    {
        QGraphicsProxyWidget::hoverEnterEvent(hover_enter_event);
        progress_animation_->pause();
    }

    void NotificationBaseWidget::hoverLeaveEvent(QGraphicsSceneHoverEvent *hover_leave_event)
    {
        QGraphicsProxyWidget::hoverLeaveEvent(hover_leave_event);
        progress_animation_->resume();
    }

    // Public

    void NotificationBaseWidget::Start()
    {
        progress_animation_->start(QAbstractAnimation::DeleteWhenStopped);
    }

    void NotificationBaseWidget::AnimateToPosition(QPointF end_pos)
    {
        move_animation_->setStartValue(scenePos());
        move_animation_->setEndValue(end_pos);
        move_animation_->start();
    }
}