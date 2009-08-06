// For conditions of distribution and use, see copyright notice in license.txt

#ifndef incl_Interfaces_InputServiceInterface_h
#define incl_Interfaces_InputServiceInterface_h

#include "ServiceInterface.h"

namespace Input
{
    //! input state. See InputModuleOIS for more information
    /*!
        \ingroup Input_group
    */
    enum State
    {
        //! unknown state. For internal use!
        State_Unknown = 0,
        //! all states, useful if you want a key to correspond to one event on all states
        State_All,
        //! 1st person camera, camera is attached to the avatar
        State_FirstPerson,
        //! 3rd person camera, camera is focused on the avatar
        State_ThirdPerson,
        //! Free ghostcamera, the camera may move around freely separate from avatar
        State_FreeCamera,
        //! Buffered state, for direct key input (UI and such)
        State_Buffered,
        // add new states here

        //! number of different states
        State_Count
    };

    //! A service for input events.
    /*! Many input events are launched as actual events, so
        they can be handled as any other events, but it is not
        practical for all input events. This service can be used
        to query the status of such input events.

        Input also has a state. Keys map to different
        input events in different states, f.ex. in 3rd person
        state 'A' and 'D' keys may rotate the camera, but
        in free camera state, the keys may slide the camera
        left or right.

        For more info, see \ref Input_page "Handling input".

        \ingroup Services_group
        \ingroup Input_group
    */
    class InputServiceInterface : public Foundation::ServiceInterface
    {
    public:
        InputServiceInterface() {}
        virtual ~InputServiceInterface() {}

        //! Polls input based on the event id, returns true if the event is true
        /*! Standard way of handling input is with events. There are two issues with this:
                - Event handling code can get messy if many different types of events are handled.
                - Event_released - types of events may not always get launched properly, f.ex.
                  if user holds a key down, then moves the focus away from the Viewer window. In
                  this case the Event_released is launched only when the window regains the focus.

            Polling presents another way of handling input. This function returns true if conditions
            specific to the input event are true, false otherwise.
        */
        virtual bool Poll(Core::event_id_t input_event) const = 0;

        //! Returns a slider movement corresponding to the specified event, but only if the slider is currently being dragged.
        /*! If more than one slider matching the event is being dragged, one is chosen arbitrarily.
            Returns empty optional() if no slider matching the event is being dragged.

            Returns both absolute and relative position in 3 dimensions. Not all dimensions are supported in all
            input devices, for example mouse only supports 2 dimensions.

            \note Currently may not be thread safe.

            \param dragged_event event corresponding to a slider
            \return Absolute and relative position in 3 dimensions
        */
        virtual boost::optional<const Input::Events::Movement&> PollSlider(Core::event_id_t dragged_event) const = 0;

        //! Sets the current input state. State determines which events are lauched by which h/w input events.
        /*!
            if state parameter is omitted, restores the previous unbuffered state. This is useful for
            temporarily setting the state to buffered with SetState(State_Buffered) and then later restoring
            the previous state with SetState().

            \param state Optional input state
        */
        virtual void SetState(State state = State_Unknown) = 0;

        //! Returns the current input state
        virtual State GetState() const = 0;
    };
}

#endif
