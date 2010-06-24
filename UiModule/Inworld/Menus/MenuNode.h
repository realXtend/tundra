// For conditions of distribution and use, see copyright notice in license.txt

#ifndef incl_UiModule_MenuNode_h
#define incl_UiModule_MenuNode_h

#include <QGraphicsProxyWidget>
#include <QPropertyAnimation>
#include <QUuid>
#include <QString>
#include <QPointF>
#include <QMap>

#include "UiDefines.h"
#include "ui_MenuNode.h"

#define MOVE_ANIM_LENGTH 350
#define RESIZE_ANIM_LENGTH 200
#define ADJUST_ANIM_LENGTH 150

class QParallelAnimationGroup;
class QPropertyAnimation;

namespace CoreUi
{
    class GroupNode;

    class MenuNode : public QGraphicsProxyWidget, protected Ui::MenuNode
    {
        Q_OBJECT

    public:
        MenuNode(const QString& node_name, const QIcon &icon, UiDefines::MenuNodeStyleMap map, QUuid id = QUuid());
        ~MenuNode();

    public slots:
        QUuid GetID() const;

        void SetTreeDepth(int tree_depth)   { tree_depth_ = tree_depth; }
        int GetTreeDepth() const { return tree_depth_; }
        int GetExpandedWidth() const { return expanded_width_; }

        QPointF GetShrunkenPos() const { return shrunken_pos_; }
        void SetShrunkenPos(QPointF shrunken_pos){ shrunken_pos_ = shrunken_pos; }

        QPointF GetExpandedPos() const { return expanded_pos_; }
        void SetExpandedPos(QPointF expanded_pos){ expanded_pos_ = expanded_pos; }

        QPointF GetOriginalPos() const { return original_pos_; }
        void SetOriginalPos(QPointF original_pos){ original_pos_  = original_pos; }

        QPropertyAnimation *CreateResizeAnimation(QString anim_property);

        void ResizeStateChanged(QAbstractAnimation::State new_state, QAbstractAnimation::State old_state);
        void ResizeFinished();
        void ChangeMoveState(bool show_borders);
        
        // Reimplement in subclass
        virtual void NodeClicked() = 0;
        virtual void AddChildNode(MenuNode *node) = 0;

    protected:
        enum Style
        {
            Normal,
            Hover,
            Pressed
        };

        void hoverEnterEvent(QGraphicsSceneHoverEvent *enter_event);
        void hoverLeaveEvent(QGraphicsSceneHoverEvent *leave_event);
        void mousePressEvent(QGraphicsSceneMouseEvent *press_event);
        void mouseReleaseEvent(QGraphicsSceneMouseEvent *release_event);
        
        void ChangeStyle(Style style);

        QString node_name_;
        QString base_stylesheet_;

        QUuid id_;
        QWidget *widget_;
        QIcon icon_;

        QPointF original_pos_;
        QPointF expanded_pos_;
        QPointF shrunken_pos_;

        int tree_depth_;
        int expanded_width_;
        int center_image_width_;

        UiDefines::MenuNodeStyleMap style_to_path_map_;
        QPropertyAnimation *resize_animation_;

    signals:
        void ActionButtonClicked(QUuid);
        void NodeGroupClicked(GroupNode *clicked_node, QParallelAnimationGroup *move_animations, QParallelAnimationGroup *size_animations);
    };
}

#endif
