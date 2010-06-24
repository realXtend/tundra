// For conditions of distribution and use, see copyright notice in license.txt

#include "StableHeaders.h"
#include "BuildingWidget.h"

#include <QDebug>

namespace WorldBuilding
{
    namespace Ui
    {
        BuildingWidget::BuildingWidget(ToolPosition tool_position) :
            QGraphicsProxyWidget(0, Qt::Widget),
            internal_widget_(new QWidget()),
            tool_position_(tool_position),
            resizing_(false)
        {
        }

        BuildingWidget::~BuildingWidget()
        {
        }

        void BuildingWidget::PrepWidget()
        {
            setWidget(internal_widget_);
            min_width_ = internal_widget_->minimumWidth();
            
            internal_widget_->setMinimumWidth(min_width_);
            if (tool_position_ == Right && scene())
                setPos(scene()->sceneRect().width() - min_width_, 0);
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
            QGraphicsProxyWidget::hoverMoveEvent(mouse_hover_move_event);
        }

        void BuildingWidget::hoverLeaveEvent(QGraphicsSceneHoverEvent *mouse_hover_leave_event)
        {
            if (QApplication::overrideCursor())
                QApplication::restoreOverrideCursor();
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

        void BuildingWidget::mouseMoveEvent(QGraphicsSceneMouseEvent *mouse_move_event)
        {
            if (resizing_)
            {
                bool resized = false;
                QPointF scene_pos = mouse_move_event->scenePos();
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
                    }
                }
                else if (tool_position_ == Right)
                {
                    if (scene_pos.x() <= scene()->sceneRect().width() / 2)
                    {
                        if (QApplication::overrideCursor())
                            if (QApplication::overrideCursor()->shape() != Qt::ForbiddenCursor)
                                QApplication::setOverrideCursor(QCursor(Qt::ForbiddenCursor));
                        setPos(scene()->sceneRect().width() / 2, 0);
                        widget()->setMinimumWidth(scene()->sceneRect().width() / 2);
                    }
                    else
                    {
                        int width = scene()->sceneRect().width() - scene_pos.x();
                        if (width >= min_width_)
                        {
                            setPos(scene_pos.x(), 0);
                            widget()->setMinimumWidth(width);
                            resized = true;
                        }
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
            }
        }
    }
}
