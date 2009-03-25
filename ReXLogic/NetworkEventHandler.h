// For conditions of distribution and use, see copyright notice in license.txt

#ifndef incl_NetworkEventHandler_h
#define incl_NetworkEventHandler_h

#include "ComponentInterface.h"
#include "Foundation.h"
#include "OpenSimProtocolModule.h"

namespace RexLogic
{
    class NetworkEventHandler
    {
    public:
        NetworkEventHandler(Foundation::Framework *framework);
        virtual ~NetworkEventHandler();

        // !Handle network events coming from OpenSimProtocolModule
        bool HandleOpenSimNetworkEvent(Core::event_id_t event_id, Foundation::EventDataInterface* data);
    private:
        bool HandleOSNE_ObjectUpdate(OpenSimProtocol::NetworkEventInboundData* data);
        bool HandleOSNE_GenericMessage(OpenSimProtocol::NetworkEventInboundData* data);
        bool HandleOSNE_RexPrimData(OpenSimProtocol::NetworkEventInboundData* data);
        
        Foundation::EntityPtr GetEntitySafe(Core::entity_id_t entityid);
        Foundation::EntityPtr CreateNewEntity(Core::entity_id_t entityid);

        Foundation::Framework *framework_;
    };
}

#endif
