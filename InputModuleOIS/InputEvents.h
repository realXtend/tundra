// For conditions of distribution and use, see copyright notice in license.txt

#ifndef incl_Input_InputEvents_h
#define incl_Input_InputEvents_h

#include "EventDataInterface.h"

namespace Input
{
    //! Contains all input events in generic fashion. Does not depend on any one input method
    namespace Events
    {
        static const Core::event_id_t MOUSE_WHEEL = 1;

        class MouseWheel : public Foundation::EventDataInterface
        {
            MouseWheel();
        public:
            MouseWheel(int rel, int abs) : rel_(rel), abs_(abs) {}
            virtual ~MouseWheel() {}
        
            //! relative movement of the mouse
            const int rel_;
            //! absolute movement of the mouse, if unsure relative is probably what you want
            const int abs_;
        };
    }
}

#endif
