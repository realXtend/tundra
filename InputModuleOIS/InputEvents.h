// For conditions of distribution and use, see copyright notice in license.txt

#ifndef incl_Input_InputEvents_h
#define incl_Input_InputEvents_h

#include "EventDataInterface.h"

namespace Input
{
    //! Contains some fo the input events in generic fashion. Does not depend on any one input method
    namespace Events
    {
        static const Core::event_id_t KEY_PRESSED = 1;
        static const Core::event_id_t KEY_RELEASED = 2;
        static const Core::event_id_t SCROLL = 3;

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


        //! Event for buffered key input. 
        //! Do not use for any continous input such as avatar movement, it will probably cause input lag
        class BufferedKey : public Foundation::EventDataInterface
        {
            BufferedKey();
        public:
            BufferedKey(int code, Core::uint text) : code_(code), text_(text)  {}
            virtual ~BufferedKey() {}
        
            //! OIS::KeyCode, include OISKeyboard.h for the codes
            const int code_;
            //! The key that was pressed as a character
            const Core::uint text_;
        };
    }
}

#endif
