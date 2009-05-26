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
         
        //! Gets avatar entity
        Scene::EntityPtr GetAvatarEntity() const { return avatarentity_; }

        //! Sets avatar entity
        void SetAvatarEntity(Scene::EntityPtr avatar);        

        //! update camera position
        void Update(Core::f64 frametime);
        
        void HandleAgentMovementComplete(const RexTypes::Vector3& position, const RexTypes::Vector3& lookat);   
        void HandleNetworkUpdate(const RexTypes::Vector3& position, const Core::Quaternion& rotation);

    private:
        Foundation::Framework *framework_;
     
        RexLogicModule *rexlogicmodule_;
        
        uint32_t controlflags_;        

        //! Own avatar
        Scene::EntityPtr avatarentity_;

        void SendMovementToServer();

        //! camera distance
        float cameradistance_;
        
        //! camera offset
        RexTypes::Vector3 cameraoffset_;
        
        //! camera min distance
        float camera_min_distance_;
        
        //! camera max distance
        float camera_max_distance_; 

        //! head rotation
        Core::Quaternion headrotation_;

        //! relative yaw of the camera for one frame
        int yaw_;  
        
        //!  flag indicating if this avatar is waiting to send a network movement update
        bool net_dirtymovement_;
        
        //! how much time has been waited for the network movement update
        float net_movementupdatetime_;      
    };
}

#endif
