// For conditions of distribution and use, see copyright notice in license.txt

#include "StableHeaders.h"
#define MATH_BULLET_INTEROP
#include "DebugOperatorNew.h"
#include "btBulletDynamicsCommon.h"
#include "PhysicsModule.h"
#include "PhysicsWorld.h"
#include "PhysicsUtils.h"
#include "Profiler.h"
#include "Scene.h"
#include "OgreWorld.h"
#include "EC_RigidBody.h"
#include "LoggingFunctions.h"
#include "Geometry/LineSegment.h"

#include <Ogre.h>
#include "MemoryLeakCheck.h"

namespace Physics
{

void TickCallback(btDynamicsWorld *world, btScalar timeStep)
{
    static_cast<Physics::PhysicsWorld*>(world->getWorldUserInfo())->ProcessPostTick(timeStep);
}

PhysicsWorld::PhysicsWorld(ScenePtr scene, bool isClient) :
    scene_(scene),
    collisionConfiguration_(0),
    collisionDispatcher_(0),
    broadphase_(0),
    solver_(0),
    world_(0),
    physicsUpdatePeriod_(1.0f / 60.0f),
    maxSubSteps_(6), // If fps is below 10, we start to slow down physics
    isClient_(isClient),
    runPhysics_(true),
    drawDebugGeometry_(false),
    drawDebugManuallySet_(false),
    debugDrawMode_(0),
    cachedOgreWorld_(0)
{
    collisionConfiguration_ = new btDefaultCollisionConfiguration();
    collisionDispatcher_ = new btCollisionDispatcher(collisionConfiguration_);
    broadphase_ = new btDbvtBroadphase();
    solver_ = new btSequentialImpulseConstraintSolver();
    world_ = new btDiscreteDynamicsWorld(collisionDispatcher_, broadphase_, solver_, collisionConfiguration_);
    world_->setDebugDrawer(this);
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

void PhysicsWorld::SetMaxSubSteps(int steps)
{
    if (steps > 0)
        maxSubSteps_ = steps;
}

void PhysicsWorld::SetGravity(const float3& gravity)
{
    world_->setGravity(gravity);
}

float3 PhysicsWorld::GetGravity() const
{
    return world_->getGravity();
}

btDiscreteDynamicsWorld* PhysicsWorld::GetWorld() const
{
    return world_;
}

void PhysicsWorld::Simulate(f64 frametime)
{
    if (!runPhysics_)
        return;
    
    PROFILE(PhysicsWorld_Simulate);
    
    emit AboutToUpdate((float)frametime);
    
    {
        PROFILE(Bullet_stepSimulation); ///\note Do not delete or rename this PROFILE() block. The DebugStats profiler uses this string as a label to know where to inject the Bullet internal profiling data.
        world_->stepSimulation((float)frametime, maxSubSteps_, physicsUpdatePeriod_);
    }
    
    // Automatically enable debug geometry if at least one debug-enabled rigidbody. Automatically disable if no debug-enabled rigidbodies
    // However, do not do this if user has used the physicsdebug console command
    if (!drawDebugManuallySet_)
    {
        if ((!drawDebugGeometry_) && (!debugRigidBodies_.empty()))
            SetDrawDebugGeometry(true);
        if ((drawDebugGeometry_) && (debugRigidBodies_.empty()))
            SetDrawDebugGeometry(false);
    }
    
    if (drawDebugGeometry_)
        DrawDebugGeometry();
}

void PhysicsWorld::ProcessPostTick(float substeptime)
{
    PROFILE(PhysicsWorld_ProcessPostTick);
    // Check contacts and send collision signals for them
    int numManifolds = collisionDispatcher_->getNumManifolds();
    
    std::set<std::pair<btCollisionObject*, btCollisionObject*> > currentCollisions;
    
    if (numManifolds > 0)
    {
        PROFILE(PhysicsWorld_SendCollisions);
        
        for(int i = 0; i < numManifolds; ++i)
        {
            btPersistentManifold* contactManifold = collisionDispatcher_->getManifoldByIndexInternal(i);
            int numContacts = contactManifold->getNumContacts();
            if (numContacts == 0)
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
            if (!bodyA || !bodyB)
            {
                LogError("Inconsistent Bullet physics scene state! An object exists in the physics scene which does not have an associated EC_RigidBody!");
                continue;
            }
            // Also, both bodies should have valid parent entities
            Entity* entityA = bodyA->ParentEntity();
            Entity* entityB = bodyB->ParentEntity();
            if (!entityA || !entityB)
            {
                LogError("Inconsistent Bullet physics scene state! A parentless EC_RigidBody exists in the physics scene!");
                continue;
            }
            // Check that at least one of the bodies is active
            if (!objectA->isActive() && !objectB->isActive())
                continue;
            
            bool newCollision = previousCollisions_.find(objectPair) == previousCollisions_.end();
            
            for(int j = 0; j < numContacts; ++j)
            {
                btManifoldPoint& point = contactManifold->getContactPoint(j);
                
                float3 position = point.m_positionWorldOnB;
                float3 normal = point.m_normalWorldOnB;
                float distance = point.m_distance1;
                float impulse = point.m_appliedImpulse;
                
                {
                    PROFILE(PhysicsWorld_emit_PhysicsCollision);
                    emit PhysicsCollision(entityA, entityB, position, normal, distance, impulse, newCollision);
                }
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
    
    {
        PROFILE(PhysicsWorld_ProcessPostTick_Updated);
        emit Updated(substeptime);
    }
}

PhysicsRaycastResult* PhysicsWorld::Raycast(const float3& origin, const float3& direction, float maxdistance, int collisiongroup, int collisionmask)
{
    PROFILE(PhysicsWorld_Raycast);
    
    static PhysicsRaycastResult result;
    
    float3 normalizedDir = direction.Normalized();
    
    btCollisionWorld::ClosestRayResultCallback rayCallback(origin, origin + maxdistance * normalizedDir);
    rayCallback.m_collisionFilterGroup = collisiongroup;
    rayCallback.m_collisionFilterMask = collisionmask;
    
    world_->rayTest(rayCallback.m_rayFromWorld, rayCallback.m_rayToWorld, rayCallback);
    
    result.entity = 0;
    result.distance = 0;
    
    if (rayCallback.hasHit())
    {
        result.pos = rayCallback.m_hitPointWorld;
        result.normal = rayCallback.m_hitNormalWorld;
        result.distance = (result.pos - origin).Length();
        if (rayCallback.m_collisionObject)
        {
            EC_RigidBody* body = static_cast<EC_RigidBody*>(rayCallback.m_collisionObject->getUserPointer());
            if (body)
                result.entity = body->ParentEntity();
        }
    }
    
    return &result;
}

void PhysicsWorld::SetDrawDebugGeometry(bool enable)
{
    if (scene_.expired() || !scene_.lock()->ViewEnabled() || drawDebugGeometry_ == enable)
        return;

    drawDebugGeometry_ = enable;
    if (!enable)
        setDebugMode(0);
    else
        setDebugMode(btIDebugDraw::DBG_DrawWireframe);
}

void PhysicsWorld::DrawDebugGeometry()
{
    if (!drawDebugGeometry_)
        return;

    PROFILE(PhysicsModule_DrawDebugGeometry);
    
    // Draw debug only for the active (visible) scene
    OgreWorldPtr ogreWorld = scene_.lock()->GetWorld<OgreWorld>();
    cachedOgreWorld_ = ogreWorld.get();
    if (!ogreWorld)
        return;
    if (!ogreWorld->IsActive())
        return;
    
    // Get all lines of the physics world
    world_->debugDrawWorld();
}

void PhysicsWorld::reportErrorWarning(const char* warningString)
{
    LogWarning("Physics: " + std::string(warningString));
}

void PhysicsWorld::drawLine(const btVector3& from, const btVector3& to, const btVector3& color)
{
    if (drawDebugGeometry_ && cachedOgreWorld_)
        cachedOgreWorld_->DebugDrawLine(from, to, color.x(), color.y(), color.z());
}

} // ~Physics

