// For conditions of distribution and use, see copyright notice in license.txt

#include "StableHeaders.h"
#include "EtherLogic.h"

#include "EtherSceneController.h"
#include "EtherStateMachine.h"

#include "Data/RealXtendAvatar.h"
#include "Data/OpenSimAvatar.h"
#include "Data/OpenSimWorld.h"
#include "Data/DataManager.h"
#include "Data/AvatarInfo.h"
#include "Data/WorldInfo.h"

#include "View/InfoCard.h"

#include <QPair>
#include <QStringList>
#include <QTimer>
#include <QPixmap>
#include <QBrush>

namespace Ether
{
    namespace Logic
    {
        EtherLogic::EtherLogic(QGraphicsView *view)
            : QObject(),
              view_(view),
              data_manager_(new Data::DataManager(this)),
              card_size_(QRectF(0, 0, 400, 400)),
              previous_scene_(0)
        {
            StoreDataToFilesIfEmpty();

            // Read avatars and worlds from file
            avatar_map_ = data_manager_->ReadAllAvatarsFromFile();
            world_map_ = data_manager_->ReadAllWorldsFromFile();

            // Chech how many should be shown in UI
            SetVisibleItems();

            // Create ether scene, store current scene
            View::EtherScene *scene = new View::EtherScene(this, QRectF(0,0,100,100));
            //QBrush bg_brush(QPixmap("./data/ui/images/ether/tile.jpg"));

            //scene->setBackgroundBrush(bg_brush);

            previous_scene_ = view_->scene();
            view_->setScene(scene);
            connect(scene, SIGNAL( changed(const QList<QRectF> &) ), view_, SLOT( SceneChange() ));

            // Create menus
            QPair<View::EllipseMenu*, View::EllipseMenu*> menus;
            menus.first = new View::EllipseMenu(View::EllipseMenu::OPENS_UP);
            menus.second = new View::EllipseMenu(View::EllipseMenu::OPENS_DOWN);

            // Create state machine
            state_machine_ = new EtherStateMachine(this, scene, menus);
            connect(state_machine_, SIGNAL( EtherSceneOutAnimationFinised() ), SLOT( ToggleEtherScene() ));
            connect(view_, SIGNAL( EtherToggleRequest() ), SLOT( ToggleEtherScene() ));
            
            // Create scene controller
            scene_controller_ = new EtherSceneController(this, scene, menus, card_size_, top_menu_visible_items_, bottom_menu_visible_items_);

            ToggleEtherScene();
        }

        void EtherLogic::Start()
        {
            GenerateAvatarInfoCards();
            GenerateWorldInfoCards();

            scene_controller_->LoadTitleWidgets();
            scene_controller_->LoadAvatarCardsToScene(avatar_card_map_);
            scene_controller_->LoadWorldCardsToScene(world_card_map_);

            // HACK to put focus for avatar row :)
            QTimer::singleShot(500, scene_controller_, SLOT(UpPressed()));
        }

        void EtherLogic::SetVisibleItems()
        {
            int avatar_count = avatar_map_.count();
            if (0 <= avatar_count && avatar_count < 5)
                top_menu_visible_items_ = 3;
            else if (5 <= avatar_count && avatar_count < 7)
                top_menu_visible_items_ = 5;
            else if (avatar_count >= 7)
                top_menu_visible_items_ = 7;

            int world_count = world_map_.count();
            if (0 <= world_count && world_count < 5)
                bottom_menu_visible_items_ = 3;
            else if (5 <= world_count && world_count < 7)
                bottom_menu_visible_items_ = 5;
            else if (world_count >= 7)
                bottom_menu_visible_items_ = 7;
        }

        void EtherLogic::StoreDataToFilesIfEmpty()
        {
            if (data_manager_->GetAvatarCountInSettings() == 0)
            {
                qDebug() << "Avatar config file was empty, adding some items to it...";
                Data::OpenSimAvatar oa("Test", "User", "myPass312");
                Data::RealXtendAvatar ra("testuser", QUrl("http://world.evocativi.com:10001"), "myPass835");
                data_manager_->StoreOrUpdateAvatar(&oa);
                data_manager_->StoreOrUpdateAvatar(&ra);
            }

            if (data_manager_->GetWorldCountInSettings() == 0)
            {
                qDebug() << "World config file was empty, adding some items to it...";
                Data::OpenSimWorld ow1(QUrl("http://world.evocativi.com:9000/"), QUrl());
                Data::OpenSimWorld ow2(QUrl("http://test.world.net/9003"), QUrl());
                data_manager_->StoreOrUpdateWorld(&ow1);
                data_manager_->StoreOrUpdateWorld(&ow2);
            }
        }

