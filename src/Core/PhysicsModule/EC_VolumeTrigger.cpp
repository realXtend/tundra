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
    byPivot(this, "By Pivot", false),
    entities(this, "Entities")
{
    connect(this, SIGNAL(ParentEntitySet()), this, SLOT(UpdateSignals()), Qt::UniqueConnection);
}

EC_VolumeTrigger::~EC_VolumeTrigger()
{
}

QList<EntityWeakPtr> EC_VolumeTrigger::GetEntitiesInside() const
{
    return entities_.keys();
}

int EC_VolumeTrigger::GetNumEntitiesInside() const
{
    return entities_.size();
}

Entity* EC_VolumeTrigger::GetEntityInside(int idx) const
{
    QList<EntityWeakPtr> entities = entities_.keys();
    if (idx >=0 && idx < entities.size())
    {
        EntityPtr entity = entities.at(idx).lock();
        if (entity)
            return entity.get();
    }
    return 0;
}

QStringList EC_VolumeTrigger::GetEntityNamesInside() const
{
    QStringList entitynames;
    QList<EntityWeakPtr> entities = entities_.keys();
    foreach (EntityWeakPtr entityw, entities)
    {
        EntityPtr entity = entityw.lock();
        if (entity)
            entitynames.append(entity->Name());
    }

    return entitynames;
}

float EC_VolumeTrigger::GetEntityInsidePercent(const Entity *entity) const
{
    if (entity)
    {
        boost::shared_ptr<EC_RigidBody> otherRigidbody = entity->GetComponent<EC_RigidBody>();

        boost::shared_ptr<EC_RigidBody> rigidbody = rigidbody_.lock();
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
    QList<EntityWeakPtr> entities = entities_.keys();
    foreach(EntityWeakPtr wentity, entities)
    {
        EntityPtr entity = wentity.lock();
        if (entity && entity->Name().compare(name) == 0)
            return GetEntityInsidePercent(entity.get());
    }
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
    boost::shared_ptr<EC_Placeable> placeable = entity->GetComponent<EC_Placeable>();
    boost::shared_ptr<EC_RigidBody> rigidbody = rigidbody_.lock();
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
    boost::shared_ptr<EC_RigidBody> rigidbody = rigidbody_.lock();
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
        boost::shared_ptr<EC_RigidBody> rigidbody = parent->GetComponent<EC_RigidBody>();
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
    QMap<EntityWeakPtr, bool>::iterator i = entities_.begin();
    while(i != entities_.end())
    {
        if (!i.value())
        {
            EntityPtr entity = i.key().lock();
            bool active = true;
            // inactive rigid bodies don't generate collisions, so before emitting EntityLeave -event, make sure the body is active.
            if (entity)
            {
                boost::shared_ptr<EC_RigidBody> rigidbody = entity->GetComponent<EC_RigidBody>();
                if (rigidbody)
                    active = rigidbody->IsActive();
            }
            if (active)
            {
                i = entities_.erase(i);
                
                if (entity)
                {
                    emit entityLeave(entity.get());
                    disconnect(entity.get(), SIGNAL(EntityRemoved(Entity*, AttributeChange::Type)), this, SLOT(OnEntityRemoved(Entity*)));
                }
                continue;
            }
        } else
        {
            // Age the internal bool from true to false to signal that this collision is now an old one.
            entities_.insert(i.key(), false);
        }
        ++i;
    }
}

void EC_VolumeTrigger::OnPhysicsCollision(Entity* otherEntity, const float3& position, const float3& normal, float distance, float impulse, bool newCollision)
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

    if (newCollision)
    {
        // make sure the entity isn't already inside the volume
        if (entities_.find(entity) == entities_.end())
        {
            emit entityEnter(otherEntity);
            connect(otherEntity, SIGNAL(EntityRemoved(Entity*, AttributeChange::Type)), this, SLOT(OnEntityRemoved(Entity*)), Qt::UniqueConnection);
        }
    }
    entities_.insert(entity, true);
}

/** Called when the given entity is deleted from the scene. In that case, remove the Entity immediately from our tracking data structure (and signal listeners). */
void EC_VolumeTrigger::OnEntityRemoved(Entity *entity)
{
    assert(entity);
    EntityWeakPtr ptr = entity->shared_from_this();
    QMap<EntityWeakPtr, bool>::iterator i = entities_.find(ptr);
    if (i != entities_.end())
    {
        entities_.erase(i);
        emit entityLeave(entity);
    }
}

