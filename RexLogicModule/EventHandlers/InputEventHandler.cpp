// For conditions of distribution and use, see copyright notice in license.txt

#include "StableHeaders.h"
#include "InputEventHandler.h"
#include "InputEvents.h"
#include "RexLogicModule.h"
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
    using namespace Input;

    switch(event_id)
    {
    case Events::SWITCH_CAMERA_STATE:
    {
        if (owner_->GetServerConnection()->IsConnected())
            owner_->SwitchCameraState();
        break;
    }
    case Events::CAMERA_TRIPOD:
    {
        if (owner_->GetServerConnection()->IsConnected())
            owner_->CameraTripod();
        break;
    }
    case Events::INWORLD_CLICK:
    {
        Input::Events::Movement *movement = checked_static_cast<Input::Events::Movement*>(data);

        Foundation::RenderServiceInterface *renderer = owner_->GetFramework()->GetService<Foundation::RenderServiceInterface>();
        if (renderer)
        {
            Foundation::RaycastResult result = renderer->Raycast(movement->x_.abs_, movement->y_.abs_);
            owner_->CheckInfoIconIntersection(movement->x_.abs_, movement->y_.abs_, &result);
        }
        break;
    }
    case Events::ALT_LEFTCLICK:
    {
        Input::Events::Movement *movement = checked_static_cast<Input::Events::Movement*>(data);

        Foundation::RenderServiceInterface *renderer = owner_->GetFramework()->GetService<Foundation::RenderServiceInterface>();
        if (renderer)
        {
            Foundation::RaycastResult result = renderer->Raycast(movement->x_.abs_, movement->y_.abs_);
            owner_->CheckInfoIconIntersection(movement->x_.abs_, movement->y_.abs_, &result);
            owner_->FocusOnObject(result.pos_.x, result.pos_.y, result.pos_.z);
        }
        break;
    }
    case Events::ALT_LEFTCLICK_REL:
    {
        owner_->ResetCameraState();
        break;
    }
    case Events::MOUSE_DOUBLECLICK:
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