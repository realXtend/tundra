// For conditions of distribution and use, see copyright notice in license.txt

#ifndef incl_UiModule_EtherLoginNotifier_h
#define incl_UiModule_EtherLoginNotifier_h

//#include "View/InfoCard.h"
//#include "Data/AvatarInfo.h"
//#include "Data/WorldInfo.h"

//#include <QGraphicsScene>
#include <QObject>
#include <QPair>
#include <QMap>
#include <QString>

namespace Ether
{
    namespace Data
    {
        class AvatarInfo;
        class WorldInfo;
    }

    namespace Logic
    {
        class EtherSceneController;

        class EtherLoginNotifier : public QObject
        {
            Q_OBJECT

        public:
            EtherLoginNotifier(QObject *parent, EtherSceneController *scene_controller);

        public slots:
            void ParseInfoFromData(QPair<Data::AvatarInfo*, Data::WorldInfo*> data_cards);
            void ExitApplication();

        private:
            EtherSceneController *scene_controller_;

        signals:
            void StartOsLogin(QMap<QString, QString> info_map);
            void StartRexLogin(QMap<QString, QString> info_map);
            void Quit();
        };
    }
}
#endif
