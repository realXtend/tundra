// For conditions of distribution and use, see copyright notice in license.txt

#ifndef __incl_Interfaces_InputServiceInterface_h__
#define __incl_Interfaces_InputServiceInterface_h__

#include "ServiceInterface.h"

namespace Foundation
{
    namespace Input
    {
        struct InputEvent
        {
            //! delegate to call
            void *mPoco_delegate;
            //! priority of the event
            Core::uint mPriority;
            //! always triggered, even if some other system with higher priority already handled the event
            bool mForce;
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
        virtual void addInputEvent(Input::InputKey input, const Input::InputEvent &evt) = 0;
    };
}

#endif
