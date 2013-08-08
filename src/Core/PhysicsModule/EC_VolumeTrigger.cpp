// For conditions of distribution and use, see copyright notice in LICENSE

#include "StableHeaders.h"
#include "DebugOperatorNew.h"

#include "EC_VolumeTrigger.h"
#include "EC_RigidBody.h"
#include "EC_Placeable.h"
#include "Entity.h"
#include "Scene/Scene.h"
#include "PhysicsModule.h"
#include "PhysicsWorld.h"
#include "PhysicsUtils.h"
#include "LoggingFunctions.h"
#include "Profiler.h"

#include <OgreAxisAlignedBox.h>
#include <QMap>
#include <btBulletDynamicsCommon.h>

#include "MemoryLeakCheck.h"

using namespace Physics;

EC_VolumeTrigger::EC_VolumeTrigger(Scene* scene) :
    IComponent(scene),
    INIT_ATTRIBUTE_VALUE(byPivot, "By Pivot", false),
    INIT_ATTRIBUTE(entities, "Entities")
{
    connect(this, SIGNAL(ParentEntitySet()), this, SLOT(UpdateSignals()), Qt::UniqueConnection);
}

EC_VolumeTrigger::~EC_VolumeTrigger()
{
}

QList<EntityWeakPtr> EC_VolumeTrigger::GetEntitiesInside() const
{
    QList<EntityWeakPtr> ret;
    for(EntitiesWithinVolumeMap::const_iterator it = entities_.begin(); it != entities_.end(); ++it)
        ret.push_back(it->first);
    return ret;
}

size_t EC_VolumeTrigger::GetNumEntitiesInside() const
{
    return entities_.size();
}

Entity* EC_VolumeTrigger::GetEntityInside(int idx) const
{
    if (idx >=0 && (size_t)idx < entities_.size())
    {
        int currentIndex = 0;
        for(EntitiesWithinVolumeMap::const_iterator it = entities_.begin(); it != entities_.end(); ++it)
        {
            if (currentIndex == idx)
                return it->first.lock().get();
            ++currentIndex;
        }
    }
    return 0;
}

QStringList EC_VolumeTrigger::GetEntityNamesInside() const
{
    QStringList entitynames;
    for(EntitiesWithinVolumeMap::const_iterator it = entities_.begin(); it != entities_.end(); ++it)
        if (!it->first.expired())
            entitynames.append(it->first.lock()->Name());
    return entitynames;
}

float EC_VolumeTrigger::GetEntityInsidePercent(const Entity *entity) const
{
    if (entity)
    {
        shared_ptr<EC_RigidBody> otherRigidbody = entity->GetComponent<EC_RigidBody>();

        shared_ptr<EC_RigidBody> rigidbody = rigidbody_.lock();
        if (rigidbody && otherRigidbody)
        {
            float3 thisBoxMin, thisBoxMax;
            rigidbody->GetAabbox(thisBoxMin, thisBoxMax);

            float3 otherBoxMin, otherBoxMax;
            otherRigidbody->GetAabbox(otherBoxMin, otherBoxMax);

            Ogre::AxisAlignedBox thisBox(thisBoxMin.x, thisBoxMin.y, thisBoxMin.z, thisBoxMax.x, thisBoxMax.y, thisBoxMax.z);
            Ogre::AxisAlignedBox otherBox(otherBoxMin.x, otherBoxMin.y, otherBoxMin.z, otherBoxMax.x, otherBoxMax.y, otherBoxMax.z);

            return (thisBox.intersection(otherBox).volume() / otherBox.volume());
        } else
            LogWarning("EC_VolumeTrigger: no EC_RigidBody for entity or volume.");
    }
    return 0.0f;
}


float EC_VolumeTrigger::GetEntityInsidePercentByName(const QString &name) const
{
    for(EntitiesWithinVolumeMap::const_iterator it = entities_.begin(); it != entities_.end(); ++it)
        if (!it->first.expired() && it->first.lock()->Name().compare(name) == 0)
            return GetEntityInsidePercent(it->first.lock().get());
    return 0.f;
}

bool EC_VolumeTrigger::IsInterestingEntity(const QString &name) const
{
    PROFILE(EC_VolumeTrigger_IsInterestingEntity); ///\todo The performance of this function feels really fishy - on each physics collision, we iterate through a list performing string comparisons.

    QVariantList interestingEntities = entities.Get();
    if (interestingEntities.isEmpty())
        return true;

    foreach (QVariant intname, interestingEntities)
    {
        if (intname.toString().compare(name) == 0)
        {
            return true;
        }
    }
    return false;
}

bool EC_VolumeTrigger::IsPivotInside(Entity *entity) const
{
    shared_ptr<EC_Placeable> placeable = entity->GetComponent<EC_Placeable>();
    shared_ptr<EC_RigidBody> rigidbody = rigidbody_.lock();
    if (placeable && rigidbody)
    {
        const Transform& trans = placeable->transform.Get();
        const float3& pivot = trans.pos;

        return ( RayTestSingle(float3(pivot.x, pivot.y - 1e7f, pivot.z), pivot, rigidbody->GetRigidBody()) &&
                 RayTestSingle(float3(pivot.x, pivot.y + 1e7f, pivot.z), pivot, rigidbody->GetRigidBody()) );
    }
    LogWarning("EC_VolumeTrigger::IsPivotInside(): entity has no EC_Placeable or volume has no EC_RigidBody.");
    return false;
}

