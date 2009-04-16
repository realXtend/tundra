// For conditions of distribution and use, see copyright notice in license.txt

#include "StableHeaders.h"
#include "EC_OpenSimAvatar.h"

namespace RexLogic
{
    EC_OpenSimAvatar::EC_OpenSimAvatar(Foundation::ModuleInterface* module) : Foundation::ComponentInterface(module->GetFramework())
    {
        RegionHandle = 0;
        LocalId = 0;
        FullId.SetNull();
        ParentId = 0;     
    
        FirstName = "";
        LastName = "";
    }

    EC_OpenSimAvatar::~EC_OpenSimAvatar()
    {
    }
}