// For conditions of distribution and use, see copyright notice in license.txt

#ifndef incl_RexLogic_Avatar_h
#define incl_RexLogic_Avatar_h

namespace OgreRenderer
{
    class EC_OgrePlaceable;
}

#include "NetworkEvents.h"
#include "RexUUID.h"
#include "EC_OpenSimAvatar.h"
#include "AnimationDefinition.h"
#include "AvatarAppearance.h"

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
        bool HandleOSNE_AvatarAnimation(OpenSimProtocol::NetworkEventInboundData* data);

        bool HandleRexGM_RexAppearance(OpenSimProtocol::NetworkEventInboundData* data);

        void HandleTerseObjectUpdate_30bytes(const uint8_t* bytes);
        void HandleTerseObjectUpdateForAvatar_60bytes(const uint8_t* bytes);
        
        /// Update the avatar name overlay positions.
        void UpdateAvatarNameOverlayPositions();
        
        //! Updates running avatar animations
        void UpdateAvatarAnimations(Core::entity_id_t avatarid, Core::f64 frametime);
        
    private:
        RexLogicModule *rexlogicmodule_;
        
        //! @return The entity corresponding to given id AND uuid. This entity is guaranteed to have an existing EC_OpenSimAvatar component,
        //!         and EC_OpenSimPresence component.
        //!         Does not return null. If the entity doesn't exist, an entity with the given entityid and fullid is created and returned.
        Scene::EntityPtr GetOrCreateAvatarEntity(Core::entity_id_t entityid, const RexUUID &fullid);
        Scene::EntityPtr CreateNewAvatarEntity(Core::entity_id_t entityid);
        
        //! Creates mesh for the avatar / sets up appearance, animations
        void CreateAvatarMesh(Core::entity_id_t entity_id);
        
        //! Creates the name overlay above the avatar.
        //! @param placeable EC_OgrePlaceable entity component.
        //! @param entity_id Entity id of the avatar.
        void CreateNameOverlay(Foundation::ComponentPtr placeable, Core::entity_id_t entity_id);
        
        //! Show the avatar name overlay.
        //! @param entity_id Entity id of the avatar.
        void ShowAvatarNameOverlay(Core::entity_id_t entity_id);
        
        //! Starts requested avatar animations, stops others
        void StartAvatarAnimations(const RexTypes::RexUUID& avatarid, const std::vector<RexTypes::RexUUID>& anim_ids);
        
        //! Sets avatar state
        void SetAvatarState(const RexTypes::RexUUID& avatarid, EC_OpenSimAvatar::State state);

        //! Avatar state map
        typedef std::map<RexTypes::RexUUID, EC_OpenSimAvatar::State> AvatarStateMap;
        AvatarStateMap avatar_states_;
        
        //! Avatar appearance controller
        AvatarAppearance avatar_appearance_;
    };
}
#endif