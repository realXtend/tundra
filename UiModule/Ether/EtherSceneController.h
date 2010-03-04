// For conditions of distribution and use, see copyright notice in license.txt

#ifndef incl_UiModule_EtherSceneController_h
#define incl_UiModule_EtherSceneController_h

#include <QObject>
#include <QGraphicsScene>
#include <QUuid>
#include <QTimer>

#include "UiModuleFwd.h"
#include <QScopedPointer>
#include <QParallelAnimationGroup>

#include "View/EtherScene.h"
#include "View/VerticalMenu.h"
#include "Data/WorldInfo.h"

namespace CoreUi
{
    class AnchorLayoutManager;

    namespace Classical
    {
        class ClassicalLoginWidget;
    }
}

namespace Ether
{
    namespace Logic
    {
        class EtherLoginNotifier;

        class EtherSceneController : public QObject
        {

        Q_OBJECT

        public:
            EtherSceneController(QObject *parent, Data::DataManager *data_manager, View::EtherScene *scene, QPair<View::EtherMenu*, View::EtherMenu*> menus,
                                 QRectF card_size, int top_items, int bottom_items);
            virtual ~EtherSceneController();

        public slots:
            void LoadActionWidgets();
            void LoadClassicLoginWidget(EtherLoginNotifier *login_notifier, bool default_view, QMap<QString,QString> stored_login_data);

            void LoadStartUpCardsToScene(QVector<View::InfoCard*> avatar_ordered_vector, int visible_top_items, QVector<View::InfoCard*> world_ordered_vector, int visible_bottom_items);
            void LoadAvatarCardsToScene(QMap<QUuid, View::InfoCard*> avatar_map, int visible_top_items, bool add_to_scene);
            void NewAvatarToScene(View::InfoCard *new_card, QMap<QUuid, View::InfoCard*> avatar_map, int visible_top_items);
            void LoadWorldCardsToScene(QMap<QUuid, View::InfoCard*> world_map, int visible_bottom_items,  bool add_to_scene);
            void NewWorldToScene(View::InfoCard *new_card, QMap<QUuid, View::InfoCard*> world_map, int visible_bottom_items);

            void UpPressed();
            void DownPressed();
            void LeftPressed();
            void RightPressed();
            void TryStartLogin();
            void ItemActivatedWithMouse(View::InfoCard *clicked_item);

            void ActionWidgetInProgress(bool action_ongoing);

            void UpdateAvatarInfoWidget();
            void RemoveAvatar(Data::AvatarInfo *avatar_info);
            void UpdateWorldInfoWidget();
            void RemoveWorld(Data::WorldInfo *world_info);

            void RecalculateMenus();
            void TheResizer();

            void StartLoginAnimation();
            void RevertLoginAnimation(bool change_scene_after_anims_finish);

            void SuppressControlWidgets(bool suppress);
            void ShowStatusInformation(QString text);
            
            void TryExitApplication();
            void StoreConfigs();

        private slots:
            void ControlsWidgetHandler(QString request_type);
            void ActiveItemChanged(View::InfoCard *);
            void SceneRectChanged(const QRectF &new_rect);

            void LoginAnimationFinished();
            void HideStatusWidget();

        private:
            //! Pointer to data manager
            Data::DataManager *data_manager_;

            //! Current scene
            View::EtherScene *scene_;

            //! Menus
            View::EtherMenu *active_menu_;
            View::EtherMenu *top_menu_;
            View::EtherMenu *bottom_menu_;
            int top_menu_visible_items_;
            int bottom_menu_visible_items_;
            int menu_cap_size_;

            //!animations
            QParallelAnimationGroup *login_animations_;

            //! Max card size
            QRectF card_size_;

            //! Active focus items
            View::InfoCard *last_active_top_card_;
            View::InfoCard *last_active_bottom_card_;

            //! Information widgets
            View::ControlProxyWidget *avatar_info_widget_;
            View::ControlProxyWidget *avatar_addremove_widget_;
            View::ControlProxyWidget *world_info_widget_;
            View::ControlProxyWidget *world_addremove_widget_;
            View::ControlProxyWidget *status_widget_;
            
            // Classic login widget
            QGraphicsProxyWidget *classic_login_proxy_;
            CoreUi::Classical::ClassicalLoginWidget *classical_login_widget_;

            //! Control widget
            View::ControlProxyWidget *control_widget_;

            //! Action widget
            View::ActionProxyWidget *action_proxy_widget_;

            //! Anchor layout manager
            CoreUi::AnchorLayoutManager *layout_manager_;

            //! Random locals
            int last_scale_;
            bool change_scene_after_anims_finish_;

            QTimer *info_hide_timer_;

        signals:
            void ApplicationExitRequested();
            void LoginRequest(QPair<View::InfoCard*, View::InfoCard*> selected_cards);
            void ObjectRemoved(QUuid);
        };
    }
}

#endif // EtherSceneController_H
