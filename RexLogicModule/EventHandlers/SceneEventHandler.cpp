/// For conditions of distribution and use, see copyright notice in license.txt

#include "StableHeaders.h"
#include "EventHandlers/SceneEventHandler.h"
#include "SceneEvents.h"
#include "RexLogicModule.h"
#include "SceneManager.h"
#include "EC_OgrePlaceable.h"
#include "WorldStream.h"
#include "EC_OpenSimPrim.h"

namespace RexLogic
{

void PopulateUpdateInfos(std::vector<ProtocolUtilities::MultiObjectUpdateInfo>& dest, const std::vector<Scene::EntityPtr>& src)
{
    for (uint i = 0; i < src.size(); ++i)
    {
        if (!src[i])
            continue;

        boost::shared_ptr<EC_OpenSimPrim> prim = src[i]->GetComponent<EC_OpenSimPrim>();
        boost::shared_ptr<OgreRenderer::EC_OgrePlaceable> ogre_pos = src[i]->GetComponent<OgreRenderer::EC_OgrePlaceable >();
        if (!prim && !ogre_pos)
            continue;

        ProtocolUtilities::MultiObjectUpdateInfo new_info;
        new_info.local_id_ = prim->LocalId;
        new_info.position_ = ogre_pos->GetPosition();
        new_info.orientation_ = ogre_pos->GetOrientation();
        new_info.scale_ = ogre_pos->GetScale();

        dest.push_back(new_info);
    }
}

SceneEventHandler::SceneEventHandler(RexLogicModule *owner) : owner_(owner)
{
}

SceneEventHandler::~SceneEventHandler()
{
}

bool SceneEventHandler::HandleSceneEvent(event_id_t event_id, IEventData* data)
{
    using namespace Scene;

    ///\todo Shouldn't we be able to do static_cast here? Seems to crash if we do so.
    Events::SceneEventData *event_data = dynamic_cast<Events::SceneEventData *>(data);

    switch(event_id)
    {
    case Events::EVENT_ENTITY_SELECT:
        ///\todo Utilize this for real with somekind of SelectionManager
        owner_->GetServerConnection()->SendObjectSelectPacket(event_data->localID);
        break;
    case Events::EVENT_ENTITY_DESELECT:
        ///\todo Utilize this for real with somekind of SelectionManager
        owner_->GetServerConnection()->SendObjectDeselectPacket(event_data->localID);
        break;
    case Events::EVENT_ENTITY_UPDATED:
    {
        std::vector<ProtocolUtilities::MultiObjectUpdateInfo> update_info_list;
        PopulateUpdateInfos(update_info_list, event_data->entity_ptr_list);
        owner_->GetServerConnection()->SendMultipleObjectUpdatePacket(update_info_list);
        break;
    }
    case Events::EVENT_ENTITY_GRAB:
        owner_->GetServerConnection()->SendObjectGrabPacket(event_data->localID);
        break;
    case Events::EVENT_ENTITY_DELETED:
        ///\todo Do something?
        break;
    case Events::EVENT_ENTITY_CREATE:
    {
        Events::CreateEntityEventData *pos_data = dynamic_cast<Events::CreateEntityEventData *>(data);
        if (pos_data)
            owner_->GetServerConnection()->SendObjectAddPacket(pos_data->position);
        break;
    }
    default:
        break;
    }

    return false;
}

}
