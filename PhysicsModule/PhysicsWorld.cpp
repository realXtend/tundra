// For conditions of distribution and use, see copyright notice in license.txt

#include "StableHeaders.h"
#include "DebugOperatorNew.h"
#include "btBulletDynamicsCommon.h"
#include "PhysicsModule.h"
#include "PhysicsWorld.h"
#include "PhysicsUtils.h"
#include "Profiler.h"
#include "Scene.h"
#include "OgreWorld.h"
#include "OgreBulletCollisionsDebugLines.h"
#include "EC_RigidBody.h"
#include "MemoryLeakCheck.h"
#include "LoggingFunctions.h"

#include <Ogre.h>

namespace Physics
{

// Assume we generate at least 10 frames per second. If less, the physics will start to slow down.
static const float cMinFps = 10.0f;

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
    isClient_(isClient),
    runPhysics_(true),
    drawDebugGeometry_(false),
    drawDebugManuallySet_(false),
    debugGeometryObject_(0),
    debugDrawMode_(0)
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
    if (!runPhysics_)
        return;
    
    PROFILE(PhysicsWorld_Simulate);
    
    emit AboutToUpdate((float)frametime);
    
    int maxSubSteps = (int)((1.0f / physicsUpdatePeriod_) / cMinFps);
    world_->stepSimulation((float)frametime, maxSubSteps, physicsUpdatePeriod_);
    
        
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
        UpdateDebugGeometry();
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
            Entity* entityA = bodyA->ParentEntity();
            Entity* entityB = bodyB->ParentEntity();
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
                result.entity_ = body->ParentEntity();
        }
    }
    
    return &result;
}

void PhysicsWorld::SetDrawDebugGeometry(bool enable)
{
    if (scene_.expired() || !scene_.lock()->ViewEnabled() || drawDebugGeometry_ == enable)
        return;
    OgreWorldPtr ogreWorld = scene_.lock()->GetWorld<OgreWorld>();
    if (!ogreWorld)
        return;
    Ogre::SceneManager* scenemgr = ogreWorld->GetSceneManager();
    
    drawDebugGeometry_ = enable;
    if (!enable)
    {
        setDebugMode(0);
        
        if (debugGeometryObject_)
        {
            scenemgr->getRootSceneNode()->detachObject(debugGeometryObject_);
            delete debugGeometryObject_;
            debugGeometryObject_ = 0;
        }
    }
    else
    {
        setDebugMode(btIDebugDraw::DBG_DrawWireframe);
        
        if (!debugGeometryObject_)
        {
#include "DisableMemoryLeakCheck.h"
            debugGeometryObject_ = new DebugLines();
#include "EnableMemoryLeakCheck.h"
            scenemgr->getRootSceneNode()->attachObject(debugGeometryObject_);
        }
    }
}

void PhysicsWorld::UpdateDebugGeometry()
{
    if ((!drawDebugGeometry_) || (!debugGeometryObject_))
        return;

    PROFILE(PhysicsModule_UpdateDebugGeometry);
    
    // Draw debug only for the active (visible) scene
    OgreWorldPtr ogreWorld = scene_.lock()->GetWorld<OgreWorld>();
    if (!ogreWorld)
        return;
    if (!ogreWorld->IsActive())
        return;
    
    // Get all lines of the physics world
    world_->debugDrawWorld();
    
    // Build the debug vertex buffer. Note: this is a no-op if there is no debug objects to draw
    debugGeometryObject_->draw();
}

void PhysicsWorld::reportErrorWarning(const char* warningString)
{
    LogWarning("Physics: " + std::string(warningString));
}

void PhysicsWorld::drawLine(const btVector3& from, const btVector3& to, const btVector3& color)
{
    if ((drawDebugGeometry_) && (debugGeometryObject_))
        debugGeometryObject_->addLine(from, to, color);
}

}

