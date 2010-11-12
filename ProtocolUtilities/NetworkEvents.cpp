// For conditions of distribution and use, see copyright notice in license.txt

#include "StableHeaders.h"
#include "NetworkEvents.h"
#include "OpenSim/BuddyList.h"

namespace ProtocolUtilities
{
    ClientParameters::ClientParameters() :
        regionX(1000),
        regionY(1000)
    {
    }

    void ClientParameters::Reset()
    {
        agentID.SetNull();
        sessionID.SetNull();
        regionID.SetNull();
        circuitCode = 0;
        sessionHash = "";
        gridUrl = "";
        avatarStorageUrl = "";
        seedCapabilities = "";
        regionX = 1000;
        regionY = 1000;
        inventory.reset();
        if (buddy_list)
            buddy_list->Clear();
        buddy_list.reset();
    }
}
