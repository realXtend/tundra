// For conditions of distribution and use, see copyright notice in license.txt

#ifndef incl_NetworkEventHandler_h
#define incl_NetworkEventHandler_h

#include "ComponentInterface.h"
#include "Foundation.h"
#include "OpenSimProtocolModule.h"


namespace RexLogic
{
    class RexLogicModule;

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
        bool HandleOSNE_LogoutReply(OpenSimProtocol::NetworkEventInboundData* data);
        bool HandleOSNE_ObjectDescription(OpenSimProtocol::NetworkEventInboundData* data);        
        bool HandleOSNE_ObjectName(OpenSimProtocol::NetworkEventInboundData* data);
        bool HandleOSNE_ObjectUpdate(OpenSimProtocol::NetworkEventInboundData* data);
        bool HandleOSNE_RegionHandshake(OpenSimProtocol::NetworkEventInboundData* data);

        //! Handler functions for GenericMessages
        bool HandleRexGM_RexMediaUrl(OpenSimProtocol::NetworkEventInboundData* data);
        bool HandleRexGM_RexPrimData(OpenSimProtocol::NetworkEventInboundData* data);

        //! @return The entity corresponding to given scene entityid, or null if not found. 
        //!         This entity is guaranteed to have an existing EC_OpenSimPrim component. \todo Actually force this guarantee.
        Foundation::EntityPtr GetPrimEntity(Core::entity_id_t entityid);
        //! @return The entity corresponding to given id AND uuid. This entity is guaranteed to have an existing EC_OpenSimPrim component.
        //!         Does not return null. If the entity doesn't exist, an entity with the given entityid and fullid is created and returned.
        Foundation::EntityPtr GetOrCreatePrimEntity(Core::entity_id_t entityid, const RexUUID &fullid);
        //! @return The entity corresponding to given id. This entity is guaranteed to have an existing EC_OpenSimPrim component.
        //!         Does not return null. If the entity doesn't exist, an entity with the given fullid is created and returned.
        Foundation::EntityPtr GetPrimEntity(const RexUUID &fullid);

        Foundation::EntityPtr CreateNewPrimEntity(Core::entity_id_t entityid);

        Foundation::EntityPtr GetAvatarEntitySafe(Core::entity_id_t entityid);
        Foundation::EntityPtr CreateNewAvatarEntity(Core::entity_id_t entityid);

        //! Creates an OBB for debug visualization of the extents of the given scene object.
        void DebugCreateOgreBoundingBox(const Foundation::ComponentInterfacePtr ogrePlaceable);

        Foundation::Framework *framework_;
        
        RexLogicModule *rexlogicmodule_;
        
        typedef std::map<RexUUID, Core::entity_id_t> IDMap;
        IDMap UUIDs_;
    };
}

#endif
