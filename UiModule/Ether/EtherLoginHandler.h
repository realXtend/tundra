// For conditions of distribution and use, see copyright notice in license.txt

#ifndef incl_UiModule_EtherLoginHandler_h
#define incl_UiModule_EtherLoginHandler_h

#include "View/InfoCard.h"
#include "Data/AvatarInfo.h"
#include "Data/WorldInfo.h"

#include <QGraphicsScene>
#include <QObject>
#include <QPair>

namespace RexLogic
{
    class OpenSimLoginHandler;
}

namespace Ether
{
    namespace Logic
    {
        class EtherSceneController;

        class EtherLoginHandler : public QObject
        {
            Q_OBJECT

        public:
            EtherLoginHandler(QObject *parent, EtherSceneController *scene_controller);

        public slots:
            void SetOpenSimLoginHandler(RexLogic::OpenSimLoginHandler *opensim_login_handler);
            void ParseInfoFromData(QPair<Data::AvatarInfo*, Data::WorldInfo*> data_cards);
            void ExitApplication();

        private:
            EtherSceneController *scene_controller_;
            RexLogic::OpenSimLoginHandler *opensim_login_handler_;

        signals:
            void StartOsLogin(QMap<QString, QString> info_map);
            void StartRexLogin(QMap<QString, QString> info_map);
            void Quit();
        };
    }
}
#endif
