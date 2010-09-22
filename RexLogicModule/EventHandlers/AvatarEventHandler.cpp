// For conditions of distribution and use, see copyright notice in license.txt

#include "StableHeaders.h"
#include "RexLogicModule.h"
#include "EventManager.h"

#include "EventHandlers/AvatarEventHandler.h"
#include "AvatarEvents.h"

namespace RexLogic
{
    bool AvatarEventHandler::HandleAvatarEvent(event_id_t event_id, IEventData* data)
    {
        using namespace Avatar;

        switch (event_id)
        {
            case Events::EVENT_REGISTER_UUID_TO_LOCALID:
            {
                Events::SceneRegisterEntityData *in_data = dynamic_cast<Events::SceneRegisterEntityData*>(data);
                if (in_data)
                    owner_->RegisterFullId(in_data->uuid, in_data->local_id);
                break;
            }
            case Events::EVENT_UNREGISTER_UUID_TO_LOCALID:
            {
                Events::SceneRegisterEntityData *in_data = dynamic_cast<Events::SceneRegisterEntityData*>(data);
                if (in_data)
                    owner_->UnregisterFullId(in_data->uuid);
                break;
            }
            case Events::EVENT_HANDLE_AVATAR_PARENT:
            {
                Events::SceneHandleParentData *in_data = dynamic_cast<Events::SceneHandleParentData*>(data);
                if (in_data)
                {
                    owner_->HandleMissingParent(in_data->local_id);
                    owner_->HandleObjectParent(in_data->local_id);
                }
                break;
            }
        }
        return false;
    }

    void AvatarEventHandler::SendRegisterEvent(const RexUUID &fullid, entity_id_t entityid)
    {
        Foundation::EventManagerPtr event_manager = owner_->GetFramework()->GetEventManager();
        if (event_manager)
        {
            Avatar::Events::SceneRegisterEntityData data(fullid, entityid);
            event_manager->SendEvent("Avatar", Avatar::Events::EVENT_REGISTER_UUID_TO_LOCALID, &data);
        }
    }

    void AvatarEventHandler::SendUnregisterEvent(const RexUUID &fullid)
    {
        Foundation::EventManagerPtr event_manager = owner_->GetFramework()->GetEventManager();
        if (event_manager)
        {
            Avatar::Events::SceneRegisterEntityData data(fullid);
            event_manager->SendEvent("Avatar", Avatar::Events::EVENT_UNREGISTER_UUID_TO_LOCALID, &data);
        }
    }
}