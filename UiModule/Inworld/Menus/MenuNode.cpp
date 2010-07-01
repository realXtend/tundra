// For conditions of distribution and use, see copyright notice in license.txt

#include "StableHeaders.h"
#include "DebugOperatorNew.h"
#include "MenuNode.h"

#include <QPainter>
#include <QGraphicsDropShadowEffect>
#include <QGraphicsSceneMouseEvent>
#include <QLabel>

#include "MemoryLeakCheck.h"

#include <QDebug> 

namespace CoreUi
{
    MenuNode::MenuNode(const QString& node_name, const QIcon &icon, UiDefines::MenuNodeStyleMap map, QUuid id):
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

        // Always render with fallback so the looks are consistent, remove this text pixmap thing
        //if (style_to_path_map_.contains(UiDefines::TextNormal))
        //{
        //    QPixmap center_piece_image = QPixmap(style_to_path_map_[UiDefines::TextNormal]);
        //    center_image_width_ = center_piece_image.width();
        //}
        //else
        //{
            if (node_name_ != "RootNode")
            {
                // fall back here for text rendering widgts name if no graphics were provded!
                QWidget *text_widget = centerContainer->findChild<QWidget*>("textWidget");
                if (text_widget)
                {
                    QHBoxLayout *layout = new QHBoxLayout();
                    QLabel *label = new QLabel(node_name_.toUpper());
                    layout->addWidget(label);
                    text_widget->setLayout(layout);

                    QFont font("facetextrabold", 11, 50, false);
                    font.setCapitalization(QFont::AllUppercase);
                    font.setStyleStrategy(QFont::PreferAntialias);

                    label->setFont(font);
                    label->setStyleSheet("background-color: transparent; color: white; text-align: center;");
                    QFontMetrics metric(label->font());
                    center_image_width_ = metric.width(label->text()) + 20;
                }
            }
            else
                center_image_width_ = 20;
        //}

        QGraphicsDropShadowEffect *shadow_effect = new QGraphicsDropShadowEffect(this);
        shadow_effect->setColor(Qt::white);
        shadow_effect->setOffset(0,0);
        shadow_effect->setBlurRadius(10);
        setGraphicsEffect(shadow_effect);

        ChangeStyle(Normal);
        ChangeMoveState(false);
    }
    
    MenuNode::~MenuNode()
    {
    }

    QUuid MenuNode::GetID() const
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
        connect(resize_animation_, SIGNAL(stateChanged(QAbstractAnimation::State, QAbstractAnimation::State)), SLOT(ResizeStateChanged(QAbstractAnimation::State, QAbstractAnimation::State)));
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

    void MenuNode::ResizeStateChanged(QAbstractAnimation::State new_state, QAbstractAnimation::State old_state)
    {
        if (old_state == QAbstractAnimation::Stopped && new_state == QAbstractAnimation::Running)
        {
            if (resize_animation_->direction() == QAbstractAnimation::Forward)
            {
                leftContainer->setStyleSheet("QWidget#leftContainer { background-image: url('./data/ui/images/menus/icon_bground.png'); background-position: top right;"	
                                             "background-repeat: no-repeat; background-color: transparent; }");
            }
        }
    }

    void MenuNode::ResizeFinished()
    {
        if (tree_depth_ == 0)
            return;

        if (resize_animation_->direction() == QAbstractAnimation::Backward)
        {
            leftContainer->setStyleSheet("QWidget#leftContainer { background-color: transparent; }");
            rightContainer->hide();
            widget()->resize(1, widget()->size().height());
        }
        else if (resize_animation_->direction() == QAbstractAnimation::Forward)
        {
            rightContainer->show();
            widget()->resize(1, widget()->size().height());
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
        press_event->accept();
        
        NodeClicked();
    }

    void MenuNode::mouseReleaseEvent(QGraphicsSceneMouseEvent *release_event)
    {
        QRectF my_scene_rect = mapRectToScene(rect());
        QPointF release_pos = release_event->scenePos();
        if (my_scene_rect.contains(release_pos))       
            ChangeStyle(Hover);
        else
            ChangeStyle(Normal);
        QGraphicsProxyWidget::mouseReleaseEvent(release_event);
    }

    void MenuNode::ChangeStyle(Style style)
    {
        if (node_name_ != "RootNode")
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

            //textWidget->setStyleSheet("QWidget#textWidget { background-image: url('" + text_image + "'); background-position: top left; background-repeat: no-repeat; }");
            iconWidget->setStyleSheet("QWidget#iconWidget { background-image: url('" + icon_image + "'); background-position: top left; background-repeat: no-repeat; }");
        }
        else
        {
            QString root_base_style = "background-color: transparent; border-radius:0px; border: 0px; background-position: top left; background-repeat: no-repeat;";
            switch (style)
            {
                case Normal:
                    centerContainer->setStyleSheet("QWidget#centerContainer { " + root_base_style + "background-image: url('./data/ui/images/menus/uibutton_EDIT_normal.png'); }");
                    break;
                case Hover:
                    centerContainer->setStyleSheet("QWidget#centerContainer { " + root_base_style + "background-image: url('./data/ui/images/menus/uibutton_EDIT_hover.png'); }");
                    break;
                case Pressed:
                    centerContainer->setStyleSheet("QWidget#centerContainer { " + root_base_style + "background-image: url('./data/ui/images/menus/uibutton_EDIT_click.png'); }");
                    break;
            }
        }
    }
 
}
