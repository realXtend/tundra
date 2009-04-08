// For conditions of distribution and use, see copyright notice in license.txt

#ifndef incl_Input_InputEventsOIS_h
#define incl_Input_InputEventsOIS_h

#undef OIS_DYNAMIC_LIB
#define OIS_DYNAMIC_LIB
#include <OISKeyboard.h>
#include "EventDataInterface.h"

namespace Input
{
    //! Contains all input events in OIS specific fashion.
    namespace Events
    {
        static const Core::event_id_t KEY_PRESSED = 1;
        static const Core::event_id_t KEY_RELEASED = 2;

        //! Event for buffered key input. 
        //! Do not use for any continous input such as avatar movement, it will cause input lag
        class BufferedKey : public Foundation::EventDataInterface
        {
            BufferedKey();
        public:
            BufferedKey(OIS::KeyCode code, Core::uint text) : code_(code), text_(text)  {}
            virtual ~BufferedKey() {}
        
            const OIS::KeyCode code_;
            const Core::uint text_;
        };
    }
}

#endif
