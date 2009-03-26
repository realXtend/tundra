// For conditions of distribution and use, see copyright notice in license.txt

#include "StableHeaders.h"
#include "EC_SpatialSound.h"

namespace RexLogic
{
    EC_SpatialSound::EC_SpatialSound(Foundation::ModuleInterface* module) : Foundation::ComponentInterface(module->GetFramework())
    {
        sound_id_ = "";
        volume_ = 0;
        radius_ = 0;
    }

    EC_SpatialSound::~EC_SpatialSound()
    {
    }

    void EC_SpatialSound::HandleNetworkData(std::string data) 
    {
        // fixme, implement, set properties based on data
        sound_id_ = data.substr(0,1); // fixme, get from real data
        volume_ = 100; // fixme, get from real data
        radius_ = 100; // fixme, get from real data
    }
}