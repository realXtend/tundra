// For conditions of distribution and use, see copyright notice in license.txt

#include "StableHeaders.h"
#include "DebugOperatorNew.h"

#include "GroupNode.h"

#include <math.h>

#include <QDebug>
#include <QSizeF>
#include <QPropertyAnimation>
#include <QGraphicsItem>
#include "MemoryLeakCheck.h"

#define M_PI_2 1.57079632679489661923

namespace CoreUi
{
    GroupNode::GroupNode(bool root, const QString& name, qreal hgap, qreal vgap, UiDefines::MenuNodeStyleMap style_map) :
        MenuNode(name, QIcon(), style_map),
        move_animations_(new QParallelAnimationGroup(this)),
        resize_animations_(new QParallelAnimationGroup(this)),
        adjust_position_animations_(new QParallelAnimationGroup(this)),
        pos_vector_(0,0),
        child_size_(0,0),
        root_(root),
        hgap_(hgap),
        vgap_(vgap),
        is_expanded_(false),
        group_stylesheet("")
    {
        // RENEW THIS
        //connect(GetMenuButton(), SIGNAL( clicked() ), SLOT( NodeClicked() )); 
        connect(move_animations_, SIGNAL( finished() ), SLOT( MoveAnimationsFinished() ));
        connect(resize_animations_, SIGNAL( finished() ), SLOT( ResizeAnimationsFinished() ));

        if (root_)
        {
            SetTreeDepth(0);
            leftContainer->hide();
            rightContainer->hide();
            widget_->setMinimumSize(101,103);
            widget_->setMaximumSize(101,103);
            centerContainer->setMinimumSize(101,103);
            centerContainer->setMaximumSize(101,103);
        }
        else
            SetTreeDepth(-1);
    }

    GroupNode::~GroupNode()
    {
        qDeleteAll(children_);
    }

    void GroupNode::AdjustNode(QAbstractAnimation::Direction dir)
    {
        resize_animations_->setDirection(dir);
        adjust_position_animations_->setDirection(dir);
        resize_animations_->start();
        adjust_position_animations_->start();

        foreach (MenuNode *child_node, children_)
        {
            child_node->setOpacity(1);
        }
    }

    //! Overrides

    void GroupNode::NodeClicked()
    {
        // Check if animations are already running
        if (GetTreeDepth() == 0)
            if (move_animations_->state() == QAbstractAnimation::Running ||
                resize_animations_->state() == QAbstractAnimation::Running)
                return;

        setOpacity(1);

        // Set correct direction
        move_animations_->setDirection(QAbstractAnimation::Forward);
        resize_animations_->setDirection(QAbstractAnimation::Forward);

        // Setup children for animations
        foreach (MenuNode *child_node, children_)
        {
            child_node->setOpacity(1);
            child_node->setZValue(zValue()-0.001f);
            child_node->show();
            if (!IsExpanded())
            {
                child_node->setPos(pos());
                child_node->ChangeMoveState(false);
            }
        }
        
        emit NodeGroupClicked(this, move_animations_, resize_animations_);
    }

    void GroupNode::AddChildNode(MenuNode *node)
    {
        children_.append(node);

        if(child_size_.isNull())
            child_size_ = node->size();

        node->show();
        node->setParent(this);
        node->SetTreeDepth(GetTreeDepth()+1);

        CalculateChildPositions();
    }

    MenuNode *GroupNode::RemoveChildNode(QUuid child_id)
    {
        MenuNode *found_node = 0;
        foreach (MenuNode *node, children_)
        {
            if (node->GetID() == child_id)
            {
                found_node = node;
                break;
            }
        }
        if (!found_node)
            return 0;

        if (children_.removeOne(found_node))
        {
            found_node->setParent(0);
            CalculateChildPositions();
            return found_node;
        }
        return 0;
    }

    // Private functions

    QPropertyAnimation* GroupNode::CalculateSizeAnimation(MenuNode *child_node)
    {
        //QSizeF anim_size = child_size_;
        //QPropertyAnimation *size_anim = child_node->resize_animation_;
        //// Set start size
        //size_anim->setStartValue(anim_size);
        //// Set end size
        //anim_size.setWidth(anim_size.width() + child_node->GetExpandedWidth());
        //size_anim->setEndValue(anim_size);
        //// Setup animation
        //size_anim->setDuration(RESIZE_ANIM_LENGTH);
        //size_anim->setEasingCurve(QEasingCurve::InOutSine);
        return 0;
    }

