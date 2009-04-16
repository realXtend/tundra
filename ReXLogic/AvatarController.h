// For conditions of distribution and use, see copyright notice in license.txt

#ifndef incl_AvatarController_h
#define incl_AvatarController_h

#include "Foundation.h"
#include "RexTypes.h"

namespace RexLogic
{
    class RexLogicModule;

    class AvatarController
    {
    public:
        AvatarController(Foundation::Framework *framework, RexLogicModule *rexlogicmodule);
        virtual ~AvatarController();
                
        Core::Quaternion GetBodyRotation();
        Core::Quaternion GetHeadRotation();
        
        
        void StartMovingForward();
        void StopMovingForward();
        void StartMovingBack();
        void StopMovingBack(); 
        void StartMovingLeft();
        void StopMovingLeft();
        void StartMovingRight();
        void StopMovingRight();
                       
    private:
        Foundation::Framework *framework_;
     
        RexLogicModule *rexlogicmodule_;
        
        uint32_t controlflags_;        

        void UpdateMovementState();
    };
}

#endif
