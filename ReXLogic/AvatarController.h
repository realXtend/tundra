// For conditions of distribution and use, see copyright notice in license.txt

#ifndef incl_AvatarController_h
#define incl_AvatarController_h

#include "Foundation.h"
#include "RexTypes.h"
#include "InputState.h"

namespace RexLogic
{
    class RexLogicModule;

    class AvatarController : public InputStateInterface
    {
    public:
        AvatarController(Foundation::Framework *framework, RexLogicModule *rexlogicmodule);
        virtual ~AvatarController();
                
        Core::Quaternion GetBodyRotation();
        Core::Quaternion GetHeadRotation();
        
        
        void StartMovingForward();
        void StopMovingForward();
        void StartMovingBackward();
        void StopMovingBackward(); 
        void StartMovingLeft();
        void StopMovingLeft();
        void StartMovingRight();
        void StopMovingRight();
        void Zoom(int value);
         
        //! Gets avatar entity
        Foundation::EntityPtr GetAvatarEntity() const { return avatarentity_; }

        //! Sets avatar entity
        void SetAvatarEntity(Foundation::EntityPtr avatar);        

        //! update camera position
        void Update(Core::f64 frametime);   

    private:
        Foundation::Framework *framework_;
     
        RexLogicModule *rexlogicmodule_;
        
        uint32_t controlflags_;        

        //! Own avatar
        Foundation::EntityPtr avatarentity_;

        void UpdateMovementState();

        //! camera distance
        float cameradistance_;
        
        //! camera offset
        RexTypes::Vector3 cameraoffset_;
        
        //! camera min distance
        float camera_min_distance_;
        
        //! camera max distance
        float camera_max_distance_;        
    };
}

#endif
