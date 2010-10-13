// For conditions of distribution and use, see copyright notice in license.txt

#include "StableHeaders.h"
#include "InputEventHandler.h"
#include "InputEvents.h"
#include "RexLogicModule.h"
#include "../Input/Input.h"
#include "WorldStream.h"
#include "RenderServiceInterface.h"
#include "ServiceManager.h"
#include "Entity.h"
#include "EventManager.h"
#include "SceneEvents.h"

#include <QDebug>

/*
 *  !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
 *  THIS FILE IS DEPRECATED. DO NOT ADD NEW CODE. USE NEW INPUT CONTEX SYSTEM INSTEAD
 *  !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
 */

namespace RexLogic
{

InputEventHandler::InputEventHandler(RexLogicModule *owner) : owner_(owner)
{
}

InputEventHandler::~InputEventHandler()
{
}

bool InputEventHandler::HandleInputEvent(event_id_t event_id, IEventData* data)
{
    /*
     *  !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
     *  THIS FILE IS DEPRECATED. DO NOT ADD NEW CODE. USE NEW INPUT CONTEX SYSTEM INSTEAD
     *  !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
     */
    using namespace InputEvents;

    switch(event_id)
    {
    case SWITCH_CAMERA_STATE:
    {
        if (owner_->GetServerConnection()->IsConnected())
            owner_->SwitchCameraState();
        break;
    }
    case CAMERA_TRIPOD:
    {
        if (owner_->GetServerConnection()->IsConnected())
            owner_->CameraTripod();
        break;
    }
    case InputEvents::INWORLD_CLICK:
    {
        InputEvents::Movement *movement = checked_static_cast<InputEvents::Movement*>(data);

        Foundation::RenderServiceInterface *renderer = owner_->GetFramework()->GetService<Foundation::RenderServiceInterface>();
        if (renderer)
        {
            Foundation::RaycastResult result = renderer->Raycast(movement->x_.abs_, movement->y_.abs_);
            owner_->CheckInfoIconIntersection(movement->x_.abs_, movement->y_.abs_, &result);
        }
        break;
    }
    
    case MOUSE_DOUBLECLICK:
    {
        break;
    }
    default:
        /*
         *  !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
         *  THIS FILE IS DEPRECATED. DO NOT ADD NEW CODE. USE NEW INPUT CONTEX SYSTEM INSTEAD
         *  !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
         */
        break;
    }

    return false;
}

}