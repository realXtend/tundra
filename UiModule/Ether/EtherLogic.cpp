// For conditions of distribution and use, see copyright notice in license.txt

#include "StableHeaders.h"
#include "DebugOperatorNew.h"

#include "EtherLogic.h"
#include "EtherSceneController.h"
#include "EtherLoginNotifier.h"

#include "Data/RealXtendAvatar.h"
#include "Data/OpenSimAvatar.h"
#include "Data/OpenSimWorld.h"
#include "Data/DataManager.h"
#include "Data/AvatarInfo.h"
#include "Data/WorldInfo.h"

#include "View/InfoCard.h"
#include "View/VerticalMenu.h"
#include "View/EtherScene.h"

#include "CoreDefines.h"
#include <QStringList>
#include <QTimer>
#include <QDir>
#include <QList>

#include <QDebug>

#include "MemoryLeakCheck.h"

namespace Ether
{
    namespace Logic
    {
        EtherLogic::EtherLogic(Foundation::Framework *framework, QGraphicsView *view) :
              framework_(framework),
              view_(view),
              data_manager_(new Data::DataManager(this)),
              card_size_(QRectF(0, 0, 470, 349)),
              previous_scene_(0)
        {
#ifdef DYNAMIC_LOGIN_SCENE
            StoreDataToFilesIfEmpty();

            // Read avatars and worlds from file
            avatar_map_ = data_manager_->ReadAllAvatarsFromFile();
            world_map_ = data_manager_->ReadAllWorldsFromFile();

            // Signals from data manager
            connect(data_manager_, SIGNAL( ObjectUpdated(QUuid, QString) ),
                    SLOT( TitleUpdate(QUuid, QString) ));
            connect(data_manager_, SIGNAL( AvatarDataCreated(Data::AvatarInfo*) ),
                    SLOT( AvatarCreated(Data::AvatarInfo*) ));
            connect(data_manager_, SIGNAL( WorldDataCreated(Data::WorldInfo*) ),
                    SLOT( WorldCreated(Data::WorldInfo*) ));

            // Chech how many should be shown in UI
            SetVisibleItems();

            // Create ether scene, store current scene
            scene_ = new View::EtherScene(this, QRectF(0,0,100,100));

            // Initialise menus
            QPair<View::EtherMenu*, View::EtherMenu*> menus;
            menus.first = new View::VerticalMenu(View::VerticalMenu::VERTICAL_MID);
            menus.second = new View::VerticalMenu(View::VerticalMenu::VERTICAL_MID);

            // Create scene controller
            scene_controller_ = new EtherSceneController(this, data_manager_, scene_, menus, card_size_, top_menu_visible_items_, bottom_menu_visible_items_);

            // Create login handler
            login_notifier_ = new EtherLoginNotifier(this, scene_controller_, framework);

            // Signals from scene contoller
            connect(scene_controller_, SIGNAL( ApplicationExitRequested() ),
                    login_notifier_, SLOT( ExitApplication() ));
            connect(scene_controller_, SIGNAL( DisconnectCurrentConnection() ),
                    login_notifier_, SLOT( EmitDisconnectRequest() ));
            connect(scene_controller_, SIGNAL( LoginRequest(QPair<View::InfoCard*, View::InfoCard*>) ),
                    SLOT( ParseInfoFromCards(QPair<View::InfoCard*, View::InfoCard*>) ));
            connect(scene_controller_, SIGNAL( ObjectRemoved(QUuid) ),
                    SLOT( RemoveObjectFromData(QUuid) ));
#else
            // Create login scene, store current scene
            scene_ = new View::EtherScene(this, QRectF(0,0,100,100));
            // Create scene controller
            scene_controller_ = new EtherSceneController(this, data_manager_, scene_);
            // Create login handler
            login_notifier_ = new EtherLoginNotifier(this, scene_controller_, framework); 
            connect(scene_controller_, SIGNAL( ApplicationExitRequested() ),
                login_notifier_, SLOT( ExitApplication() ));
#endif
        }

