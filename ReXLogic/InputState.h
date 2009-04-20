// For conditions of distribution and use, see copyright notice in license.txt

#ifndef incl_RexLogicInputState_h
#define incl_RexLogicInputState_h

#include "AvatarController.h"

namespace RexLogic
{
    //! abstract interface for input states
    class InputStateInterface
    {
    public:
        virtual ~InputStateInterface() {}

        virtual void StartMovingForward() {};
        virtual void StartMovingBackward() {};
        virtual void StartMovingLeft() {};
        virtual void StartMovingRight() {};
        virtual void StartMovingUp() {};
        virtual void StartMovingDown() {};
        virtual void StartRotatingLeft() {};
        virtual void StartRotatingRight() {};
        virtual void StartRotatingUp() {};
        virtual void StartRotatingDown() {};
        virtual void StartRollingLeft() {};
        virtual void StartRollingRight() {};
        virtual void StopMovingForward() {};
        virtual void StopMovingBackward() {};
        virtual void StopMovingLeft() {};
        virtual void StopMovingRight() {};
        virtual void StopMovingUp() {};
        virtual void StopMovingDown() {};
        virtual void StopRotatingLeft() {};
        virtual void StopRotatingRight() {};
        virtual void StopRotatingUp() {};
        virtual void StopRotatingDown() {};
        virtual void StopRollingLeft() {};
        virtual void StopRollingRight() {};

    };

    typedef InputStateInterface InputState;
}

#endif
