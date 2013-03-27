/**
    For conditions of distribution and use, see copyright notice in LICENSE

    @file   EntityIdChangeTracker.cpp
    @brief  EntityIdChangeTracker helper class stores entity id changes that result from QUndoCommand manipulations. */

#include "StableHeaders.h"

#include "EntityIdChangeTracker.h"
#include "Scene.h"
#include "Entity.h"

#include "MemoryLeakCheck.h"

EntityIdChangeTracker::EntityIdChangeTracker(const ScenePtr &scene, QObject * parent) :
    scene_(scene),
    QObject(parent)
{
    //connect(scene, SIGNAL(EntityCreated(Entity*, AttributeChange::Type)), this, SLOT(OnEntityCreated(Entity*, AttributeChange::Type)));
    //connect(scene, SIGNAL(EntityRemoved(Entity*, AttributeChange::Type)), this, SLOT(OnEntityRemoved(Entity*, AttributeChange::Type)));
    connect(scene.get(), SIGNAL(EntityAcked(Entity*, entity_id_t)), this, SLOT(OnEntityAcked(Entity*, entity_id_t)));
}

/*
void EntityIdChangeTracker::OnEntityCreated(Entity * entity, AttributeChange::Type change)
{
}

void EntityIdChangeTracker::OnEntityRemoved(Entity * entity, AttributeChange::Type change)
{
}
*/

void EntityIdChangeTracker::OnEntityAcked(Entity * entity, entity_id_t oldId)
{
    if (unackedToAckedIds_.contains(oldId))
    {
        unackedToAckedIds_[oldId] = entity->Id();
    }

    QMapIterator<entity_id_t, entity_id_t> i(changedIds_);
    while (i.hasNext())
    {
        i.next();
        if (i.value() == oldId)
            changedIds_[i.key()] = entity->Id();
    }

}

void EntityIdChangeTracker::AppendUnackedId(entity_id_t id)
{
    if ((id & 0x80000000) == 0x80000000)
        unackedToAckedIds_[id] = id;
    else
        unackedToAckedIds_[id] = 0;
}

entity_id_t EntityIdChangeTracker::RetrieveId(entity_id_t oldId)
{
    if (unackedToAckedIds_.contains(oldId) && (unackedToAckedIds_.value(oldId) != 0))
        return TraceId(unackedToAckedIds_.value(oldId));

    return TraceId(oldId);
}

void EntityIdChangeTracker::TrackId(entity_id_t oldId, entity_id_t newId)
{
    if (unackedToAckedIds_.contains(oldId) && unackedToAckedIds_.value(oldId) != 0)
        changedIds_[unackedToAckedIds_.value(oldId)] = newId;
    else
        changedIds_[oldId] = newId;
}

entity_id_t EntityIdChangeTracker::TraceId(entity_id_t id)
{
    if (changedIds_.contains(id))
        return TraceId(changedIds_[id]);

    return id;
}

void EntityIdChangeTracker::Clear()
{
    unackedToAckedIds_.clear();
    changedIds_.clear();
}
