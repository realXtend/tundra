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
        seedCapabilities("") {}

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
        //inventory.reset();
        if (buddy_list)
            buddy_list->Clear();
    }
}
