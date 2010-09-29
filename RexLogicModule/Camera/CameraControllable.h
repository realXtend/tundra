// For conditions of distribution and use, see copyright notice in license.txt

#ifndef incl_RexLogic_CameraControllable_h
#define incl_RexLogic_CameraControllable_h

#include "InputEvents.h"
#include "ForwardDefines.h"
#include "Vector3D.h"
#include "SceneManager.h"
#include "Entity.h"
#include <QObject>
#include <QMap>

namespace Foundation
{
    class Framework;
}

namespace RexLogic
{
    class CameraZoomEvent : public IEventData
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
            FreeLook,
            Tripod
        };

        //! default constructor
        //! \param fw Framework pointer
        CameraControllable(Foundation::Framework *fw);

        //! destructor
        ~CameraControllable() {}

        //! Input event handler for handling controllable events
        bool HandleInputEvent(event_id_t event_id, IEventData* data);

        //! Scene event handler for handling controllable events
        bool HandleSceneEvent(event_id_t event_id, IEventData* data);

        //! Action event handler for handling controllable events
        bool HandleActionEvent(event_id_t event_id, IEventData* data);

        //! update camera simulation for all controllable cameras
        void AddTime(f64 frametime);

        //! set camera entity
        void SetCameraEntity(Scene::EntityPtr camera);

        //! returns camera pitch
        float GetPitch() const { return firstperson_pitch_; }
        float GetYaw() const { return firstperson_yaw_; }
        void SetYawPitch(float newyaw, float newpitch); //experimental for py api

        //! returns current state of camera
        State GetState() const { return current_state_; }

        //! Sets do we want to use terrain constraint for camera's position.
        //! param value Do we use terrain constraint.
        void SetUseTerrainConstraint(bool value) { useTerrainConstraint_ = value; }

        //! Returns true if we use terrain constraint for camera's position.
        bool GetUseTerrainConstraint() const { return useTerrainConstraint_; }

        //! Sets the terrain constraint offset.
        //! param offset New offset.
        void SetTerrainConstraintOffset(float offset) { terrainConstraintOffset_ = offset; }

        //! Return terrain constraint offset.
        float GetTerrainConstraintOffset()const { return terrainConstraintOffset_; }

        //! param value Do we use terrain constraint.
        void SetUseBoundingBoxConstraint(bool value) { useBoundaryBoxConstraint_ = value; }

        //! Returns true if we use terrain constraint for camera's position.
        bool GetUseBoundingBoxConstraint() const { return useBoundaryBoxConstraint_; }

        //! Sets camera constraint boundary box.
        //! \param min Mimimum corner point.
        //! \param max Maximum corner point.
        void SetBoundaryBox(const Vector3df &min, const Vector3df &max);

        //! Clamps current camera position taking account of the constraints used (terrain, boundary box).
        //! \param position Current camera position.
        void ClampPosition(Vector3df &position);

        Scene::EntityPtr GetCameraEntity();
        void SetThirdPersonLookAt(Vector3df lookat) { third_person_lookat_ = lookat; };
        Vector3df GetThirdPersonLookAt() { return third_person_lookat_; };

    private:
        typedef std::map<int, Vector3df> ActionTransMap;

        //! current camera state
        State current_state_;

        //! Entity this camera is attached to in third / first person modes
        Scene::EntityWeakPtr target_entity_;

        //! Camera entity
        Scene::EntityWeakPtr camera_entity_;

        //! current camera distance from target
        float camera_distance_;

        //! minimum camera distance from target
        float camera_min_distance_;

        //! maximum camera distance from target
        float camera_max_distance_;

        //! third person camera offset
        Vector3df camera_offset_;

        //! first person camera offset
        Vector3df camera_offset_firstperson_;

        Vector3df third_person_lookat_;

        //! move speed
        float sensitivity_;

        //! zoom speed
        float zoom_sensitivity_;

        //! camera pitch when dragging
        float firstperson_pitch_;

        //! camera yaw when dragging
        float firstperson_yaw_;

        //! mouse look sensitivity
        float firstperson_sensitivity_;

        //! drag pitch
        float drag_pitch_;

        //! drag yaw
        float drag_yaw_;

        //! cached value for event category
        event_category_id_t action_event_category_;

        //! translation in free look mode
        Vector3df free_translation_;

        Vector3df normalized_free_translation_;

        //! Action to translation map in free look mode
        ActionTransMap action_trans_;

        //! Framework pointer
        Foundation::Framework *framework_;

        //! Mouse-look movement
        InputEvents::Movement movement_;

        //! Do we use terrain height as a camera Z-axis constraint.
        bool useTerrainConstraint_;

        //! Terrain height constraint.offset.
        float terrainConstraintOffset_;

        //! Do we use terrain height as a camera Z-axis constraint.
        bool useBoundaryBoxConstraint_;

        //! Boundary box mimimum corner point.
        Vector3df boundaryBoxMin_;

        //! Boundary box maximum corner point.
        Vector3df boundaryBoxMax_;
    };
}

#endif