    void GroupNode::CalculatePosVec()
    {
        QVector2D addition(size().width(), size().height());
        pos_vector_ = addition;
        addition.normalize();
        pos_vector_ += addition * hgap_;
        if (!root_)
            pos_vector_ += dynamic_cast<GroupNode*>(parent())->GetPosVec();
    }

    void GroupNode::CalculateChildPositions()
    {
        if(pos_vector_.isNull())
            CalculatePosVec();

        adjust_position_animations_->clear();
        resize_animations_->clear();
        move_animations_->clear();

        qreal rad = pos_vector_.length();
        qreal cheight = 0;
        qreal cwidth = 0;

        cheight = child_size_.height();
        cwidth = child_size_.width();

        int index = 0;
        
        int number_of_visible_objects = rad/cheight;
        qreal phase_change = M_PI_2/number_of_visible_objects;

        QPointF top_midway_point;

        foreach (MenuNode *child_node, children_)
        {
            QPointF expanded_end_position;
            QPointF shrunken_end_position;

            qreal expanded_y_pos = index*(cheight + vgap_);
            qreal expanded_x_pos = rad*cos(asin((index*(cheight))/rad));

            qreal shrunken_y_pos = rad*sin(phase_change*index);
            qreal shrunken_x_pos = rad*cos(phase_change*index);

            expanded_end_position.setX(expanded_x_pos);
            expanded_end_position.setY(expanded_y_pos);
            shrunken_end_position.setX(shrunken_x_pos);
            shrunken_end_position.setY(shrunken_y_pos);

            child_node->SetExpandedPos(expanded_end_position);
            child_node->SetShrunkenPos(shrunken_end_position);
            child_node->SetOriginalPos(pos());

            if (top_midway_point.isNull())
                top_midway_point = shrunken_end_position;

            child_node->setPos(shrunken_end_position);
            child_node->hide();

            GroupNode* gn = dynamic_cast<GroupNode*>(child_node);
            if(gn)
                gn->CalculatePosVec();

            // Move animations to move objects into their place
            QPropertyAnimation *anim = new QPropertyAnimation(child_node, "pos", move_animations_);
            anim->setStartValue(pos());
            anim->setKeyValueAt(0.1, QPointF(pos().x() + cwidth, pos().y()));
            anim->setKeyValueAt(0.5, top_midway_point);
            anim->setEndValue(expanded_end_position);
            anim->setDuration(MOVE_ANIM_LENGTH);
            anim->setEasingCurve(QEasingCurve::InOutSine);
            move_animations_->addAnimation(anim);

            anim = new QPropertyAnimation(child_node, "pos");
            anim->setStartValue(shrunken_end_position);
            anim->setEndValue(expanded_end_position);
            anim->setDuration(ADJUST_ANIM_LENGTH);
            adjust_position_animations_->addAnimation(anim);

            // Adjusting move animations that will just move object to its expanded/shrunken position
            anim = child_node->CreateResizeAnimation("minimumWidth");
            resize_animations_->addAnimation(anim);
            anim = child_node->CreateResizeAnimation("maximumWidth");
            resize_animations_->addAnimation(anim);
            index++;
        }
    }

    void GroupNode::MoveAnimationsFinished()
    {
        if (move_animations_->direction() == QAbstractAnimation::Forward)
        {
            is_expanded_ = true;
            foreach (MenuNode *child_node, children_)
            {
                child_node->ChangeMoveState(true);
            }
        }
        else if (move_animations_->direction() == QAbstractAnimation::Backward)
        {
            is_expanded_ = false;
            foreach (MenuNode *child_node, children_)
            {
                child_node->hide();
                child_node->setZValue(zValue()+0.001f);
                //child_node->resize(35,35);
            }
        }
    }

    void GroupNode::ResizeAnimationsFinished()
    {
        if (resize_animations_->direction() == QAbstractAnimation::Forward)
        {
            foreach (MenuNode* child_node, children_)
            {
                //child_node->EnableText();
                //child_node->GetMenuButton()->setStyleSheet(base_stylesheet_ + group_stylesheet + "background-color: rgba(255,255,255,50);");
            }
        }
        else if (resize_animations_->direction() == QAbstractAnimation::Backward)
        {
            foreach (MenuNode* child_node, children_)
            {
                //child_node->ChangeMoveState(false);
            }
            if (IsExpanded())
            {
                adjust_position_animations_->setDirection(QAbstractAnimation::Backward);
                adjust_position_animations_->start();
            }
        }
    }
}