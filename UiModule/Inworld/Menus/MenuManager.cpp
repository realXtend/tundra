// For conditions of distribution and use, see copyright notice in license.txt

#include "StableHeaders.h"
#include "MenuManager.h"
#include "Common/AnchorLayoutManager.h"

#include <QSequentialAnimationGroup>
#include <QPropertyAnimation>
#include <QTimer>
#include <QDebug>

namespace CoreUi
{
    MenuManager::MenuManager(QObject *parent, CoreUi::AnchorLayoutManager *layout_manager) :
            QObject(parent),
            layout_manager_(layout_manager),
            last_clicked_node_(0),
            next_move_animations_(0),
            last_resize_animations_(0),
            ongoing_animations_(false)
    {
        InitInternals();
    }

    void MenuManager::InitInternals()
    {
        // Create the root menu
        root_menu_ = new GroupNode(true, "", 30, 5);
        category_map_["Root"] = root_menu_;
        layout_manager_->AddCornerAnchor(root_menu_, Qt::TopLeftCorner, Qt::TopLeftCorner);
        connect(root_menu_, SIGNAL(NodeGroupClicked(GroupNode*, QParallelAnimationGroup*, QParallelAnimationGroup*)),
                SLOT(GroupNodeClicked(GroupNode*, QParallelAnimationGroup *, QParallelAnimationGroup *)));

        // Add submenus
        QStringList group_list;
        group_list << "Personal" << "Building";
        foreach(QString group_name, group_list)
            AddMenuGroup(group_name, 10, 5);
    }

    void MenuManager::AddMenuGroup(QString name, qreal hgap, qreal vgap)
    {
        GroupNode *group_node = new GroupNode(false, name, hgap, vgap);
        root_menu_->AddChildNode(group_node);
        category_map_[name] = group_node;
        layout_manager_->AddItemToScene(group_node);

        connect(group_node, SIGNAL(NodeGroupClicked(GroupNode*, QParallelAnimationGroup*, QParallelAnimationGroup*)),
                SLOT(GroupNodeClicked(GroupNode*, QParallelAnimationGroup *, QParallelAnimationGroup *)));
    }

    void MenuManager::AddMenuItem(Category category, QGraphicsProxyWidget *controlled_widget, QString button_title)
    {
        ActionNode *child_node = new ActionNode(button_title);
        switch (category)
        {
            case Root:
               category_map_["Root"]->AddChildNode(child_node);
               break;
            case Personal:
               category_map_["Personal"]->AddChildNode(child_node);
               break;
            case Building:
               category_map_["Building"]->AddChildNode(child_node);
               break;
            default:
               return;
        }
        controller_map_[child_node->GetID()] = controlled_widget;
        connect(child_node, SIGNAL(ActionButtonClicked(QUuid)), SLOT(ActionNodeClicked(QUuid)));
        layout_manager_->AddItemToScene(child_node);
    }

    void MenuManager::RemoveMenuItem(Category category, QGraphicsProxyWidget *controlled_widget)
    {
        QUuid remove_id = controller_map_.key(controlled_widget);
        if (remove_id.isNull())
            return;

        MenuNode *recovered_node = 0;
        switch (category)
        {
            case Root: // should not happen, can be removed, wont hurt either, item will not be found
               recovered_node = category_map_["Root"]->RemoveChildNode(remove_id);
               break;
            case Personal:
               recovered_node = category_map_["Personal"]->RemoveChildNode(remove_id);
               break;
            case Building:
               recovered_node = category_map_["Building"]->RemoveChildNode(remove_id);
               break;
            default:
               return;
        }

        if (recovered_node)
        {
            controller_map_.remove(remove_id);
            disconnect(recovered_node, SIGNAL(ActionButtonClicked(QUuid)), this, SLOT(ActionNodeClicked(QUuid)));
            layout_manager_->RemoveItemFromScene(recovered_node);
        }
    }