        EtherLogic::~EtherLogic()
        {
            scene_controller_->StoreConfigs();

            foreach (Data::AvatarInfo *avatar, avatar_map_.values())
                SAFE_DELETE(avatar);
            foreach (Data::WorldInfo *world, world_map_.values())
                SAFE_DELETE(world);
            foreach (View::InfoCard *infocard, avatar_card_map_.values())
                SAFE_DELETE(infocard);
            foreach (View::InfoCard *infocard, world_card_map_.values())
                SAFE_DELETE(infocard);
        }

        void EtherLogic::Start()
        {
            // Check default view for ether and setup classic login widget
            QMap<QString, QString> last_login_data = data_manager_->GetClassicLoginInfo();

#ifdef DYNAMIC_LOGIN_SCENE
            // In dynamic view check which view is defaulted
            QString view = data_manager_->GetDefaultView();

            bool classic_view = false;
            if (view == "classic")
                classic_view = true;
            scene_controller_->LoadClassicLoginWidget(login_notifier_, classic_view, last_login_data);

            // Generate ethers graphical content
            last_login_cards_.first = 0;
            last_login_cards_.second = 0;

            GenerateAvatarInfoCards();
            GenerateWorldInfoCards();

            // For first start of ether, lets look if the user has used ether before and read the selected cards
            // Set them to be selected for first start
            QVector<View::InfoCard*> avatar_vector = avatar_card_map_.values().toVector();
            QVector<View::InfoCard*> world_vector = world_card_map_.values().toVector();
            QPair<QUuid, QUuid> last_run_selected = data_manager_->GetLastSelectedCards();

            if (!last_run_selected.first.isNull())
            {
                if (avatar_card_map_.contains(last_run_selected.first))
                {
                    View::InfoCard *last_selected_avatar = avatar_card_map_[last_run_selected.first];
                    if (last_selected_avatar)
                    {
                        avatar_vector.remove(avatar_vector.indexOf(last_selected_avatar));
                        avatar_vector.insert(0, last_selected_avatar);
                    }
                }
            }
            if (!last_run_selected.second.isNull())
            {
                if (world_card_map_.contains(last_run_selected.second))
                {
                    View::InfoCard *last_selected_world = world_card_map_[last_run_selected.second];
                    if (last_selected_world)
                    {
                        world_vector.remove(world_vector.indexOf(last_selected_world));
                        world_vector.insert(0, last_selected_world);
                    }
                }
            }

            scene_controller_->LoadActionWidgets();
            scene_controller_->LoadStartUpCardsToScene(avatar_vector, top_menu_visible_items_, world_vector, bottom_menu_visible_items_);

            // HACK to put focus for avatar row :)
            QTimer::singleShot(500, scene_controller_, SLOT(UpPressed()));
#else
            scene_controller_->LoadStaticEther(login_notifier_, last_login_data);
#endif
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
                qDebug() << "Avatar config file was empty, adding one anonymous opensim user to it";
                Data::OpenSimAvatar oa1("Mr.", "Anonymous", "");
                data_manager_->StoreOrUpdateAvatar(&oa1);
            }

