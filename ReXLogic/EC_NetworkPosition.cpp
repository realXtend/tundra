// For conditions of distribution and use, see copyright notice in license.txt

#include "StableHeaders.h"
#include "EC_NetworkPosition.h"

namespace RexLogic
{
    EC_NetworkPosition::EC_NetworkPosition(Foundation::ModuleInterface* module) : Foundation::ComponentInterface(module->GetFramework()),
        time_since_update_(0.0)
    {        
    }

    EC_NetworkPosition::~EC_NetworkPosition()
    {
    }
}