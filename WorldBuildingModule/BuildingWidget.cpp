// For conditions of distribution and use, see copyright notice in license.txt

#include "StableHeaders.h"
#include "BuildingWidget.h"
#include "WorldObjectView.h"

#include <QDebug>

namespace WorldBuilding
{
    namespace Ui
    {
        BuildingWidget::BuildingWidget(ToolPosition tool_position) :
            QGraphicsProxyWidget(0, Qt::Widget),
            internal_widget_(new QWidget()),
            tool_position_(tool_position),
            resizing_(false),
            scrolled_to_side_(false),
            view_(0)
        {
            QPropertyAnimation *animation_max = new QPropertyAnimation(internal_widget_, "maximumWidth");
            animation_max->setDuration(300);
            animation_max->setEasingCurve(QEasingCurve::InOutSine);
            QPropertyAnimation *animation_min = new QPropertyAnimation(internal_widget_, "minimumWidth");
            animation_min->setDuration(300);
            animation_min->setEasingCurve(QEasingCurve::InOutSine);
            
            animations_ = new QParallelAnimationGroup();
            animations_->addAnimation(animation_max);
            animations_->addAnimation(animation_min);
            connect(animations_, SIGNAL(finished()), SLOT(OnFinishedAnimation()));
        }

        BuildingWidget::~BuildingWidget()
        {
        }

        void BuildingWidget::PrepWidget()
        {
            setWidget(internal_widget_);
            min_width_ = internal_widget_->minimumWidth();
            last_width_ = min_width_;
        }
        
        void BuildingWidget::SetVisibilityButton(QPushButton *button)
        {
            visib_button_ = button;
        }

        void BuildingWidget::CheckSize()
        {
            int current_min_width = widget()->minimumWidth();
            if (current_min_width < min_width_)
            {
                QRect w_rect = widget()->rect();
                if (tool_position_ == Right && scene())
                    setPos(scene()->sceneRect().width() - min_width_, 0);
                SetWidth(min_width_);
                if (view_)
                {
                    view_->setMinimumWidth(10);
                    view_->setMaximumWidth(min_width_-25);
                }
            }
        }

        void BuildingWidget::hoverEnterEvent(QGraphicsSceneHoverEvent *mouse_hover_enter_event)
        {
            QGraphicsProxyWidget::hoverEnterEvent(mouse_hover_enter_event);
        }

        void BuildingWidget::hoverMoveEvent(QGraphicsSceneHoverEvent *mouse_hover_move_event)
        {
            QPointF m_pos = mouse_hover_move_event->pos();
            bool change_cursor = false;
            if (tool_position_ == Left)
            {
                if (m_pos.x() >= size().width()-5 && m_pos.x() <= size().width())
                    change_cursor = true;
            }
            else if (tool_position_ == Right)
            {
                if (m_pos.x() >= 0 && m_pos.x() <= 5)
                    change_cursor = true;
            }

            if (!scrolled_to_side_)
            {
                if (change_cursor && !QApplication::overrideCursor())
                    QApplication::setOverrideCursor(QCursor(Qt::SizeHorCursor));
                else if (!change_cursor && QApplication::overrideCursor())
                    QApplication::restoreOverrideCursor();
            }
            
            if (view_)
            {
                QPointF widget_pos = mouse_hover_move_event->pos();
                QCursor *current_cursor = QApplication::overrideCursor();
                if (widget()->childAt(widget_pos.toPoint()) == view_)
                {
                    if (current_cursor == 0)
                        QApplication::setOverrideCursor(Qt::PointingHandCursor);
                    else if (current_cursor->shape() != Qt::PointingHandCursor)
                        QApplication::changeOverrideCursor(Qt::PointingHandCursor);
                }
                else if (current_cursor)
                {
                    if (current_cursor->shape() == Qt::PointingHandCursor)
                        QApplication::restoreOverrideCursor();
                }
            }
            QGraphicsProxyWidget::hoverMoveEvent(mouse_hover_move_event);
        }

        void BuildingWidget::hoverLeaveEvent(QGraphicsSceneHoverEvent *mouse_hover_leave_event)
        {
            QCursor *stack_cursor = QApplication::overrideCursor();
            while (stack_cursor)
            {
                QApplication::restoreOverrideCursor();
                stack_cursor = QApplication::overrideCursor();
            }
            QGraphicsProxyWidget::hoverLeaveEvent(mouse_hover_leave_event);
        }

        void BuildingWidget::mousePressEvent(QGraphicsSceneMouseEvent *mouse_press_event)
        {
            if (QApplication::overrideCursor())
            {
                if (!resizing_ && !scrolled_to_side_ && QApplication::overrideCursor()->shape() == Qt::SizeHorCursor)
                {
                    resizing_ = true;
                    mouse_press_event->accept();
                    return;
                }
            }
            QGraphicsProxyWidget::mousePressEvent(mouse_press_event);
        }

        void BuildingWidget::SetWorldObjectView(WorldObjectView* view)
        {
            view_ = view;
        }

