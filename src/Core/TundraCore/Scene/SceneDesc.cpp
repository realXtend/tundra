// For conditions of distribution and use, see copyright notice in LICENSE

#include "StableHeaders.h"
#include "SceneDesc.h"

#include "LoggingFunctions.h"
#include "Scene.h"
#include "Entity.h"
#include "IAttribute.h"
#include "EntityReference.h"

// ParentingTracker

bool ParentingTracker::IsTracking() const
{
    return !Unacked.isEmpty();
}

void ParentingTracker::Track(Entity *ent)
{
    if (ent)
    {
        LogDebug(QString("[ParentingTracker]: Tracking unacked id %1").arg(ent->Id()));
        Unacked.push_back(ent->Id());
    }
}

void ParentingTracker::Ack(Scene *scene, entity_id_t newId, entity_id_t oldId)
{
    // Check that we are tracking this entity.
    if (Unacked.isEmpty() || !Unacked.contains(oldId))
        return;
    Unacked.removeAll(oldId);
    
    // Store the new to old id for later processing.
    UnackedToAcked[oldId] = newId;
    
    // If new ids for all tracked entities are now known, return true.
    if (Unacked.isEmpty())
        _fixParenting(scene);
}

void ParentingTracker::_fixParenting(Scene *scene)
{
    LogDebug(QString("[ParentingTracker]: Received new Entity ids for %1 tracked Entities. Processing broken parenting refs.").arg(UnackedToAcked.size()));
    
    QList<entity_id_t> ackedIds = UnackedToAcked.values();
    for (int ai=0, ailen=ackedIds.size(); ai<ailen; ++ai)
    {
        EntityPtr ent = scene->EntityById(ackedIds[ai]);
        if (!ent.get())
        {
            LogWarning(QString("[ParentingTracker]: Failed to find Entity by new acked id %1").arg(ackedIds[ai]));
            continue; 
        }

        /** @todo Check and fix ent->Parent() stuff here!? See if ent->Parent()->Id()
            is old unacked or new acked at this point! Or if there is a better place 
            to handle this adjustment. */

        /** Check if EC_Placeable::parentRef needs to be adjusted.
            This is replicated change to the scene that the client that started the import will perform.
            @note How this is done is a hack that does not require us to link or include EC_Placeable.
            Scenes will (or should) be ported over time to the new Entity level parenting. */
        ComponentPtr comp = ent->Component(20); // EC_Placeable
        if (comp.get())
        {
            Attribute<EntityReference> *parentRef = dynamic_cast<Attribute<EntityReference> *>(comp->AttributeById("parentRef"));
            if (parentRef && !parentRef->Get().IsEmpty())
            {
                // We only need to fix the id parent refs. Ones with entity names should
                // work as expected (if names are unique which would be a authoring problem
                // and not addressed by Tundra).
                bool isNumber = false;
                entity_id_t potentialUnackedId = parentRef->Get().ref.toUInt(&isNumber);
                if (isNumber && potentialUnackedId > 0 && UnackedToAcked.contains(potentialUnackedId))
                {
                    entity_id_t ackedId = UnackedToAcked[potentialUnackedId];
                    LogDebug(QString("[ParentingTracker]:    EC_Placeable::parentRef from unacked id %1 to acked id %2").arg(potentialUnackedId).arg(ackedId));
                    parentRef->Set(EntityReference(ackedId), AttributeChange::Replicate);
                }
            }
        }
    }
    UnackedToAcked.clear();
}
