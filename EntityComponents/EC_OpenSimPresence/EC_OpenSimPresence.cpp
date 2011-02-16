// For conditions of distribution and use, see copyright notice in license.txt

#include "StableHeaders.h"
#include "DebugOperatorNew.h"
#include <QList>
#include "MemoryLeakCheck.h"
#include "EC_OpenSimPresence.h"
#include "IModule.h"

EC_OpenSimPresence::EC_OpenSimPresence(IModule* module) :
    IComponent(module->GetFramework()),
    regionHandle(0),
    localId(0),
    parentId(0),
    agentId(RexUUID()),
    firstName(this, "firstName"),
    lastName(this, "lastName")
{
}

EC_OpenSimPresence::~EC_OpenSimPresence()
{
}

QString EC_OpenSimPresence::GetFullName() const
{
    return QString("%1 %2").arg(getfirstName(), getlastName());
}

QString EC_OpenSimPresence::QGetFullName() const
{
    return GetFullName();
}

QString EC_OpenSimPresence::QGetUUIDString() const
{
    return agentId.ToQString();
}
