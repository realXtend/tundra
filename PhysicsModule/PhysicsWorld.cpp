// For conditions of distribution and use, see copyright notice in license.txt

#include "StableHeaders.h"
#include "DebugOperatorNew.h"
#include "btBulletDynamicsCommon.h"
#include "PhysicsModule.h"
#include "PhysicsWorld.h"
#include "PhysicsUtils.h"
#include "Profiler.h"
#include "EC_RigidBody.h"
#include "MemoryLeakCheck.h"

namespace Physics
{

// Assume we generate at least 10 frames per second. If less, the physics will start to slow down.
static const float cMinFps = 10.0f;

void TickCallback(btDynamicsWorld *world, btScalar timeStep)
{
    static_cast<Physics::PhysicsWorld*>(world->getWorldUserInfo())->ProcessPostTick(timeStep);
}

PhysicsWorld::PhysicsWorld(PhysicsModule* owner, bool isClient) :
    collisionConfiguration_(0),
    collisionDispatcher_(0),
    broadphase_(0),
    solver_(0),
    world_(0),
    physicsUpdatePeriod_(1.0f / 60.0f),
    isClient_(isClient)
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

btDiscreteDynamicsWorld* PhysicsWorld::GetWorld() const
{
    return world_;
}

void PhysicsWorld::Simulate(f64 frametime)
{
    PROFILE(PhysicsWorld_Simulate);
    
    emit AboutToUpdate((float)frametime);
    
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
        
        for(int i = 0; i < numManifolds; ++i)
        {
            btPersistentManifold* contactManifold = collisionDispatcher_->getManifoldByIndexInternal(i);
            int numContacts = contactManifold->getNumContacts();
            if (!numContacts)
                continue;
            
            btCollisionObject* objectA = static_cast<btCollisionObject*>(contactManifold->getBody0());
            btCollisionObject* objectB = static_cast<btCollisionObject*>(contactManifold->getBody1());
            std::pair<btCollisionObject*, btCollisionObject*> objectPair;
            if (objectA < objectB)
                objectPair = std::make_pair(objectA, objectB);
            else
                objectPair = std::make_pair(objectB, objectA);
            
            EC_RigidBody* bodyA = static_cast<EC_RigidBody*>(objectA->getUserPointer());
            EC_RigidBody* bodyB = static_cast<EC_RigidBody*>(objectB->getUserPointer());
            
            // We are only interested in collisions where both EC_RigidBody components are known
            if ((!bodyA) || (!bodyB))
                continue;
            // Also, both bodies should have valid parent entities
            Entity* entityA = bodyA->GetParentEntity();
            Entity* entityB = bodyB->GetParentEntity();
            if ((!entityA) || (!entityB))
                continue;
            // Check that at least one of the bodies is active
            if ((!objectA->isActive()) && (!objectB->isActive()))
                continue;
            
            bool newCollision = previousCollisions_.find(objectPair) == previousCollisions_.end();
            
            for(int j = 0; j < numContacts; ++j)
            {
                btManifoldPoint& point = contactManifold->getContactPoint(j);
                
                Vector3df position = ToVector3(point.m_positionWorldOnB);
                Vector3df normal = ToVector3(point.m_normalWorldOnB);
                float distance = point.m_distance1;
                float impulse = point.m_appliedImpulse;
                
                emit PhysicsCollision(entityA, entityB, position, normal, distance, impulse, newCollision);
                bodyA->EmitPhysicsCollision(entityB, position, normal, distance, impulse, newCollision);
                bodyB->EmitPhysicsCollision(entityA, position, normal, distance, impulse, newCollision);
                
                // Report newCollision = true only for the first contact, in case there are several contacts, and application does some logic depending on it
                // (for example play a sound -> avoid multiple sounds being played)
                newCollision = false;
            }
            
            currentCollisions.insert(objectPair);
        }
    }
    
    previousCollisions_ = currentCollisions;
    
    emit Updated(substeptime);
}

PhysicsRaycastResult* PhysicsWorld::Raycast(const Vector3df& origin, const Vector3df& direction, float maxdistance, int collisiongroup, int collisionmask)
{
    PROFILE(PhysicsWorld_Raycast);
    
    static PhysicsRaycastResult result;
    
    Vector3df normalizedDir = direction;
    normalizedDir.normalize();
    
    btCollisionWorld::ClosestRayResultCallback rayCallback(ToBtVector3(origin), ToBtVector3(origin + maxdistance * normalizedDir));
    rayCallback.m_collisionFilterGroup = collisiongroup;
    rayCallback.m_collisionFilterMask = collisionmask;
    
    world_->rayTest(rayCallback.m_rayFromWorld, rayCallback.m_rayToWorld, rayCallback);
    
    result.entity_ = 0;
    result.distance_ = 0;
    
    if (rayCallback.hasHit())
    {
        result.pos_ = ToVector3(rayCallback.m_hitPointWorld);
        result.normal_ = ToVector3(rayCallback.m_hitNormalWorld);
        result.distance_ = (result.pos_ - origin).getLength();
        if (rayCallback.m_collisionObject)
        {
            EC_RigidBody* body = static_cast<EC_RigidBody*>(rayCallback.m_collisionObject->getUserPointer());
            if (body)
                result.entity_ = body->GetParentEntity();
        }
    }
    
    return &result;
}

}

