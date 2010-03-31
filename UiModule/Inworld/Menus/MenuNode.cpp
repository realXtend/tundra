// For conditions of distribution and use, see copyright notice in license.txt

#include "StableHeaders.h"
#include "MenuNode.h"

#include <QPainter>
#include <QGraphicsDropShadowEffect>
#include <QDebug>

namespace CoreUi
{
    MenuNode::MenuNode(const QString& node_name, QIcon icon, UiDefines::MenuNodeStyleMap map, QUuid id):
            QGraphicsProxyWidget(0),
            node_name_(node_name),
            widget_(new QWidget),
            original_pos_(pos()),
            expanded_pos_(pos()),
            shrunken_pos_(pos()),
            icon_(icon),
            style_to_path_map_(map)
    {
        setupUi(widget_);
        setWidget(widget_);

        if (id.isNull())
            id_ = QUuid::createUuid();
        else
            id_ = id;

        centerContainer->setMinimumWidth(0);

        if (style_to_path_map_.contains(UiDefines::TextNormal))
        {
            QPixmap center_piece_image = QPixmap(style_to_path_map_[UiDefines::TextNormal]);
            center_image_width_ = center_piece_image.width();
        }
        else
            center_image_width_ = 10;

        QGraphicsDropShadowEffect *shadow_effect = new QGraphicsDropShadowEffect(this);
        shadow_effect->setColor(Qt::white);
        shadow_effect->setOffset(0,0);
        shadow_effect->setBlurRadius(10);
        setGraphicsEffect(shadow_effect);

        ChangeStyle(Normal);
        ChangeMoveState(false);
    }

    QUuid MenuNode::GetID()
    {
        return id_;
    }

    QPropertyAnimation *MenuNode::CreateResizeAnimation(QString anim_property)
    {
        resize_animation_ = new QPropertyAnimation(centerContainer, QByteArray(anim_property.toStdString().c_str()));
        resize_animation_->setEasingCurve(QEasingCurve::InOutSine);
        resize_animation_->setDuration(RESIZE_ANIM_LENGTH);
        resize_animation_->setStartValue(0);
        resize_animation_->setEndValue(center_image_width_);
        connect(resize_animation_, SIGNAL(finished()), SLOT(ResizeFinished()));
        return resize_animation_;
    }

    void MenuNode::ChangeMoveState(bool show_borders)
    {
        if (node_name_ == "RootNode")
            return;

        if (show_borders)
        {
            leftContainer->setStyleSheet("QWidget#leftContainer { background-image: url('./data/ui/images/menus/icon_bground.png'); background-position: top right;"	
                                         "background-repeat: no-repeat; background-color: transparent; }");
            rightContainer->show();
            centerContainer->show();
        }
        else
        {
            leftContainer->setStyleSheet("QWidget#leftContainer { background-color: transparent; }");
            rightContainer->hide();
            centerContainer->hide();
        }
    }

    void MenuNode::ResizeFinished()
    {
        if (resize_animation_->direction() == QAbstractAnimation::Backward)
        {
            leftContainer->setStyleSheet("QWidget#leftContainer { background-color: transparent; }");
            if (tree_depth_ != 0)
                rightContainer->hide();
        }
    }

    void MenuNode::hoverEnterEvent(QGraphicsSceneHoverEvent *enter_event)
    {
        QGraphicsProxyWidget::hoverEnterEvent(enter_event);
        ChangeStyle(Hover);
    }

    void MenuNode::hoverLeaveEvent(QGraphicsSceneHoverEvent *leave_event)
    {
        QGraphicsProxyWidget::hoverLeaveEvent(leave_event);
        ChangeStyle(Normal);
    }

    void MenuNode::mousePressEvent(QGraphicsSceneMouseEvent *press_event)
    {
        ChangeStyle(Pressed);
        QGraphicsProxyWidget::mousePressEvent(press_event);
        NodeClicked();
    }

    void MenuNode::mouseReleaseEvent(QGraphicsSceneMouseEvent *release_event)
    {
        ChangeStyle(Normal); // propably? what if releases outside button, could be normal + hoverMoveEvent impl
        QGraphicsProxyWidget::mouseReleaseEvent(release_event);
    }

    void MenuNode::ChangeStyle(Style style)
    {
        if (style_to_path_map_.count() == 0)
            return;

        QString text_image;
        QString icon_image;

        switch (style)
        {
            case Normal:
                text_image = style_to_path_map_[UiDefines::TextNormal];
                icon_image = style_to_path_map_[UiDefines::IconNormal];
                break;
            case Hover:
                text_image = style_to_path_map_[UiDefines::TextHover];
                icon_image = style_to_path_map_[UiDefines::IconHover];
                break;
            case Pressed:
                text_image = style_to_path_map_[UiDefines::TextPressed];
                icon_image = style_to_path_map_[UiDefines::IconPressed];
                break;
        }

        textWidget->setStyleSheet("QWidget#textWidget { background-image: url('" + text_image + "'); background-position: top left; background-repeat: no-repeat; }");
        iconWidget->setStyleSheet("QWidget#iconWidget { background-image: url('" + icon_image + "'); background-position: top left; background-repeat: no-repeat; }");
    }
 
}