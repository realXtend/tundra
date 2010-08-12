// For conditions of distribution and use, see copyright notice in license.txt

#ifndef incl_UiModule_MenuManager_h
#define incl_UiModule_MenuManager_h

//#include "UiDefines.h"

#include <QObject>
#include <QMap>
#include <QUuid>

class QGraphicsProxyWidget;
class QParallelAnimationGroup;

namespace UiServices
{
    class UiWidgetProperties;
}

namespace CoreUi
{
    class AnchorLayoutManager;
    class GroupNode;

    class MenuManager : public QObject
    {
        Q_OBJECT

    public:
        /** Constuctor. Creates the root menu.
         *  @param parent 
         *  @param layout_manager 
         */
        MenuManager(QObject *parent, AnchorLayoutManager *layout_manager);

        /// Destructor. Deletes the root menu and all its children.
        ~MenuManager();

    public slots:
        void AddMenuItem(QGraphicsProxyWidget *controlled_widget, const UiServices::UiWidgetProperties &properties);
        void AddMenuItem(const QString &name, const QString &category, QGraphicsProxyWidget *controlled_widget);

        void RemoveMenuItem(const QString &category, QGraphicsProxyWidget *controlled_widget);

    private slots:
        void AddMenuGroup(const QString &name, const QString &icon = "", qreal hgap = 5.0, qreal vgap = 5.0);

        void ActionNodeClicked(const QUuid &id);
        void GroupNodeClicked(GroupNode *node, QParallelAnimationGroup *move_animations, QParallelAnimationGroup *size_animations);
        void RevertAnimationsFinished();
        void MoveAnimationsFinished();
        void AdjustTreeOpacity();

    private:
        /// Organizes the menu so that submenus are before normal menu entries.
        void Sort();

        //! Layout manager for scene interaction
        AnchorLayoutManager *layout_manager_;

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
        bool root_collapsing_;

        //! Pointer to the main root menu
        GroupNode *root_menu_;
    };
}

#endif