    void MenuManager::ActionNodeClicked(QUuid id)
    {
        if (!controller_map_.contains(id))
            return;

        QGraphicsProxyWidget *controlled_widget = controller_map_[id];
        if (!controlled_widget)
            return;

        if (!controlled_widget->isVisible())
            controlled_widget->show();
        else
            controlled_widget->hide();
    }

    void MenuManager::GroupNodeClicked(GroupNode *clicked_node, QParallelAnimationGroup *move_animations, QParallelAnimationGroup *size_animations)
    {
        if (ongoing_animations_)
            return;

        QParallelAnimationGroup *revert_group = new QParallelAnimationGroup(this);
        QSequentialAnimationGroup *sequential_move_animations_ = new QSequentialAnimationGroup(this);
        sequential_move_animations_->setDirection(QAbstractAnimation::Backward);

        // Make a sequential animation of currently expanding nodes
        // Only revert into the same tree depth as the clicked item
        int click_tree_depth = clicked_node->GetTreeDepth();
        foreach (GroupNode *node, expanded_nodes_)
        {
            if (node->GetTreeDepth() >= click_tree_depth && node->IsExpanded())
            {
                QParallelAnimationGroup *node_move_anim = node->GetMoveAnimations();
                sequential_move_animations_->addAnimation(node_move_anim);
                expanded_nodes_.removeOne(node);
            }
        }
        revert_group->addAnimation(sequential_move_animations_);
        if (last_resize_animations_)
            revert_group->addAnimation(last_resize_animations_);

        // New animations
        next_move_animations_ = move_animations;
        last_resize_animations_ = size_animations;
        expanded_nodes_.append(clicked_node);

        connect(move_animations, SIGNAL( finished() ), size_animations, SLOT( start() ));
        connect(size_animations, SIGNAL( finished() ), SLOT( AdjustTreeOpacity() ));
        // Normal item, we are expanding now, hook up move animations after revert is done
        if (!clicked_node->IsExpanded())
        {
            connect(revert_group, SIGNAL(finished()), SLOT(RevertAnimationsFinished()));
        }
        // Clicked item is root, and its collapsing
        else if (!clicked_node->parent())
        {
            expanded_nodes_.clear();
            next_move_animations_ = 0;
            last_resize_animations_ = 0;
        }
        // Else its a normal item collapsing, lets remove it from tracking map of expanded items
        else
        {
            expanded_nodes_.removeOne(clicked_node);
            last_resize_animations_ = 0;
            GroupNode* node = dynamic_cast<GroupNode*>(clicked_node->parent());
            if(node)
            {
                node->AdjustNode(QAbstractAnimation::Forward);
                last_resize_animations_ = node->GetResizeAnimations();
            }
        }

        revert_group->setDirection(QAbstractAnimation::Backward);
        revert_group->start();
        ongoing_animations_ = true;
    }

    void MenuManager::RevertAnimationsFinished()
    {
        if (next_move_animations_)
        {
            connect(next_move_animations_, SIGNAL( finished() ), SLOT( MoveAnimationsFinished() ));
            next_move_animations_->start();
        }
    }

    void MenuManager::MoveAnimationsFinished()
    {
        ongoing_animations_ = false;
    }

    void MenuManager::AdjustTreeOpacity()
    {
        if (expanded_nodes_.count() <= 1)
            return;

        GroupNode *process_layer = expanded_nodes_.last();
        int depth = process_layer->GetTreeDepth();
        bool level_found = false;

        process_layer = root_menu_;
        while (!level_found)
        {
            foreach(MenuNode *node, process_layer->GetChildNodeList())
            {
                if (node->GetTreeDepth() == depth)
                {
                    node->setOpacity(0.65);
                    level_found = true;
                }
                else if (dynamic_cast<GroupNode*>(node))
                {
                    process_layer = dynamic_cast<GroupNode*>(node);
                    break;
                }

                if (dynamic_cast<GroupNode*>(node))
                    if (dynamic_cast<GroupNode*>(node)->IsExpanded())
                        node->setOpacity(1);
            }
        }
    }
}