            if (data_manager_->GetWorldCountInSettings() == 0)
            {
                qDebug() << "World config file was empty, adding two anonymous login servers";
                Data::OpenSimWorld ow1(QUrl("http://home.hulkko.net:9007"), "", QMap<QString, QVariant>()); // empty grid info at this point
                Data::OpenSimWorld ow2(QUrl("http://world.realxtend.org:9000"), "", QMap<QString, QVariant>()); // empty grid info at this point
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
            foreach(Data::AvatarInfo *avatar_info, avatar_map_.values())
            {
                switch (avatar_info->avatarType())
                {
                    case AvatarTypes::RealXtend:
                    {
                        Data::RealXtendAvatar *ra = dynamic_cast<Data::RealXtendAvatar *>(avatar_info);
                        if (ra)
                            card = new View::InfoCard(View::InfoCard::TopToBottom, card_size_, QUuid(ra->id()),
                                QString("%1").arg(ra->account()), ra->pixmapPath());
                        break;
                    }

                    case AvatarTypes::OpenSim:
                    {
                        Data::OpenSimAvatar *oa = dynamic_cast<Data::OpenSimAvatar *>(avatar_info);
                        if (oa)
                            card = new View::InfoCard(View::InfoCard::TopToBottom, card_size_, QUuid(oa->id()),
                                QString("%1 %2").arg(oa->firstName(), oa->lastName()), oa->pixmapPath());
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
                        {
                            QString title;
                            if (ow->loginUrl().port() != -1)
                                title = QString("%1:%2").arg(ow->loginUrl().host(), QString::number(ow->loginUrl().port()));
                            else
                                title = ow->loginUrl().host();
                            card = new View::InfoCard(View::InfoCard::BottomToTop, card_size_, QUuid(ow->id()), title, ow->pixmapPath());
                        }
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

        void EtherLogic::ParseInfoFromCards(QPair<View::InfoCard*, View::InfoCard*> ui_cards)
        {
            QPair<Data::AvatarInfo*, Data::WorldInfo*> data_cards;
            if (avatar_map_.contains(ui_cards.first->id()) &&
                world_map_.contains(ui_cards.second->id()) )
            {
                data_cards.first = avatar_map_[ui_cards.first->id()];
                data_cards.second = world_map_[ui_cards.second->id()];
                login_notifier_->ParseInfoFromData(data_cards);
                last_login_cards_ = data_cards;
            }
        }

        QPair<QString, QString> EtherLogic::GetLastLoginScreenshotData(std::string conf_path)
        {
            QPair<QString, QString> paths_pair;
            paths_pair.first = QString("");
            paths_pair.second = QString("");
            // Return if no login has been done via ether
            if (!last_login_cards_.first || !last_login_cards_.second)
                return paths_pair;

            QString worldpath, avatarpath, worldfile, avatarfile, path_with_file, appdata_path;
            QDir dir_check;

            // Get users appdata path (semi hack, from frameworks config manager path)
            appdata_path = QString::fromStdString(conf_path);
            appdata_path.replace("/", QDir::separator());
            appdata_path.replace("\\", QDir::separator());
            appdata_path = appdata_path.leftRef(appdata_path.lastIndexOf(QDir::separator())+1).toString();
            
            // Check that dirs exists
            dir_check = QDir(appdata_path + "ether");
            if (!dir_check.exists())
            {
                dir_check = QDir(appdata_path);
                dir_check.mkdir("ether");
            }
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
                last_login_cards_.first->setPixmapPath(path_with_file);
                data_manager_->StoreOrUpdateAvatar(last_login_cards_.first);
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
            paths_pair.first = worldpath + worldfile;
            paths_pair.second = avatarpath + avatarfile;
            return paths_pair;
        }

        void EtherLogic::UpdateUiPixmaps()
        {
            // Update new world image to ui
            if (last_login_cards_.second)
                if (world_card_map_.contains(last_login_cards_.second->id()))
                    world_card_map_[last_login_cards_.second->id()]->UpdatePixmap(last_login_cards_.second->pixmapPath());

            // Update new avatar image to ui
            if (last_login_cards_.first)
                if (avatar_card_map_.contains(last_login_cards_.first->id()))
                    avatar_card_map_[last_login_cards_.first->id()]->UpdatePixmap(last_login_cards_.first->pixmapPath());
        }

        void EtherLogic::TitleUpdate(QUuid card_uuid, QString new_title)
        {
            if (avatar_card_map_.contains(card_uuid))
            {
                avatar_card_map_[card_uuid]->setTitle(new_title);
                scene_controller_->UpdateAvatarInfoWidget();
            }
            else if (world_card_map_.contains(card_uuid))
            {
                world_card_map_[card_uuid]->setTitle(new_title);
                scene_controller_->UpdateWorldInfoWidget();
            }
        }

        void EtherLogic::AvatarCreated(Data::AvatarInfo *avatar_data)
        {
            View::InfoCard *new_avatar_card = 0;
            if (avatar_data->avatarType() == AvatarTypes::OpenSim)
            {
                Data::OpenSimAvatar *oa = dynamic_cast<Data::OpenSimAvatar *>(avatar_data);
                if (oa)
                    new_avatar_card = new View::InfoCard(View::InfoCard::TopToBottom, card_size_, QUuid(oa->id()), 
                                                         oa->userName(), oa->pixmapPath());
            }
            else if (avatar_data->avatarType() == AvatarTypes::RealXtend)
            {
                Data::RealXtendAvatar *ra = dynamic_cast<Data::RealXtendAvatar *>(avatar_data);
                if (ra)
                    new_avatar_card = new View::InfoCard(View::InfoCard::TopToBottom, card_size_, QUuid(ra->id()), 
                                                         ra->account(), ra->pixmapPath());
            }

            if (!new_avatar_card)
                return;

            if (!avatar_card_map_.contains(new_avatar_card->id()))
            {
                avatar_map_[avatar_data->id()] = avatar_data;
                avatar_card_map_[new_avatar_card->id()] = new_avatar_card;

                SetVisibleItems();
                scene_controller_->NewAvatarToScene(new_avatar_card, avatar_card_map_, top_menu_visible_items_);
            }
        }

        void EtherLogic::WorldCreated(Data::WorldInfo *world_data)
        {
            View::InfoCard *new_world_card = 0;
            if (world_data->worldType() == WorldTypes::OpenSim)
            {
                Data::OpenSimWorld *ow = dynamic_cast<Data::OpenSimWorld *>(world_data);
                QString title;
                if (ow->loginUrl().port() != -1)
                    title = QString("%1:%2").arg(ow->loginUrl().host(), QString::number(ow->loginUrl().port()));
                else
                    title = ow->loginUrl().host();
                if (ow)
                    new_world_card = new View::InfoCard(View::InfoCard::BottomToTop, card_size_, QUuid(ow->id()), 
                                                        title, ow->pixmapPath());
            }

            if (!new_world_card)
                return;

            if (!world_card_map_.contains(new_world_card->id()))
            {
                world_map_[new_world_card->id()] = world_data;
                world_card_map_[new_world_card->id()] = new_world_card;

                SetVisibleItems();
                scene_controller_->NewWorldToScene(new_world_card, world_card_map_, bottom_menu_visible_items_);
            }
        }

        void EtherLogic::SetConnectionState(UiServices::ConnectionState connection_state, const QString &message)
        {
            switch (connection_state)
            {
                case UiServices::Connected:
                    scene_controller_->SetConnectingState(false);
                    scene_controller_->SetConnected(true);
                    scene_controller_->ShowStatusInformation("Connected");
                    scene_controller_->RevertLoginAnimation(true);
                    scene_->SetConnectionStatus(true);
                    break;
                case UiServices::Disconnected:
                    UpdateUiPixmaps();
                    scene_controller_->SetConnected(false);
                    scene_controller_->ShowStatusInformation("Disconnected");
                    scene_->SetConnectionStatus(false);
                    break;
                case UiServices::Failed:
                    scene_controller_->SetConnectingState(false);
                    scene_controller_->SetConnected(false);
                    scene_controller_->ShowStatusInformation("Failed to connect: "+ message);
                    scene_controller_->RevertLoginAnimation(false);
                    scene_->SetConnectionStatus(false);
                    break;
                case UiServices::Kicked:
                    UpdateUiPixmaps();
                    scene_controller_->SetConnected(false);
                    scene_controller_->ShowStatusInformation("Kicked from server");
                    scene_->SetConnectionStatus(false);
                    break;
            }
            scene_->SupressKeyEvents(false);
        }

        void EtherLogic::RemoveObjectFromData(QUuid uuid)
        {
            if (avatar_card_map_.contains(uuid))
            {
                View::InfoCard *removed_item = avatar_card_map_[uuid];
                if (avatar_card_map_.remove(uuid) > 0 && avatar_map_.remove(uuid) > 0)
                {
                    SetVisibleItems();
                    scene_controller_->LoadAvatarCardsToScene(avatar_card_map_, top_menu_visible_items_, false);
                    SAFE_DELETE(removed_item);
                }
            }
            else if (world_card_map_.contains(uuid))
            {
                View::InfoCard *removed_item = world_card_map_[uuid];
                if (world_card_map_.remove(uuid) > 0 && world_map_.remove(uuid) > 0)
                {
                    SetVisibleItems();
                    scene_controller_->LoadWorldCardsToScene(world_card_map_, bottom_menu_visible_items_, false);
                    SAFE_DELETE(removed_item);
                }
            }
        }
    }
}
