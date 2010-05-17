// For conditions of distribution and use, see copyright notice in license.txt

#include "StableHeaders.h"
#include "NetworkEvents.h"
#include "OpenSim/BuddyList.h"

namespace ProtocolUtilities
{
    ClientParameters::ClientParameters() :
        agentID(RexUUID()),
        sessionID(RexUUID()),
        regionID(RexUUID()),
        circuitCode(0),
        sessionHash(""),
        gridUrl(""),
        avatarStorageUrl(""),
        seedCapabilities(""),
        regionX(1000),
        regionY(1000) {}

    /// Resets parameters.
    void ClientParameters::Reset()
    {
        agentID = RexUUID();
        sessionID = RexUUID();
        regionID = RexUUID();
        uint32_t circuitCode = 0;
        std::string sessionHash = "";
        std::string gridUrl = "";
        std::string avatarStorageUrl = "";
        std::string seedCapabilities = "";        
        uint16_t regionX = 1000;
        uint16_t regionY = 1000;
        //inventory.reset();
        if (buddy_list)
            buddy_list->Clear();
    }
}
