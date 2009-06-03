// For conditions of distribution and use, see copyright notice in license.txt

#ifndef incl_Avatar_h
#define incl_Avatar_h

namespace Poco
{
    namespace XML
    {
        class Node;
    }
}

namespace OgreRenderer
{
    class EC_OgrePlaceable;
}

#include "NetworkEvents.h"
#include "RexUUID.h"
#include "EC_OpenSimAvatar.h"

namespace RexLogic
{
    class RexLogicModule;

    //! Defines an animation for the avatar
    struct AnimationDefinition
    {
        //! Most likely a UUID
        std::string id_;
        //! Identifying human-readable name, not mandatory and not used directly in code
        std::string name_;
        //! Actual animation name in the mesh/skeleton
        std::string animation_name_;
        //! Should play looped?
        bool looped_;
        //! Exclusive; override (stop) other animations
        bool exclusive_;
        //! Speed scaled with avatar movement speed?
        bool use_velocity_;
        //! Always restart animation when it starts playing?
        bool always_restart_;
        //! Blend-in period in seconds
        Core::Real fadein_;
        //! Blend-out period in seconds
        Core::Real fadeout_;
        //! Speed modification (1.0 original)
        Core::Real speedfactor_;
        //! Weight modification (1.0 full)
        Core::Real weightfactor_;
        
        AnimationDefinition() :
            looped_(true),
            exclusive_(false),
            use_velocity_(false),
            always_restart_(false),
            fadein_(0.0),
            fadeout_(0.0),
            speedfactor_(1.0),
            weightfactor_(1.0)
        {
        }
    };

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
        
        //! Creates a default mesh for the avatar
        void CreateDefaultAvatarMesh(Core::entity_id_t entity_id);
        
        //! Creates the name overlay above the avatar.
        //! @param placeable EC_OgrePlaceable entity component.
        //! @param entity_id Entity id of the avatar.
        void CreateNameOverlay(Foundation::ComponentPtr placeable, Core::entity_id_t entity_id);
        
        //! Show the avatar name overlay.
        //! @param entity_id Entity id of the avatar.
        void ShowAvatarNameOverlay(Core::entity_id_t entity_id);
        
        //! Reads default avatar animations from an xml file
        void ReadAnimationDefinitions(const std::string& path);
        
        //! Reads an avatar animation definition
        void ReadAnimationDefinition(Poco::XML::Node* node);
        
        //! Looks up an animation definition by animation name
        const AnimationDefinition& GetAnimDefByAnimName(const std::string& name);
        
        //! Starts requested avatar animations, stops others
        void StartAvatarAnimations(const RexTypes::RexUUID& avatarid, const std::vector<RexTypes::RexUUID>& anim_ids);
        
        //! Sets avatar state
        void SetAvatarState(const RexTypes::RexUUID& avatarid, EC_OpenSimAvatar::State state);

        //! Animation definition map
        typedef std::map<RexTypes::RexUUID, AnimationDefinition> AnimationDefinitionMap;
        AnimationDefinitionMap default_anim_defs_;
        
        //! Avatar state map
        typedef std::map<RexTypes::RexUUID, EC_OpenSimAvatar::State> AvatarStateMap;
        AvatarStateMap avatar_states_;
        
        //! Default avatar mesh name
        std::string default_avatar_mesh_;
    };
}
#endif