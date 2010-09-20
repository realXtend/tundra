// For conditions of distribution and use, see copyright notice in license.txt

#include "StableHeaders.h"
#include "EC_OpenSimPresence.h"
#include "IModule.h"

EC_OpenSimPresence::EC_OpenSimPresence(IModule* module) :
    IComponent(module->GetFramework()),
    regionHandle(0),
    localId(0),
    parentId(0),
    agentId(RexUUID())
{
}

EC_OpenSimPresence::~EC_OpenSimPresence()
{
}

void EC_OpenSimPresence::SetFirstName(const std::string &name)
{
    first_name_ = name;
}

std::string EC_OpenSimPresence::GetFirstName() const
{
    return first_name_;
}

void EC_OpenSimPresence::SetLastName(const std::string &name)
{
    last_name_ = name;
}

std::string EC_OpenSimPresence::GetLastName() const
{
    return last_name_;
}

std::string EC_OpenSimPresence::GetFullName() const
{
    return first_name_ + " " + last_name_;
}

QString EC_OpenSimPresence::QGetFullName() const
{
    std::string fullname = first_name_ + " " + last_name_;
    return QString(fullname.c_str());
}

QString EC_OpenSimPresence::QGetUUIDString() const
{
    return agentId.ToQString();
}