bool EC_VolumeTrigger::IsInsideVolume(const float3& point) const
{
    shared_ptr<EC_RigidBody> rigidbody = rigidbody_.lock();
    if (!rigidbody)
    {
        LogWarning("Volume has no EC_RigidBody.");
        return false;
    }

    return RayTestSingle(float3(point.x, point.y - 1e7f, point.z), point, rigidbody->GetRigidBody()) &&
           RayTestSingle(float3(point.x, point.y + 1e7f, point.z), point, rigidbody->GetRigidBody());
}

void EC_VolumeTrigger::AttributesChanged()
{
    /// \todo Attribute updates not handled yet, there are a bit too many problems of what signals to send after the update -cm

    //if (mass.ValueChanged())
    //    ReadBody();
}

void EC_VolumeTrigger::UpdateSignals()
{
    Entity* parent = ParentEntity();
    if (!parent)
        return;
    
    connect(parent, SIGNAL(ComponentAdded(IComponent*, AttributeChange::Type)), this, SLOT(CheckForRigidBody()), Qt::UniqueConnection);

    Scene* scene = parent->ParentScene();
    PhysicsWorld* world = scene->GetWorld<PhysicsWorld>().get();
    if (world)
        connect(world, SIGNAL(Updated(float)), this, SLOT(OnPhysicsUpdate()), Qt::UniqueConnection);
}

void EC_VolumeTrigger::CheckForRigidBody()
{
    Entity* parent = ParentEntity();
    if (!parent)
        return;
    
    if (!rigidbody_.lock())
    {
        shared_ptr<EC_RigidBody> rigidbody = parent->GetComponent<EC_RigidBody>();
        if (rigidbody)
        {
            rigidbody_ = rigidbody;
            connect(rigidbody.get(), SIGNAL(PhysicsCollision(Entity*, const float3&, const float3&, float, float, bool)),
                this, SLOT(OnPhysicsCollision(Entity*, const float3&, const float3&, float, float, bool)), Qt::UniqueConnection);
        }
    }
}

void EC_VolumeTrigger::OnPhysicsUpdate()
{
    PROFILE(EC_VolumeTrigger_OnPhysicsUpdate);
    for(EntitiesWithinVolumeMap::iterator it = entities_.begin(); it != entities_.end();)
    {
        bool remove = false;
        EntityPtr entity = it->first.lock();
        
        // Entity was destroyed without us knowing? Remove from map in that case
        if (!entity)
            remove = true;
        else
        {
            // If collision is old, remove the entity if its rigidbody is active
            // (inactive rigidbodies do not refresh the collision, so we would remove the entity mistakenly)
            if (!it->second)
            {
                bool active = true;
                shared_ptr<EC_RigidBody> rigidbody = entity->GetComponent<EC_RigidBody>();
                if (rigidbody)
                    active = rigidbody->IsActive();
                if (active)
                    remove = true;
            }
            else
            {
                // Age the collision from new to old
                it->second = false;
            }
        }
        
        if (!remove)
            ++it;
        else
        {
            EntitiesWithinVolumeMap::iterator current = it;
            ++it;
            entities_.erase(current);
            if (entity)
            {
                emit EntityLeave(entity.get());
                emit entityLeave(entity.get());
                disconnect(entity.get(), SIGNAL(EntityRemoved(Entity*, AttributeChange::Type)), this, SLOT(OnEntityRemoved(Entity*)));
            }
        }
    }
}

void EC_VolumeTrigger::OnPhysicsCollision(Entity* otherEntity, const float3& /*position*/,
    const float3& /*normal*/, float /*distance*/, float /*impulse*/, bool newCollision)
{
    PROFILE(EC_VolumeTrigger_OnPhysicsCollision);

    assert(otherEntity && "Physics collision with no entity.");

    if (!entities.Get().isEmpty() && !IsInterestingEntity(otherEntity->Name()))
        return;

    EntityPtr entity = otherEntity->shared_from_this();

    // If byPivot attribute is enabled, we require the object pivot to enter the volume trigger area.
    // Otherwise, we react on each physics collision message (i.e. we accept if the volumetrigger and other entity just touch).
    if (byPivot.Get() && !IsPivotInside(entity.get()))
        return;

    bool refreshed = false;
    
    if (newCollision)
    {
        // make sure the entity isn't already inside the volume
        if (entities_.find(entity) == entities_.end())
        {
            entities_[entity] = true;
            emit EntityEnter(otherEntity);
            refreshed = true;
            emit entityEnter(otherEntity);
            connect(otherEntity, SIGNAL(EntityRemoved(Entity*, AttributeChange::Type)), this, SLOT(OnEntityRemoved(Entity*)), Qt::UniqueConnection);
        }
    }

    if (!refreshed)
    {
        // Refresh the collision status to new
        entities_[entity] = true;
    }
}

/** Called when the given entity is deleted from the scene. In that case, remove the Entity immediately from our tracking data structure (and signal listeners). */
void EC_VolumeTrigger::OnEntityRemoved(Entity *entity)
{
    assert(entity);
    EntitiesWithinVolumeMap::iterator i = entities_.find(entity->shared_from_this());
    if (i != entities_.end())
    {
        entities_.erase(i);
        emit EntityLeave(entity);
        emit entityLeave(entity);
    }
}
