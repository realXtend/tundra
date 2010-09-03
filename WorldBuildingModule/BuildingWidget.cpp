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
            view_(0)
        {
        }

        BuildingWidget::~BuildingWidget()
        {
        }

        void BuildingWidget::PrepWidget()
        {
            setWidget(internal_widget_);
            min_width_ = internal_widget_->minimumWidth(); // 15px for toolbar
        }

        void BuildingWidget::CheckSize()
        {
            int current_min_width = widget()->minimumWidth();
            if (current_min_width < min_width_)
            {
                QRect w_rect = widget()->rect();
                if (tool_position_ == Right && scene())
                    setPos(scene()->sceneRect().width() - min_width_, 0);
                widget()->setMinimumWidth(min_width_);
                widget()->setGeometry(w_rect.x(), w_rect.y(), min_width_, w_rect.height());
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

            if (change_cursor && !QApplication::overrideCursor())
                QApplication::setOverrideCursor(QCursor(Qt::SizeHorCursor));
            else if (!change_cursor && QApplication::overrideCursor())
                QApplication::restoreOverrideCursor();
            
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
                if (!resizing_ && QApplication::overrideCursor()->shape() == Qt::SizeHorCursor)
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
                        widget()->setMinimumWidth(scene()->sceneRect().width() / 2);
                    }
                    else
                    {
                        if (scene_pos.x() >= min_width_)
                        {
                            widget()->setMinimumWidth(scene_pos.x());
                            resized = true;
                        }
                        else
                        {
                            widget()->setMinimumWidth(min_width_);
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
                        widget()->setMinimumWidth(scene()->sceneRect().width() / 2);
                        setPos(scene()->sceneRect().width() / 2, 0);
                    }
                    else
                    {
                        int width = scene()->sceneRect().width() - scene_pos.x();
                        if (width >= min_width_)
                        {
                            widget()->setMinimumWidth(width);
                            setPos(scene_pos.x(), 0);
                            resized = true;
                        }
                        else
                        {
                            widget()->setMinimumWidth(min_width_);
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

        void BuildingWidget::SceneRectChanged(const QRectF &new_rect)
        {
            int scene_half_width = new_rect.width() / 2;
            if (widget()->minimumWidth() > scene_half_width)
            {
                if (tool_position_ == Right)
                    setPos(scene_half_width, 0);
                widget()->setMinimumWidth(scene_half_width);
                if (view_)
                    view_->setMaximumWidth(scene_half_width-25);
            }
        }
    }
}
