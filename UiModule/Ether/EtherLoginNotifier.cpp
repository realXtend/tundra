// For conditions of distribution and use, see copyright notice in license.txt

#include "StableHeaders.h"
#include "DebugOperatorNew.h"

#include "EtherLoginNotifier.h"
#include "Data/RealXtendAvatar.h"
#include "Data/OpenSimAvatar.h"
#include "Data/OpenSimWorld.h"

#include "ModuleManager.h"
#include "UiModule.h"
#include "Inworld/InworldSceneController.h"
#include "Inworld/ControlPanelManager.h"
#include "Inworld/ControlPanel/TeleportWidget.h"
#include "UiNotificationServices.h"

#include <QWebFrame>
#include <QTimer>

#include "MemoryLeakCheck.h"

namespace Ether
{
    namespace Logic
    {
        EtherLoginNotifier::EtherLoginNotifier(QObject *parent, EtherSceneController *scene_controller, Foundation::Framework *framework)
            : QObject(parent),
              scene_controller_(scene_controller),
              framework_(framework),
              teleporting_(false)
        {
            boost::shared_ptr<UiServices::UiModule> ui_module =  framework_->GetModuleManager()->GetModule<UiServices::UiModule>(Foundation::Module::MT_UiServices).lock();
            if (ui_module)
                connect(ui_module->GetInworldSceneController()->GetControlPanelManager()->GetTeleportWidget(), SIGNAL(StartTeleport(QString)), SLOT(Teleport(QString)));
        }

        void EtherLoginNotifier::ParseInfoFromData(QPair<Data::AvatarInfo*, Data::WorldInfo*> data_cards)
        {
            QMap<QString, QString> info_map;
            switch (data_cards.second->worldType())
            {
                case WorldTypes::OpenSim:
                {
                    Data::OpenSimWorld *ow = checked_static_cast<Data::OpenSimWorld*>(data_cards.second);
                    assert(ow);
                    info_map["WorldAddress"] = ow->loginUrl().toString();
                    info_map["StartLocation"] = ow->startLocation();
                    break;
                }
            }

            switch (data_cards.first->avatarType())
            {
                case AvatarTypes::OpenSim:
                {
                    Data::OpenSimAvatar *oa = checked_static_cast<Data::OpenSimAvatar*>(data_cards.first);
                    assert(oa);
                    info_map["Username"] = oa->userName();
                    info_map["Password"] = oa->password();
                    last_info_map_ = info_map;
                    last_info_map_["AvatarType"] = "OpenSim";
                    emit StartOsLogin(info_map);
                    break;
                }
                case AvatarTypes::RealXtend:
                {
                    Data::RealXtendAvatar *ra = checked_static_cast<Data::RealXtendAvatar*>(data_cards.first);
                    assert(ra);
                    info_map["Username"] = ra->account();
                    info_map["Password"] = ra->password();
                    info_map["AuthenticationAddress"] = ra->authUrl().toString();
                    last_info_map_ = info_map;
                    last_info_map_["AvatarType"] = "RealXtend";
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

        void EtherLoginNotifier::ScriptTeleportAnswer(QString answer, QString region_name)
        {
            answer = answer.toLower();
            if (answer == "yes")
            {
                QTimer::singleShot(1000, this, SLOT(ScriptTeleport()));
                region_name_ = region_name;
            }
            else if (answer == "no")
            {
                teleporting_ = false;
                region_name_ = "";
            }
        }

        void EtherLoginNotifier::ScriptTeleport()
        {
            if (!region_name_.isEmpty())
                Teleport(region_name_);
            teleporting_ = false;
        }

        void EtherLoginNotifier::Teleport(QString start_location)
        {
            if (start_location.isEmpty())
                return;

            last_info_map_["StartLocation"] = start_location;
            QString avatar_type = last_info_map_["AvatarType"].toLower();

            if (avatar_type == "opensim")
                emit StartOsLogin(last_info_map_);
            else if (avatar_type == "realxtend")
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
}
