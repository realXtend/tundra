/**
 *  For conditions of distribution and use, see copyright notice in license.txt
 *
 *  @file   EC_Movable.cpp
 *  @brief  Contains Entity Actions for moving entity with this component in scene.
 */

#include "StableHeaders.h"
#include "EC_Movable.h"

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

void EC_Movable::SetWorldStreamPtr(ProtocolUtilities::WorldStreamPtr worldStream)
{
    worldStream_ = worldStream;
}

void EC_Movable::Move(const QString &direction)
{
    Vector3df change, orientation;
    if (direction == "Forward")
        change.x += 1;
    else if (direction == "Backward")
        change.x -= 1;
    else if (direction == "Left")
        change.y -= 1;
    else if (direction == "Right")
        change.y += 1;
    else
    {
        LogWarning("Invalid direction for Move action: " + direction.toStdString());
        return;
    }

    SendMultipleObjectUpdatePacket(change, Quaternion());
}

void EC_Movable::Rotate(const QString &direction)
{
    Quaternion orientation;
    if (direction == "Left")
    {
        orientation.z = 0.7071f;
        orientation.w = 0.7071f;
    }
    else if (direction == "Right")
    {
        orientation.z = -0.7071f;
        orientation.w = 0.7071f;
    }
    else
    {
        LogWarning("Invalid direction for Rotate action: " + direction.toStdString());
        return;
    }

    SendMultipleObjectUpdatePacket(Vector3df(), orientation);
}

EC_Movable::EC_Movable(IModule *module):
    Foundation::ComponentInterface(module->GetFramework())
{
    connect(this, SIGNAL(ParentEntitySet()), SLOT(RegisterActions()));
}

void EC_Movable::SendMultipleObjectUpdatePacket(const Vector3df &deltaPos, const Quaternion &deltaOri)
{
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
        update.position_ = ogre_pos->GetPosition() + deltaPos;
        update.orientation_ = ogre_pos->GetOrientation() * deltaOri;
        update.scale_ = ogre_pos->GetScale();
        updates.push_back(update);

        worldStream_->SendMultipleObjectUpdatePacket(updates);
    }
}

void EC_Movable::RegisterActions()
{
    Scene::Entity *entity = GetParentEntity();
    assert(entity);
    if (entity)
    {
        entity->ConnectAction("Move", this, SLOT(Move(const QString &)));
        entity->ConnectAction("Rotate", this, SLOT(Rotate(const QString &)));
    }
}

