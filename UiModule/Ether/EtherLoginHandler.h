// For conditions of distribution and use, see copyright notice in license.txt

#ifndef incl_UiModule_EtherLoginHandler_h
#define incl_UiModule_EtherLoginHandler_h

#include "EtherSceneController.h"

#include "View/InfoCard.h"
#include "Data/AvatarInfo.h"
#include "Data/WorldInfo.h"
#include "EventHandlers/LoginHandler.h"

#include <QGraphicsScene>
#include <QObject>
#include <QPair>

namespace Ether
{
    namespace Logic
    {
        class EtherLoginHandler : public QObject
        {

        Q_OBJECT

        public:
            EtherLoginHandler(QObject *parent, EtherSceneController *scene_controller);

        public slots:
            void SetOpenSimLoginHandler(RexLogic::OpenSimLoginHandler *opensim_login_handler);
            void ParseInfoFromData(QPair<Data::AvatarInfo*, Data::WorldInfo*> data_cards);

        private:
            EtherSceneController *scene_controller_;
            RexLogic::OpenSimLoginHandler *opensim_login_handler_;

        signals:
            void StartOsLogin(QMap<QString, QString> info_map);
            void StartRexLogin(QMap<QString, QString> info_map);

        };
    }
}
#endif