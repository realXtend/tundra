// For conditions of distribution and use, see copyright notice in license.txt

#ifndef incl_AvatarController_h
#define incl_AvatarController_h

#include "Foundation.h"
#include "RexTypes.h"
#include "InputState.h"


namespace RexLogic
{
    class RexLogicModule;

    //! Implements functionality to move the client's avatar.
    class AvatarController : public InputStateInterface
    {
    public:
        AvatarController(Foundation::Framework *framework, RexLogicModule *rexlogicmodule);
        virtual ~AvatarController();
        
        //! get rotation of avatar's body        
        Core::Quaternion GetBodyRotation();
        //! set rotation of avatar's body
        void SetBodyRotation(Core::Quaternion rotation);
        //! get rotation of avatar's head
        Core::Quaternion GetHeadRotation(){  return headrotation_; }    
        
        void StartMovingForward();
        void StopMovingForward();
        void StartMovingBackward();
        void StopMovingBackward(); 
        void StartMovingLeft();
        void StopMovingLeft();
        void StartMovingRight();
        void StopMovingRight();
        void StartMovingUp();
        void StopMovingUp();
        void StartMovingDown();
        void StopMovingDown();
        void Zoom(int value);
        void StartRotatingLeft();
        void StopRotatingLeft();
        void StartRotatingRight();
        void StopRotatingRight();
        void ToggleFlyMode();
         
        void Drag(const Input::Events::Movement *movement);
                 
        //! Gets avatar entity
        Scene::EntityPtr GetAvatarEntity() const { return avatarentity_.lock(); }

        //! Sets avatar entity
        void SetAvatarEntity(Scene::EntityPtr avatar);        

        //! update camera position
        void Update(Core::f64 frametime);
        
        void HandleAgentMovementComplete(const RexTypes::Vector3& position, const RexTypes::Vector3& lookat);   
        void HandleNetworkUpdate(const RexTypes::Vector3& position, const Core::Quaternion& rotation);

        //! Check first/third mode when switching from free camera
        /*! \param if cached true, assume that firstperson mode within avatar controller is up-to-date
         */
        void CheckMode(bool cached = false);
        
    private:
        //! Constructs network control flags from user input + some automation
        void SetNetControlFlags();
        
        //! Sends agent update to server, if flags/rotation dirty & enough time elapsed of last update
        void SendMovementToServer();

        //! Stops all rotation
        void StopRotating();
        
        //! Stops all strafing
        void StopStrafing();
        
        Foundation::Framework *framework_;
     
        RexLogicModule *rexlogicmodule_;
        
        //! Control flags from user input
        uint32_t controlflags_;

        //! Own avatar
        Scene::EntityWeakPtr avatarentity_;

        //! camera distance
        float cameradistance_;
        
        //! camera offset
        RexTypes::Vector3 cameraoffset_;

        //! camera offset in first person
        /*! fallback mode only, if can't get from avatar head position
         */
        RexTypes::Vector3 cameraoffset_firstperson_;
        
        //! camera min distance
        float camera_min_distance_;
        
        //! camera max distance
        float camera_max_distance_; 

        //! relative pitch of the camera for one frame, frame time independent
        Core::Real drag_pitch_;

        //! relative yaw of the camera for one frame, frame time independent
        Core::Real drag_yaw_;

        //! third person mode turn sensitivity
        Core::Real turn_sensitivity_;
        
        //! first person mode camera sensitivity
        Core::Real rot_sensitivity_;
        
        //! first person camera pitch (radians)
        Core::Real firstperson_pitch_;
        
        //! head rotation
        Core::Quaternion headrotation_;

        //! relative yaw of the camera for one frame
        int yaw_;  
        
        //!  flag indicating if this avatar is waiting to send a network movement update
        bool net_dirtymovement_;
        
        //! how much time has been waited for the network movement update
        float net_movementupdatetime_;      
        
        //! minimum permissible time between network movement updates
        float net_updateinterval_;
        
        //! control flags to be sent to server
        uint32_t net_controlflags_;
        
        //! first person mode flag
        bool firstperson_;
        
        //! name of avatar skeleton head bone for first person tracking
        std::string head_bone_;
    };
}

#endif
