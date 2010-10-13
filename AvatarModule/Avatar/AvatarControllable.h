// For conditions of distribution and use, see copyright notice in license.txt

#ifndef incl_Avatar_AvatarControllable_h
#define incl_Avatar_AvatarControllable_h

#include "RexTypes.h"
#include "InputEvents.h"

#include "AvatarModule.h"
#include "AvatarModuleApi.h" 

class EC_Controllable;

namespace ProtocolUtilities
{
    class WorldStream;
}

namespace Avatar
{
    //! A controller for avatar.
    /*! For more information about controllables, see EC_Controllable.
    */
    class AV_MODULE_API AvatarControllable
    {
        enum State
        {
            FirstPerson,
            ThirdPerson,
            InActive
        };

        //! default constructor
        AvatarControllable();

    public:
        //! constructor that takes rex logic module
        AvatarControllable(AvatarModule *avatar_module);

        //! destructor
        ~AvatarControllable() {}

        //! Input event handler for handling controllable events
        bool HandleInputEvent(event_id_t event_id, IEventData* data);

        //! Scene event handler for handling controllable events
        bool HandleSceneEvent(event_id_t event_id, IEventData* data);

        //! Action event handler for handling controllable events
        bool HandleActionEvent(event_id_t event_id, IEventData* data);

        //! Add time to avatar control simulation for all controllable avatars
        void AddTime(f64 frametime);

        //! Sets the yaw of the avatar, experimental for py api
        void SetYaw(float newyaw);

        //! Sets the rotation of the avatar, experimental for py api
        void SetRotation(const Quaternion &newrot);

        //! Agent movement complete (network) event handling
        void HandleAgentMovementComplete(const RexTypes::Vector3& position, const RexTypes::Vector3& lookat);

    private:
        //! Returns avatar entity rotation
        const Quaternion &GetBodyRotation() const;

        ////! Sets rotation for the avatar entity
        //void SetBodyRotation(const Quaternion &rotation);

        //! sends avatar movement packet to server immediatelly
        void SendMovementToServer(uint controlflags);

        //! sends avatar movement packet to server only if one is scheduled currently
        void SendScheduledMovementToServer(uint controlflags);

        //! Returns true if controllable component is of type avatar (CT_AVATAR)
        /*!
            \param component Controllable component, if null, false is returned
            \return True if controllable is avatar, false otherwise
        */
        bool IsAvatar(const ComponentPtr &component) const;

        typedef std::map<int, RexTypes::ControlFlags> ActionControlFlagMap;

        //! convenient cached value for action event category
        event_category_id_t action_event_category_;

        //! Entity this controller controls
        Scene::EntityWeakPtr entity_;

        //! Convenient access to controllable component
        ComponentWeakPtr component_;

        //! event manager
        Foundation::EventManagerPtr event_manager_;

        //! mappings from input events to actions
        RexTypes::Actions::ActionInputMap input_events_;

        //! map from actions to control flags
        ActionControlFlagMap control_flags_;

        //! default speed for avatar rotation
        float rotation_sensitivity_;

        //! First or third person mode
        State current_state_;

        //! If true, we have an update that needs to be send to network
        bool net_dirty_;

        //! mouse look yaw
        float drag_yaw_;

        //! interval in seconds how often updates are send to server
        float net_updateinterval_;

        //! Tracks time between network updates
        float net_movementupdatetime_;

        //! Framework ptr
        Foundation::Framework *framework_;

        //! AvatarModule ptr
        AvatarModule *avatar_module_;

        //! Mouse-look movement
        InputEvents::Movement movement_;
    };
}
#endif
