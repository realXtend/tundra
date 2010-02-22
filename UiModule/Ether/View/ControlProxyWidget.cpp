// For conditions of distribution and use, see copyright notice in license.txt

#include "StableHeaders.h"
#include "ControlProxyWidget.h"

#include <QGraphicsDropShadowEffect>
#include <QParallelAnimationGroup>
#include <QPropertyAnimation>
#include <QGraphicsSceneMouseEvent>
#include <QHBoxLayout>

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
              parent_(new QWidget())
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
                widget_map_[add_button] = TopRight;
                widget_map_[remove_button] = TopLeft;
                widget_map_[info_button] = BottomRight;
                //widget_map_[question_button] = BottomLeft;
            }
            else if (direction_ == TopToBottom)
            {
                parent_->setStyleSheet("QWidget#containerWidget { background: transparent; background-image: url('./data/ui/images/ether/card_frame_selected_bottom.png'); background-position: top left; background-repeat: no-repeat; }");
                widget_map_[text_label_] = TopCenter;
                widget_map_[add_button] = BottomRight ;
                widget_map_[remove_button] = BottomLeft;
                widget_map_[info_button] = TopRight;
                //widget_map_[question_button] = TopLeft;
            }

            text_label_->show();
            foreach(QWidget *w, widget_map_.keys())
                w->show();
        }

        void ControlProxyWidget::InitActionWidgets()
        {
            //QSize button_size(100,80);
            //QString button_style = "QPushButton { padding: 0px; margin: 0px; background-color: transparent; border: 0px; color: white;";

            if (direction_ == TopToBottom)
            {
                // Exit 
                QPushButton *exit_button = new QPushButton("EXIT", parent_);
                exit_button->setFont(QFont("Narkisim", 10));
                exit_button->setMinimumWidth(60);
                exit_button->setMinimumHeight(20);
                //exit_button->setFlat(true);
                //exit_button->resize(button_size);
//                exit_button->setStyleSheet(QString("%1 background-image: url('./data/ui/images/ether/control_bg.png'); }").arg(button_style));
//                                                   "QPushButton::hover { background-image: url('./data/ui/images/ether/control_left_hover.png'); }"
//                                                   "QPushButton::pressed { background-image: url('./data/ui/images/ether/control_left_pressed.png'); }").arg(button_style));
                connect(exit_button, SIGNAL( clicked() ), SLOT( ExitHandler() ));

                // Connect
                QPushButton *connect_button = new QPushButton("CONNECT", parent_);
                connect_button->setFont(QFont("Narkisim", 12));
                connect_button->setMinimumWidth(130);
                connect_button->setMinimumHeight(35);
                //connect_button->setFlat(true);
                //connect_button->resize(button_size);
//                connect_button->setStyleSheet(QString("%1 background-image: url('./data/ui/images/ether/control_bg.png'); }").arg(button_style));
//                                                      "QPushButton::hover { background-image: url('./data/ui/images/ether/control_right_hover.png'); }"
//                                                      "QPushButton::pressed { background-image: url('./data/ui/images/ether/control_right_pressed.png'); }").arg(button_style));
                connect(connect_button, SIGNAL( clicked() ), SLOT( ConnectHandler() ));

                // Connect
                QPushButton *help_button = new QPushButton("HELP", parent_);
                help_button->setFont(QFont("Narkisim", 10));
                help_button->setMinimumWidth(60);
                help_button->setMinimumHeight(20);
                help_button->setEnabled(false); // enable when has functionality
                //help_button->setFlat(true);
                //help_button->resize(button_size);
//                help_button->setStyleSheet(QString("%1 background-image: url('./data/ui/images/ether/control_bg.png'); }").arg(button_style));
//                                                      "QPushButton::hover { background-image: url('./data/ui/images/ether/control_right_hover.png'); }"
//                                                      "QPushButton::pressed { background-image: url('./data/ui/images/ether/control_right_pressed.png'); }").arg(button_style));
                connect(help_button, SIGNAL( clicked() ), SLOT( HelpHandler() ));

                widget_map_[exit_button] = NonePosition;
                widget_map_[connect_button] = NonePosition;
                widget_map_[help_button] = NonePosition;

                parent_->setLayout(new QHBoxLayout(parent_));
                parent_->layout()->setMargin(0);

                dynamic_cast<QBoxLayout*>(parent_->layout())->addSpacerItem(new QSpacerItem(1,1, QSizePolicy::Expanding));
                foreach (QWidget *w, widget_map_.keys())
                    parent_->layout()->addWidget(w);
                dynamic_cast<QBoxLayout*>(parent_->layout())->addSpacerItem(new QSpacerItem(1,1, QSizePolicy::Expanding));

                widget()->setMinimumHeight(50);
                widget()->setMaximumHeight(50);
                setGeometry(widget()->geometry());
            }
        }

        void ControlProxyWidget::UpdateGeometry(QRectF new_rect, qreal scale)
        {
            if (type_ == CardControl)
            {
                hide();

                QSize overlay_size(539,529);
                int name_height = 74 * scale;
                int button_height = 82 * scale;

                qreal xoffset = 0;
                qreal yoffset = 0;
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
                            pos.setY(new_rect.height() - 45);
                            break;
                        case TopCenter:
                            pos.setX(new_rect.width()/2 - w->width()/2);
                            pos.setY(30);
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
                QRectF calculated_rect;
                calculated_rect.setX(new_rect.width()/2 - rect().width()/2);
                calculated_rect.setY(new_rect.y());
                setPos(calculated_rect.topLeft());
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
            UpdateGeometry(controlled_card_->mapRectToScene(controlled_card_->boundingRect()), controlled_card_->scale());
            show();
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
