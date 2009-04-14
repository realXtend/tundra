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
                // RexLogicModule::LogInfo("FORWARD START");
                break;
            case Input::Events::MOVE_FORWARD_RELEASED:
                // RexLogicModule::LogInfo("FORWARD END");
                break;
            case Input::Events::MOVE_BACK_PRESSED:
                // RexLogicModule::LogInfo("BACK START");
                break;        
            case Input::Events::MOVE_BACK_RELEASED:
                // RexLogicModule::LogInfo("BACK END");
                break;
            case Input::Events::MOVE_LEFT_PRESSED:
                // RexLogicModule::LogInfo("LEFT START");
                break; 
            case Input::Events::MOVE_LEFT_RELEASED:
                // RexLogicModule::LogInfo("LEFT END");
                break; 
            case Input::Events::MOVE_RIGHT_PRESSED:
                // RexLogicModule::LogInfo("RIGHT START");
                break;         
            case Input::Events::MOVE_RIGHT_RELEASED:
                // RexLogicModule::LogInfo("RIGHT END");
                break;      
        }
        return false;
    }
}
