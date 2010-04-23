// For conditions of distribution and use, see copyright notice in license.txt

#include "StableHeaders.h"
#include "DebugOperatorNew.h"
#include "EtherLoginNotifier.h"

#include "Data/RealXtendAvatar.h"
#include "Data/OpenSimAvatar.h"
#include "Data/OpenSimWorld.h"

#include "MemoryLeakCheck.h"

namespace Ether
{
    namespace Logic
    {
        EtherLoginNotifier::EtherLoginNotifier(QObject *parent, EtherSceneController *scene_controller)
            : QObject(parent),
              scene_controller_(scene_controller)
        {
        }

        void EtherLoginNotifier::ParseInfoFromData(QPair<Data::AvatarInfo*, Data::WorldInfo*> data_cards)
        {
            QMap<QString, QString> info_map;

            switch (data_cards.second->worldType())
            {
                case WorldTypes::OpenSim:
                {
                    Data::OpenSimWorld *ow = dynamic_cast<Data::OpenSimWorld*>(data_cards.second);
                    info_map["WorldAddress"] = ow->loginUrl().toString();
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
                    emit StartOsLogin(info_map);
                    break;
                }
                case AvatarTypes::RealXtend:
                {
                    Data::RealXtendAvatar *ra = dynamic_cast<Data::RealXtendAvatar*>(data_cards.first);
                    info_map["Username"] = ra->account();
                    info_map["Password"] = ra->password();
                    info_map["AuthenticationAddress"] = ra->authUrl().toString();
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
    }
}
