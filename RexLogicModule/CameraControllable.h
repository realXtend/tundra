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
        bool HandleInputEvent(event_id_t event_id, Foundation::EventDataInterface* data);

        //! Scene event handler for handling controllable events
        bool HandleSceneEvent(event_id_t event_id, Foundation::EventDataInterface* data);

        //! Action event handler for handling controllable events
        bool HandleActionEvent(event_id_t event_id, Foundation::EventDataInterface* data);

        //! update camera simulation for all controllable cameras
        void AddTime(f64 frametime);

        //! returns camera pitch
        Real GetPitch() const { return firstperson_pitch_; }
		void SetYawPitch(Real newyaw, Real newpitch); //experimental for py api

    private:
        typedef std::map<int, Vector3df> ActionTransMap;

        //! current camera state
        State current_state_;

        //! Entity this camera is attached to in third / first person modes
        Scene::EntityWeakPtr target_entity_;

        //! current camera distance from target
        Real camera_distance_;

        //! minimum camera distance from target
        Real camera_min_distance_;

        //! maximum camera distance from target
        Real camera_max_distance_;

        //! third person camera offset
        Vector3Df camera_offset_;

        //! first person camera offset
        Vector3df camera_offset_firstperson_;

        //! move speed
        Real sensitivity_;

        //! zoom speed
        Real zoom_sensitivity_;

        //! camera pitch when dragging
        Real firstperson_pitch_;

        //! camera yaw when dragging
        Real firstperson_yaw_;

        //! mouse look sensitivity
        Real firstperson_sensitivity_;

        //! drag pitch
        Real drag_pitch_;

        //! drag yaw
        Real drag_yaw_;

        //! cached value for event category
        event_category_id_t action_event_category_;

        //! translation in free look mode
        Vector3df free_translation_;

        Vector3df normalized_free_translation_;

        //! Action to translation map in free look mode
        ActionTransMap action_trans_;

        Foundation::Framework *framework_;
    };
}

#endif
