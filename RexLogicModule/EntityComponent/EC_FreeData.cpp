/**
 *  For conditions of distribution and use, see copyright notice in license.txt
 *
 *  @file   EC_FreeData.cpp
 *  @brief  An entity component that attaches to an entity a generic string of data, for quickly doing custom data extension/storage.
 */

#include "StableHeaders.h"
#include "EntityComponent/EC_FreeData.h"

#include "IModule.h"

namespace RexLogic
{
    EC_FreeData::EC_FreeData(IModule* module) :
        IComponent(module->GetFramework())
    {
    }

    EC_FreeData::~EC_FreeData()
    {
    }
}
