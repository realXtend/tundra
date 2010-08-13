// For conditions of distribution and use, see copyright notice in license.txt

#include "StableHeaders.h"
#include "DebugOperatorNew.h"
#include "MenuNode.h"

#include <QPainter>
#include <QGraphicsDropShadowEffect>
#include <QGraphicsSceneMouseEvent>
#include <QLabel>

#include "MemoryLeakCheck.h"

namespace CoreUi
{
    MenuNode::MenuNode(const QString& node_name, const QString &icon, const QUuid &id):
        node_name_(node_name),
        widget_(new QWidget),
        original_pos_(pos()),
        expanded_pos_(pos()),
        shrunken_pos_(pos()),
        icon_filepath_(icon),
        id_(id)
    {
        setupUi(widget_);
        setWidget(widget_);

        centerContainer->setMinimumWidth(0);

        if (node_name_ != "RootNode")
        {
            QWidget *text_widget = centerContainer->findChild<QWidget*>("textWidget");
            if (text_widget)
            {
                QHBoxLayout *layout = new QHBoxLayout();
                //label_ = new QLabel(node_name_.toUpper());
                label_ = new QLabel(node_name_);
                layout->addWidget(label_);
                text_widget->setLayout(layout);

                QFont font("facetextrabold", 11, 50, false);
                //font.setCapitalization(QFont::AllUppercase);
                font.setStyleStrategy(QFont::PreferAntialias);

                label_->setFont(font);
                label_->setStyleSheet("background-color: transparent; color: white; text-align: center;");
                QFontMetrics metric(label_->font());
                center_image_width_ = metric.width(label_->text()) + 20;

                iconWidget->setStyleSheet("QWidget#iconWidget { background-image: url('" + icon_filepath_ +
                    "'); background-position: top left; background-repeat: no-repeat; }");
            }
        }
        else
            center_image_width_ = 20;

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

    QPropertyAnimation *MenuNode::CreateResizeAnimation(QString anim_property)
    {
        resize_animation_ = new QPropertyAnimation(centerContainer, QByteArray(anim_property.toStdString().c_str()));
        resize_animation_->setEasingCurve(QEasingCurve::InOutSine);
        resize_animation_->setDuration(RESIZE_ANIM_LENGTH);
        resize_animation_->setStartValue(0);
        resize_animation_->setEndValue(center_image_width_);
        connect(resize_animation_, SIGNAL(finished()), SLOT(ResizeFinished()));
        connect(resize_animation_, SIGNAL(stateChanged(QAbstractAnimation::State, QAbstractAnimation::State)),
            SLOT(ResizeStateChanged(QAbstractAnimation::State, QAbstractAnimation::State)));
        return resize_animation_;
    }

    void MenuNode::ChangeMoveState(bool show_borders)
    {
        if (node_name_ == "RootNode")
            return;

        if (show_borders)
        {
            leftContainer->setStyleSheet("QWidget#leftContainer { background-image: url('./data/ui/images/menus/icon_bground.png');"
                "background-position: top right; background-repeat: no-repeat; background-color: transparent; }");
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
        if (old_state == QAbstractAnimation::Stopped && new_state == QAbstractAnimation::Running &&
            resize_animation_->direction() == QAbstractAnimation::Forward)
        {
                leftContainer->setStyleSheet("QWidget#leftContainer { background-image: url('./data/ui/images/menus/icon_bground.png');"
                    "background-position: top right; background-repeat: no-repeat; background-color: transparent; }");
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
            ///\todo Effects for text labels also.
            switch (style)
            {
            case Normal:
            {
                QGraphicsEffect *iconEffect = iconWidget->graphicsEffect();
//                QGraphicsEffect *textEffect = label_->graphicsEffect();
                SAFE_DELETE(iconEffect);
//                SAFE_DELETE(textEffect);
                update();
                break;
            }
            case Hover:
            {
                QGraphicsColorizeEffect *hoverEffect = new QGraphicsColorizeEffect;
                QColor color(255, 255, 255, 100);
                hoverEffect->setColor(color);
                iconWidget->setGraphicsEffect(hoverEffect);
//                label_->setGraphicsEffect(hoverEffect);
                break;
            }
            case Pressed:
            {
                QGraphicsColorizeEffect *pressedEffect = new QGraphicsColorizeEffect;
                QColor color(112, 112, 112, 100);
                pressedEffect->setColor(color);
                iconWidget->setGraphicsEffect(pressedEffect);
//                label_->setGraphicsEffect(pressedEffect);
                break;
            }
            }
        }
        else
        {
            QString root_base_style = "background-color: transparent; border-radius:0px; border: 0px;"
                "background-position: top left; background-repeat: no-repeat;";
            switch (style)
            {
                case Normal:
                    centerContainer->setStyleSheet("QWidget#centerContainer { " + root_base_style + 
                        "background-image: url('./data/ui/images/menus/uibutton_EDIT_normal.png'); }");
                    break;
                case Hover:
                    centerContainer->setStyleSheet("QWidget#centerContainer { " + root_base_style + 
                        "background-image: url('./data/ui/images/menus/uibutton_EDIT_hover.png'); }");
                    break;
                case Pressed:
                    centerContainer->setStyleSheet("QWidget#centerContainer { " + root_base_style + 
                        "background-image: url('./data/ui/images/menus/uibutton_EDIT_click.png'); }");
                    break;
            }
        }
    }
 
}
