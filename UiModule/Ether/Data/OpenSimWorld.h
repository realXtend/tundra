// For conditions of distribution and use, see copyright notice in license.txt

#ifndef incl_UiModule_OpenSimWorld_h
#define incl_UiModule_OpenSimWorld_h

#include "WorldInfo.h"

namespace Ether
{
    namespace Data
    {
        class OpenSimWorld : public WorldInfo
        {

        Q_OBJECT

        public:
            OpenSimWorld(QUrl login_url, QString start_location, QMap<QString, QVariant> grid_info, QString path_to_pixmap = QString(), QUuid id = 0)
                : WorldInfo(WorldTypes::OpenSim, login_url, start_location, grid_info, path_to_pixmap, id)
            {
            }

        public slots:
            void Print()
            {
                qDebug() << "<OpenSimWorld>";
                qDebug() << "    id: " << id();
                qDebug() << "    login url: " << loginUrl();
				qDebug() << "    start location: " << startLocation();
                qDebug() << "    image path: " << pixmapPath();
                qDebug() << "    grid info: " << gridInfo();
            }
        };
    }
}

#endif // OPENSIMWORLD_H
