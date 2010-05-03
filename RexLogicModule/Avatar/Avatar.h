/**
 *  For conditions of distribution and use, see copyright notice in license.txt
 *
 *  @file   Avatar.h
 *  @brief  Avatar logic handler.
 */

#ifndef incl_RexLogic_Avatar_h
#define incl_RexLogic_Avatar_h

#include "NetworkEvents.h"
#include "RexUUID.h"
#include "EntityComponent/EC_OpenSimAvatar.h"
#include "Avatar/AvatarAppearance.h"

#include <QStringList>

namespace OgreRenderer
{
    class EC_OgrePlaceable;
}

namespace RexLogic
{
    class RexLogicModule;

    class REXLOGIC_MODULE_API Avatar
    {
     public:
        Avatar(RexLogicModule *owner);
        ~Avatar();

        bool HandleOSNE_ObjectUpdate(ProtocolUtilities::NetworkEventInboundData* data);
        bool HandleOSNE_KillObject(uint32_t objectid);
        bool HandleOSNE_AvatarAnimation(ProtocolUtilities::NetworkEventInboundData* data);

        bool HandleRexGM_RexAppearance(ProtocolUtilities::NetworkEventInboundData* data);
        bool HandleRexGM_RexAnim(ProtocolUtilities::NetworkEventInboundData* data);

        void HandleTerseObjectUpdate_30bytes(const uint8_t* bytes);
        void HandleTerseObjectUpdateForAvatar_60bytes(const uint8_t* bytes);
        
        //! Misc. frame-based update
        void Update(f64 frametime);
        
        //! Updates running avatar animations
        void UpdateAvatarAnimations(entity_id_t avatarid, f64 frametime);
        
        //! Handles resource event
        bool HandleResourceEvent(event_id_t event_id, Foundation::EventDataInterface* data);
                
        //! Handles inventory event
        bool HandleInventoryEvent(event_id_t event_id, Foundation::EventDataInterface* data);

        //! Handles asset event
        bool HandleAssetEvent(event_id_t event_id, Foundation::EventDataInterface* data);

        //! Handles logout
        void HandleLogout();
        
        //! Returns user's avatar
        Scene::EntityPtr GetUserAvatar();
        
        //! Returns whether export avatar currently supported
        bool AvatarExportSupported();
        
        //! Exports user's avatar, if in scene
        void ExportUserAvatar();
        
        //! Reloads user's avatar, if in scene
        void ReloadUserAvatar();

        //! Returns the avatar appearance handler
        AvatarAppearance& GetAppearanceHandler() { return avatar_appearance_; }

    private:
        RexLogicModule *owner_;

        //! @return The entity corresponding to given id AND uuid. This entity is guaranteed to have an existing EC_OpenSimAvatar component,
        //!         and EC_OpenSimPresence component.
        //!         Does not return null. If the entity doesn't exist, an entity with the given entityid and fullid is created and returned.
        Scene::EntityPtr GetOrCreateAvatarEntity(entity_id_t entityid, const RexUUID &fullid);
        Scene::EntityPtr CreateNewAvatarEntity(entity_id_t entityid);
        
        //! Creates mesh for the avatar / sets up appearance, animations
        void CreateAvatarMesh(entity_id_t entity_id);

        // /\todo Deprecated. use ShowAvatarNameOverlay. Remove completely when sure that this is not needed anymore.
        //! Creates the name overlay above the avatar.
        //! @param placeable EC_OgrePlaceable entity component.
        //! @param entity_id Entity id of the avatar.
//        void CreateNameOverlay(Foundation::ComponentPtr placeable, entity_id_t entity_id);

        //! Show the avatar name overlay.
        //! @param entity_id Entity id of the avatar.
        //! @param visible Do we want the overlay to be visible or not.
        void ShowAvatarNameOverlay(entity_id_t entity_id, bool visible = true);

        //! Starts requested avatar animations, stops others
        void StartAvatarAnimations(const RexUUID& avatarid, const std::vector<RexUUID>& anim_ids);

        //! Sets avatar state
        void SetAvatarState(const RexUUID& avatarid, EC_OpenSimAvatar::State state);

        //! Avatar state map
        typedef std::map<RexUUID, EC_OpenSimAvatar::State> AvatarStateMap;
        AvatarStateMap avatar_states_;

        //! Avatar appearance controller
        AvatarAppearance avatar_appearance_;

        //! Pending avatar appearances
        typedef std::map<RexUUID, std::string> AvatarAppearanceMap;
        AvatarAppearanceMap pending_appearances_;
    };
}

#endif
