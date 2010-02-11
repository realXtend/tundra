// For conditions of distribution and use, see copyright notice in license.txt

#ifndef incl_UiModule_EtherSceneController_h
#define incl_UiModule_EtherSceneController_h

#include <QObject>
#include <QGraphicsScene>

#include "View/EtherScene.h"
#include "View/EllipseMenu.h"

#include "Data/AvatarInfo.h"
#include "Data/WorldInfo.h"
#include "View/InfoCard.h"
#include "View/ControlProxyWidget.h"

namespace Ether
{
    namespace Logic
    {
        class EtherSceneController : public QObject
        {

        Q_OBJECT

        public:
            EtherSceneController(QObject *parent, View::EtherScene *scene, QPair<View::EllipseMenu*, View::EllipseMenu*> menus,
                                 QRectF card_size, int top_items, int bottom_items);

        public slots:
            void LoadTitleWidgets();
            void LoadAvatarCardsToScene(QMap<QUuid, View::InfoCard*> avatar_map);
            void LoadWorldCardsToScene(QMap<QUuid, View::InfoCard*> world_map);

            void UpPressed();
            void DownPressed();
            void LeftPressed() { active_menu_->moveLeft(); }
            void RightPressed() { active_menu_->moveRight(); }
            void ItemActivatedWithMouse(View::InfoCard *clicked_item);

        private slots:
            void ActiveItemChanged(View::InfoCard *);
            void SceneRectChanged(const QRectF &new_rect);

        private:
            //! Current scene
            View::EtherScene *scene_;

            //! Menus
            View::EllipseMenu *active_menu_;
            View::EllipseMenu *top_menu_;
            View::EllipseMenu *bottom_menu_;
            int top_menu_visible_items_;
            int bottom_menu_visible_items_;

            //! Max card size
            QRectF card_size_;

            //! Active focus items
            View::InfoCard *last_active_top_card_;
            View::InfoCard *last_active_bottom_card_;

            //! Information widgets
            View::ControlProxyWidget *avatar_info_widget_;
            View::ControlProxyWidget *world_info_widget_;

            //! Action widgets
            View::ControlProxyWidget *connect_control_widget_;
            View::ControlProxyWidget *exit_control_widget_;

        };
    }
}

#endif // EtherSceneController_H
