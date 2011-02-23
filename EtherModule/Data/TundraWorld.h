// For conditions of distribution and use, see copyright notice in license.txt

#ifndef incl_EtherModule_TundraWorld_h
#define incl_EtherModule_TundraWorld_h

#include "WorldInfo.h"

namespace Ether
{
    namespace Data
    {
        class TundraWorld : public WorldInfo
        {

        Q_OBJECT

        public:
            TundraWorld(QUrl login_url, QString start_location, QMap<QString, QVariant> grid_info, QString path_to_pixmap = QString(), QUuid id = 0)
                : WorldInfo(WorldTypes::Tundra, login_url, start_location, grid_info, path_to_pixmap, id)
            {
            }

        public slots:
            void Print()
            {
                qDebug() << "<TundraWorld>";
                qDebug() << "    id: " << id();
                qDebug() << "    login url: " << loginUrl();
                qDebug() << "    start location: " << startLocation();
                qDebug() << "    image path: " << pixmapPath();
                qDebug() << "    grid info: " << gridInfo();
            }
        };
    }
}

#endif // TundraWORLD_H
