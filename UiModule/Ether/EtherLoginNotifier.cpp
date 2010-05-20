// For conditions of distribution and use, see copyright notice in license.txt

#include "StableHeaders.h"
#include "DebugOperatorNew.h"
#include "EtherLoginNotifier.h"

#include "Data/RealXtendAvatar.h"
#include "Data/OpenSimAvatar.h"
#include "Data/OpenSimWorld.h"

#include "MemoryLeakCheck.h"

#include "ModuleManager.h"
#include "UiModule.h"
#include "Inworld/InworldSceneController.h"
#include "Inworld/ControlPanelManager.h"
#include "Inworld/ControlPanel/TeleportWidget.h"
#include "UiNotificationServices.h"

namespace Ether
{
    namespace Logic
    {
        EtherLoginNotifier::EtherLoginNotifier(QObject *parent, EtherSceneController *scene_controller, Foundation::Framework *framework)
            : QObject(parent),
              scene_controller_(scene_controller),
              framework_(framework)
        {
            boost::shared_ptr<UiServices::UiModule> ui_module =  framework_->GetModuleManager()->GetModule<UiServices::UiModule>(Foundation::Module::MT_UiServices).lock();
            if (ui_module)
                connect(ui_module->GetInworldSceneController()->GetControlPanelManager()->GetTeleportWidget(), SIGNAL(StartTeleport(QString)), SLOT(Teleport(QString)));                
            teleporting = false;
        }

        void EtherLoginNotifier::ParseInfoFromData(QPair<Data::AvatarInfo*, Data::WorldInfo*> data_cards)
        {
            // Set last info card to default. Used for in world Teleporting
            last_info_map_.clear();

            QMap<QString, QString> info_map;
            switch (data_cards.second->worldType())
            {
                case WorldTypes::OpenSim:
                {
                    Data::OpenSimWorld *ow = dynamic_cast<Data::OpenSimWorld*>(data_cards.second);
                    info_map["WorldAddress"] = ow->loginUrl().toString();
                    info_map["StartLocation"] = ow->startLocation();
                    last_info_map_.insert("WorldAddress", ow->loginUrl().toString());
                    last_info_map_.insert("StartLocation", ow->startLocation());
                    break;
                }
            }

            switch (data_cards.first->avatarType())
            {
                case AvatarTypes::OpenSim:
                {
                    Data::OpenSimAvatar *oa = dynamic_cast<Data::OpenSimAvatar*>(data_cards.first);
                    info_map["Username"] = oa->userName();
                    info_map["Password"] = oa->password();
                    last_info_map_.insert("Username", oa->userName());
                    last_info_map_.insert("Password", oa->password());
                    last_info_map_.insert("AvatarType", "OpenSim");
                    emit StartOsLogin(info_map);
                    break;
                }
                case AvatarTypes::RealXtend:
                {
                    Data::RealXtendAvatar *ra = dynamic_cast<Data::RealXtendAvatar*>(data_cards.first);
                    info_map["Username"] = ra->account();
                    info_map["Password"] = ra->password();
                    info_map["AuthenticationAddress"] = ra->authUrl().toString();
                    last_info_map_.insert("Username", ra->account());
                    last_info_map_.insert("Password", ra->password());
                    last_info_map_.insert("AuthenticationAddress", ra->authUrl().toString());
                    last_info_map_.insert("AvatarType", "RealXtend");
                    emit StartRexLogin(info_map);
                    break;
                }
            }
        }

        void EtherLoginNotifier::EmitOpenSimLogin(QMap<QString, QString> info_map)
        {
            emit StartOsLogin(info_map);
        }

        void EtherLoginNotifier::EmitRealXtendLogin(QMap<QString, QString> info_map)
        {
            emit StartRexLogin(info_map);
        }

        void EtherLoginNotifier::EmitTaigaLogin(QWebFrame *web_frame)
        {
            emit StartTaigaLogin(web_frame);
        }

        void EtherLoginNotifier::EmitTaigaLogin(QString url)
        {
            emit StartTaigaLogin(url);
        }

        void EtherLoginNotifier::ExitApplication()
        {
            emit Quit();
        }

        void EtherLoginNotifier::Teleport(QString start_location)
        {
            if (!start_location.isEmpty())
            {
                last_info_map_.remove("StartLocation");
                last_info_map_.insert("StartLocation", start_location);
                
                QString avatarType = last_info_map_.value("AvatarType");
                if (avatarType.compare("OpenSim") == 0)
                    emit StartOsLogin(last_info_map_);
                else if (avatarType.compare("RealXtend") == 0)
                    emit StartRexLogin(last_info_map_);
                else
                {
                    UiServices::UiModule::LogError("Webauth avatars can't teleport yet.");
                    boost::shared_ptr<UiServices::UiModule> ui_module =  framework_->GetModuleManager()->GetModule<UiServices::UiModule>(Foundation::Module::MT_UiServices).lock();
                    if (ui_module)
                        ui_module->GetNotificationManager()->ShowNotification(new UiServices::MessageNotification("Webauth avatars cannot teleport yet, sorry."));
                }                
            }

        }

        void EtherLoginNotifier::ScriptTeleportAnswer(QString answer, QString region_name)
        {            
            if (answer.isEmpty())
                return;

            if (answer.compare("Yes") == 0)
            {                
                QTimer::singleShot(1000, this, SLOT(ScriptTeleport()));
                region_name_ = region_name;
            }
            else if (answer.compare("No") == 0)
            {
                teleporting = false;
            }
        }

        void EtherLoginNotifier::ScriptTeleport()
        {
            teleporting = false;
            Teleport(region_name_);
        }
    }
}
