// For conditions of distribution and use, see copyright notice in license.txt

#include "StableHeaders.h"
#include "UiProxyWidget.h"

#include "MainPanel/MainPanelButton.h"

#include <QWidget>
#include <QTimeLine>
#include <QGraphicsEffect>
#include <QGraphicsScene>

namespace UiServices
{
    UiProxyWidget::UiProxyWidget(QWidget *widget, const UiWidgetProperties &in_widget_properties) :
        QGraphicsProxyWidget(0, in_widget_properties.GetWindowStyle()),
        widget_properties_(in_widget_properties),
        show_timeline_(new QTimeLine(300, this)),
        control_button_(0)
    {
        // QWidget setup
        widget->setWindowFlags(widget_properties_.GetWindowStyle());
        widget->setWindowTitle(widget_properties_.GetWidgetName());

        // QGraphicsProxyWidget setup
        setWidget(widget);
        setPos(widget_properties_.GetPosition().x(), widget_properties_.GetPosition().y());
        setGeometry(QRectF(widget_properties_.GetPosition(), QSizeF(widget_properties_.GetSize())));

        InitAnimations();
    }

    UiProxyWidget::~UiProxyWidget()
    {
    }

    void UiProxyWidget::InitAnimations()
    {
        if (!widget_properties_.IsFullscreen())
        {
            QGraphicsDropShadowEffect *shadow_effect = new QGraphicsDropShadowEffect(this);
            shadow_effect->setBlurRadius(3);
            shadow_effect->setOffset(3.0, 3.0);
            setGraphicsEffect(shadow_effect);
        }

        if (widget_properties_.GetWidgetName() != "Login loader") // fix
            QObject::connect(show_timeline_, SIGNAL(valueChanged(qreal)), SLOT(AnimationStep(qreal)));
    }

    void UiProxyWidget::AnimationStep(qreal step)
    {
        setOpacity(step);
    }

    void UiProxyWidget::SetControlButton(CoreUi::MainPanelButton *control_button)
    {
        control_button_ = control_button;
    }

    void UiProxyWidget::showEvent(QShowEvent *show_event)
    {
        emit Visible(true);
        QGraphicsProxyWidget::showEvent(show_event);
        show_timeline_->start();
        emit BringToFrontRequest(this);
    }

    void UiProxyWidget::hideEvent(QHideEvent *hide_event)
    {
        emit Visible(false);
        QGraphicsProxyWidget::hideEvent(hide_event);

        if (widget_properties_.GetWidgetName() != "Login loader") // fix
            setOpacity(0.0);

        if (control_button_)
            control_button_->ControlledWidgetHidden();
    }

    void UiProxyWidget::closeEvent(QCloseEvent *close_event)
    {
        QGraphicsProxyWidget::closeEvent(close_event);
        emit Closed();
    }

    void UiProxyWidget::focusInEvent(QFocusEvent *focus_event)
    {
        QGraphicsProxyWidget::focusInEvent(focus_event);
        if (control_button_)
            control_button_->ControlledWidgetFocusIn();
    }

    void UiProxyWidget::focusOutEvent(QFocusEvent *focus_event)
    {
        QGraphicsProxyWidget::focusOutEvent(focus_event);
        if (control_button_)
            control_button_->ControlledWidgetFocusOut();
    }

    QVariant UiProxyWidget::itemChange(GraphicsItemChange change, const QVariant &value)
    {
        if (change == QGraphicsItem::ItemPositionChange && scene()) 
        {
            QPointF new_position = value.toPointF();
            QRectF scene_rect = scene()->sceneRect();
            scene_rect.setRight(scene_rect.right()-20.0); // Right margin
            if (!scene_rect.contains(new_position))
            {
                new_position.setX(qMin(scene_rect.right(), qMax(new_position.x(), scene_rect.left())));
                new_position.setY(qMin(scene_rect.bottom(), qMax(new_position.y(), scene_rect.top()+20))); // Top margin
                return new_position;
            }
        }
        return QGraphicsProxyWidget::itemChange(change, value);
    }
}
