// For conditions of distribution and use, see copyright notice in license.txt

#include "StableHeaders.h"
#include "EC_SpatialSound.h"

namespace RexLogic
{
    EC_SpatialSound::EC_SpatialSound(Foundation::ModuleInterface* module) : Foundation::ComponentInterface(module->GetFramework())
    {
        SoundUUID.SetNull();
        Volume = 0;
        Radius = 0;
    }

    EC_SpatialSound::~EC_SpatialSound()
    {
    }
}