// For conditions of distribution and use, see copyright notice in license.txt

#ifndef incl_NetworkEventHandler_h
#define incl_NetworkEventHandler_h

#include "ComponentInterface.h"
#include "Foundation.h"
#include "NetworkEvents.h"

namespace OpenSimProtocol
{
    class OpenSimProtocolModule;
}

namespace RexLogic
{
    struct DecodedTerrainPatch;
    class RexLogicModule;

    /// Handles incoming SLUDP network events in a reX-specific way. \todo Break down into more logical functions.
    class NetworkEventHandler
    {
    public:
        NetworkEventHandler(Foundation::Framework *framework, RexLogicModule *rexlogicmodule);
        virtual ~NetworkEventHandler();

        // !Handle network events coming from OpenSimProtocolModule
        bool HandleOpenSimNetworkEvent(Core::event_id_t event_id, Foundation::EventDataInterface* data);
    private:
        // !Handler functions for Opensim network events
        bool HandleOSNE_AgentMovementComplete(OpenSimProtocol::NetworkEventInboundData* data);
        bool HandleOSNE_GenericMessage(OpenSimProtocol::NetworkEventInboundData* data);
        bool HandleOSNE_ImprovedTerseObjectUpdate(OpenSimProtocol::NetworkEventInboundData* data);
        bool HandleOSNE_KillObject(OpenSimProtocol::NetworkEventInboundData* data);
        bool HandleOSNE_LogoutReply(OpenSimProtocol::NetworkEventInboundData* data);
        bool HandleOSNE_ObjectUpdate(OpenSimProtocol::NetworkEventInboundData* data);

        bool HandleOSNE_RegionHandshake(OpenSimProtocol::NetworkEventInboundData* data);

        //! Handler functions for GenericMessages

        void DebugCreateTerrainVisData(const DecodedTerrainPatch &heightData, int patchSize);

        Foundation::Framework *framework_;
        
        boost::weak_ptr<OpenSimProtocol::OpenSimProtocolModule> netInterface_;
        
        RexLogicModule *rexlogicmodule_;
    };
}

#endif
