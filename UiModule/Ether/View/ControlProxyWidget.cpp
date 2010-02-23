// For conditions of distribution and use, see copyright notice in license.txt

#include "StableHeaders.h"
#include "ControlProxyWidget.h"

#include <QGraphicsDropShadowEffect>
#include <QGraphicsSceneMouseEvent>
#include <QHBoxLayout>
#include <QGraphicsScene>

#include <QDebug>

namespace Ether
{
    namespace View
    {
        ControlProxyWidget::ControlProxyWidget(ControlType type, LayoutDirection direction, QString text)
            : QGraphicsProxyWidget(0, Qt::Widget),
              type_(type),
              direction_(direction),
              text_(text),
              parent_(new QWidget()),
              all_action_widget_animations_(0),
              fade_animation_(0),
              scale_animation_(0),
              move_animation_(0)
        {
            parent_->setObjectName("containerWidget");
            setWidget(parent_);
            setZValue(50);

            switch (type_)
            {
                case CardControl:
                    InitCardWidgets();
                    break;

                case ActionControl:
                    parent_->setStyleSheet("QWidget#containerWidget { background: transparent; }");
                    InitActionWidgets();
                    break;
            }

            QGraphicsDropShadowEffect *shadow_effect = new QGraphicsDropShadowEffect(this);
            shadow_effect->setBlurRadius(15);
            shadow_effect->setOffset(0,5);
            shadow_effect->setColor(Qt::black);
            setGraphicsEffect(shadow_effect);    
        }

        void ControlProxyWidget::InitCardWidgets()
        {
            QSize button_size(30,30);
            QString button_style = "QPushButton { padding: 0px; margin: 0px; background-color: transparent; border: 0px;";

            // Text label
            text_label_ = new QLabel(text_, parent_);
            text_label_->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
            text_label_->setMaximumWidth(300);
            text_label_->setStyleSheet("color: white");
            text_label_->setFont(QFont("Narkisim", 24));
            text_label_->setAlignment(Qt::AlignCenter);
            text_label_->setMinimumWidth(300);

            // Buttons
            QPushButton *add_button = new QPushButton(parent_);
            add_button->setStyleSheet(QString("%1 background-image: url('./data/ui/images/ether/add_button.png'); }"
                                              "QPushButton::hover { background-image: url('./data/ui/images/ether/add_button_hover.png'); }"
                                              "QPushButton::pressed { background-image: url('./data/ui/images/ether/add_button_pressed.png'); }").arg(button_style));
            add_button->setFlat(true);
            add_button->resize(button_size);

            QPushButton *remove_button = new QPushButton(parent_);
            remove_button->setStyleSheet(QString("%1 background-image: url('./data/ui/images/ether/remove_button.png'); }"
                                                 "QPushButton::hover { background-image: url('./data/ui/images/ether/remove_button_hover.png'); }"
                                                 "QPushButton::pressed { background-image: url('./data/ui/images/ether/remove_button_pressed.png'); }").arg(button_style));
            remove_button->setFlat(true);
            remove_button->resize(button_size);

            QPushButton *info_button = new QPushButton(parent_);
            info_button->setStyleSheet(QString("%1 background-image: url('./data/ui/images/ether/info_button.png'); }"
                                               "QPushButton::hover { background-image: url('./data/ui/images/ether/info_button_hover.png'); }"
                                               "QPushButton::pressed { background-image: url('./data/ui/images/ether/info_button_pressed.png'); }").arg(button_style));
            info_button->setFlat(true);
            info_button->resize(button_size);

            QPushButton *question_button = new QPushButton(parent_);
            question_button->setStyleSheet(QString("%1 background-image: url('./data/ui/images/ether/question_button.png'); }"
                                                   "QPushButton::hover { background-image: url('./data/ui/images/ether/question_button_hover.png'); }"
                                                   "QPushButton::pressed { background-image: url('./data/ui/images/ether/question_button_pressed.png'); }").arg(button_style));
            question_button->setFlat(true);
            question_button->resize(button_size);

            // Set handlers
            connect(add_button, SIGNAL(clicked()), SLOT(AddHandler()));
            connect(remove_button, SIGNAL(clicked()), SLOT(RemoveHandler()));
            connect(info_button, SIGNAL(clicked()), SLOT(InfoHandler()));
            connect(question_button, SIGNAL(clicked()), SLOT(QuestionHandler()));

            if (direction_ == BottomToTop)
            {
                parent_->setStyleSheet("QWidget#containerWidget { background: transparent; background-image: url('./data/ui/images/ether/card_frame_selected_top.png'); background-position: top left; background-repeat: no-repeat; }");   
                widget_map_[text_label_] = BottomCenter;
                //widget_map_[add_button] = TopRight;
                //widget_map_[remove_button] = TopLeft;
                //widget_map_[info_button] = BottomRight;
                //widget_map_[question_button] = BottomLeft;
            }
            else if (direction_ == TopToBottom)
            {
                parent_->setStyleSheet("QWidget#containerWidget { background: transparent; background-image: url('./data/ui/images/ether/card_frame_selected_bottom.png'); background-position: top left; background-repeat: no-repeat; }");
                widget_map_[text_label_] = TopCenter;
                //widget_map_[add_button] = BottomRight ;
                //widget_map_[remove_button] = BottomLeft;
                //widget_map_[info_button] = TopRight;
                //widget_map_[question_button] = TopLeft;
            }

            // Animations
            fade_animation_ = new QPropertyAnimation(this, "opacity", this);
            fade_animation_->setDuration(1000);
            fade_animation_->setEasingCurve(QEasingCurve::Linear);
            fade_animation_->setStartValue(0);
            fade_animation_->setEndValue(1);  

            foreach(QWidget *w, widget_map_.keys())
                w->show();            
        }

