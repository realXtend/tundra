// For conditions of distribution and use, see copyright notice in license.txt

#ifndef incl_Interfaces_InputServiceInterface_h
#define incl_Interfaces_InputServiceInterface_h

#include "ServiceInterface.h"

namespace Foundation
{
    class InputServiceInterface : public ServiceInterface
    {
    public:
        InputServiceInterface() {}
        virtual ~InputServiceInterface() {}

        virtual boost::optional<const Input::Events::Movement&> GetDragMovement(Core::event_id_t dragged_event) const = 0;
    };
}

#endif
