// For conditions of distribution and use, see copyright notice in license.txt

#include "StableHeaders.h"
#include "DebugOperatorNew.h"
#include "ControlProxyWidget.h"
#include "Ether/View/InfoCard.h"
#include "Ether/View/ActionProxyWidget.h"

#include <QGraphicsDropShadowEffect>
#include <QGraphicsSceneMouseEvent>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QGraphicsScene>

#include <QDebug>

#include "MemoryLeakCheck.h"

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
              move_animation_(0),
              text_label_(0),
              suppress_buttons_(false),
              overlay_widget_(0),
              connected_(false)
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
                    InitActionWidgets();
                    break;
                    
                case AddRemoveControl:
                    InitAddRemoveControl();
                    break;

                case StatusWidget:
                    InitStatusWidget();
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
            // Common text label for both types
            text_label_ = new QLabel(text_, parent_);
            text_label_->setMinimumSize(470,50);
            text_label_->setMaximumSize(470,50);
            text_label_->setAlignment(Qt::AlignCenter);
            text_label_->setStyleSheet("color: rgb(238,238,238);");
            text_label_->setFont(QFont("Narkisim", 22));     

            QString button_style = "QPushButton { padding: 0px; margin: 0px; background-color: transparent; border: 0px;";

            if (direction_ == BottomToTop)
            {
                parent_->setStyleSheet("QWidget#containerWidget { background: transparent; background-image: url('./data/ui/images/ether/card_frame_selected_top.png');"
                                       "background-position: top left; background-repeat: no-repeat; }");   

                QPushButton *edit_avatar_button = new QPushButton(parent_);
                edit_avatar_button->setFlat(true);
                edit_avatar_button->setMaximumSize(430,42);
                edit_avatar_button->setMinimumSize(430,42);
                edit_avatar_button->setStyleSheet(QString("%1 background-image: url('./data/ui/images/ether/buttons/button_EDITAVA_normal.png'); }"
                                                  "QPushButton::hover { background-image: url('./data/ui/images/ether/buttons/button_EDITAVA_hover.png'); }"
                                                  "QPushButton::pressed { background-image: url('./data/ui/images/ether/buttons/button_EDITAVA_click.png'); }").arg(button_style));

                // Fill layout
                QVBoxLayout *main_layout = new QVBoxLayout(parent_);
                main_layout->setSpacing(0);
                main_layout->setContentsMargins(0,8,0,0);

                QHBoxLayout *buttons_layout = new QHBoxLayout();
                buttons_layout->setSpacing(0);
                buttons_layout->setMargin(0);
                buttons_layout->addWidget(edit_avatar_button);

                main_layout->addLayout(buttons_layout);
                main_layout->addSpacerItem(new QSpacerItem(1,1, QSizePolicy::Fixed, QSizePolicy::Expanding));
                main_layout->addWidget(text_label_);
                parent_->setLayout(main_layout);

                // Set handlers
                connect(edit_avatar_button, SIGNAL(clicked()), SLOT(EditHandler()));
            }
            else if (direction_ == TopToBottom)
            {
                parent_->setStyleSheet("QWidget#containerWidget { background: transparent; background-image: url('./data/ui/images/ether/card_frame_selected_bottom.png');"
                                       "background-position: top left; background-repeat: no-repeat; }");

                text_label_->setMinimumSize(430,50);
                text_label_->setMaximumSize(430,50);

                QPushButton *info_world_button = new QPushButton(parent_);
                info_world_button->setFlat(true);
                info_world_button->setMaximumSize(149,42);
                info_world_button->setMinimumSize(149,42);
                info_world_button->setStyleSheet(QString("%1 background-image: url('./data/ui/images/ether/buttons/button_INFOW_normal.png'); }"
                                                 "QPushButton::hover { background-image: url('./data/ui/images/ether/buttons/button_INFOW_hover.png'); }"
                                                 "QPushButton::pressed { background-image: url('./data/ui/images/ether/buttons/button_INFOW_click.png'); }").arg(button_style));

                QPushButton *register_world_button = new QPushButton(parent_);
                register_world_button->setFlat(true);
                register_world_button->setMaximumSize(118,42);
                register_world_button->setMinimumSize(118,42);
                register_world_button->setStyleSheet(QString("%1 background-image: url('./data/ui/images/ether/buttons/button_REGW_normal.png'); }"
                                                     "QPushButton::hover { background-image: url('./data/ui/images/ether/buttons/button_REGW_hover.png'); }"
                                                     "QPushButton::pressed { background-image: url('./data/ui/images/ether/buttons/button_REGW_click.png'); }").arg(button_style));

                QPushButton *edit_world_button = new QPushButton(parent_);
                edit_world_button->setFlat(true);
                edit_world_button->setMaximumSize(148,42);
                edit_world_button->setMinimumSize(148,42);
                edit_world_button->setStyleSheet(QString("%1 background-image: url('./data/ui/images/ether/buttons/button_EDITW_normal.png'); }"
                                                 "QPushButton::hover { background-image: url('./data/ui/images/ether/buttons/button_EDITW_hover.png'); }"
                                                 "QPushButton::pressed { background-image: url('./data/ui/images/ether/buttons/button_EDITW_click.png'); }").arg(button_style));

                // Fill layout
                QVBoxLayout *main_layout = new QVBoxLayout(parent_);
                main_layout->setSpacing(0);
                main_layout->setContentsMargins(20,0,20,8);
                QHBoxLayout *buttons_layout = new QHBoxLayout();
                buttons_layout->setSpacing(0);
                buttons_layout->setMargin(0);
                
                buttons_layout->addWidget(info_world_button);
                buttons_layout->addWidget(register_world_button);
                buttons_layout->addWidget(edit_world_button);

                main_layout->addWidget(text_label_);
                main_layout->addSpacerItem(new QSpacerItem(1,1, QSizePolicy::Fixed, QSizePolicy::Expanding));
                main_layout->addLayout(buttons_layout);
                parent_->setLayout(main_layout);

                // Set handlers
                connect(info_world_button, SIGNAL(clicked()), SLOT(InfoHandler()));
                connect(register_world_button, SIGNAL(clicked()), SLOT(RegisterHandler()));
                connect(edit_world_button, SIGNAL(clicked()), SLOT(EditHandler()));
            }

            // Init animations
            fade_animation_ = new QPropertyAnimation(this, "opacity", this);
            fade_animation_->setDuration(1000);
            fade_animation_->setEasingCurve(QEasingCurve::Linear);
            fade_animation_->setStartValue(0);
            fade_animation_->setEndValue(1);              
        }

        void ControlProxyWidget::InitActionWidgets()
        {
            parent_->setStyleSheet("QWidget#containerWidget { background: transparent; }");
            QString button_style = "QPushButton { padding: 0px; margin: 0px; background-color: transparent; border: 0px; color: white;";

            if (direction_ == TopToBottom)
            {
                // Exit 
                exit_button_ = new QPushButton(parent_);
                exit_button_->setMinimumWidth(216);
                exit_button_->setMinimumHeight(56);
                exit_button_->setFlat(true);
                exit_button_->setStyleSheet(QString("%1 background-image: url('./data/ui/images/ether/buttons/bbutton_EXIT_normal.png'); }"
                                                   "QPushButton::hover { background-image: url('./data/ui/images/ether/buttons/bbutton_EXIT_hover.png'); }"
                                                   "QPushButton::pressed { background-image: url('./data/ui/images/ether/buttons/bbutton_EXIT_click.png'); }").arg(button_style));
                connect(exit_button_, SIGNAL( clicked() ), SLOT( ExitHandler() ));

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

                QHBoxLayout *layout = new QHBoxLayout(parent_);
                layout->setSpacing(0);
                layout->setMargin(0);
                parent_->setLayout(layout);

                // Fill layout
                layout->addSpacerItem(new QSpacerItem(1,1, QSizePolicy::Expanding));
                layout->addWidget(exit_button_);

                QVBoxLayout *layout_middle = new QVBoxLayout();
                layout_middle->setSpacing(0);
                layout_middle->setContentsMargins(0,0,0,20);
                layout_middle->addWidget(enter_world_button);
                layout->addLayout(layout_middle);

                layout->addWidget(help_button);
                layout->addSpacerItem(new QSpacerItem(1,1, QSizePolicy::Expanding));

                // Set geometry
                widget()->setMinimumHeight(76);
                widget()->setMaximumHeight(76);
                setGeometry(widget()->geometry());

                // Init Animations
                all_action_widget_animations_ = new QParallelAnimationGroup(this);
                move_animation_ = new QPropertyAnimation(this, "pos", all_action_widget_animations_);
                move_animation_->setDuration(300);
                move_animation_->setEasingCurve(QEasingCurve::InOutSine);
                scale_animation_ = new QPropertyAnimation(this, "scale", all_action_widget_animations_);
                scale_animation_->setDuration(300);
                scale_animation_->setEasingCurve(QEasingCurve::InOutSine);
            }
        }

        void ControlProxyWidget::InitAddRemoveControl()
        {
            parent_->setStyleSheet("QWidget#containerWidget { background: transparent; }");
            QString button_style = "QPushButton { padding: 0px; margin: 0px; background-color: transparent; border: 0px; color: white;";

            // Remove card button
            QPushButton *remove_button = new QPushButton(parent_);
            remove_button->setObjectName("RemoveButton");
            remove_button->setMaximumSize(49,41);
            remove_button->setMinimumSize(49,41);
            remove_button->setFlat(true);

            QPushButton *add_button = new QPushButton(parent_);
            add_button->setObjectName("AddButton");
            add_button->setMaximumSize(154,42);
            add_button->setMinimumSize(152,42);
            add_button->setFlat(true);

            QHBoxLayout *layout = new QHBoxLayout(parent_);
            layout->setSpacing(0);
            layout->setMargin(0);
            parent_->setLayout(layout);

            if (direction_ == BottomToTop)
            {
                remove_button->setStyleSheet(QString("%1 background-image: url('./data/ui/images/ether/buttons/button_DELAVA_normal.png'); }"
                                             "QPushButton::hover { background-image: url('./data/ui/images/ether/buttons/button_DELAVA_hover.png'); }"
                                             "QPushButton::pressed { background-image: url('./data/ui/images/ether/buttons/button_DELAVA_click.png'); }").arg(button_style));

                add_button->setStyleSheet(QString("%1 background-image: url('./data/ui/images/ether/buttons/button_NEWAVA_normal.png'); }"
                                          "QPushButton::hover { background-image: url('./data/ui/images/ether/buttons/button_NEWAVA_hover.png'); }"
                                          "QPushButton::pressed { background-image: url('./data/ui/images/ether/buttons/button_NEWAVA_click.png'); }").arg(button_style));

                layout->addWidget(remove_button);
                layout->addSpacerItem(new QSpacerItem(1,1, QSizePolicy::Expanding, QSizePolicy::Preferred));
                layout->addWidget(add_button);
            }
            else if (direction_ == TopToBottom)
            {
                remove_button->setStyleSheet(QString("%1 background-image: url('./data/ui/images/ether/buttons/button_DELW_normal.png'); }"
                                             "QPushButton::hover { background-image: url('./data/ui/images/ether/buttons/button_DELW_hover.png'); }"
                                             "QPushButton::pressed { background-image: url('./data/ui/images/ether/buttons/button_DELW_click.png'); }").arg(button_style));

                add_button->setStyleSheet(QString("%1 background-image: url('./data/ui/images/ether/buttons/button_NEWW_normal.png'); }"
                                          "QPushButton::hover { background-image: url('./data/ui/images/ether/buttons/button_NEWW_hover.png'); }"
                                          "QPushButton::pressed { background-image: url('./data/ui/images/ether/buttons/button_NEWW_click.png'); }").arg(button_style));

                layout->addWidget(add_button);
                layout->addSpacerItem(new QSpacerItem(1,1, QSizePolicy::Expanding, QSizePolicy::Preferred));
                layout->addWidget(remove_button);
            }

            widget_map_[add_button] = TopLeft;
            widget_map_[remove_button] = TopLeft;

            connect(remove_button, SIGNAL( clicked() ), SLOT( RemoveHandler() ));
            connect(add_button, SIGNAL( clicked() ), SLOT( AddHandler() ));

            widget()->setMaximumHeight(42);
            widget()->setMinimumHeight(42);

            // Init animations
            fade_animation_ = new QPropertyAnimation(this, "opacity", this);
            fade_animation_->setDuration(1000);
            fade_animation_->setEasingCurve(QEasingCurve::Linear);
            fade_animation_->setStartValue(0);
            fade_animation_->setEndValue(1);   
        }

        void ControlProxyWidget::InitStatusWidget()
        {
            setZValue(51);
            parent_->setStyleSheet("QWidget#containerWidget { background: transparent; }");

            text_label_ = new QLabel(parent_);
            text_label_->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
            text_label_->setAlignment(Qt::AlignCenter);
            text_label_->setStyleSheet("color: rgb(238,238,238);");
            text_label_->setFont(QFont("Narkisim", 24));

            QHBoxLayout *layout = new QHBoxLayout(parent_);
            layout->setSpacing(0);
            layout->setContentsMargins(0,5,0,5);
            parent_->setLayout(layout);

            layout->addWidget(text_label_);    
        }

        void ControlProxyWidget::UpdateGeometry(QRectF new_rect, qreal scale, bool do_fade)
        {
            if (do_fade)
            {
                hide();
                setOpacity(0);
                return;
            }

            if (type_ == CardControl)
            {
                // Calculate new geometry, set scale
                QSize overlay_size(470,431);
                int name_height = 48 * scale;
                int button_height = 55 * scale;
                if (direction_ == TopToBottom)
                    new_rect.setY(new_rect.y()-button_height + 15 * scale);
                else if (direction_ == BottomToTop)
                    new_rect.setY(new_rect.y()-button_height + 15 * scale);
                new_rect.setWidth(overlay_size.width());
                new_rect.setHeight(overlay_size.height());
                
                setGeometry(new_rect);
                setScale(scale);
            }
            else if (type_ == ActionControl)
            {
                // Calculate new position, set animations
                setTransformOriginPoint(rect().center());
                QRectF calculated_rect = rect();
                calculated_rect.setX(new_rect.width()/2 - rect().width()/2);
                calculated_rect.setY(new_rect.y());

                if (scale <= 0.6)
                    scale = 0.75;
                else if (scale > 0.6 && scale < 1.0)
                    scale = 1.0;

                move_animation_->setStartValue(pos());
                move_animation_->setEndValue(calculated_rect.topLeft());
                scale_animation_->setStartValue(this->scale());
                scale_animation_->setEndValue(scale);
                all_action_widget_animations_->start();
            }
            else if (type_ == AddRemoveControl)
            {
                qreal widgets_width = 0.0;
                qreal widest_widget, shortest_widget;
                foreach (QWidget *w, widget_map_.keys())
                {
                    qreal width = w->width();
                    if (w->objectName() == "RemoveButton")
                        shortest_widget = width + 10;
                    else if (w->objectName() == "AddButton")
                        widest_widget = width + 10;
                    widgets_width += width + 10;
                }
                qreal width = 470 * scale + widgets_width;
                setMaximumWidth(width);
                setMinimumWidth(width);

                QPointF calculated_pos;
                if (direction_ == BottomToTop)
                {
                    calculated_pos.setY(new_rect.bottom() - size().height());
                    calculated_pos.setX(new_rect.left() - shortest_widget);
                }
                else if (direction_ == TopToBottom)
                {
                    calculated_pos.setY(new_rect.top());
                    calculated_pos.setX(new_rect.left() - widest_widget);
                }
                setPos(calculated_pos);
            }
        }

        void ControlProxyWidget::UpdateContollerCard(InfoCard *new_card)
        {
            controlled_card_ = new_card;
            UpdateStatusText(controlled_card_->title());

            disconnect();
            connect(controlled_card_->GetMoveAnimationPointer(), SIGNAL( finished()),
                    this, SLOT( ControlledWidgetStopped() ));
        }

        void ControlProxyWidget::ControlledWidgetStopped()
        {
            if (type_ == AddRemoveControl)
                UpdateGeometry(overlay_widget_->mapRectToScene(overlay_widget_->boundingRect()), controlled_card_->scale(), false);
            else
                UpdateGeometry(controlled_card_->mapRectToScene(controlled_card_->boundingRect()), controlled_card_->scale(), false);
            
            if (!isVisible())
                show();
            if (opacity() < 1.0)
                setOpacity(0);
            fade_animation_->setStartValue(opacity());
            fade_animation_->setDirection(QAbstractAnimation::Forward);
            if (fade_animation_->state() == QAbstractAnimation::Stopped)
                fade_animation_->start();
        }

        void ControlProxyWidget::UpdateStatusText(QString text)
        {
            if (text_label_)
                text_label_->setText(text);
        }

        // Emit type of action request

        void ControlProxyWidget::RegisterHandler()
        {
            EmitActionRequest("register");
        }

        void ControlProxyWidget::InfoHandler()
        {
            EmitActionRequest("info");
        }

        void ControlProxyWidget::EditHandler()
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
            if (connected_)
                emit ActionRequest("disconnect");
            else if (!suppress_buttons_)
                emit ActionRequest("exit");
            else
                emit ActionRequest("cancel");
        }

        void ControlProxyWidget::ConnectHandler()
        {
            if (!suppress_buttons_)
                emit ActionRequest("connect");
            else 
                qDebug() << "<Ether::ControlProxyWidget> Suppressed connect action, login is in progress";
        }

        void ControlProxyWidget::HelpHandler()
        {
            emit ActionRequest("help");
        }

        void ControlProxyWidget::HideHandler()
        {
            emit ActionRequest("hide");
        }

        void ControlProxyWidget::EmitActionRequest(QString type)
        {
            if (!suppress_buttons_)
                action_widget_->ShowWidget(type, controlled_card_);
            else 
                qDebug() << "<Ether::ControlProxyWidget> Suppressed action '" << type << "'";
        }

        void ControlProxyWidget::SuppressButtons(bool suppress)
        {
            suppress_buttons_ = suppress;

            QString button_style = "QPushButton { padding: 0px; margin: 0px; background-color: transparent; border: 0px; color: white;";
            if (suppress_buttons_)
                exit_button_->setStyleSheet(QString("%1 background-image: url('./data/ui/images/ether/buttons/bbutton_CANCEL_normal.png'); }"
                    "QPushButton::hover { background-image: url('./data/ui/images/ether/buttons/bbutton_CANCEL_hover.png'); }"
                    "QPushButton::pressed { background-image: url('./data/ui/images/ether/buttons/bbutton_CANCEL_click.png'); }").arg(button_style));  
        }

        void ControlProxyWidget::SetConnected(bool connected)
        {
            connected_ = connected;

            QString button_style = "QPushButton { padding: 0px; margin: 0px; background-color: transparent; border: 0px; color: white;";
            if (connected_)
                exit_button_->setStyleSheet(QString("%1 background-image: url('./data/ui/images/ether/buttons/bbutton_DISCON_normal.png'); }"
                    "QPushButton::hover { background-image: url('./data/ui/images/ether/buttons/bbutton_DISCON_hover.png'); }"
                    "QPushButton::pressed { background-image: url('./data/ui/images/ether/buttons/bbutton_DISCON_click.png'); }").arg(button_style));
            else
                exit_button_->setStyleSheet(QString("%1 background-image: url('./data/ui/images/ether/buttons/bbutton_EXIT_normal.png'); }"
                    "QPushButton::hover { background-image: url('./data/ui/images/ether/buttons/bbutton_EXIT_hover.png'); }"
                    "QPushButton::pressed { background-image: url('./data/ui/images/ether/buttons/bbutton_EXIT_click.png'); }").arg(button_style));       
        }
    }
}
