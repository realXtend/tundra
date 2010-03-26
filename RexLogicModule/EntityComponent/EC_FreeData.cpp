// For conditions of distribution and use, see copyright notice in license.txt

#include "StableHeaders.h"
#include "ModuleInterface.h"
#include "EntityComponent/EC_FreeData.h"

namespace RexLogic
{
    EC_FreeData::EC_FreeData(Foundation::ModuleInterface* module) : Foundation::ComponentInterface(module->GetFramework())
    {
        FreeData = ""; 
    }

    EC_FreeData::~EC_FreeData()
    {
    }
}