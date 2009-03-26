// For conditions of distribution and use, see copyright notice in license.txt

#include "StableHeaders.h"
#include "EC_SelectPriority.h"

namespace RexLogic
{
    EC_SelectPriority::EC_SelectPriority(Foundation::ModuleInterface* module) : Foundation::ComponentInterface(module->GetFramework())
    {
        select_priority_ = 0;
    }

    EC_SelectPriority::~EC_SelectPriority()
    {
    }
}
