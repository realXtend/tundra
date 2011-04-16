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
#include "PhysicsModule.h"
#include "PhysicsWorld.h"
#include "PhysicsUtils.h"
#include <OgreAxisAlignedBox.h>

#include "LoggingFunctions.h"

EC_VolumeTrigger::EC_VolumeTrigger(IModule* module) :
    IComponent(module->GetFramework()),
    byPivot(this, "By Pivot", false),
    entities(this, "Entities"),
    owner_(checked_static_cast<Physics::PhysicsModule*>(module))
{
    connect(this, SIGNAL(AttributeChanged(IAttribute*, AttributeChange::Type)), SLOT(OnAttributeUpdated(IAttribute*)));
    connect(this, SIGNAL(ParentEntitySet()), this, SLOT(UpdateSignals()));
}

EC_VolumeTrigger::~EC_VolumeTrigger()
{
}

QList<Scene::EntityWeakPtr> EC_VolumeTrigger::GetEntitiesInside() const
{
    return entities_.keys();
}

int EC_VolumeTrigger::GetNumEntitiesInside() const
{
    return entities_.size();
}

Scene::Entity* EC_VolumeTrigger::GetEntityInside(int idx) const
{
    QList<Scene::EntityWeakPtr> entities = entities_.keys();
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
    QList<Scene::EntityWeakPtr> entities = entities_.keys();
    foreach (Scene::EntityWeakPtr entityw, entities)
    {
        EntityPtr entity = entityw.lock();
        if (entity)
            entitynames.append(entity->GetName());
    }

    return entitynames;
}

float EC_VolumeTrigger::GetEntityInsidePercent(const Scene::Entity *entity) const
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
    QList<Scene::EntityWeakPtr> entities = entities_.keys();
    foreach(Scene::EntityWeakPtr wentity, entities)
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

bool EC_VolumeTrigger::IsPivotInside(Scene::Entity *entity) const
{
    boost::shared_ptr<EC_Placeable> placeable = entity->GetComponent<EC_Placeable>();
    boost::shared_ptr<EC_RigidBody> rigidbody = rigidbody_.lock();
    if (placeable && rigidbody)
    {
        const Transform& trans = placeable->transform.Get();
        const Vector3df& pivot = trans.position;

        return ( RayTestSingle(Vector3df(pivot.x, pivot.y, pivot.z - 1e7), pivot, rigidbody->GetRigidBody()) &&
                 RayTestSingle(Vector3df(pivot.x, pivot.y, pivot.z + 1e7), pivot, rigidbody->GetRigidBody()) );
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

    return RayTestSingle(Vector3df(point.x, point.y, point.z - 1e7), point, rigidbody->GetRigidBody()) &&
           RayTestSingle(Vector3df(point.x, point.y, point.z + 1e7), point, rigidbody->GetRigidBody());
}

void EC_VolumeTrigger::OnAttributeUpdated(IAttribute* attribute)
{
    /// \todo Attribute updates not handled yet, there are a bit too many problems of what signals to send after the update -cm

    //if (attribute == &mass)
    //    ReadBody();
}

void EC_VolumeTrigger::UpdateSignals()
{
    Scene::Entity* parent = GetParentEntity();
    if (!parent)
        return;
    
    connect(parent, SIGNAL(ComponentAdded(IComponent*, AttributeChange::Type)), this, SLOT(CheckForRigidBody()));

    Scene::SceneManager* scene = parent->GetScene();
    Physics::PhysicsWorld* world = owner_->GetPhysicsWorldForScene(scene);
    if (world)
        connect(world, SIGNAL(Updated(float)), this, SLOT(OnPhysicsUpdate()));
}

void EC_VolumeTrigger::CheckForRigidBody()
{
    Scene::Entity* parent = GetParentEntity();
    if (!parent)
        return;
    
    if (!rigidbody_.lock())
    {
        boost::shared_ptr<EC_RigidBody> rigidbody = parent->GetComponent<EC_RigidBody>();
        if (rigidbody)
        {
            rigidbody_ = rigidbody;
            connect(rigidbody.get(), SIGNAL(PhysicsCollision(Scene::Entity*, const Vector3df&, const Vector3df&, float, float, bool)),
                this, SLOT(OnPhysicsCollision(Scene::Entity*, const Vector3df&, const Vector3df&, float, float, bool)));
        }
    }
}

void EC_VolumeTrigger::OnPhysicsUpdate()
{
    QMap<Scene::EntityWeakPtr, bool>::iterator i = entities_.begin();
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
                    disconnect(entity.get(), SIGNAL(EntityRemoved(Scene::Entity*, AttributeChange::Type)), this, SLOT(OnEntityRemoved(Scene::Entity*)));
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

void EC_VolumeTrigger::OnPhysicsCollision(Scene::Entity* otherEntity, const Vector3df& position, const Vector3df& normal, float distance, float impulse, bool newCollision)
{
    assert (otherEntity && "Physics collision with no entity.");

    if (!entities.Get().isEmpty() && !IsInterestingEntity(otherEntity->GetName()))
        return;

    EntityPtr entity = otherEntity->shared_from_this();

    if (byPivot.Get())
    {
        if (IsPivotInside(entity.get()))
        {
            if (entities_.find(entity) == entities_.end())
            {
                emit entityEnter(otherEntity);
                connect(otherEntity, SIGNAL(EntityRemoved(Scene::Entity*, AttributeChange::Type)), this, SLOT(OnEntityRemoved(Scene::Entity*)));
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
                connect(otherEntity, SIGNAL(EntityRemoved(Scene::Entity*, AttributeChange::Type)), this, SLOT(OnEntityRemoved(Scene::Entity*)));
            }
        }
        entities_.insert(entity, true);
    }
}

void EC_VolumeTrigger::OnEntityRemoved(Scene::Entity *entity)
{
    Scene::EntityWeakPtr ptr = entity->shared_from_this();
    QMap<Scene::EntityWeakPtr, bool>::iterator i = entities_.find(ptr);
    if (i != entities_.end())
    {
        entities_.erase(i);

        emit entityLeave(entity);
    }
}

