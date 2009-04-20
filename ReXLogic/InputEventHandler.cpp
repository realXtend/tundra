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
        boost::shared_ptr<InputState> state = state_.lock();

        if (state)
        {
            switch(event_id)
            {
                case Input::Events::MOVE_FORWARD_PRESSED:
                    state->StartMovingForward();
                    break;
                case Input::Events::MOVE_FORWARD_RELEASED:
                    state->StopMovingForward();
                    break;
                case Input::Events::MOVE_BACK_PRESSED:  
                    state->StartMovingBackward();
                    break;
                case Input::Events::MOVE_BACK_RELEASED:
                    state->StopMovingBackward();
                    break;
                case Input::Events::MOVE_LEFT_PRESSED:
                    state->StartMovingLeft();
                    break;
                case Input::Events::MOVE_LEFT_RELEASED:
                    state->StopMovingLeft();
                    break;
                case Input::Events::MOVE_RIGHT_PRESSED:       
                    state->StartMovingRight();
                    break;
                case Input::Events::MOVE_RIGHT_RELEASED:
                    state->StopMovingRight();
                    break;    
            }
        } else
        {
            RexLogicModule::LogDebug("Warning: no input state present, input not handled.");
        }
        return false;
    }
}
