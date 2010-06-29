// For conditions of distribution and use, see copyright notice in license.txt

#ifndef incl_WorldBuildingModule_BuildingWidget_h
#define incl_WorldBuildingModule_BuildingWidget_h

#include <QGraphicsProxyWidget>
#include <QGraphicsSceneMouseEvent>
#include <QGraphicsSceneHoverEvent>

namespace WorldBuilding
{
    namespace Ui
    {
        class BuildingWidget : public QGraphicsProxyWidget
        {
        
        Q_OBJECT

        public:
            enum ToolPosition { Left, Right };

            BuildingWidget(ToolPosition tool_position);
            virtual ~BuildingWidget();          

        protected:
            void hoverEnterEvent(QGraphicsSceneHoverEvent *mouse_hover_enter_event);
            void hoverMoveEvent(QGraphicsSceneHoverEvent *mouse_hover_move_event);
            void hoverLeaveEvent(QGraphicsSceneHoverEvent *mouse_hover_leave_event);
            void mousePressEvent(QGraphicsSceneMouseEvent *mouse_press_event);
            void mouseMoveEvent(QGraphicsSceneMouseEvent *mouse_move_event);
            void mouseReleaseEvent(QGraphicsSceneMouseEvent *mouse_release_event);

        public slots:
            void PrepWidget();
            void CheckSize();
            QWidget *GetInternal() { return internal_widget_; }

        private slots:
            void SceneRectChanged(const QRectF &new_rect);
        
        private:
            QWidget *internal_widget_;
            ToolPosition tool_position_;
            bool resizing_;
            int min_width_;

        };
    }
}
#endif
