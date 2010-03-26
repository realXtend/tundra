// For conditions of distribution and use, see copyright notice in license.txt

#ifndef incl_UiModule_MenuManager_h
#define incl_UiModule_MenuManager_h

#include <QObject>
#include <QGraphicsScene>
#include <QGraphicsAnchorLayout>
#include <QGraphicsProxyWidget>
#include <QMap>
#include <QPair>

#include "ActionNode.h"
#include "GroupNode.h"

namespace CoreUi
{
    class AnchorLayoutManager;

    class MenuManager : public QObject
    {

    Q_OBJECT
    Q_ENUMS(Category)

    public:
        MenuManager(QObject *parent, CoreUi::AnchorLayoutManager *layout_manager);

        enum Category { Root, Personal, Building };

    public slots:
        void AddMenuItem(Category category, QGraphicsProxyWidget *controlled_widget, QString button_title);
        void RemoveMenuItem(Category category, QGraphicsProxyWidget *controlled_widget);

    private slots:
        void AddMenuGroup(QString name, qreal hgap, qreal vgap);

        void ActionNodeClicked(QUuid id);
        void GroupNodeClicked(GroupNode *clicked_node, QParallelAnimationGroup *move_animations, QParallelAnimationGroup *size_animations);
        void RevertAnimationsFinished();
        void MoveAnimationsFinished();
        void AdjustTreeOpacity();

    private:
        void InitInternals();

        //! Layout manager for scene interaction
        CoreUi::AnchorLayoutManager *layout_manager_;

        //! Track maps
        QMap<QUuid, QGraphicsProxyWidget*> controller_map_;
        QMap<QString, GroupNode*> category_map_;

        //! These are for click animations starts/reverts when you navigate trough the menu
        GroupNode *last_clicked_node_;
        QList<GroupNode*> expanded_nodes_;
        QParallelAnimationGroup *next_move_animations_;
        QParallelAnimationGroup *last_resize_animations_;
        QUuid last_id_;
        bool ongoing_animations_;

        //! Pointer to the main root menu
        GroupNode *root_menu_;
    };
}

#endif
