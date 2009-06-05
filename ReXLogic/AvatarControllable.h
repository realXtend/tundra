// For conditions of distribution and use, see copyright notice in license.txt

#ifndef incl_RexLogic_AvatarControllable_h
#define incl_RexLogic_AvatarControllable_h

#include "EC_Controllable.h"
#include "RexTypes.h"

namespace RexLogic
{
    class RexServerConnection;
    typedef boost::shared_ptr<RexServerConnection> RexServerConnectionPtr;

    //! Type of this controllable
    //const int CT_AVATAR = 1;
    //namespace Actions
    //{
    //    const int FlyMode = 100;
    //}

    //! A controller for avatar.
    /*! For more information about controllables, see EC_Controllable.
    */
    class AvatarControllable
    {
        enum State
        {
            FirstPerson,
            ThirdPerson
        };

    public:
        //! default constructor
        AvatarControllable(Foundation::Framework *fw, const RexServerConnectionPtr &connection, const Foundation::EventManagerPtr &event_manager);
        //! destructor
        ~AvatarControllable() {}

        //! Input event handler for handling controllable events
        bool HandleInputEvent(Core::event_id_t event_id, Foundation::EventDataInterface* data);

        //! Scene event handler for handling controllable events
        bool HandleSceneEvent(Core::event_id_t event_id, Foundation::EventDataInterface* data);

        //! Action event handler for handling controllable events
        bool HandleActionEvent(Core::event_id_t event_id, Foundation::EventDataInterface* data);

        //! Add time to avatar control simulation for all controllable avatars
        void AddTime(Core::f64 frametime);

    private:
        //! Returns avatar entity rotation
        const Core::Quaternion &GetBodyRotation() const;

        ////! Sets rotation for the avatar entity
        //void SetBodyRotation(const Core::Quaternion &rotation);

        //! sends avatar movement packet to server immediatelly
        void SendMovementToServer(Core::uint controlflags);

        //! sends avatar movement packet to server only if one is scheduled currently
        void SendScheduledMovementToServer(Core::uint controlflags);

        //! Returns true if controllable component is of type avatar (CT_AVATAR)
        /*!
            \param component Controllable component, if null, false is returned
            \return True if controllable is avatar, false otherwise
        */
        bool IsAvatar(const Foundation::ComponentPtr &component)
        {
            if (!component) return false;
            return (checked_static_cast<EC_Controllable*>(component.get())->GetType() == RexTypes::CT_AVATAR);
        }

        typedef std::map<int, RexTypes::ControlFlags> ActionControlFlagMap;

        //! convenient cached value for input event category
        Core::event_category_id_t input_event_category_;

        //! convenient cached value for scene event category
        Core::event_category_id_t scene_event_category_;

        //! convenient cached value for action event category
        Core::event_category_id_t action_event_category_;

        //! Entity this controller controls
        Scene::EntityWeakPtr entity_;

        //! Convenient access to controllable component
        Foundation::ComponentWeakPtr component_;

        //! event manager
        Foundation::EventManagerPtr event_manager_;

        //! mappings from input events to actions
        RexTypes::Actions::ActionInputMap input_events_;

        //! map from actions to control flags
        ActionControlFlagMap control_flags_;

        //! Rex server connection used to send updates
        RexServerConnectionPtr connection_;

        //! default speed for avatar rotation
        Core::f32 rotation_sensitivity_;

        //! First or third person mode
        State current_state_;

        //! If true, we have an update that needs to be send to network
        bool net_dirty_;

        //! mouse look yaw
        Core::Real drag_yaw_;

        //! interval in seconds how often updates are send to server
        Core::Real net_updateinterval_;

        //! Tracks time between network updates
        Core::Real net_movementupdatetime_;

        Foundation::Framework *framework_;
    };
}
#endif
