// For conditions of distribution and use, see copyright notice in license.txt

#ifndef incl_UiModule_GroupNode_h
#define incl_UiModule_GroupNode_h

#include "MenuNode.h"

#include <QList>
#include <QVector2D>
#include <QParallelAnimationGroup>
#include <QGraphicsItem>
#include <QSizeF>

class QPropertyAnimation;

namespace CoreUi
{
    class GroupNode : public MenuNode
    {
        Q_OBJECT

    public:
        //! Constructor
        GroupNode(bool root, const QString &name, const QString &icon = "", qreal gap = 0, qreal vgap = 0);

        //! Destructor.
        ~GroupNode();

    public slots:
        //! Parent overrides
        void NodeClicked();
        void AddChildNode(MenuNode *node);
        MenuNode *RemoveChildNode(const QUuid &child_id);

        //! Getters
        QVector2D GetPosVec() const { return pos_vector_; }
        QList<MenuNode *> GetChildNodeList() const { return children_; }
        QParallelAnimationGroup *GetMoveAnimations() const { return move_animations_; }
        QParallelAnimationGroup *GetResizeAnimations() const { return resize_animations_; }
        bool IsExpanded() const { return is_expanded_; }
        void CalculatePosVec();
        void AdjustNode(QAbstractAnimation::Direction dir);

        /// Returns number of children this node has.
        int ChildCount() const { return children_.size(); }

        /// Sorts the children: groups before actions.
        void Sort();

    private slots:
        QPropertyAnimation* CalculateSizeAnimation(MenuNode *child_node);
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

        QSizeF child_size_;

        bool root_;
        bool is_expanded_;
        qreal hgap_;
        qreal vgap_;

        QString group_stylesheet;

    };
}

#endif