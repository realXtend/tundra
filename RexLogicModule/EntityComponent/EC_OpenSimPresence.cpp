// For conditions of distribution and use, see copyright notice in license.txt

#include "StableHeaders.h"
#include "ModuleInterface.h"
#include "EntityComponent/EC_OpenSimPresence.h"

namespace RexLogic
{
    EC_OpenSimPresence::EC_OpenSimPresence(Foundation::ModuleInterface* module) : 
        Foundation::ComponentInterface(module->GetFramework()),
        RegionHandle(0),
        LocalId(0),
        ParentId(0)
    {
        FullId.SetNull();    
    }

    EC_OpenSimPresence::~EC_OpenSimPresence()
    {
    }

    void EC_OpenSimPresence::SetFirstName(const std::string &name)
    {
        first_name_ = name;
    } 

    std::string EC_OpenSimPresence::GetFirstName()
    {
        return first_name_;
    }

    void EC_OpenSimPresence::SetLastName(const std::string &name)
    {
        last_name_ = name;
    }

    std::string EC_OpenSimPresence::GetLastName()
    {
        return last_name_;
    }

    std::string EC_OpenSimPresence::GetFullName()
    {
        return first_name_ + " " + last_name_;
    }
}
