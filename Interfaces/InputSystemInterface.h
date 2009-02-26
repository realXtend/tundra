

#ifndef __incl_Interfaces_InputSystemInterface_h__
#define __incl_Interfaces_InputSystemInterface_h__

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

   class InputSystemInterface : public ServiceInterface
   {
   public:
      InputSystemInterface() {}
      virtual ~InputSystemInterface() {}

      //! add input event. The event is triggered when specified input is detected (key down, mouse moved...)
      /*! Because each input can have several events (many systems can be interested in one input event), 
          each inputEvent should have priority. The order in which event are raised is determined by the priority.
          If event is handled by one of the system, it won't get propagated to other systems, unless the event is
          specifically overridden to do so.
      */
      virtual void addInputEvent(Input::InputKey input, const Input::InputEvent &evt) = 0;
   };
}

#endif // __incl_Interfaces_InputSystemInterface_h__
