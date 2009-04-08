// For conditions of distribution and use, see copyright notice in license.txt

#include "StableHeaders.h"
#include "InputEventHandler.h"

namespace RexLogic
{
    InputEventHandler::InputEventHandler(Foundation::Framework *framework, RexLogicModule *rexlogicmodule)
    {
        framework_ = framework;
        rexlogicmodule_ = rexlogicmodule;    
    }

    InputEventHandler::~InputEventHandler()
    {
    }
    
    bool InputEventHandler::HandleInputEvent(Core::event_id_t event_id, Foundation::EventDataInterface* data)
    {
        // todo tucofixme, handle input events
        return false;
    }
}
