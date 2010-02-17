// For conditions of distribution and use, see copyright notice in license.txt

#ifndef incl_UiModule_EtherLogic_h
#define incl_UiModule_EtherLogic_h

#include <QObject>
#include <QGraphicsView>
#include <QGraphicsScene>
#include <QGraphicsProxyWidget>
#include <QUuid>
#include <QPair>

namespace RexLogic
{
    class OpenSimLoginHandler;
}

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
        class EtherScene;
    }

    namespace Logic
    {
        class EtherSceneController;
        class EtherStateMachine;
        class EtherLoginHandler;

        class EtherLogic : public QObject
        {

        Q_OBJECT

        public:
            EtherLogic(QGraphicsView *view);

        public slots:
            void Start();

            void PrintAvatarMap();
            void PrintWorldMap();

            View::EtherScene *GetScene() { return scene_; }
            EtherSceneController *GetSceneController() { return scene_controller_; }

            void SetLoginHandlers(RexLogic::OpenSimLoginHandler *os_login_handler);
            QPair<QString, QString> GetLastLoginScreenshotData(std::string conf_path);
            void UpdateUiPixmaps();

        private slots:
            void SetVisibleItems();
            void StoreDataToFilesIfEmpty();
            void GenerateAvatarInfoCards();
            void GenerateWorldInfoCards();

            void ParseInfoFromCards(QPair<View::InfoCard*, View::InfoCard*> ui_cards);

            void TitleUpdate(QUuid card_uuid, QString new_title);
            void AvatarCreated(Data::AvatarInfo *avatar_data);
            void WorldCreated(Data::WorldInfo *world_data);

        private:
            QGraphicsView *view_;

            Data::DataManager *data_manager_;
            View::EtherScene *scene_;
            EtherSceneController *scene_controller_;
            EtherLoginHandler *login_handler_;

            QGraphicsScene *previous_scene_;

            QMap<QUuid, Data::AvatarInfo*> avatar_map_;
            QMap<QUuid, Data::WorldInfo*> world_map_;
            QMap<QUuid, View::InfoCard*> avatar_card_map_;
            QMap<QUuid, View::InfoCard*> world_card_map_;

            QPair<Data::AvatarInfo*, Data::WorldInfo*> last_login_cards_;

            QRectF card_size_;

            int top_menu_visible_items_;
            int bottom_menu_visible_items_;
        };
    }
}

#endif // ETHERLOGIC_H
