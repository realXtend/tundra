// For conditions of distribution and use, see copyright notice in license.txt

#include "StableHeaders.h"
#include "ControlProxyWidget.h"

#include <QGraphicsDropShadowEffect>
#include <QGraphicsSceneMouseEvent>
#include <QHBoxLayout>
#include <QVBoxLayout>
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
            QString button_style = "QPushButton { padding: 0px; margin: 0px; background-color: transparent; border: 0px;";

            // Common text label for both types
            text_label_ = new QLabel(text_, parent_);
            text_label_->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
            text_label_->setMaximumWidth(490);
            text_label_->setStyleSheet("color: white");
            text_label_->setFont(QFont("Narkisim", 18));
            text_label_->setAlignment(Qt::AlignCenter);
            text_label_->setMinimumWidth(490);

            if (direction_ == BottomToTop)
            {
                parent_->setStyleSheet("QWidget#containerWidget { background: transparent; background-image: url('./data/ui/images/ether/card_frame_selected_top.png');"
                                       "background-position: top left; background-repeat: no-repeat; }");   

                // Buttons
                //QPushButton *add_button = new QPushButton(parent_);
                //add_button->setStyleSheet(QString("%1 background-image: url('./data/ui/images/ether/add_button.png'); }"
                //                                  "QPushButton::hover { background-image: url('./data/ui/images/ether/add_button_hover.png'); }"
                //                                  "QPushButton::pressed { background-image: url('./data/ui/images/ether/add_button_pressed.png'); }").arg(button_style));
                //add_button->setFlat(true);
                //add_button->resize(button_size);

                //QPushButton *remove_button = new QPushButton(parent_);
                //remove_button->setStyleSheet(QString("%1 background-image: url('./data/ui/images/ether/remove_button.png'); }"
                //                                     "QPushButton::hover { background-image: url('./data/ui/images/ether/remove_button_hover.png'); }"
                //                                     "QPushButton::pressed { background-image: url('./data/ui/images/ether/remove_button_pressed.png'); }").arg(button_style));
                //remove_button->setFlat(true);
                //remove_button->resize(button_size);

                QPushButton *edit_avatar_button = new QPushButton(parent_);
                edit_avatar_button->setFlat(true);
                edit_avatar_button->setMaximumSize(299,30);
                edit_avatar_button->setMinimumSize(299,30);
                edit_avatar_button->setStyleSheet(QString("%1 background-image: url('./data/ui/images/ether/buttons/button_EDITAVA_normal.png'); }"
                                                  "QPushButton::hover { background-image: url('./data/ui/images/ether/buttons/button_EDITAVA_hover.png'); }"
                                                  "QPushButton::pressed { background-image: url('./data/ui/images/ether/buttons/button_EDITAVA_click.png'); }").arg(button_style));

                // Fill layout
                QVBoxLayout *main_layout = new QVBoxLayout(parent_);
                main_layout->setSpacing(0);
                main_layout->setMargin(0);

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

                // Buttons
                //QPushButton *add_button = new QPushButton(parent_);
                //add_button->setStyleSheet(QString("%1 background-image: url('./data/ui/images/ether/add_button.png'); }"
                //                                  "QPushButton::hover { background-image: url('./data/ui/images/ether/add_button_hover.png'); }"
                //                                  "QPushButton::pressed { background-image: url('./data/ui/images/ether/add_button_pressed.png'); }").arg(button_style));
                //add_button->setFlat(true);
                //add_button->resize(button_size);

                //QPushButton *remove_button = new QPushButton(parent_);
                //remove_button->setStyleSheet(QString("%1 background-image: url('./data/ui/images/ether/remove_button.png'); }"
                //                                     "QPushButton::hover { background-image: url('./data/ui/images/ether/remove_button_hover.png'); }"
                //                                     "QPushButton::pressed { background-image: url('./data/ui/images/ether/remove_button_pressed.png'); }").arg(button_style));
                //remove_button->setFlat(true);
                //remove_button->resize(button_size);

                QPushButton *info_world_button = new QPushButton(parent_);
                info_world_button->setFlat(true);
                info_world_button->setMaximumSize(105,30);
                info_world_button->setMinimumSize(105,30);
                info_world_button->setStyleSheet(QString("%1 background-image: url('./data/ui/images/ether/buttons/button_INFOW_normal.png'); }"
                                                       "QPushButton::hover { background-image: url('./data/ui/images/ether/buttons/button_INFOW_hover.png'); }"
                                                       "QPushButton::pressed { background-image: url('./data/ui/images/ether/buttons/button_INFOW_click.png'); }").arg(button_style));

                QPushButton *register_world_button = new QPushButton(parent_);
                register_world_button->setFlat(true);
                register_world_button->setMaximumSize(83,30);
                register_world_button->setMinimumSize(83,30);
                register_world_button->setStyleSheet(QString("%1 background-image: url('./data/ui/images/ether/buttons/button_REGW_normal.png'); }"
                                                     "QPushButton::hover { background-image: url('./data/ui/images/ether/buttons/button_REGW_hover.png'); }"
                                                     "QPushButton::pressed { background-image: url('./data/ui/images/ether/buttons/button_REGW_click.png'); }").arg(button_style));

                QPushButton *edit_world_button = new QPushButton(parent_);
                edit_world_button->setFlat(true);
                edit_world_button->setMaximumSize(105,30);
                edit_world_button->setMinimumSize(105,30);
                edit_world_button->setStyleSheet(QString("%1 background-image: url('./data/ui/images/ether/buttons/button_EDITW_normal.png'); }"
                                                 "QPushButton::hover { background-image: url('./data/ui/images/ether/buttons/button_EDITW_hover.png'); }"
                                                 "QPushButton::pressed { background-image: url('./data/ui/images/ether/buttons/button_EDITW_click.png'); }").arg(button_style));

                // Fill layout
                QVBoxLayout *main_layout = new QVBoxLayout(parent_);
                main_layout->setSpacing(0);
                main_layout->setMargin(0);
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

                QHBoxLayout *layout = new QHBoxLayout(parent_);
                layout->setSpacing(0);
                layout->setMargin(0);
                parent_->setLayout(layout);

                // Fill layout
                layout->addSpacerItem(new QSpacerItem(1,1, QSizePolicy::Expanding));
                layout->addWidget(exit_button);

                QVBoxLayout *layout_middle = new QVBoxLayout();
                layout_middle->setSpacing(0);
                layout_middle->setContentsMargins(0,0,0,30);
                layout_middle->addWidget(enter_world_button);
                layout->addLayout(layout_middle);

                layout->addWidget(help_button);
                layout->addSpacerItem(new QSpacerItem(1,1, QSizePolicy::Expanding));

                // Set geometry
                widget()->setMinimumHeight(86);
                widget()->setMaximumHeight(86);
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

        void ControlProxyWidget::UpdateGeometry(QRectF new_rect, qreal scale, bool do_fade)
        {
            if (type_ == CardControl)
            {
                if (do_fade)
                {
                    hide();
                    setOpacity(0);
                }

                // Calculate new geometry, set scale
                QSize overlay_size(490,449);
                int name_height = 50 * scale;
                int button_height = 58 * scale;
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
                
                if (scale < 0.5)
                    scale = 0.75;
                else if (scale > 0.5 && scale < 1.0)
                    scale = 1.0;

                move_animation_->setStartValue(pos());
                move_animation_->setEndValue(calculated_rect.topLeft());
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