        void ControlProxyWidget::InitActionWidgets()
        {
            QString button_style = "QPushButton { padding: 0px; margin: 0px; background-color: transparent; border: 1px; color: white;";

            if (direction_ == TopToBottom)
            {
                // Exit 
                QPushButton *exit_button = new QPushButton(parent_);
                exit_button->setMinimumWidth(216);
                exit_button->setMinimumHeight(56);
                exit_button->setFlat(true);
                exit_button->setStyleSheet(QString("%1 background-image: url('./data/ui/images/ether/buttons/bbutton_EXIT_normal.png'); }"
                                                   "QPushButton::hover { background-image: url('./data/ui/images/ether/buttons/bbutton_EXIT_hover.png'); }"
                                                   "QPushButton::pressed { background-image: url('./data/ui/images/ether/buttons/bbutton_EXIT_click.png'); }").arg(button_style));
                connect(exit_button, SIGNAL( clicked() ), SLOT( ExitHandler() ));

                // Enter world
                QPushButton *enter_world_button = new QPushButton(parent_);
                enter_world_button->setMinimumWidth(335);
                enter_world_button->setMinimumHeight(56);
                enter_world_button->setFlat(true);
                enter_world_button->setStyleSheet(QString("%1 background-image: url('./data/ui/images/ether/buttons/bbutton_ENTER_normal.png'); }"
                                                          "QPushButton::hover { background-image: url('./data/ui/images/ether/buttons/bbutton_ENTER_hover.png'); }"
                                                          "QPushButton::pressed { background-image: url('./data/ui/images/ether/buttons/bbutton_ENTER_click.png'); }").arg(button_style));
                connect(enter_world_button, SIGNAL( clicked() ), SLOT( ConnectHandler() ));

                // Help
                QPushButton *help_button = new QPushButton(parent_);
                help_button->setMinimumWidth(216);
                help_button->setMinimumHeight(56);
                help_button->setFlat(true);
                help_button->setStyleSheet(QString("%1 background-image: url('./data/ui/images/ether/buttons/bbutton_HELP_normal.png'); }"
                                                   "QPushButton::hover { background-image: url('./data/ui/images/ether/buttons/bbutton_HELP_hover.png'); }"
                                                   "QPushButton::pressed { background-image: url('./data/ui/images/ether/buttons/bbutton_HELP_click.png'); }").arg(button_style));
                connect(help_button, SIGNAL( clicked() ), SLOT( HelpHandler() ));

                widget_map_[exit_button] = NonePosition;
                widget_map_[enter_world_button] = NonePosition;
                widget_map_[help_button] = NonePosition;

                QHBoxLayout *layout = new QHBoxLayout(parent_);
                layout->setSpacing(0);
                layout->setMargin(0);
                parent_->setLayout(layout);

                dynamic_cast<QBoxLayout*>(parent_->layout())->addSpacerItem(new QSpacerItem(1,1, QSizePolicy::Expanding));
                foreach (QWidget *w, widget_map_.keys())
                    parent_->layout()->addWidget(w);
                dynamic_cast<QBoxLayout*>(parent_->layout())->addSpacerItem(new QSpacerItem(1,1, QSizePolicy::Expanding));

                widget()->setMinimumHeight(65);
                widget()->setMaximumHeight(65);
                setGeometry(widget()->geometry());

                // Animations
                all_action_widget_animations_ = new QParallelAnimationGroup(this);
                move_animation_ = new QPropertyAnimation(this, "pos", all_action_widget_animations_);
                move_animation_->setDuration(300);
                move_animation_->setEasingCurve(QEasingCurve::InOutSine);
                scale_animation_ = new QPropertyAnimation(this, "scale", all_action_widget_animations_);
                scale_animation_->setDuration(300);
                scale_animation_->setEasingCurve(QEasingCurve::InOutSine);
            }
        }

