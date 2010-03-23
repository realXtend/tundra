// For conditions of distribution and use, see copyright notice in license.txt

#ifndef incl_UiModule_MenuNode_h
#define incl_UiModule_MenuNode_h

#include <QGraphicsProxyWidget>
#include <QParallelAnimationGroup>
#include <QUuid>
#include <QPushButton>
#include <QString>
#include <QPointF>

#include "ui_MenuNode.h"

#define MOVE_ANIM_LENGTH 400
#define RESIZE_ANIM_LENGTH 200
#define ADJUST_ANIM_LENGTH 150

namespace CoreUi
{
    class GroupNode;

    class MenuNode : public QGraphicsProxyWidget, protected Ui::MenuNode
    {

    Q_OBJECT

    public:
        MenuNode(const QString& node_name, QUuid id = QUuid());

    public slots:
        QPushButton *GetMenuButton();
        QUuid GetID();

        void EnableText();
        void DisableText();
        void SetTreeDepth(int tree_depth)   { tree_depth_ = tree_depth; }
        int GetTreeDepth()                  { return tree_depth_; }
        int GetExpandedWidth()              { return expanded_width_; }

        QPointF GetShrunkenPos(){ return shrunken_pos_; }
        void SetShrunkenPos(QPointF shrunken_pos){ shrunken_pos_ = shrunken_pos; }

        QPointF GetExpandedPos(){ return expanded_pos_; }
        void SetExpandedPos(QPointF expanded_pos){ expanded_pos_ = expanded_pos; }

        QPointF GetOriginalPos(){ return original_pos_; }
        void SetOriginalPos(QPointF original_pos){ original_pos_  = original_pos; }


        // Reimplement in subclass
        virtual void NodeClicked() = 0;
        virtual void AddChildNode(MenuNode *node) = 0;

    protected:
        QString node_name_;
        QString base_stylesheet_;

        QUuid id_;
        QWidget *widget_;

        QPointF original_pos_;
        QPointF expanded_pos_;
        QPointF shrunken_pos_;

        int tree_depth_;
        int expanded_width_;

    signals:
        void ActionButtonClicked(QUuid);
        void NodeGroupClicked(GroupNode *clicked_node, QParallelAnimationGroup *move_animations, QParallelAnimationGroup *size_animations);

    };
}

#endif
