// For conditions of distribution and use, see copyright notice in license.txt

#ifndef incl_UiModule_DataManager_h
#define incl_UiModule_DataManager_h

#include <QObject>
#include <QSettings>

#include "AvatarInfo.h"
#include "WorldInfo.h"
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

            QMap<QUuid, Data::WorldInfo*> ReadAllWorldsFromFile();
            void StoreOrUpdateWorld(Data::WorldInfo *world_info);

        private:
            QString avatar_settings_name_;
            QString worldserver_settings_name_;
        };
    }
}

#endif // DATAMANAGER_H
