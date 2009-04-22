// For conditions of distribution and use, see copyright notice in license.txt

#ifndef incl_Interfaces_InputServiceInterface_h
#define incl_Interfaces_InputServiceInterface_h

#include "ServiceInterface.h"

namespace Input
{
    //! input state. See InputModuleOIS for more information
    enum State
    {
        //! unknown state
        State_Unknown = 0,
        //! all states, useful if you want a key to correspond the one event on all states
        State_All,
        //! 1st person camera
        State_FirstPerson,
        //! 3rd person camera
        State_ThirdPerson,
        //! Free ghostcamera
        State_FreeCamera,
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
    */
    class InputServiceInterface : public Foundation::ServiceInterface
    {
    public:
        InputServiceInterface() {}
        virtual ~InputServiceInterface() {}

        //! Returns a slider movement corresponding to the specified event, but only if the slider is currently being dragged.
        /*! If more than one slider matching the event is being dragged, one is chosen arbitrarily.
            Returns empty optional() if no slider matching the event is being dragged.

            Returns both absolute and relative position in 3 dimensions. Not all dimensions are supported in all
            input devices, for example mouse only supports 2 dimensions.

            \param dragged_event event corresponding to a slider
            \return Absolute and relative position in 3 dimensions
        */
        virtual boost::optional<const Input::Events::Movement&> GetSliderMovement(Core::event_id_t dragged_event) const = 0;

        //! Sets the current input state. State determines which events are lauched by which h/w input events.
        virtual void SetState(State state) = 0;

        //! Returns the current input state
        virtual State GetState() const = 0;
    };
}

#endif
