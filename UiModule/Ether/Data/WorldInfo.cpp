// For conditions of distribution and use, see copyright notice in license.txt

#include "StableHeaders.h"
#include "DebugOperatorNew.h"
#include "WorldInfo.h"

#include "MemoryLeakCheck.h"

namespace Ether
{
    namespace Data
    {
        WorldInfo::WorldInfo(Ether::WorldTypes::World world_type, QUrl login_url, QMap<QString, QVariant> grid_info, QString path_to_pixmap, QUuid id)
            : QObject(),
              path_to_pixmap_(path_to_pixmap),
              world_type_(world_type),
              login_url_(login_url),
              grid_info_(grid_info)
        {
            if (id.isNull())
                id_ = QUuid::createUuid();
            else
                id_ = id;
            id_string_ = id_.toString().midRef(1, id_.toString().length()-2).toString();
        }
    }
}
