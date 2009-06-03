// For conditions of distribution and use, see copyright notice in license.txt

#ifndef incl_RexLogic_AvatarControllable_h
#define incl_RexLogic_AvatarControllable_h

#include "EC_Controllable.h"

namespace RexLogic
{
    //! A controller for avatar.
    /*! For more information about controllables, see EC_Controllable.
    */
    class AvatarControllable
    {
    public:
        //! default constructor
        AvatarControllable(const Foundation::EventManagerPtr &event_manager);
        //! destructor
        ~AvatarControllable() {}

        //! Input event handler for handling controller events
        bool HandleInputEvent(Core::event_id_t event_id, Foundation::EventDataInterface* data);

        //! Scene event handler for handling controller events
        bool HandleSceneEvent(Core::event_id_t event_id, Foundation::EventDataInterface* data);

        //! Action event handler for handling controller events
        bool HandleActionEvent(Core::event_id_t event_id, Foundation::EventDataInterface* data);

    private:
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
        Actions::ActionInputMap input_events_;
    };
}
#endif
