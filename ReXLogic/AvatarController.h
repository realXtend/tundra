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
        
        void UpdateMovementState(Core::event_id_t input_event_id);
        
        Core::Quaternion GetBodyRotation();
        Core::Quaternion GetHeadRotation();
    private:
        Foundation::Framework *framework_;
     
        RexLogicModule *rexlogicmodule_;
        
        uint32_t controlflags_;        
    };
}

#endif
