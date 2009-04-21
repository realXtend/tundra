// For conditions of distribution and use, see copyright notice in license.txt

#include "StableHeaders.h"
#include "InputEventHandler.h"
#include "InputEvents.h"
#include "RexLogicModule.h"
#include "InputServiceInterface.h"

namespace RexLogic
{
    InputEventHandler::InputEventHandler(Foundation::Framework *framework, RexLogicModule *rexlogicmodule)
    {
        framework_ = framework;
        rexlogicmodule_ = rexlogicmodule;
        dragging_ = false;
    }

    InputEventHandler::~InputEventHandler()
    {
    }
    
    bool InputEventHandler::HandleInputEvent(Core::event_id_t event_id, Foundation::EventDataInterface* data)
    {
        if (event_id == Input::Events::SWITCH_CONTROLLER)
        {
            rexlogicmodule_->SwitchController();
            return true;
        }

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
                case Input::Events::MOVE_UP_PRESSED:
                    state->StartMovingUp();
                    break;
                case Input::Events::MOVE_UP_RELEASED:
                    state->StopMovingUp();
                    break;
                case Input::Events::MOVE_DOWN_PRESSED:
                    state->StartMovingDown();
                    break;
                case Input::Events::MOVE_DOWN_RELEASED:
                    state->StopMovingDown();
                    break;
                case Input::Events::ROTATE_LEFT_PRESSED:
                    state->StartRotatingLeft();
                    break;
                case Input::Events::ROTATE_LEFT_RELEASED:
                    state->StopRotatingLeft();
                    break;
                case Input::Events::ROTATE_RIGHT_PRESSED:
                    state->StartRotatingRight();
                    break;
                case Input::Events::ROTATE_RIGHT_RELEASED:
                    state->StopRotatingRight();
                    break;
                case Input::Events::ROTATE_UP_PRESSED:
                    state->StartRotatingUp();
                    break;
                case Input::Events::ROTATE_UP_RELEASED:
                    state->StopRotatingUp();
                    break;
                case Input::Events::ROTATE_DOWN_PRESSED:
                    state->StartRotatingDown();
                    break;
                case Input::Events::ROTATE_DOWN_RELEASED:
                    state->StopRotatingDown();
                    break;
                case Input::Events::ROLL_LEFT_PRESSED:
                    state->StartRollingLeft();
                    break;
                case Input::Events::ROLL_LEFT_RELEASED:
                    state->StopRollingLeft();
                    break;
                case Input::Events::ROLL_RIGHT_PRESSED:
                    state->StartRollingRight();
                    break;
                case Input::Events::ROLL_RIGHT_RELEASED:
                    state->StopRollingRight();
                    break;
            }
        } else
        {
            RexLogicModule::LogDebug("Warning: no input state present, movement input not handled.");
        }
        return false;
    }

    void InputEventHandler::Update(Core::f64 frametime)
    {
        boost::shared_ptr<InputState> state = state_.lock();
        Foundation::InputServiceInterface *input = framework_->GetService<Foundation::InputServiceInterface>(Foundation::Service::ST_Input);
        if (input)
        {
            boost::optional<const Input::Events::Movement&> movement = input->GetSliderMovement(Input::Events::MOUSELOOK);
            if (movement)
            {
                dragging_ = true;
                state->Drag(&*movement);
            } else if (dragging_)
            {
                dragging_ = false;
                Input::Events::Movement zero;
                state->Drag(&zero);
            }
        }

        state->Update(frametime);
    }
}
