// For conditions of distribution and use, see copyright notice in license.txt

#include "StableHeaders.h"
#include "EC_FreeData.h"

namespace RexLogic
{
    EC_FreeData::EC_FreeData(Foundation::ModuleInterface* module) : Foundation::ComponentInterface(module->GetFramework())
    {
        free_data_ = ""; 
    }

    EC_FreeData::~EC_FreeData()
    {
    }
}