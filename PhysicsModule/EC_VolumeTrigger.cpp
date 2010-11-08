// For conditions of distribution and use, see copyright notice in license.txt

#include "StableHeaders.h"
#include "EC_VolumeTrigger.h"
#include "EC_RigidBody.h"
#include "Entity.h"
#include "PhysicsModule.h"
#include "PhysicsWorld.h"
#include <OgreAxisAlignedBox.h>

#include "LoggingFunctions.h"
DEFINE_POCO_LOGGING_FUNCTIONS("EC_VolumeTrigger");


EC_VolumeTrigger::EC_VolumeTrigger(IModule* module) :
    IComponent(module->GetFramework()),
    byPivot(this, "By Pivot", false),
    entities(this, "Entities"),
    owner_(checked_static_cast<Physics::PhysicsModule*>(module))
{
    QObject::connect(this, SIGNAL(OnAttributeChanged(IAttribute*, AttributeChange::Type)),
            SLOT(AttributeUpdated(IAttribute*)));

    connect(this, SIGNAL(ParentEntitySet()), this, SLOT(UpdateSignals()));
}

EC_VolumeTrigger::~EC_VolumeTrigger()
{

}

QList<Scene::EntityWeakPtr> EC_VolumeTrigger::GetEntitiesInside() const
{
    return entities_.keys();
}

QStringList EC_VolumeTrigger::GetEntityNamesInside() const
{
    QStringList entitynames;
    QList<Scene::EntityWeakPtr> entities = entities_.keys();
    foreach (Scene::EntityWeakPtr entityw, entities)
    {
        Scene::EntityPtr entity = entityw.lock();
        if (entity)
            entitynames.append(entity->GetName());
    }

    return entitynames;
}

float EC_VolumeTrigger::GetEntityInsidePercent(Scene::Entity *entity) const
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
        }
    }
    return 0.f;
}

f32 EC_VolumeTrigger::GetEntityInsidePercentByName(const QString &name) const
{
    QList<Scene::EntityWeakPtr> entities = entities_.keys();
    foreach(Scene::EntityWeakPtr wentity, entities)
    {
        Scene::EntityPtr entity = wentity.lock();
        if (entity && entity->GetName().compare(name) == 0)
            return GetEntityInsidePercent(entity.get());
    }
    return 0.f;
}


void EC_VolumeTrigger::AttributeUpdated(IAttribute* attribute)
{    
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
    connect(world, SIGNAL(Updated(float)), this, SLOT(OnPhysicsUpdate()));
}

void EC_VolumeTrigger::CheckForRigidBody()
{
    Scene::Entity* parent = GetParentEntity();
    if (!parent)
        return;
    
    if (!rigidbody_.lock().get())
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
    while (i != entities_.end())
    {
        //! \todo Handle entities that get removed from the scene, they should also emit EntityLeave but currently don't. -cmayhem
        if (!i.value())
        {
            bool active = true;
            Scene::EntityPtr entity = i.key().lock();
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
                    emit EntityLeave(entity.get());
                    LogInfo("leave");
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

    QVariantList interestingEntities = entities.Get();
    bool found = interestingEntities.isEmpty();
    if (!found)
    {
        QString otherEntityName = otherEntity->GetName();
        foreach (QVariant name, interestingEntities)
        {
            if (name.toString().compare(otherEntityName) == 0)
            {
                found = true;
                break;
            }
        }
    }
    
    if (!found)
        return;

    Scene::EntityPtr entity = otherEntity->GetSharedPtr();

    if (newCollision)
    {
        // make sure the entity isn't already inside the volume
        if (entities_.find(entity) == entities_.end())
        {
            emit EntityEnter(otherEntity);
            connect(otherEntity, SIGNAL(EntityRemoved(Scene::Entity*, AttributeChange::Type)), this, SLOT(OnEntityRemoved(Scene::Entity*)));
            LogInfo("enter");
        }

        entities_.insert(entity, true);
    } else
    {
        entities_.insert(entity, true);
    }
}

void EC_VolumeTrigger::OnEntityRemoved(Scene::Entity *entity)
{
    Scene::EntityWeakPtr ptr = entity->GetSharedPtr();
    QMap<Scene::EntityWeakPtr, bool>::iterator i = entities_.find(ptr);
    if (i != entities_.end())
    {
        entities_.erase(i);

        emit EntityLeave(entity);
        //LogInfo("leave");
    }
}
