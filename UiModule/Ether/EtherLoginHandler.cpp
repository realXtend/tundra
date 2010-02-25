// For conditions of distribution and use, see copyright notice in license.txt

#include "StableHeaders.h"
#include "EtherLoginHandler.h"

#include "Data/RealXtendAvatar.h"
#include "Data/OpenSimAvatar.h"
#include "Data/OpenSimWorld.h"

#include "EventHandlers/LoginHandler.h"

namespace Ether
{
    namespace Logic
    {
        EtherLoginHandler::EtherLoginHandler(QObject *parent, EtherSceneController *scene_controller)
            : QObject(parent),
              scene_controller_(scene_controller),
              opensim_login_handler_(0)
        {
        }

        void EtherLoginHandler::SetOpenSimLoginHandler(RexLogic::OpenSimLoginHandler *opensim_login_handler)
        {
            opensim_login_handler_ = opensim_login_handler;

            connect(this, SIGNAL( StartOsLogin(QMap<QString, QString>) ), 
                    opensim_login_handler_, SLOT( ProcessOpenSimLogin(QMap<QString, QString>) ));
            connect(this, SIGNAL( StartRexLogin(QMap<QString, QString>) ), 
                    opensim_login_handler_, SLOT( ProcessRealXtendLogin(QMap<QString, QString>) ));
            connect(this, SIGNAL( Quit() ),
                    opensim_login_handler_, SLOT( Quit() ));
        }

        void EtherLoginHandler::ParseInfoFromData(QPair<Data::AvatarInfo*, Data::WorldInfo*> data_cards)
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

        void EtherLoginHandler::ExitApplication()
        {
            emit Quit();
        }
    }
}
