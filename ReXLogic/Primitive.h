// For conditions of distribution and use, see copyright notice in license.txt

#ifndef incl_Primitive_h
#define incl_Primitive_h

#include "NetworkEvents.h"

namespace RexLogic
{
    class RexLogicModule;

    class Primitive
    {
     public:
        Primitive(RexLogicModule *rexlogicmodule);
        virtual ~Primitive();
        
        bool HandleOSNE_ObjectUpdate(OpenSimProtocol::NetworkEventInboundData* data);
        bool HandleOSNE_KillObject(uint32_t objectid); 
        bool HandleOSNE_ObjectProperties(OpenSimProtocol::NetworkEventInboundData* data);
                
        bool HandleRexGM_RexMediaUrl(OpenSimProtocol::NetworkEventInboundData* data);
        bool HandleRexGM_RexPrimData(OpenSimProtocol::NetworkEventInboundData* data);
        
        void HandleTerseObjectUpdateForPrim_60bytes(const uint8_t* bytes);
                             
    private:
        RexLogicModule *rexlogicmodule_;
    
        //! @return The entity corresponding to given id AND uuid. This entity is guaranteed to have an existing EC_OpenSimPrim component.
        //!         Does not return null. If the entity doesn't exist, an entity with the given entityid and fullid is created and returned.
        Foundation::EntityPtr GetOrCreatePrimEntity(Core::entity_id_t entityid, const RexUUID &fullid);
        Foundation::EntityPtr CreateNewPrimEntity(Core::entity_id_t entityid);
        
        //! handle rexprimdata blob coming from server in a genericmessage
        void HandleRexPrimDataBlob(Core::entity_id_t entityid, const uint8_t* primdata);
    };
}
#endif