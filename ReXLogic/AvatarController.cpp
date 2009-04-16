// For conditions of distribution and use, see copyright notice in license.txt

#include "StableHeaders.h"
#include "AvatarController.h"
#include "RexLogicModule.h"
#include "RexServerConnection.h"
#include "InputEvents.h"

namespace RexLogic
{
    AvatarController::AvatarController(Foundation::Framework *framework, RexLogicModule *rexlogicmodule)
    {
        framework_ = framework;
        rexlogicmodule_ = rexlogicmodule;
     
        controlflags_ = 0; 
    }

    AvatarController::~AvatarController()
    {
    
    }
    
    Core::Quaternion AvatarController::GetBodyRotation()
    {
        return Core::Quaternion(0,0,0,1); //! \todo tucofixme
    }
    
    Core::Quaternion AvatarController::GetHeadRotation()
    {
        return Core::Quaternion(0,0,0,1); //! \todo tucofixme    
    }

    void AvatarController::UpdateMovementState(Core::event_id_t input_event_id)
    {
        switch(input_event_id)
        {
            case Input::Events::MOVE_FORWARD_PRESSED:
                controlflags_ |= RexTypes::AGENT_CONTROL_AT_POS;
                break;
            case Input::Events::MOVE_FORWARD_RELEASED:
                controlflags_ &= ~RexTypes::AGENT_CONTROL_AT_POS;
                break;
            case Input::Events::MOVE_BACK_PRESSED:
                controlflags_ |= RexTypes::AGENT_CONTROL_AT_NEG;
                break;        
            case Input::Events::MOVE_BACK_RELEASED:
                controlflags_ &= ~RexTypes::AGENT_CONTROL_AT_NEG;
                break;
            case Input::Events::MOVE_LEFT_PRESSED:
                controlflags_ |= RexTypes::AGENT_CONTROL_LEFT_POS;
                break; 
            case Input::Events::MOVE_LEFT_RELEASED:
                controlflags_ &= ~RexTypes::AGENT_CONTROL_LEFT_POS;
                break; 
            case Input::Events::MOVE_RIGHT_PRESSED:
                controlflags_ |= RexTypes::AGENT_CONTROL_LEFT_NEG;
                break;         
            case Input::Events::MOVE_RIGHT_RELEASED:
                controlflags_ &= ~RexTypes::AGENT_CONTROL_LEFT_NEG;
                break;
        }
        
        // 0 = walk, 1 = mouselook, 2 = type
        uint8_t flags = 0;
        
        Core::Quaternion bodyrot = GetBodyRotation();
        bodyrot.normalize();
        
        Core::Quaternion headrot = GetHeadRotation();
        headrot.normalize();
    
        RexTypes::Vector3 camcenter = Vector3(0,0,0);
        RexTypes::Vector3 camataxis = Vector3(0,0,0);
        RexTypes::Vector3 camleftaxis = Vector3(0,0,0);
        RexTypes::Vector3 camupaxis = Vector3(0,0,0);       
        float fardist = 4000.0f;
        
        rexlogicmodule_->GetServerConnection()->SendAgentUpdatePacket(bodyrot,headrot,0,camcenter,camataxis,camleftaxis,camupaxis,fardist,controlflags_,flags);            
    }
}

