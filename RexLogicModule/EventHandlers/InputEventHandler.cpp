// For conditions of distribution and use, see copyright notice in license.txt

#include "StableHeaders.h"
#include "EventHandlers/InputEventHandler.h"
#include "InputEvents.h"
#include "RexLogicModule.h"
#include "InputServiceInterface.h"
#include "WorldStream.h"
#include "RenderServiceInterface.h"
#include "ServiceManager.h"
#include "Entity.h"
#include "EventManager.h"
#include "SceneEvents.h"
#include "EC_Touchable.h"

namespace RexLogic
{

InputEventHandler::InputEventHandler(RexLogicModule *owner) :
    owner_(owner),
    lastCallTime_(0),
    raycastInterval_(0.05)
{
    lastMousePosition_.first = -1;
    lastMousePosition_.second = -1;
}

InputEventHandler::~InputEventHandler()
{
}

bool InputEventHandler::HandleInputEvent(event_id_t event_id, Foundation::EventDataInterface* data)
{
    if (event_id == Input::Events::SWITCH_CAMERA_STATE)
    {
        if (owner_->GetServerConnection()->IsConnected())
            owner_->SwitchCameraState();
        return false;
    }

    if (event_id == Input::Events::MOUSEMOVE)
    {
        if (owner_->GetServerConnection()->IsConnected())
        {
            Input::Events::Movement *move= checked_static_cast<Input::Events::Movement*>(data);
            assert(move);
            lastMousePosition_.first = move->x_.abs_;
            lastMousePosition_.second = move->y_.abs_;
        }

        return false;
    }

    return false;
}

void InputEventHandler::Update(f64 frametime)
{
    using namespace Foundation;

    lastCallTime_ += frametime;

    // Do raycast if the last mouse position is valid and elapsed time has exceeded raycast interval.
    if (lastMousePosition_.first > 0 && lastMousePosition_.second > 0 && lastCallTime_ >= raycastInterval_)
    {
        lastCallTime_ = 0;
        RenderServiceInterface *renderer = owner_->GetFramework()->GetService<RenderServiceInterface>(Service::ST_Renderer).lock().get();
        RaycastResult result = renderer->Raycast(lastMousePosition_.first, lastMousePosition_.second);
        if (result.entity_)
        {
            // Send event notifying the mouse is hovering above an entity.
            EventManagerPtr eventMgr = owner_->GetFramework()->GetEventManager();
            assert(eventMgr.get());
            Scene::Events::RaycastEventData data(result.entity_->GetId());
            data.pos = result.pos_;
            data.submesh = result.submesh_;
            data.u = result.u_;
            data.v = result.v_;
            event_category_id_t category = eventMgr->QueryEventCategory("Scene");
            eventMgr->SendEvent(category, Scene::Events::EVENT_ENTITY_MOUSE_HOVER, &data);
        }
    }
}

}
