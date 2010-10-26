// For conditions of distribution and use, see copyright notice in license.txt

#include "StableHeaders.h"
#include "PhysicsModule.h"
#include "PhysicsWorld.h"
#include "PhysicsUtils.h"
#include "PhysicsContact.h"
#include "Profiler.h"
#include "EC_RigidBody.h"

#include "btBulletDynamicsCommon.h"

namespace Physics
{

// Assume we generate at least 10 frames per second. If less, the physics will start to slow down.
static const float cMinFps = 10.0f;

void TickCallback(btDynamicsWorld *world, btScalar timeStep)
{
    static_cast<Physics::PhysicsWorld*>(world->getWorldUserInfo())->ProcessPostTick(timeStep);
}

PhysicsWorld::PhysicsWorld(PhysicsModule* owner) :
    collisionConfiguration_(0),
    collisionDispatcher_(0),
    broadphase_(0),
    solver_(0),
    world_(0),
    physicsUpdatePeriod_(1.0f / 60.0f)
{
    collisionConfiguration_ = new btDefaultCollisionConfiguration();
    collisionDispatcher_ = new btCollisionDispatcher(collisionConfiguration_);
    broadphase_ = new btDbvtBroadphase();
    solver_ = new btSequentialImpulseConstraintSolver();
    world_ = new btDiscreteDynamicsWorld(collisionDispatcher_, broadphase_, solver_, collisionConfiguration_);
    world_->setDebugDrawer(owner);
    world_->setInternalTickCallback(TickCallback, (void*)this, false);
}

PhysicsWorld::~PhysicsWorld()
{
    delete world_;
    world_ = 0;
    
    delete solver_;
    solver_ = 0;
    
    delete broadphase_;
    broadphase_ = 0;
    
    delete collisionDispatcher_;
    collisionDispatcher_ = 0;
    
    delete collisionConfiguration_;
    collisionConfiguration_ = 0;
    
    for (uint i = 0; i < contactsStore_.size(); ++i)
        delete contactsStore_[i];
    contactsStore_.clear();
}

void PhysicsWorld::SetPhysicsUpdatePeriod(float updatePeriod)
{
    // Allow max.1000 fps
    if (updatePeriod <= 0.001f)
        updatePeriod = 0.001f;
    physicsUpdatePeriod_ = updatePeriod;
}

void PhysicsWorld::SetGravity(const Vector3df& gravity)
{
    world_->setGravity(ToBtVector3(gravity));
}

Vector3df PhysicsWorld::GetGravity() const
{
    return ToVector3(world_->getGravity());
}

btDynamicsWorld* PhysicsWorld::GetWorld() const
{
    return world_;
}

void PhysicsWorld::Simulate(f64 frametime)
{
    PROFILE(PhysicsWorld_Simulate);
    
    int maxSubSteps = (int)((1.0f / physicsUpdatePeriod_) / cMinFps);
    world_->stepSimulation((float)frametime, maxSubSteps, physicsUpdatePeriod_);
}

void PhysicsWorld::ProcessPostTick(float substeptime)
{
    // Check contacts and send collision signals for them
    int numManifolds = collisionDispatcher_->getNumManifolds();
    
    std::set<std::pair<btCollisionObject*, btCollisionObject*> > currentCollisions;
    
    if (numManifolds)
    {
        PROFILE(PhysicsWorld_SendCollisions);
        
        for (int i = 0; i < numManifolds; ++i)
        {
            btPersistentManifold* contactManifold = collisionDispatcher_->getManifoldByIndexInternal(i);
            int numContacts = contactManifold->getNumContacts();
            if (!numContacts)
                continue;
            
            btCollisionObject* objectA = static_cast<btCollisionObject*>(contactManifold->getBody0());
            btCollisionObject* objectB = static_cast<btCollisionObject*>(contactManifold->getBody1());
            std::pair<btCollisionObject*, btCollisionObject*> objectPair(objectA, objectB);
            
            EC_RigidBody* bodyA = static_cast<EC_RigidBody*>(objectA->getUserPointer());
            EC_RigidBody* bodyB = static_cast<EC_RigidBody*>(objectB->getUserPointer());
            
            // We are only interested in collisions where both EC_RigidBody components are known
            if ((!bodyA) || (!bodyB))
                continue;
            // Also, both bodies should have valid parent entities
            Scene::Entity* entityA = bodyA->GetParentEntity();
            Scene::Entity* entityB = bodyB->GetParentEntity();
            if ((!entityA) || (!entityB))
                continue;
            // Check that at least one of the bodies is active
            if ((!objectA->isActive()) && (!objectB->isActive()))
                continue;
            
            bool newCollision = previousCollisions_.find(objectPair) == previousCollisions_.end();
            
            //! Create new contact structures if not enough room
            int oldSize = contactsStore_.size();
            if (numContacts > oldSize)
            {
                contactsStore_.resize(numContacts);
                for (int j = oldSize; j < numContacts; ++j)
                    contactsStore_[j] = new PhysicsContact();
            }
            
            QVector<PhysicsContact*> contacts;
            
            for (int j = 0; j < numContacts; ++j)
            {
                btManifoldPoint& point = contactManifold->getContactPoint(j);
                
                PhysicsContact* newContact = contactsStore_[j];
                
                newContact->position = ToVector3(point.m_positionWorldOnB);
                newContact->normal = ToVector3(point.m_normalWorldOnB);
                newContact->distance = point.m_distance1;
                newContact->impulse = point.m_appliedImpulse;
                newContact->newCollision = newCollision;
                contacts.push_back(newContact);
            }
            
            emit PhysicsCollision(entityA, entityB, contacts);
            bodyA->EmitPhysicsCollision(entityB, contacts);
            bodyB->EmitPhysicsCollision(entityA, contacts);
            
            currentCollisions.insert(objectPair);
        }
    }
    
    previousCollisions_ = currentCollisions;
}

}

