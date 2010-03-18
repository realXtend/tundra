// For conditions of distribution and use, see copyright notice in license.txt

#include "StableHeaders.h"
#include "NotificationBaseWidget.h"

#include <QTimer>
#include <QPropertyAnimation>
#include <QGraphicsDropShadowEffect>

namespace CoreUi
{
    NotificationBaseWidget::NotificationBaseWidget(int hide_in_msec, QString message) :
        QGraphicsProxyWidget(0, Qt::Widget),
        internal_widget_(new QWidget()),
        content_widget_(0),
        fade_animation_(new QPropertyAnimation(this)),
        progress_animation_(new QPropertyAnimation(this)),
        move_animation_(new QPropertyAnimation(this)),
        timestamp_(QDateTime::currentDateTime()),
        hide_in_msec_(hide_in_msec),
        is_active_(false),
        message_(message),
        result_title_(QString()),
        result_(QString())
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
        if (opacity() != 1.0)
            return;

        progress_animation_->stop();
        fade_animation_->start();
    }

    void NotificationBaseWidget::WidgetHidden()
    {
        emit Completed(this);
    }

    // Protected

    void NotificationBaseWidget::SetCentralWidget(QWidget *widget)
    {
        content_widget_ = widget;
        content_widget_->layout()->setMargin(0);
        mainLayout->insertWidget(0, content_widget_);
    }

    void NotificationBaseWidget::hoverEnterEvent(QGraphicsSceneHoverEvent *hover_enter_event)
    {
        QGraphicsProxyWidget::hoverEnterEvent(hover_enter_event);
        if (progress_animation_->state() == QAbstractAnimation::Running)
            progress_animation_->pause();
    }

    void NotificationBaseWidget::hoverLeaveEvent(QGraphicsSceneHoverEvent *hover_leave_event)
    {
        QGraphicsProxyWidget::hoverLeaveEvent(hover_leave_event);
        if (progress_animation_->state() == QAbstractAnimation::Paused)
            progress_animation_->resume();
    }

    // Public

    void NotificationBaseWidget::Start()
    {
        if (hide_in_msec_ > 0)
            progress_animation_->start();
        else
            timeoutProgressBar->setValue(0);
    }

    void NotificationBaseWidget::Hide()
    {
        TimedOut();
    }

    void NotificationBaseWidget::AnimateToPosition(QPointF end_pos)
    {
        move_animation_->setStartValue(scenePos());
        move_animation_->setEndValue(end_pos);
        move_animation_->start();
    }

    void NotificationBaseWidget::SetActive(bool active)
    {
        is_active_ = active; 
        if (!is_active_)
        {
            emit HideInteractionWidgets();
            emit ResultsAreIn(content_widget_, result_title_, result_);
        }
    }
}