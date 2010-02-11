// For conditions of distribution and use, see copyright notice in license.txt

#include "StableHeaders.h"
#include "ControlProxyWidget.h"

#include <QGraphicsDropShadowEffect>
#include <QParallelAnimationGroup>
#include <QPropertyAnimation>
#include <QGraphicsSceneMouseEvent>

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
            parent_->setStyleSheet("QWidget#containerWidget { background-color: transparent; }");
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
            }

            QGraphicsDropShadowEffect *shadow_effect = new QGraphicsDropShadowEffect(this);

            if (type_ == ActionControl)
            {
                QPointF offset_pos;
                if (direction_ == RightToLeft)
                    offset_pos = QPointF(-5,0);
                else
                    offset_pos = QPointF(5,0);
                shadow_effect->setBlurRadius(10);
                shadow_effect->setOffset(offset_pos);
                shadow_effect->setColor(QColor(255,255,255,100));
            }
            else
            {
                shadow_effect->setBlurRadius(5);
                shadow_effect->setOffset(0,3);
                shadow_effect->setColor(Qt::black);
            }
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
            text_label_->setFont(QFont("Narkisim", 18));
            text_label_->setAlignment(Qt::AlignCenter);
            text_label_->setMinimumWidth(200);

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
                widget_map_[text_label_] = BottomCenter;
                widget_map_[add_button] = TopRight;
                widget_map_[remove_button] = TopLeft;
                widget_map_[info_button] = BottomRight;
                widget_map_[question_button] = BottomLeft;
            }
            else if (direction_ == TopToBottom)
            {
                widget_map_[text_label_] = TopCenter;
                widget_map_[add_button] = BottomRight ;
                widget_map_[remove_button] = BottomLeft;
                widget_map_[info_button] = TopRight;
                widget_map_[question_button] = TopLeft;
            }

            foreach(QWidget *w, widget_map_.keys())
                w->show();
        }

        void ControlProxyWidget::InitActionWidgets()
        {
            QSize button_size(61,249);
            QString button_style = "QPushButton { padding: 0px; margin: 0px; background-color: transparent; border: 0px;";

            if (direction_ == RightToLeft)
            {
                // Buttons
                QPushButton *connect_button = new QPushButton(parent_);
                //connect_button->set
                connect_button->setStyleSheet(QString("%1 background-image: url('./data/ui/images/ether/control_right_2.png'); }").arg(button_style));
//                                                      "QPushButton::hover { background-image: url('./data/ui/images/ether/control_right_hover.png'); }"
//                                                      "QPushButton::pressed { background-image: url('./data/ui/images/ether/control_right_pressed.png'); }").arg(button_style));
                connect_button->setFlat(true);
                connect_button->resize(button_size);

                widget_map_[connect_button] = RightCenter;
            }
            else if (direction_ == LeftToRight)
            {

                QPushButton *exit_button = new QPushButton(parent_);
                exit_button->setStyleSheet(QString("%1 background-image: url('./data/ui/images/ether/control_left_2.png'); }").arg(button_style));
//                                                   "QPushButton::hover { background-image: url('./data/ui/images/ether/control_left_hover.png'); }"
//                                                   "QPushButton::pressed { background-image: url('./data/ui/images/ether/control_left_pressed.png'); }").arg(button_style));
                exit_button->setFlat(true);
                exit_button->resize(button_size);

                widget_map_[exit_button] = LeftCenter;
            }

            setGeometry(QRectF(QPointF(0,0), button_size));

            foreach(QWidget *w, widget_map_.keys())
                w->show();
        }

        void ControlProxyWidget::UpdateGeometry(QRectF rect)
        {
            if (isVisible())
                hide();

            qreal xoffset = 40;
            qreal yoffset = 3;
            rect.setWidth(rect.width() + xoffset * 2);
            rect.moveLeft(rect.left() - xoffset);
            setGeometry(rect);

            xoffset += yoffset;
            QPoint pos;
            foreach(QWidget *w, widget_map_.keys())
            {

                switch (widget_map_.value(w))
                {
                    case BottomCenter:
                        pos.setX(rect.width()/2 - w->width()/2);
                        pos.setY(rect.height() - 35);
                        break;
                    case TopCenter:
                        pos.setX(rect.width()/2 - w->width()/2);
                        pos.setY(10);
                        break;
                    case LeftCenter:
                    case RightCenter:
                        pos = QPoint(0,0);
                        break;
                    case BottomLeft:
                        if (direction_== TopToBottom)
                        {
                            pos.setX(0);
                            pos.setY(rect.height() - w->height());
                        }
                        else
                        {
                            pos.setX(xoffset);
                            pos.setY(rect.height() - w->height() - yoffset);
                        }
                        break;
                    case BottomRight:
                        if (direction_ == TopToBottom)
                        {
                            pos.setX(rect.width() - w->width());
                            pos.setY(rect.height() - w->height());
                        }
                        else
                        {
                            pos.setX(rect.width() - w->width() - xoffset);
                            pos.setY(rect.height() - w->height() - yoffset);
                        }
                        break;
                    case TopRight:
                        if (direction_ == TopToBottom)
                        {
                            pos.setX(rect.width() - w->width() - xoffset);
                            pos.setY(yoffset);
                        }
                        else
                        {
                            pos.setX(rect.width() - w->width());
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
            UpdateGeometry(controlled_card_->mapRectToScene(controlled_card_->boundingRect()));
            show();
        }

        void ControlProxyWidget::InfoHandler()
        {
            qDebug()<< "ControlProxyWidget::InfoHandler()";
        }

        void ControlProxyWidget::QuestionHandler()
        {
            qDebug()<< "ControlProxyWidget::QuestionHandler()";
        }

        void ControlProxyWidget::AddHandler()
        {
            qDebug()<< "ControlProxyWidget::AddHandler()";
        }

        void ControlProxyWidget::RemoveHandler()
        {
            qDebug()<< "ControlProxyWidget::RemoveHandler()";
        }
    }
}