        void BuildingWidget::mouseMoveEvent(QGraphicsSceneMouseEvent *mouse_move_event)
        {
            QPointF scene_pos = mouse_move_event->scenePos();
            if (resizing_)
            {
                bool resized = false;
               
                if (tool_position_ == Left)
                {
                    if (scene_pos.x() >= scene()->sceneRect().width() / 2)
                    {
                        if (QApplication::overrideCursor())
                            if (QApplication::overrideCursor()->shape() != Qt::ForbiddenCursor)
                                QApplication::setOverrideCursor(QCursor(Qt::ForbiddenCursor));
                        SetWidth(scene()->sceneRect().width() / 2);
                    }
                    else
                    {
                        if (scene_pos.x() >= min_width_)
                        {
                            SetWidth(scene_pos.x());
                            resized = true;
                        }
                        else
                        {
                            SetWidth(min_width_);
                            if (QApplication::overrideCursor())
                                if (QApplication::overrideCursor()->shape() != Qt::ForbiddenCursor)
                                    QApplication::setOverrideCursor(QCursor(Qt::ForbiddenCursor));
                        }
                    }
                }
                else if (tool_position_ == Right)
                {
                    if (scene_pos.x() <= scene()->sceneRect().width() / 2)
                    {
                        if (QApplication::overrideCursor())
                            if (QApplication::overrideCursor()->shape() != Qt::ForbiddenCursor)
                                QApplication::setOverrideCursor(QCursor(Qt::ForbiddenCursor));
                        SetWidth(scene()->sceneRect().width() / 2);
                        setPos(scene()->sceneRect().width() / 2, 0);
                    }
                    else
                    {
                        int width = scene()->sceneRect().width() - scene_pos.x();
                        if (width >= min_width_)
                        {
                            SetWidth(width);
                            setPos(scene_pos.x(), 0);
                            resized = true;
                        }
                        else
                        {
                            SetWidth(min_width_);
                            setPos(scene()->sceneRect().width() - min_width_, 0);
                            if (QApplication::overrideCursor())
                                if (QApplication::overrideCursor()->shape() != Qt::ForbiddenCursor)
                                    QApplication::setOverrideCursor(QCursor(Qt::ForbiddenCursor));
                        }
                        if (view_)
                            view_->setMaximumWidth(widget()->minimumWidth()-25);
                    }
                }

                if (resized && QApplication::overrideCursor())
                    if (QApplication::overrideCursor()->shape() == Qt::ForbiddenCursor)
                        QApplication::restoreOverrideCursor();
            }
            QGraphicsProxyWidget::mouseMoveEvent(mouse_move_event);
        }

        void BuildingWidget::mouseReleaseEvent(QGraphicsSceneMouseEvent *mouse_release_event)
        {
            resizing_= false;
            if (QApplication::overrideCursor())
                QApplication::restoreOverrideCursor();
            QGraphicsProxyWidget::mouseReleaseEvent(mouse_release_event);
        }

        void BuildingWidget::SetWidth(int width)
        {
            widget()->setMinimumWidth(width);
            widget()->setMaximumWidth(width);
        }

        void BuildingWidget::SceneRectChanged(const QRectF &new_rect)
        {
            int scene_half_width = new_rect.width() / 2;
            if (widget()->minimumWidth() > scene_half_width)
            {
                if (tool_position_ == Right)
                    setPos(scene_half_width, 0);
                SetWidth(scene_half_width);
                if (view_)
                    view_->setMaximumWidth(scene_half_width-25);
            }
        }

        void BuildingWidget::ToggleVisibility()
        {
            OnStartAnimation();
        }

        void BuildingWidget::OnStartAnimation()
        {
            if (animations_->state() == QAbstractAnimation::Running)
                return;
            
            // Get the min/max animations
            QPropertyAnimation *max = dynamic_cast<QPropertyAnimation*>(animations_->animationAt(0));
            QPropertyAnimation *min = dynamic_cast<QPropertyAnimation*>(animations_->animationAt(1));
            if (!max || !min)
                return;

            // Start value
            max->setStartValue(internal_widget_->width());
            min->setStartValue(internal_widget_->width());
            if (!scrolled_to_side_)
                last_width_ = internal_widget_->width();

            // End value
            int end_width = 0;
            if (scrolled_to_side_)
                end_width = last_width_;
            max->setEndValue(end_width);
            min->setEndValue(end_width);

            // Start
            animations_->start();
        }

        void BuildingWidget::OnFinishedAnimation()
        {
            if (!visib_button_)
                return;

            // Change button style accordingly
            scrolled_to_side_ = !scrolled_to_side_;
            if (scrolled_to_side_)
            {
                if (tool_position_ == Right)
                    visib_button_->setStyleSheet("QPushButton { background-color: transparent; background-image: url('./data/ui/images/worldbuilding/draw-arrow-back.png'); } QPushButton::hover { background-color: transparent; background-image: url('./data/ui/images/worldbuilding/draw-arrow-back_bright.png'); } QPushButton::pressed { background-color: transparent; border: 0px; }");
                else
                    visib_button_->setStyleSheet("QPushButton { background-color: transparent; background-image: url('./data/ui/images/worldbuilding/draw-arrow-forward.png'); } QPushButton::hover { background-color: transparent; background-image: url('./data/ui/images/worldbuilding/draw-arrow-forward_bright.png'); } QPushButton::pressed { background-color: transparent; border: 0px; }");
            }
            else
            {
                if (tool_position_ == Right)
                    visib_button_->setStyleSheet("QPushButton { background-color: transparent; background-image: url('./data/ui/images/worldbuilding/draw-arrow-forward.png'); } QPushButton::hover { background-color: transparent; background-image: url('./data/ui/images/worldbuilding/draw-arrow-forward_bright.png'); } QPushButton::pressed { background-color: transparent; border: 0px; }");    
                else
                    visib_button_->setStyleSheet("QPushButton { background-color: transparent; background-image: url('./data/ui/images/worldbuilding/draw-arrow-back.png'); } QPushButton::hover { background-color: transparent; background-image: url('./data/ui/images/worldbuilding/draw-arrow-back_bright.png'); } QPushButton::pressed { background-color: transparent; border: 0px; }");
            }
        }
    }
}
