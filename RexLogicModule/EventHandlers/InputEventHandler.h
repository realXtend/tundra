// For conditions of distribution and use, see copyright notice in license.txt

#ifndef incl_InputEventHandler_h
#define incl_InputEventHandler_h

#include "CoreTypes.h"

namespace Foundation
{
    class EventDataInterface;
}

namespace RexLogic
{
    class InputStateInterface;
    class RexLogicModule;

    //! Performs the keyboard and mouse input handling of RexLogic.
    class InputEventHandler
    {
    public:
        explicit InputEventHandler(RexLogicModule *rexlogicmodule);
        virtual ~InputEventHandler();

        //! handle an input event
        bool HandleInputEvent(event_id_t event_id, Foundation::EventDataInterface* data);

        void Update(f64 frametime);

    private:
        RexLogicModule *rexlogicmodule_;
    };
}

#endif