        void ControlProxyWidget::UpdateGeometry(QRectF new_rect, qreal scale, bool do_fade)
        {
            if (type_ == CardControl)
            {
                if (do_fade)
                {
                    hide();
                    setOpacity(0);
                }

                qreal xoffset = 0;
                qreal yoffset = 0;

                // Calculate new position
                QSize overlay_size(539,494);
                int name_height = 55 * scale;
                int button_height = 64 * scale;
                if (direction_ == TopToBottom)
                    new_rect.setY(new_rect.y()-button_height + 19 * scale);
                else if (direction_ == BottomToTop)
                    new_rect.setY(new_rect.y()-button_height + 15 * scale);
                new_rect.setWidth(overlay_size.width());
                new_rect.setHeight(overlay_size.height());
                setGeometry(new_rect);

                xoffset += yoffset;
                QPoint pos;
                foreach(QWidget *w, widget_map_.keys())
                {
                    switch (widget_map_.value(w))
                    {
                        case BottomCenter:
                            pos.setX(new_rect.width()/2 - w->width()/2);
                            pos.setY(new_rect.height() - 40);
                            break;
                        case TopCenter:
                            pos.setX(new_rect.width()/2 - w->width()/2);
                            pos.setY(15);
                            break;
                        case LeftCenter:
                        case RightCenter:
                            pos = QPoint(0,0);
                            break;
                        case BottomLeft:
                            if (direction_== TopToBottom)
                            {
                                pos.setX(0);
                                pos.setY(new_rect.height() - w->height());
                            }
                            else
                            {
                                pos.setX(xoffset);
                                pos.setY(new_rect.height() - w->height() - yoffset);
                            }
                            break;
                        case BottomRight:
                            if (direction_ == TopToBottom)
                            {
                                pos.setX(new_rect.width() - w->width());
                                pos.setY(new_rect.height() - w->height());
                            }
                            else
                            {
                                pos.setX(new_rect.width() - w->width() - xoffset);
                                pos.setY(new_rect.height() - w->height() - yoffset);
                            }
                            break;
                        case TopRight:
                            if (direction_ == TopToBottom)
                            {
                                pos.setX(new_rect.width() - w->width() - xoffset);
                                pos.setY(yoffset);
                            }
                            else
                            {
                                pos.setX(new_rect.width() - w->width());
                                pos.setY(0);
                            }
                            break;
                        case TopLeft:
                            if (direction_ == TopToBottom)
                            {
                                pos.setX(xoffset);
                                pos.setY(yoffset);
                            }
                            else
                                pos =  QPoint(0,0);
                            break;
                    }
                    w->move(pos);
                }
                setScale(scale);
            }
            else if (type_ == ActionControl)
            {
                // Calculate new position, set scale
                setTransformOriginPoint(rect().center());
                QRectF calculated_rect = rect();
                calculated_rect.setX(new_rect.width()/2 - rect().width()/2);
                calculated_rect.setY(new_rect.y());
                
                move_animation_->setStartValue(pos());
                move_animation_->setEndValue(calculated_rect.topLeft());

                if (scale < 0.5)
                    scale = 0.75;
                else if (scale > 0.5 && scale < 1.0)
                    scale = 1.0;

                scale_animation_->setStartValue(this->scale());
                scale_animation_->setEndValue(scale);

                all_action_widget_animations_->start();
            }
        }

        void ControlProxyWidget::UpdateContollerCard(InfoCard *new_card)
        {
            controlled_card_ = new_card;
            text_label_->setText(controlled_card_->title());

            disconnect();
            connect(controlled_card_->GetMoveAnimationPointer(), SIGNAL( finished()),
                    this, SLOT( ControlledWidgetStopped() ));
        }

        void ControlProxyWidget::ControlledWidgetStopped()
        {
            UpdateGeometry(controlled_card_->mapRectToScene(controlled_card_->boundingRect()), controlled_card_->scale(), false);
            
            if (!isVisible())
                show();
            if (opacity() < 1.0)
                setOpacity(0);
            fade_animation_->setStartValue(opacity());
            fade_animation_->setDirection(QAbstractAnimation::Forward);
            if (fade_animation_->state() == QAbstractAnimation::Stopped)
                fade_animation_->start();
            last_scene_rect_ = scene()->sceneRect();
        }

        void ControlProxyWidget::InfoHandler()
        {
            EmitActionRequest("info");
        }

        void ControlProxyWidget::QuestionHandler()
        {
            EmitActionRequest("edit");
        }

        void ControlProxyWidget::AddHandler()
        {
            EmitActionRequest("add");
        }

        void ControlProxyWidget::RemoveHandler()
        {
            EmitActionRequest("remove");
        }

        void ControlProxyWidget::ExitHandler()
        {
            emit ActionRequest("exit");
        }

        void ControlProxyWidget::ConnectHandler()
        {
            emit ActionRequest("connect");
        }

        void ControlProxyWidget::HelpHandler()
        {
            emit ActionRequest("help");
        }

        void ControlProxyWidget::EmitActionRequest(QString type)
        {
            action_widget_->ShowWidget(type, controlled_card_);
        }   
    }
}
