// For conditions of distribution and use, see copyright notice in license.txt

#include "StableHeaders.h"
#include "WorldLogic.h"
#include "NetworkEventHandler.h"



WorldLogic::WorldLogic(Foundation::Framework *framework)
{
    mFramework = framework;
    mNetworkHandler = new NetworkEventHandler(framework);
}

WorldLogic::~WorldLogic()
{
    SAFE_DELETE (mNetworkHandler);
}

