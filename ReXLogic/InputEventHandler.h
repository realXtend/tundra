// For conditions of distribution and use, see copyright notice in license.txt

#ifndef incl_InputEventHandler_h
#define incl_InputEventHandler_h

#include "ComponentInterface.h"
#include "Foundation.h"

namespace RexLogic
{
    class RexLogicModule;

    class InputEventHandler
    {
    public:
        InputEventHandler(Foundation::Framework *framework, RexLogicModule *rexlogicmodule);
        virtual ~InputEventHandler();
        
        bool HandleInputEvent(Core::event_id_t event_id, Foundation::EventDataInterface* data);
    private:
        Foundation::Framework *framework_;
     
        RexLogicModule *rexlogicmodule_;
    };
}

#endif
