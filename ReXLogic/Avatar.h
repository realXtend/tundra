// For conditions of distribution and use, see copyright notice in license.txt

#ifndef incl_Avatar_h
#define incl_Avatar_h

#include "NetworkEvents.h"

namespace RexLogic
{
    class RexLogicModule;

    class Avatar
    {
     public:
        Avatar(RexLogicModule *rexlogicmodule);
        ~Avatar();

        bool HandleOSNE_ObjectUpdate(OpenSimProtocol::NetworkEventInboundData* data);
        bool HandleOSNE_KillObject(uint32_t objectid);

        bool HandleRexGM_RexAppearance(OpenSimProtocol::NetworkEventInboundData* data);

        void HandleTerseObjectUpdate_30bytes(const uint8_t* bytes);
        void HandleTerseObjectUpdateForAvatar_60bytes(const uint8_t* bytes);

    private:
        RexLogicModule *rexlogicmodule_;

        //! @return The entity corresponding to given id AND uuid. This entity is guaranteed to have an existing EC_OpenSimAvatar component.
        //!         Does not return null. If the entity doesn't exist, an entity with the given entityid and fullid is created and returned.
        Foundation::EntityPtr GetOrCreateAvatarEntity(Core::entity_id_t entityid, const RexUUID &fullid);
        Foundation::EntityPtr CreateNewAvatarEntity(Core::entity_id_t entityid);
    };
}
#endif