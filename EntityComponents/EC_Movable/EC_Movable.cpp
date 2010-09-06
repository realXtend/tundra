/**
 *  For conditions of distribution and use, see copyright notice in license.txt
 *
 *  @file   EC_Movable.cpp
 *  @brief  
 */

#include "StableHeaders.h"
#include "EC_Movable.h"

#include "EventManager.h"
#include "NetworkEvents.h"
#include "WorldStream.h"
#include "Entity.h"
#include "EC_OpenSimPrim.h"
#include "EC_OgrePlaceable.h"
#include "LoggingFunctions.h"

DEFINE_POCO_LOGGING_FUNCTIONS("EC_Movable")

EC_Movable::~EC_Movable()
{
}

bool EC_Movable::HandleEvent(event_category_id_t category_id, event_id_t event_id, Foundation::EventDataInterface* data)
{
    if (category_id == frameworkCategory_)
    {
        if (event_id == Foundation::WORLD_STREAM_READY)
        {
            ProtocolUtilities::WorldStreamReadyEvent *event_data = static_cast<ProtocolUtilities::WorldStreamReadyEvent *>(data);
            if (event_data)
                worldStream_ = event_data->WorldStream;
        }
    }
    return false;
}

void EC_Movable::SetWorldStreamPtr(ProtocolUtilities::WorldStreamPtr worldStream)
{
    worldStream_ = worldStream;
}

void EC_Movable::Exec(const QString &action, const QVector<QString> &params)
{
    Vector3df change, orientation;
    if (action == "MoveForward")
    {
        LogDebug("MoveForward");
        change.x += 1;
    }
    else if (action == "MoveBackward")
    {
        LogDebug("MoveBackward");
        change.x -= 1;
    }
    else if (action == "MoveLeft")
    {
        change.y -= 1;
        LogDebug("MoveLeft");
    }
    else if (action == "MoveRight")
    {
        LogDebug("MoveRight");
        change.y += 1;
    }
    else if (action == "RotateLeft")
    {
        //orientation
        LogDebug("RotateLeft");
    }
    else if (action == "RotateRight")
    {
        //orientation
        LogDebug("RotateRight");
    }
    else
    {
        return;
    }

    Scene::Entity *entity = GetParentEntity();
    if (worldStream_ && entity)
    {
        std::vector<ProtocolUtilities::MultiObjectUpdateInfo> updates;
        EC_OpenSimPrim *prim = entity->GetComponent<EC_OpenSimPrim>().get();
        OgreRenderer::EC_OgrePlaceable *ogre_pos = entity->GetComponent<OgreRenderer::EC_OgrePlaceable >().get();
        if (!prim && !ogre_pos)
            return;

        ProtocolUtilities::MultiObjectUpdateInfo update;
        update.local_id_ = prim->LocalId;
        update.position_ = ogre_pos->GetPosition() + change;
        update.orientation_ = ogre_pos->GetOrientation();
        update.scale_ = ogre_pos->GetScale();
        updates.push_back(update);

        worldStream_->SendMultipleObjectUpdatePacket(updates);
    }
}

EC_Movable::EC_Movable(Foundation::ModuleInterface *module):
    Foundation::ComponentInterface(module->GetFramework())
{
    Foundation::EventManagerPtr eventMgr = framework_->GetEventManager();
    eventMgr->RegisterEventSubscriber(this, 99);
    frameworkCategory_ = eventMgr->QueryEventCategory("Framework");

}

