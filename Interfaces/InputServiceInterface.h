// For conditions of distribution and use, see copyright notice in license.txt

#ifndef incl_Interfaces_InputServiceInterface_h
#define incl_Interfaces_InputServiceInterface_h

#include "ServiceInterface.h"

namespace Foundation
{
    //! A service for input events.
    /*! Many input events are launched as actual events, so
        they can be handled as any other events, but it is not
        practical for all input events. This service can be used
        to query the status of such input events.
    */
    class InputServiceInterface : public ServiceInterface
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
    };
}

#endif
