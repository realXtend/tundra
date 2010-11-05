// For conditions of distribution and use, see copyright notice in license.txt

#include "StableHeaders.h"
#include "EC_VolumeTrigger.h"
#include "EC_RigidBody.h"
#include "Entity.h"
#include "PhysicsModule.h"
#include "PhysicsWorld.h"

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
        //! \todo Handle entities that get removed from the scene, they should also emit EntityLeave but currently don't
        if (!i.value())
        {
            bool active = true;
            Scene::EntityPtr entity = i.key().lock();
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
                    emit EntityLeave(entity.get(), Vector3df());
                   // LogInfo("leave");
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
    /*Scene::EntityPtr entity = otherEntity->GetSharedPtr();
    if (newCollision && entities_.find(entity) == entities_.end())
    {
        emit EntityEnter(otherEntity, position);
    }*/

    Scene::EntityPtr entity = otherEntity->GetSharedPtr();

    if (newCollision)
    {
        // make sure the entity isn't already inside the volume
        if (entities_.find(entity) == entities_.end())
        {
            emit EntityEnter(otherEntity, position);
           // LogInfo("enter");
        }

        entities_.insert(entity, true);
    } else
    {
        entities_.insert(entity, true);
    }
}
