// For conditions of distribution and use, see copyright notice in license.txt

#include "StableHeaders.h"
#include "EtherLogic.h"

#include "EtherSceneController.h"
#include "EtherLoginHandler.h"

#include "Data/RealXtendAvatar.h"
#include "Data/OpenSimAvatar.h"
#include "Data/OpenSimWorld.h"
#include "Data/DataManager.h"
#include "Data/AvatarInfo.h"
#include "Data/WorldInfo.h"

#include "View/InfoCard.h"

#include <QStringList>
#include <QTimer>
#include <QDir>

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
            scene_ = new View::EtherScene(this, QRectF(0,0,100,100));

            // Create menus
            QPair<View::EllipseMenu*, View::EllipseMenu*> menus;
            menus.first = new View::EllipseMenu(View::EllipseMenu::OPENS_UP);
            menus.second = new View::EllipseMenu(View::EllipseMenu::OPENS_DOWN);

            // Create scene controller
            scene_controller_ = new EtherSceneController(this, scene_, menus, card_size_, top_menu_visible_items_, bottom_menu_visible_items_);
            connect(scene_controller_, SIGNAL( LoginRequest(QPair<View::InfoCard*, View::InfoCard*>) ),
                    this, SLOT( ParseInfoFromCards(QPair<View::InfoCard*, View::InfoCard*>) ));

            // Create login handler
            login_handler_ = new EtherLoginHandler(this, scene_controller_); 
        }

        void EtherLogic::Start()
        {
            last_login_cards_.first = 0;
            last_login_cards_.second = 0;

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
                Data::OpenSimAvatar oa1("Mr.", "Anonymous", "nopass");
                Data::OpenSimAvatar oa2("d", "d", "d");
                Data::RealXtendAvatar ra("testrexuser", QUrl("http://world.evocativi.com:10001"), "test");
                data_manager_->StoreOrUpdateAvatar(&oa1);
                data_manager_->StoreOrUpdateAvatar(&oa2);
                data_manager_->StoreOrUpdateAvatar(&ra);
            }

            if (data_manager_->GetWorldCountInSettings() == 0)
            {
                qDebug() << "World config file was empty, adding some items to it...";
                Data::OpenSimWorld ow1(QUrl("http://world.evocativi.com:8002"), QUrl());
                Data::OpenSimWorld ow2(QUrl("http://world.realxtend.org:9000"), QUrl());
                Data::OpenSimWorld ow3(QUrl("http://localhost:9000"), QUrl());
                Data::OpenSimWorld ow4(QUrl("http://localhost:8002"), QUrl());
                data_manager_->StoreOrUpdateWorld(&ow1);
                data_manager_->StoreOrUpdateWorld(&ow2);
                data_manager_->StoreOrUpdateWorld(&ow3);
                data_manager_->StoreOrUpdateWorld(&ow4);
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
        }

        void EtherLogic::GenerateWorldInfoCards()
        {
            View::InfoCard *card = 0;
            foreach(Data::WorldInfo *world_info, world_map_.values())
            {
                switch (world_info->worldType())
                {
                    case WorldTypes::OpenSim:
                    {
                        Data::OpenSimWorld *ow = dynamic_cast<Data::OpenSimWorld *>(world_info);
                        if (ow)
                            card = new View::InfoCard(View::InfoCard::BottomToTop, card_size_, QUuid(ow->id()), 
                                                      QString("%1:%2").arg(ow->loginUrl().host(), QString::number(ow->loginUrl().port())), ow->pixmapPath());
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
        }

        void EtherLogic::SetLoginHandlers(RexLogic::OpenSimLoginHandler *os_login_handler)
        {
            login_handler_->SetOpenSimLoginHandler(os_login_handler);
        }

        void EtherLogic::ParseInfoFromCards(QPair<View::InfoCard*, View::InfoCard*> ui_cards)
        {
            QPair<Data::AvatarInfo*, Data::WorldInfo*> data_cards;
            if (avatar_map_.contains(ui_cards.first->id()) &&
                world_map_.contains(ui_cards.second->id()) )
            {
                data_cards.first = avatar_map_[ui_cards.first->id()];
                data_cards.second = world_map_[ui_cards.second->id()];
                login_handler_->ParseInfoFromData(data_cards);
                last_login_cards_ = data_cards;
            }
        }

        QMap<QString, QString> EtherLogic::GetLastLoginScreenshotData(std::string conf_path)
        {
            QMap<QString, QString> paths_map;
            // Return if no login has been done via ether
            if (!last_login_cards_.first || !last_login_cards_.second)
                return paths_map;

            QString worldpath, avatarpath, worldfile, avatarfile, path_with_file, appdata_path;
            QDir dir_check;

            // Get users appdata path (semi hack, from frameworks config manager path)
            appdata_path = QString::fromStdString(conf_path);
            appdata_path.replace("/", QDir::separator());
            appdata_path.replace("\\", QDir::separator());
            appdata_path = appdata_path.leftRef(appdata_path.lastIndexOf(QDir::separator())+1).toString();
            
            // Check that dirs exists
            dir_check = QDir(appdata_path + "ether" + QDir::separator() + "avatarimages");
            if (!dir_check.exists())
            {
                dir_check = QDir(appdata_path + "ether");
                dir_check.mkdir("avatarimages");
            }
            dir_check = QDir(appdata_path + "ether" + QDir::separator() + "worldimages");
            if (!dir_check.exists())
            {
                dir_check = QDir(appdata_path + "ether");
                dir_check.mkdir("worldimages");
            }
 
            // Avatar paths
            avatarpath = appdata_path + "ether" + QDir::separator() + "avatarimages" + QDir::separator();
            avatarfile = last_login_cards_.first->id() + ".png";
            path_with_file = avatarpath + avatarfile;
            if (last_login_cards_.first->pixmapPath() != path_with_file)
            {
                // Lets not do this yet, cant take the shot for it
                //last_login_cards_.first->setPixmapPath(path_with_file);
                //data_manager_->StoreOrUpdateAvatar(last_login_cards_.first);
            }

            // World paths
            worldpath = appdata_path + "ether" + QDir::separator() + "worldimages" + QDir::separator();
            worldfile = last_login_cards_.second->id() + ".png";
            path_with_file = worldpath + worldfile;
            if (last_login_cards_.second->pixmapPath() != path_with_file)
            {
                last_login_cards_.second->setPixmapPath(path_with_file);
                data_manager_->StoreOrUpdateWorld(last_login_cards_.second);
            }

            // Return values
            paths_map["AvatarPath"] = avatarpath;
            paths_map["AvatarFile"] = avatarfile;
            paths_map["WorldPath"] = worldpath;
            paths_map["WorldFile"] = worldfile;
            return paths_map;
        }

        void EtherLogic::UpdateUiPixmaps()
        {
            // Update new world image to ui
            if (last_login_cards_.second)
                if (world_card_map_.contains(last_login_cards_.second->id()))
                    world_card_map_[last_login_cards_.second->id()]->UpdatePixmap(last_login_cards_.second->pixmapPath());

            // Avatar later when we get the picture...
        }
    }
}
