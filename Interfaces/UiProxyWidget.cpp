/**
 *  For conditions of distribution and use, see copyright notice in license.txt
 *
 *  @file   UiProxyWidget.cpp
 *  @brief  
 */

#include "StableHeaders.h"
#include "DebugOperatorNew.h"

#include "UiProxyWidget.h"

#include <QWidget>
#include <QGraphicsScene>
#include <QGraphicsSceneMoveEvent>
#include <QParallelAnimationGroup>
#include <QPropertyAnimation>
#include <QGraphicsDropShadowEffect>
#include <QApplication>

#include "MemoryLeakCheck.h"

UiProxyWidget::UiProxyWidget(QWidget *widget, Qt::WindowFlags flags):
    QGraphicsProxyWidget(0, flags),
    show_animation_enabled_(true),
    unfocus_opacity_(1.0),
    animations_(0),
    fade_animation_(0)
{
    QString name = "UiProxyWidget";
    if (!widget->objectName().isEmpty())
        name.append(":" + widget->objectName());
    else if (widget->windowTitle().isEmpty())
        name.append(":" + widget->windowTitle());
    setObjectName(name);

    // Embed widget to this proxy widget.
    setWidget(widget);
    widget->installEventFilter(this);

    // Init effects and animations
    if (windowFlags() != Qt::Widget)
    {
        QGraphicsDropShadowEffect *shadow_effect = new QGraphicsDropShadowEffect(this);
        shadow_effect->setBlurRadius(3);
        shadow_effect->setOffset(3.0, 3.0);
        setGraphicsEffect(shadow_effect);

        animations_ = new QParallelAnimationGroup(this);
        animations_->setDirection(QAbstractAnimation::Forward);
        connect(animations_, SIGNAL(finished()), this, SLOT(FinishHide()));

        fade_animation_ = new QPropertyAnimation(this, "opacity", this);
        fade_animation_->setDuration(300);
        fade_animation_->setStartValue(0.0);
        fade_animation_->setEndValue(1.0);

        animations_->addAnimation(fade_animation_);
    }
}

UiProxyWidget::~UiProxyWidget()
{
}

void UiProxyWidget::SetUnfocusedOpacity(int opacity)
{
    unfocus_opacity_ = opacity;
    unfocus_opacity_ /= 100;
    if (isVisible() && !hasFocus())
        setOpacity(unfocus_opacity_);
}

void UiProxyWidget::SetShowAnimationSpeed(int speed)
{
    if (!animations_)
        return;

    if (speed == 0)
    {
        show_animation_enabled_ = false;
    }
    else if (animations_->state() != QAbstractAnimation::Running)
    {
        fade_animation_->setDuration(speed);
        show_animation_enabled_ = true;
    }
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

void UiProxyWidget::ToggleVisibility()
{
    if (isVisible())
        AnimatedHide();
    else
        show();
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

void UiProxyWidget::showEvent(QShowEvent *e)
{
    QGraphicsProxyWidget::showEvent(e);
    emit Visible(true);
    emit BringProxyToFrontRequest(this);

    if (show_animation_enabled_ && animations_)
    {
        fade_animation_->setEndValue(1.0);
        animations_->setDirection(QAbstractAnimation::Forward);
        animations_->start();
    }
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

void UiProxyWidget::focusInEvent(QFocusEvent *e)
{
    QGraphicsProxyWidget::focusInEvent(e);
    if (windowFlags() != Qt::Widget)
        if (isVisible() && animations_->state() != QAbstractAnimation::Running)
            setOpacity(1.0);
}

void UiProxyWidget::focusOutEvent(QFocusEvent *e)
{
    QGraphicsProxyWidget::focusOutEvent(e);

    if (windowFlags() != Qt::Widget)
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

void UiProxyWidget::moveEvent(QGraphicsSceneMoveEvent *e)
{
    emit ProxyMoved(this, e->newPos());
    QGraphicsProxyWidget::moveEvent(e);
}

void UiProxyWidget::ungrabMouseEvent(QEvent *e)
{
    emit ProxyUngrabbed(this, scenePos());
    QGraphicsProxyWidget::ungrabMouseEvent(e);
}

void UiProxyWidget::FinishHide()
{
    if (animations_->direction() == QAbstractAnimation::Backward)
        hide();
}
