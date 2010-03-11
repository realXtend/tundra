// For conditions of distribution and use, see copyright notice in license.txt

#ifndef incl_UiModule_GroupNode_h
#define incl_UiModule_GroupNode_h

#include "MenuNode.h"

#include <QList>
#include <QVector2D>
#include <QParallelAnimationGroup>
#include <QGraphicsItem>

class QPropertyAnimation;

namespace CoreUi
{
    class GroupNode : public MenuNode
    { 

    Q_OBJECT

    public:
        //! Constructor
        GroupNode(bool root, const QString &name, qreal gap = 0, qreal vgap = 0);

        //! Setters (QGraphicsProxyWidget overrides)
        void setPos(const QPointF &pos);
        void setPos(qreal x, qreal y);

    public slots:
        //! Parent overrides
        void NodeClicked();
        void AddChildNode(MenuNode *node);
        MenuNode *RemoveChildNode(QUuid child_id);

        //! Getters
        QVector2D GetPosVec()                           { return pos_vector_; }
        QList<MenuNode *> GetChildNodeList()            { return children_; }
        QParallelAnimationGroup *GetMoveAnimations()    { return move_animations_; }
        QParallelAnimationGroup *GetResizeAnimations()  { return resize_animations_; }
        bool IsExpanded()                               { return is_expanded_; }

    private slots:
        QPropertyAnimation* CalculateSizeAnimation(MenuNode *child_node);
        void CalculatePosVec();
        void CalculateChildPositions();
        void MoveAnimationsFinished();
        void ResizeAnimationsFinished();

    private:
        //Move animations to move objects into their place
        QParallelAnimationGroup *move_animations_;

        QParallelAnimationGroup *resize_animations_;

        QParallelAnimationGroup *adjust_position_animations_;

        QVector2D pos_vector_;
        QList<MenuNode *> children_;

        bool root_;
        bool is_expanded_;
        qreal hgap_;
        qreal vgap_;

    };
}

#endif