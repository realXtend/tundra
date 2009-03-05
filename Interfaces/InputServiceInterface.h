// For conditions of distribution and use, see copyright notice in license.txt

#ifndef incl_Interfaces_InputServiceInterface_h
#define incl_Interfaces_InputServiceInterface_h

#include "ServiceInterface.h"

namespace Foundation
{
    namespace Input
    {
        struct InputEvent
        {
            //! delegate to call
            void *poco_delegate_;
            //! priority of the event
            Core::uint priority_;
            //! always triggered, even if some other system with higher priority already handled the event
            bool force_;
        };

        typedef int InputKey;
   }

    class InputServiceInterface : public ServiceInterface
    {
    public:
        InputServiceInterface() {}
        virtual ~InputServiceInterface() {}

        //! add input event. The event is triggered when specified input is detected (key down, mouse moved...)
        /*! Because each input can have several events (many systems can be interested in one input event), 
            each inputEvent should have priority. The order in which event are raised is determined by the priority.
            If event is handled by one of the system, it won't get propagated to other systems, unless the event is
            specifically overridden to do so.
        */
        virtual void AddInputEvent(Input::InputKey input, const Input::InputEvent &evt) = 0;
    };
}

#endif
