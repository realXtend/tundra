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
                rexlogicmodule_->GetAvatarController()->StartMovingForward();
                break;
            case Input::Events::MOVE_FORWARD_RELEASED:
                rexlogicmodule_->GetAvatarController()->StopMovingForward();
                break;
            case Input::Events::MOVE_BACK_PRESSED:  
                rexlogicmodule_->GetAvatarController()->StartMovingBack();
                break;
            case Input::Events::MOVE_BACK_RELEASED:
                rexlogicmodule_->GetAvatarController()->StopMovingBack();
                break;
            case Input::Events::MOVE_LEFT_PRESSED:
                rexlogicmodule_->GetAvatarController()->StartMovingLeft();
                break;
            case Input::Events::MOVE_LEFT_RELEASED:
                rexlogicmodule_->GetAvatarController()->StopMovingLeft();
                break;
            case Input::Events::MOVE_RIGHT_PRESSED:       
                rexlogicmodule_->GetAvatarController()->StartMovingRight();
                break;
            case Input::Events::MOVE_RIGHT_RELEASED:
                rexlogicmodule_->GetAvatarController()->StopMovingRight();
                break;    
        }
        return false;
    }
}
