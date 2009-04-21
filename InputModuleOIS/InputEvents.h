// For conditions of distribution and use, see copyright notice in license.txt

#ifndef incl_Input_InputEvents_h
#define incl_Input_InputEvents_h

#include "EventDataInterface.h"

namespace Input
{
    //! Contains some fo the input events in generic fashion. Does not depend on any one input method
    /*!
        To add new event, first add event id. For continuos events declare a 'start' and 'end' event ids.

    */
    namespace Events
    {
        // Note: Continous events should go in pairs and in sequence; for every
        //       'start' event (MOVE_FORWARD_PRESSED) there should be an
        //       'end' event (MOVE_FORWARD_RELEASED) and (MOVE_FORWARD_PRESSED == MOVE_FORWARD_RELEASED - 1)
        //       Yhis is for simplicity, so one can get the 'end' event easily from the 'start' event.
        static const Core::event_id_t KEY_PRESSED = 1;
        static const Core::event_id_t KEY_RELEASED = 2;
        static const Core::event_id_t SCROLL = 3;
        static const Core::event_id_t MOVE_FORWARD_PRESSED = 4;
        static const Core::event_id_t MOVE_FORWARD_RELEASED = 5;
        static const Core::event_id_t MOVE_BACK_PRESSED = 6;
        static const Core::event_id_t MOVE_BACK_RELEASED = 7;
        static const Core::event_id_t MOVE_LEFT_PRESSED = 8;
        static const Core::event_id_t MOVE_LEFT_RELEASED = 9;
        static const Core::event_id_t MOVE_RIGHT_PRESSED = 10;
        static const Core::event_id_t MOVE_RIGHT_RELEASED = 11;
        static const Core::event_id_t MOVE_UP_PRESSED = 12;
        static const Core::event_id_t MOVE_UP_RELEASED = 13;
        static const Core::event_id_t MOVE_DOWN_PRESSED = 14;
        static const Core::event_id_t MOVE_DOWN_RELEASED = 15;
        static const Core::event_id_t ROTATE_LEFT_PRESSED = 16;
        static const Core::event_id_t ROTATE_LEFT_RELEASED = 17;
        static const Core::event_id_t ROTATE_RIGHT_PRESSED = 18;
        static const Core::event_id_t ROTATE_RIGHT_RELEASED = 19;
        static const Core::event_id_t ROTATE_UP_PRESSED = 20;
        static const Core::event_id_t ROTATE_UP_RELEASED = 21;
        static const Core::event_id_t ROTATE_DOWN_PRESSED = 22;
        static const Core::event_id_t ROTATE_DOWN_RELEASED = 23;
        static const Core::event_id_t ROLL_LEFT_PRESSED = 24;
        static const Core::event_id_t ROLL_LEFT_RELEASED = 25;
        static const Core::event_id_t ROLL_RIGHT_PRESSED = 26;
        static const Core::event_id_t ROLL_RIGHT_RELEASED = 27;

        static const Core::event_id_t SHOW_DEBUG_CONSOLE = 28;
        static const Core::event_id_t SHOW_DEBUG_CONSOLE_REL = 29;
        static const Core::event_id_t SWITCH_CONTROLLER = 30;
        static const Core::event_id_t SWITCH_CONTROLLER_REL = 31;

        //! mouse dragged
        static const Core::event_id_t MOUSELOOK = 32;
        static const Core::event_id_t MOUSELOOK_STOPPED = 33;


        //! Event for buffered key input. 
        //! Do not use for any continous input such as avatar movement, it will probably cause input lag
        //! General use case is text input.
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

        //! Movement along axis
        struct Axis
        {
            Axis() : rel_(0), abs_(0) {}
            Axis(const Axis &other) : rel_(other.rel_), abs_(other.abs_) {}
            ~Axis() {}

            Axis &operator = (const Axis &other)
            {
                if (this != &other)
                {
                    rel_ = other.rel_;
                    abs_ = other.abs_;
                }
                return *this;
            }
            bool operator == (const Axis &other) const { return (rel_ == other.rel_ && abs_ == other.abs_); }
            bool operator != (const Axis &other) const { return !(*this == other); }


            //! relative movement
            int rel_;
            //! absolute movement
            int abs_;
        };

        //! input position that contains both relative and absolute movement for single axis
        class SingleAxisMovement : public Foundation::EventDataInterface
        {
        public:
            SingleAxisMovement() {}
            SingleAxisMovement(const SingleAxisMovement &other) : z_(other.z_) {}
            ~SingleAxisMovement() {}
            SingleAxisMovement &operator = (const SingleAxisMovement &other)
            {
                if (this != &other)
                {
                    z_ = other.z_;
                }
                return *this;
            }
            bool operator == (const SingleAxisMovement &other) const { return (z_ == other.z_); }
            bool operator != (const SingleAxisMovement &other) const { return !(*this == other); }

            Axis z_;
        };

        //! input position that contains both relative and absolute movement for several axis
        class Movement : public Foundation::EventDataInterface
        {
        public:
            Movement() {}
            Movement(const Movement &other) : x_(other.x_), y_(other.y_), z_(other.z_) {}
            ~Movement() {}
            Movement &operator = (const Movement &other)
            {
                if (this != &other)
                {
                    x_ = other.x_;
                    y_ = other.y_;
                    z_ = other.z_;
                }
                return *this;
            }
            bool operator == (const Movement &other) const { return (x_ == other.x_ && y_ == other.y_ && z_ == other.z_); }
            bool operator != (const Movement &other) const { return !(*this == other); }

            Axis x_;
            Axis y_;
            Axis z_;
        };
    }
}

#endif
