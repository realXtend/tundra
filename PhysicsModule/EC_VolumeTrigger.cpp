// For conditions of distribution and use, see copyright notice in license.txt

#include "StableHeaders.h"
#include "DebugOperatorNew.h"
#include <QMap>
#include "btBulletDynamicsCommon.h"
#include "MemoryLeakCheck.h"
#include "EC_VolumeTrigger.h"
#include "EC_RigidBody.h"
#include "EC_Placeable.h"
#include "Entity.h"
#include "Scene.h"
#include "PhysicsModule.h"
#include "PhysicsWorld.h"
#include "PhysicsUtils.h"
#include <OgreAxisAlignedBox.h>

#include "LoggingFunctions.h"

using namespace Physics;

EC_VolumeTrigger::EC_VolumeTrigger(Scene* scene) :
    IComponent(scene),
    byPivot(this, "By Pivot", false),
    entities(this, "Entities")
{
    connect(this, SIGNAL(AttributeChanged(IAttribute*, AttributeChange::Type)), SLOT(OnAttributeUpdated(IAttribute*)));
    connect(this, SIGNAL(ParentEntitySet()), this, SLOT(UpdateSignals()));
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
            entitynames.append(entity->GetName());
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
            Vector3df thisBoxMin, thisBoxMax;
            rigidbody->GetAabbox(thisBoxMin, thisBoxMax);

            Vector3df otherBoxMin, otherBoxMax;
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
        if (entity && entity->GetName().compare(name) == 0)
            return GetEntityInsidePercent(entity.get());
    }
    return 0.f;
}

bool EC_VolumeTrigger::IsInterestingEntity(const QString &name) const
{
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
        const Vector3df& pivot = trans.pos;

        return ( RayTestSingle(Vector3df(pivot.x, pivot.y - 1e7f, pivot.z), pivot, rigidbody->GetRigidBody()) &&
                 RayTestSingle(Vector3df(pivot.x, pivot.y + 1e7f, pivot.z), pivot, rigidbody->GetRigidBody()) );
    }
    LogWarning("EC_VolumeTrigger::IsPivotInside(): entity has no EC_Placeable or volume has no EC_RigidBody.");
    return false;
}

bool EC_VolumeTrigger::IsInsideVolume(const Vector3df& point) const
{
    boost::shared_ptr<EC_RigidBody> rigidbody = rigidbody_.lock();
    if (!rigidbody)
    {
        LogWarning("Volume has no EC_RigidBody.");
        return false;
    }

    return RayTestSingle(Vector3df(point.x, point.y - 1e7f, point.z), point, rigidbody->GetRigidBody()) &&
           RayTestSingle(Vector3df(point.x, point.y + 1e7f, point.z), point, rigidbody->GetRigidBody());
}

void EC_VolumeTrigger::OnAttributeUpdated(IAttribute* attribute)
{
    /// \todo Attribute updates not handled yet, there are a bit too many problems of what signals to send after the update -cm

    //if (attribute == &mass)
    //    ReadBody();
}

void EC_VolumeTrigger::UpdateSignals()
{
    Entity* parent = GetParentEntity();
    if (!parent)
        return;
    
    connect(parent, SIGNAL(ComponentAdded(IComponent*, AttributeChange::Type)), this, SLOT(CheckForRigidBody()));

    Scene* scene = parent->GetScene();
    PhysicsWorld* world = scene->GetWorld<PhysicsWorld>().get();
    if (world)
        connect(world, SIGNAL(Updated(float)), this, SLOT(OnPhysicsUpdate()));
}

void EC_VolumeTrigger::CheckForRigidBody()
{
    Entity* parent = GetParentEntity();
    if (!parent)
        return;
    
    if (!rigidbody_.lock())
    {
        boost::shared_ptr<EC_RigidBody> rigidbody = parent->GetComponent<EC_RigidBody>();
        if (rigidbody)
        {
            rigidbody_ = rigidbody;
            connect(rigidbody.get(), SIGNAL(PhysicsCollision(Entity*, const Vector3df&, const Vector3df&, float, float, bool)),
                this, SLOT(OnPhysicsCollision(Entity*, const Vector3df&, const Vector3df&, float, float, bool)));
        }
    }
}

void EC_VolumeTrigger::OnPhysicsUpdate()
{
    QMap<EntityWeakPtr, bool>::iterator i = entities_.begin();
    while(i != entities_.end())
    {
        if (!i.value())
        {
            EntityPtr entity = i.key().lock();
            /* disabled the check 'cause couldn't get the targets active, and the (possible) extran signaling doesn't do harm? --antont 
            bool active = true;
            // inactive rigid bodies don't generate collisions, so before emitting EntityLeave -event, make sure the body is active.
            if (entity)
            {
                boost::shared_ptr<EC_RigidBody> rigidbody = entity->GetComponent<EC_RigidBody>();
                if (rigidbody)
                    active = rigidbody->IsActive();
            }
            if (active)*/
            if (true)
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
            entities_.insert(i.key(), false);
        }
        i++;
    }
}

void EC_VolumeTrigger::OnPhysicsCollision(Entity* otherEntity, const Vector3df& position, const Vector3df& normal, float distance, float impulse, bool newCollision)
{
    assert (otherEntity && "Physics collision with no entity.");

    if (!entities.Get().isEmpty() && !IsInterestingEntity(otherEntity->GetName()))
        return;

    EntityPtr entity = otherEntity->shared_from_this();

    // Forcibly keep the other rigidbody awake while inside the trigger, because otherwise the trigger will bug once the body goes to rest
    EC_RigidBody* rb = entity->GetComponent<EC_RigidBody>().get();
    if (rb)
        rb->KeepActive();
    
    if (byPivot.Get())
    {
        if (IsPivotInside(entity.get()))
        {
            if (entities_.find(entity) == entities_.end())
            {
                emit entityEnter(otherEntity);
                connect(otherEntity, SIGNAL(EntityRemoved(Entity*, AttributeChange::Type)), this, SLOT(OnEntityRemoved(Entity*)));
            }

            entities_.insert(entity, true);
        }
    } else
    {
        if (newCollision)
        {
            // make sure the entity isn't already inside the volume
            if (entities_.find(entity) == entities_.end())
            {
                emit entityEnter(otherEntity);
                connect(otherEntity, SIGNAL(EntityRemoved(Entity*, AttributeChange::Type)), this, SLOT(OnEntityRemoved(Entity*)));
            }
        }
        entities_.insert(entity, true);
    }
}

void EC_VolumeTrigger::OnEntityRemoved(Entity *entity)
{
    EntityWeakPtr ptr = entity->shared_from_this();
    QMap<EntityWeakPtr, bool>::iterator i = entities_.find(ptr);
    if (i != entities_.end())
    {
        entities_.erase(i);

        emit entityLeave(entity);
    }
}

