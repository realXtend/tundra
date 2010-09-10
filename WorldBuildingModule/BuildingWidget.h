// For conditions of distribution and use, see copyright notice in license.txt

#ifndef incl_WorldBuildingModule_BuildingWidget_h
#define incl_WorldBuildingModule_BuildingWidget_h

#include <QGraphicsProxyWidget>
#include <QGraphicsSceneMouseEvent>
#include <QGraphicsSceneHoverEvent>
#include <QParallelAnimationGroup>
#include <QPropertyAnimation>
#include <QPushButton>

namespace WorldBuilding
{
    class WorldObjectView;

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
            void SetVisibilityButton(QPushButton *button);
            QWidget *GetInternal() { return internal_widget_; }
            void SetWorldObjectView(WorldObjectView* view);
            void ToggleVisibility();

        private slots:
            void SetWidth(int width);
            void SceneRectChanged(const QRectF &new_rect);
            void OnStartAnimation();
            void OnFinishedAnimation();

        private:
            QWidget *internal_widget_;
            WorldObjectView *view_;            
            ToolPosition tool_position_;
            
            bool resizing_;
            bool scrolled_to_side_;
            
            int min_width_;
            int last_width_;

            QPushButton *visib_button_;
            QParallelAnimationGroup *animations_;           
        };
    }
}
#endif
