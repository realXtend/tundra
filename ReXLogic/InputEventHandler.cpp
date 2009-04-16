// For conditions of distribution and use, see copyright notice in license.txt

#include "StableHeaders.h"
#include "InputEventHandler.h"
#include "InputEvents.h"
#include "RexLogicModule.h"

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
        switch(event_id)
        {
            case Input::Events::MOVE_FORWARD_PRESSED:
            case Input::Events::MOVE_FORWARD_RELEASED:
            case Input::Events::MOVE_BACK_PRESSED:  
            case Input::Events::MOVE_BACK_RELEASED:
            case Input::Events::MOVE_LEFT_PRESSED:
            case Input::Events::MOVE_LEFT_RELEASED:
            case Input::Events::MOVE_RIGHT_PRESSED:       
            case Input::Events::MOVE_RIGHT_RELEASED:
                rexlogicmodule_->GetAvatarController()->UpdateMovementState(event_id);
                break;    
        }
        return false;
    }
}
