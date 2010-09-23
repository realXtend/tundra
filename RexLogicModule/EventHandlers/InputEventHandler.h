// For conditions of distribution and use, see copyright notice in license.txt

#ifndef incl_InputEventHandler_h
#define incl_InputEventHandler_h

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

        //! Updates xxx
        //! \param frametime Time since the last frame.
        void Update(f64 frametime);

    private:
        //! Owner module.
        RexLogicModule *owner_;

        //! Last position of mouse cursor on the screen.
        std::pair<int, int> lastMousePosition_;

        //! Last call time of Update() function.
        f64 lastCallTime_;

        //! Raycast interval for mouse hovering in seconds.
        double raycastInterval_; ///\todo Remove this altogether. -jj.
    };
}

#endif
