// For conditions of distribution and use, see copyright notice in license.txt

#ifndef incl_InputEventHandler_h
#define incl_InputEventHandler_h

/*
 *  !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
 *  THIS FILE IS DEPRECATED. DO NOT ADD NEW CODE. USE NEW INPUT CONTEX SYSTEM INSTEAD
 *  !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
 */

#include "CoreTypes.h"

class IEventData;

namespace RexLogic
{
    class RexLogicModule;

    //! Performs the keyboard and mouse input handling of RexLogic.
    class InputEventHandler
    {
    public:
        //! Constructor.
        //! \param owner Owner module.
        explicit InputEventHandler(RexLogicModule *owner);

        //! Destructor.
        virtual ~InputEventHandler();

        //! Handles an input event.
        bool HandleInputEvent(event_id_t event_id, IEventData* data);

    private:
        //! Owner module.
        RexLogicModule *owner_;
    };
}

#endif
