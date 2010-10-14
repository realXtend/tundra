// For conditions of distribution and use, see copyright notice in license.txt

#include "Foundation.h"
#include "IEventData.h"
#include "Entity.h"
#include "RexUUID.h"

namespace Avatar
{
    namespace Events
    {
        /// Used for notifying registering from avatar to rexlogic and vice versa
        static const event_id_t EVENT_REGISTER_UUID_TO_LOCALID = 0x0;

        /// Used for notifying unregistering from avatar module to rexlogic and vice versa
        static const event_id_t EVENT_UNREGISTER_UUID_TO_LOCALID = 0x1;

        /// Used for rexlogic to handle avatar parent checks. Sent from avatar module to rexlogic
        static const event_id_t EVENT_HANDLE_AVATAR_PARENT = 0x2;

        /// Used for starting build mode, providing camera logic the pos and lookat for animating camera
        static const event_id_t EVENT_AVATAR_MODE_BEGIN = 0x3;

        /// Used for ending the avatar editor mode, so camera logic can return
        static const event_id_t EVENT_AVATAR_MODE_END = 0x4;

        class SceneRegisterEntityData : public IEventData
        {
        public:
            explicit SceneRegisterEntityData(const RexUUID &in_uuid, entity_id_t in_local_id) :
                uuid(in_uuid), local_id(in_local_id) {}
            explicit SceneRegisterEntityData(const RexUUID &in_uuid) :
                uuid(in_uuid), local_id(0) {}
            virtual ~SceneRegisterEntityData() {};
            
            RexUUID uuid;
            entity_id_t local_id;
        };

        class SceneHandleParentData : public IEventData
        {
        public:
            explicit SceneHandleParentData(uint32_t in_local_id) :
                local_id(in_local_id) {}
            virtual ~SceneHandleParentData() {};
            
            uint32_t local_id;
        };

        class AvatarModeBeginData : public IEventData
        {
        public:
            explicit AvatarModeBeginData(Scene::Entity *av_entity, Vector3df position, Vector3df lookat) :
            avatar_entity(av_entity), end_position(position), end_lookat(lookat) {}
            virtual ~AvatarModeBeginData() {};

            Scene::Entity *avatar_entity;
            Vector3df end_position;
            Vector3df end_lookat;
        };
    }
}