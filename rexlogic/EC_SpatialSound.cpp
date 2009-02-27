// For conditions of distribution and use, see copyright notice in license.txt

#include "StableHeaders.h"
#include "EC_SpatialSound.h"

EC_SpatialSound::EC_SpatialSound()
{
    mSoundId = "";
    mVolume = 0;
    mRadius = 0;
}

EC_SpatialSound::~EC_SpatialSound()
{
}

void EC_SpatialSound::handleNetworkData(std::string data) 
{
    // fixme, implement, set properties based on data
    mSoundId = data.substr(0,1); // fixme, get from real data
    mVolume = 100; // fixme, get from real data
    mRadius = 100; // fixme, get from real data
}