        void EtherLogic::PrintAvatarMap()
        {
            qDebug() << "Avatar map in memory" << endl;
            foreach(Data::AvatarInfo *avatar_info, avatar_map_.values())
            {
                Data::RealXtendAvatar *ra = dynamic_cast<Data::RealXtendAvatar *>(avatar_info);
                if (ra)
                {
                    ra->Print();
                    continue;
                }
                Data::OpenSimAvatar *oa = dynamic_cast<Data::OpenSimAvatar *>(avatar_info);
                if (oa)
                {
                    oa->Print();
                    continue;
                }
            }
            qDebug() << endl;
        }

        void EtherLogic::PrintWorldMap()
        {
            qDebug() << "World map in memory" << endl;
            foreach(Data::WorldInfo *world_info, world_map_.values())
            {
                Data::OpenSimWorld *ow = dynamic_cast<Data::OpenSimWorld *>(world_info);
                if (ow)
                {
                    ow->Print();
                    continue;
                }
                // Add other types later
            }
            qDebug() << endl;
        }

        void EtherLogic::GenerateAvatarInfoCards()
        {
            View::InfoCard *card = 0;
            qDebug() << "Creating avatar cards and loading them to scene";
            foreach(Data::AvatarInfo *avatar_info, avatar_map_.values())
            {
                switch (avatar_info->avatarType())
                {
                    case AvatarTypes::RealXtend:
                    {
                        Data::RealXtendAvatar *ra = dynamic_cast<Data::RealXtendAvatar *>(avatar_info);
                        if (ra)
                            card = new View::InfoCard(View::InfoCard::TopToBottom, card_size_, QUuid(ra->id()), QString("%1").arg(ra->account()), ra->pixmapPath());
                        break;
                    }

                    case AvatarTypes::OpenSim:
                    {
                        Data::OpenSimAvatar *oa = dynamic_cast<Data::OpenSimAvatar *>(avatar_info);
                        if (oa)
                            card = new View::InfoCard(View::InfoCard::TopToBottom, card_size_, QUuid(oa->id()), QString("%1 %2").arg(oa->firstName(), oa->lastName()), oa->pixmapPath());
                        break;
                    }

                    case AvatarTypes::OpenID:
                        qDebug() << "Can't load OpenID type yet, so can't read them either";
                        break;

                    default:
                        qDebug() << "AvatarInfo had unrecognized avatar type, this should never happen!";
                        break;
                }

                if (card)
                {
                    avatar_card_map_[QUuid(avatar_info->id())] = card;
                    card = 0;
                }
            }
            qDebug() << "-> Created " << avatar_card_map_.count() << "avatar cards";
        }

        void EtherLogic::GenerateWorldInfoCards()
        {
            View::InfoCard *card = 0;
            qDebug() << "Creating world cards and loading them to scene";
            foreach(Data::WorldInfo *world_info, world_map_.values())
            {
                switch (world_info->worldType())
                {
                    case WorldTypes::OpenSim:
                    {
                        Data::OpenSimWorld *ow = dynamic_cast<Data::OpenSimWorld *>(world_info);
                        if (ow)
                            card = new View::InfoCard(View::InfoCard::BottomToTop, card_size_, QUuid(ow->id()), ow->loginUrl().host(), ow->pixmapPath());
                        break;
                    }

                    case WorldTypes::Taiga:
                    case WorldTypes::None:
                    {
                        qDebug() << "Cant make cards from Taiga or None types yet...";
                        break;
                    }

                    default:
                        qDebug() << "WorldInfo had unrecognized world type, this should never happen!";
                        break;
                }

                if (card)
                {
                    world_card_map_[QUuid(world_info->id())] = card;
                    card = 0;
                }
            }
            qDebug() << "-> Created " << world_card_map_.count() << "world cards";
        }

        void EtherLogic::ToggleEtherScene()
        {
            QGraphicsScene *scene = view_->scene();
            previous_scene_->setSceneRect(view_->rect());
            view_->setScene(previous_scene_);
            connect(previous_scene_, SIGNAL( changed(const QList<QRectF> &) ), view_, SLOT( SceneChange() )); 
            previous_scene_ = scene;
        }
    }
}
