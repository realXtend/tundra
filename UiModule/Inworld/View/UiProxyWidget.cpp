// For conditions of distribution and use, see copyright notice in license.txt

#include "StableHeaders.h"
#include "DebugOperatorNew.h"
#include "UiProxyWidget.h"

#include <QWidget>
#include <QGraphicsEffect>
#include <QGraphicsScene>
#include <QTimeLine>
#include <QParallelAnimationGroup>
#include <QPropertyAnimation>

#include "MemoryLeakCheck.h"

namespace UiServices
{
    UiProxyWidget::UiProxyWidget(QWidget *widget, const UiWidgetProperties in_widget_properties) 
        : QGraphicsProxyWidget(0, in_widget_properties.GetWindowStyle()),
          widget_properties_(in_widget_properties),
          show_animation_enabled_(true),
          unfocus_opacity_(1.0),
          animations_(0),
          fade_animation_(0)
    {
        setObjectName(in_widget_properties.GetWidgetName());
        InitWidgetAndProxy(widget);
        InitEffectsAndAnimations();
    }

    UiProxyWidget::~UiProxyWidget()
    {
    }

    void UiProxyWidget::InitWidgetAndProxy(QWidget *widget)
    {
        widget->setWindowFlags(widget_properties_.GetWindowStyle());
        widget->setWindowTitle(widget_properties_.GetWidgetName());
        setWidget(widget);
        setGeometry(QRectF(widget_properties_.GetPosition(), QSizeF(widget->size())));
    }

    void UiProxyWidget::InitEffectsAndAnimations()
    {
        if (widget_properties_.GetWidgetType() != UiServices::CoreLayoutWidget)
        {
            QGraphicsDropShadowEffect shadow_effect(this);
            shadow_effect.setBlurRadius(3);
            shadow_effect.setOffset(3.0, 3.0);
            setGraphicsEffect(&shadow_effect);

            animations_ = new QParallelAnimationGroup(this);
            animations_->setDirection(QAbstractAnimation::Forward);
            connect(animations_, SIGNAL( finished() ), this, SLOT( FinishHide() ));

            fade_animation_ = new QPropertyAnimation(this, "opacity", this);
            fade_animation_->setDuration(300);
            fade_animation_->setStartValue(0.0);
            fade_animation_->setEndValue(1.0);

            animations_->addAnimation(fade_animation_);
        }
    }

    void UiProxyWidget::SetUnfocusedOpacity(int new_opacity)
    {
        unfocus_opacity_ = new_opacity;
        unfocus_opacity_ /= 100;
        if (isVisible() && !hasFocus())
            setOpacity(unfocus_opacity_);
    }

    void UiProxyWidget::SetShowAnimationSpeed(int new_speed)
    {
        if (!animations_)
            return;

        if (new_speed == 0)
            show_animation_enabled_ = false;
        else if (animations_->state() != QAbstractAnimation::Running)
        {
            fade_animation_->setDuration(new_speed);
            show_animation_enabled_ = true;
        }
    }

    void UiProxyWidget::BringToFront()
    {
        emit BringProxyToFrontRequest(this);
        if (!isVisible())
            show();
    }

    void UiProxyWidget::showEvent(QShowEvent *show_event)
    {
        QGraphicsProxyWidget::showEvent(show_event);
        emit Visible(true);
        emit BringProxyToFrontRequest(this);

        if (show_animation_enabled_ && animations_)
        {
            fade_animation_->setEndValue(1.0);
            animations_->setDirection(QAbstractAnimation::Forward);
            animations_->start();
        }
    }

    void UiProxyWidget::hideEvent(QHideEvent *hide_event)
    {
        QGraphicsProxyWidget::hideEvent(hide_event);
        emit Visible(false);
    }

    void UiProxyWidget::AnimatedHide()
    {
        if (show_animation_enabled_ && animations_)
        {
            if (!hasFocus())
                fade_animation_->setEndValue(unfocus_opacity_);
            else
                fade_animation_->setEndValue(1.0);
            animations_->setDirection(QAbstractAnimation::Backward);
            animations_->start();
        }
        else
            hide();
    }

    void UiProxyWidget::FinishHide()
    {
        if (animations_->direction() == QAbstractAnimation::Backward)
            hide();
    }

    void UiProxyWidget::closeEvent(QCloseEvent *close_event)
    {
        QGraphicsProxyWidget::closeEvent(close_event);
        emit Closed();
    }

    void UiProxyWidget::focusInEvent(QFocusEvent *focus_event)
    {
        QGraphicsProxyWidget::focusInEvent(focus_event);

        if (widget_properties_.GetWidgetType() != UiServices::CoreLayoutWidget)
        {
            if (isVisible() && animations_->state() != QAbstractAnimation::Running)
                setOpacity(1.0);
        }
    }

    void UiProxyWidget::focusOutEvent(QFocusEvent *focus_event)
    {
        QGraphicsProxyWidget::focusOutEvent(focus_event);

        if (widget_properties_.GetWidgetType() != UiServices::CoreLayoutWidget)
            setOpacity(unfocus_opacity_);
    }

    QVariant UiProxyWidget::itemChange(GraphicsItemChange change, const QVariant &value)
    {
        if (change == QGraphicsItem::ItemPositionChange && scene()) 
        {
            QPointF new_position = value.toPointF();
            QRectF scene_rect = scene()->sceneRect();
            scene_rect.setRight(scene_rect.right()-20.0);
            if (!scene_rect.contains(new_position))
            {
                new_position.setX(qMin(scene_rect.right(), qMax(new_position.x(), scene_rect.left())));
                new_position.setY(qMin(scene_rect.bottom(), qMax(new_position.y(), scene_rect.top()+20)));
                return new_position;
            }
        }
        return QGraphicsProxyWidget::itemChange(change, value);
    }
}
