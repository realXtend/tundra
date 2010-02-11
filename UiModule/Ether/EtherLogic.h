// For conditions of distribution and use, see copyright notice in license.txt

#ifndef incl_UiModule_EtherLogic_h
#define incl_UiModule_EtherLogic_h

#include <QObject>
#include <QGraphicsView>
#include <QGraphicsScene>
#include <QGraphicsProxyWidget>
#include <QUuid>

namespace Ether
{
    namespace Data
    {
        class DataManager;
        class AvatarInfo;
        class WorldInfo;
    }

    namespace View
    {
        class InfoCard;
    }

    namespace Logic
    {
        class EtherSceneController;
        class EtherStateMachine;

        class EtherLogic : public QObject
        {

        Q_OBJECT

        public:
            EtherLogic(QGraphicsView *view);

        public slots:
            void Start();

            void PrintAvatarMap();
            void PrintWorldMap();

            EtherSceneController *GetSceneController() { return scene_controller_; }
            EtherStateMachine *GetStateMachine() { return state_machine_; }

        private slots:
            void SetVisibleItems();
            void StoreDataToFilesIfEmpty();
            void GenerateAvatarInfoCards();
            void GenerateWorldInfoCards();
            void ToggleEtherScene();

        private:
            QGraphicsView *view_;

            Data::DataManager *data_manager_;
            EtherSceneController *scene_controller_;
            EtherStateMachine *state_machine_;

            QGraphicsScene *previous_scene_;

            QMap<QUuid, Data::AvatarInfo*> avatar_map_;
            QMap<QUuid, Data::WorldInfo*> world_map_;
            QMap<QUuid, View::InfoCard*> avatar_card_map_;
            QMap<QUuid, View::InfoCard*> world_card_map_;

            QRectF card_size_;

            int top_menu_visible_items_;
            int bottom_menu_visible_items_;
        };
    }
}

#endif // ETHERLOGIC_H
