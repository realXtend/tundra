// For conditions of distribution and use, see copyright notice in license.txt

#include "StableHeaders.h"
#include "DebugOperatorNew.h"
#include "AvatarInfo.h"

#include "MemoryLeakCheck.h"

namespace Ether
{
    namespace Data
    {
        AvatarInfo::AvatarInfo(Ether::AvatarTypes::Avatar avatar_type, QString password, QString path_to_pixmap, QUuid id)
            : QObject(),
              password_(password),
              path_to_pixmap_(path_to_pixmap),
              avatar_type_(avatar_type)
        {
            if (id.isNull())
                id_ = QUuid::createUuid();
            else
                id_ = id;
            id_string_ = id_.toString().midRef(1, id_.toString().length()-2).toString();
        }
    }
}
