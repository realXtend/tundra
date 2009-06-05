// For conditions of distribution and use, see copyright notice in license.txt

#ifndef incl_RexLogic_CameraControllable_h
#define incl_RexLogic_CameraControllable_h

#include "RexTypes.h"

namespace RexLogic
{
    class CameraZoomEvent : public Foundation::EventDataInterface
    {
    public:
        ~CameraZoomEvent() {}

        Scene::EntityPtr entity;
        int amount;
    };


    //! A controller for cameras.
    /*! For more information about controllables, see EC_Controllable.

        \todo Implement properly as a controllable.
    */
    class CameraControllable
    {
    public:
        //! State flags for the camera
        enum State
        {
            FirstPerson,
            ThirdPerson,
            FreeLook
        };

        //! default constructor
        CameraControllable(Foundation::Framework *fw);
        //! destructor
        ~CameraControllable() {}

        //! Input event handler for handling controllable events
        bool HandleInputEvent(Core::event_id_t event_id, Foundation::EventDataInterface* data);

        //! Scene event handler for handling controllable events
        bool HandleSceneEvent(Core::event_id_t event_id, Foundation::EventDataInterface* data);

        //! Action event handler for handling controllable events
        bool HandleActionEvent(Core::event_id_t event_id, Foundation::EventDataInterface* data);

        //! update camera simulation for all controllable cameras
        void AddTime(Core::f64 frametime);

    private:
        State current_state_;

        //! Entity this camera is attached to in third / first person modes
        Scene::EntityWeakPtr target_entity_;

        //! current camera distance from target
        Core::Real camera_distance_;

        //! minimum camera distance from target
        Core::Real camera_min_distance_;

        //! maximum camera distance from target
        Core::Real camera_max_distance_;

        //! third person camera offset
        Core::Vector3Df camera_offset_;

        //! first person camera offset
        Core::Vector3df camera_offset_firstperson_;

        //! zoom speed
        Core::Real zoom_sensitivity_;

        //! camera pitch when dragging
        Core::Real firstperson_pitch_;

        //! mouse look sensitivity
        Core::Real firstperson_sensitivity_;

        //! drag pitch
        Core::Real drag_pitch_;

        //! name of the head bone to use for positioning first person camera
        std::string head_bone_;

        //! cached value for event category
        Core::event_category_id_t action_event_category_;

        Foundation::Framework *framework_;
    };
}

#endif
