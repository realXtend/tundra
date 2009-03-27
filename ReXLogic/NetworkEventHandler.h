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
        // !Handler functions for Opensim network events
        bool HandleOSNE_GenericMessage(OpenSimProtocol::NetworkEventInboundData* data);
        bool HandleOSNE_ObjectDescription(OpenSimProtocol::NetworkEventInboundData* data);        
        bool HandleOSNE_ObjectName(OpenSimProtocol::NetworkEventInboundData* data);
        bool HandleOSNE_ObjectUpdate(OpenSimProtocol::NetworkEventInboundData* data);

        //! Handler functions for GenericMessages
        bool HandleRexGM_RexMediaUrl(OpenSimProtocol::NetworkEventInboundData* data);
        bool HandleRexGM_RexPrimData(OpenSimProtocol::NetworkEventInboundData* data);
        
        Foundation::EntityPtr GetPrimEntitySafe(Core::entity_id_t entityid);
        Foundation::EntityPtr GetPrimEntitySafe(Core::entity_id_t entityid, RexUUID fullid);
        Foundation::EntityPtr GetPrimEntity(const RexUUID &entityuuid);
        Foundation::EntityPtr CreateNewPrimEntity(Core::entity_id_t entityid);

        Foundation::EntityPtr GetAvatarEntitySafe(Core::entity_id_t entityid);
        Foundation::EntityPtr CreateNewAvatarEntity(Core::entity_id_t entityid);

        Foundation::Framework *framework_;
        
        typedef std::map<RexUUID, Core::entity_id_t> IDMap;
        IDMap UUIDs_;
    };
}

#endif
