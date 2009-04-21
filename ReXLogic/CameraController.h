// For conditions of distribution and use, see copyright notice in license.txt

#ifndef incl_RexLogicCameraController_h
#define incl_RexLogicCameraController_h

#include "Foundation.h"
#include "InputState.h"

#include <Ogre.h>

namespace RexLogic
{
    class CameraController : public InputStateInterface
    {
    public:
        //! constructor that takes parent module
        CameraController(Foundation::ModuleInterface *module);
        virtual ~CameraController();
        
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
        void StartRotatingLeft();
        void StopRotatingLeft();
        void StartRotatingRight();
        void StopRotatingRight();
        void StartRotatingUp();
        void StopRotatingUp();
        void StartRotatingDown();
        void StopRotatingDown();

        void Drag(const Input::Events::Movement *movement);

        //! update camera position
        void Update(Core::f64 frametime);
                       
    private:
        //! parent module
        Foundation::ModuleInterface *module_;

        //! relative translation of the camera for one frame
        Ogre::Vector3 translation_;

        //! relative pitch of the camera for one frame
        Core::Real pitch_;
        //! relative yaw of the camera for one frame
        Core::Real yaw_;

        // relative pitch of the camera for one frame, frame time independent
        Ogre::Real drag_pitch_;

        // relative yaw of the camera for one frame, frame time independent
        Ogre::Real drag_yaw_;

        //! translation speed of the camera
        Core::Real sensitivity_;
        //! rotation speed of the camera
        Core::Real rot_sensitivity_;
    };

}

#endif
