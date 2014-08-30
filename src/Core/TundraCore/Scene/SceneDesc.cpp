// For conditions of distribution and use, see copyright notice in LICENSE

#include "StableHeaders.h"
#include "SceneDesc.h"

#include "LoggingFunctions.h"
#include "Scene.h"
#include "Entity.h"
#include "IAttribute.h"
#include "EntityReference.h"

// SceneDesc

SceneDesc::SceneDesc(const QString &_filename) :
    viewEnabled(false),
    filename(_filename)
{
    if (!filename.isEmpty())
        assetCache.basePath = QFileInfo(filename).dir().path();
}

// AssetDescCache

bool AssetDescCache::Fill(const QString assetRef, AssetDesc &desc)
{
    bool found = cache.contains(assetRef);
    if (found)
    {
        FileInfoPair value = cache[assetRef];
        desc.source = value.first;
        desc.destinationName = value.second;
    }
    return found;
}

bool AssetDescCache::Add(const QString &assetRef, const AssetDesc &desc)
{
    if (cache.contains(assetRef) || desc.source.isEmpty() || desc.destinationName.isEmpty())
        return false;

    cache[assetRef] = FileInfoPair(desc.source, desc.destinationName);
    return true;
}

// ParentingTracker

bool ParentingTracker::IsTracking() const
{
    return !unacked.isEmpty();
}

void ParentingTracker::Track(Entity *ent)
{
    if (ent)
    {
        LogDebug(QString("[ParentingTracker]: Tracking unacked id %1").arg(ent->Id()));
        unacked.push_back(ent->Id());
    }
}

void ParentingTracker::Ack(Scene *scene, entity_id_t newId, entity_id_t oldId)
{
    // Check that we are tracking this entity.
    if (unacked.isEmpty() || !unacked.contains(oldId))
        return;
    unacked.removeAll(oldId);
    
    // Store the new to old id for later processing.
    unackedToAcked[oldId] = newId;
    
    // If new ids for all tracked entities are now known, return true.
    if (unacked.isEmpty())
    {
        _fixParenting(scene);
        unackedToAcked.clear();
    }
}

void ParentingTracker::_fixParenting(Scene *scene)
{
    LogInfo(QString("[ParentingTracker]: Received new ids for %1 tracked Entities. Processing scene hierarchy.").arg(unackedToAcked.size()));
    
    /** @todo Check and fix ent->Parent() stuff here!? See if ent->Parent()->Id()
        is old unacked or new acked at this point! Or if there is a better place 
        to handle this adjustment? */

    QList<Entity*> entitiesToCheck;
    QList<entity_id_t> ackedIds = unackedToAcked.values();
    for (int ai=0, ailen=ackedIds.size(); ai<ailen; ++ai)
    {
        EntityPtr ent = scene->EntityById(ackedIds[ai]);
        if (!ent.get())
        {
            LogWarning(QString("[ParentingTracker]: Failed to find Entity by new acked id %1").arg(ackedIds[ai]));
            continue; 
        }
        entitiesToCheck << ent.get();
    }
    scene->FixPlaceableParentIds(entitiesToCheck, unackedToAcked, AttributeChange::Replicate, true);
}
