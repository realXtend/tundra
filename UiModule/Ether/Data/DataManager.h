// For conditions of distribution and use, see copyright notice in license.txt

#ifndef incl_UiModule_DataManager_h
#define incl_UiModule_DataManager_h

#include <QObject>
#include <QSettings>
#include <QMap>
#include <QUuid>

#include "UiModuleFwd.h"

//#include "AvatarInfo.h"
//#include "WorldInfo.h"
#include "EtherDataTypes.h"

namespace Ether
{
    namespace Data
    {
        class DataManager : public QObject
        {

        Q_OBJECT

        public:
            DataManager(QObject *parent);

        public slots:
            int GetAvatarCountInSettings();
            int GetWorldCountInSettings();

            QMap<QUuid, Data::AvatarInfo*> ReadAllAvatarsFromFile();
            void StoreOrUpdateAvatar(Data::AvatarInfo *avatar_info);
            bool RemoveAvatar(Data::AvatarInfo *avatar_info);

            QMap<QUuid, Data::WorldInfo*> ReadAllWorldsFromFile();
            void StoreOrUpdateWorld(Data::WorldInfo *world_info);
            bool RemoveWorld(Data::WorldInfo *world_info);

            Data::AvatarInfo *GetAvatarInfo(QString uuid);
            QMap<QUuid, Data::AvatarInfo *> GetAvatarMap();
            Data::WorldInfo *GetWorldInfo(QString uuid);
            QMap<QUuid, Data::WorldInfo *> GetWorldMap();

        private:
            QString avatar_settings_name_;
            QString worldserver_settings_name_;

            QMap<QUuid, Data::AvatarInfo *> avatar_map_;
            QMap<QUuid, Data::WorldInfo *> world_map_;

        signals:
            void ObjectUpdated(QUuid, QString);
            void AvatarDataCreated(Data::AvatarInfo *);
            void WorldDataCreated(Data::WorldInfo *);

        };
    }
}

#endif // DATAMANAGER_H
