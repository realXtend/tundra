// For conditions of distribution and use, see copyright notice in LICENSE

#include "StableHeaders.h"
#define MATH_BULLET_INTEROP
#include "DebugOperatorNew.h"

#include "PhysicsModule.h"
#include "PhysicsWorld.h"
#include "PhysicsUtils.h"
#include "Profiler.h"
#include "Scene/Scene.h"
#include "OgreWorld.h"
#include "EC_RigidBody.h"
#include "LoggingFunctions.h"
#include "Geometry/LineSegment.h"
#include "Geometry/OBB.h"
#include "Math/float3x3.h"
#include "Math/Quat.h"
#include "Entity.h"

// Disable unreferenced formal parameter coming from Bullet
#ifdef _MSC_VER
#pragma warning(push)
#pragma warning(disable : 4100)
#endif
#include <btBulletDynamicsCommon.h>
#ifdef _MSC_VER
#pragma warning(pop)
#endif

#include <Ogre.h>

#include "MemoryLeakCheck.h"

namespace
{

struct CollisionSignal
{
    EC_RigidBody *bodyA;
    EC_RigidBody *bodyB;
    float3 position;
    float3 normal;
    float distance;
    float impulse;
    bool newCollision;
};

struct ObbCallback : public btCollisionWorld::ContactResultCallback
{
    ObbCallback(std::set<btCollisionObjectWrapper*>& result) : result_(result) {}

    virtual btScalar addSingleResult(btManifoldPoint &cp, const btCollisionObjectWrapper *colObj0, int, int, const btCollisionObjectWrapper *colObj1, int, int)
    {
        result_.insert((btCollisionObjectWrapper*)(colObj0));
        result_.insert((btCollisionObjectWrapper*)(colObj1));
        return 0.0f;
    }
    
    std::set<btCollisionObjectWrapper*>& result_;
};

} // ~unnamed namespace

namespace Physics
{

void TickCallback(btDynamicsWorld *world, btScalar timeStep)
{
    static_cast<Physics::PhysicsWorld*>(world->getWorldUserInfo())->ProcessPostTick(timeStep);
}

PhysicsWorld::PhysicsWorld(const ScenePtr &scene, bool isClient) :
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
    drawDebugManuallySet_(false),
    useVariableTimestep_(false),
    debugDrawMode_(0),
    cachedOgreWorld_(0)
{
#include "DisableMemoryLeakCheck.h"
    collisionConfiguration_ = new btDefaultCollisionConfiguration();
    collisionDispatcher_ = new btCollisionDispatcher(collisionConfiguration_);
    broadphase_ = new btDbvtBroadphase();
    solver_ = new btSequentialImpulseConstraintSolver();
    world_ = new btDiscreteDynamicsWorld(collisionDispatcher_, broadphase_, solver_, collisionConfiguration_);
    world_->setDebugDrawer(this);
    world_->setInternalTickCallback(TickCallback, (void*)this, false);
#include "EnableMemoryLeakCheck.h"

    if (scene->GetFramework()->HasCommandLineParameter("--variablephysicsstep"))
        useVariableTimestep_ = true;
}

PhysicsWorld::~PhysicsWorld()
{
    SAFE_DELETE(world_);
    SAFE_DELETE(solver_);
    SAFE_DELETE(broadphase_);
    SAFE_DELETE(collisionDispatcher_);
    SAFE_DELETE(collisionConfiguration_);
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

float3 PhysicsWorld::Gravity() const
{
    return world_->getGravity();
}

btDiscreteDynamicsWorld* PhysicsWorld::BulletWorld() const
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
        
        // Use variable timestep if enabled, and if frame timestep exceeds the single physics simulation substep
        if (useVariableTimestep_ && frametime > physicsUpdatePeriod_)
        {
            float clampedTimeStep = (float)frametime;
            if (clampedTimeStep > 0.1f)
                clampedTimeStep = 0.1f; // Advance max. 1/10 sec. during one frame
            world_->stepSimulation(clampedTimeStep, 0, clampedTimeStep);
        }
        else
            world_->stepSimulation((float)frametime, maxSubSteps_, physicsUpdatePeriod_);
    }
    
    // Automatically enable debug geometry if at least one debug-enabled rigidbody. Automatically disable if no debug-enabled rigidbodies
    // However, do not do this if user has used the physicsdebug console command
    if (!drawDebugManuallySet_)
    {
        if (!IsDebugGeometryEnabled() && !debugRigidBodies_.empty())
            SetDebugGeometryEnabled(true);
        if (IsDebugGeometryEnabled() && debugRigidBodies_.empty())
            SetDebugGeometryEnabled(false);
    }
    
    if (IsDebugGeometryEnabled())
        DrawDebugGeometry();
}

void PhysicsWorld::ProcessPostTick(float substeptime)
{
    PROFILE(PhysicsWorld_ProcessPostTick);
    // Check contacts and send collision signals for them
    int numManifolds = collisionDispatcher_->getNumManifolds();
    
    std::set<std::pair<const btCollisionObject*, const btCollisionObject*> > currentCollisions;
    
    // Collect all collision signals to a list before emitting any of them, in case a collision
    // handler changes physics state before the loop below is over (which would lead into catastrophic
    // consequences)
    std::vector<CollisionSignal> collisions;
    collisions.reserve(numManifolds * 3); // Guess some initial memory size for the collision list.

    if (numManifolds > 0)
    {
        PROFILE(PhysicsWorld_SendCollisions);
        
        for(int i = 0; i < numManifolds; ++i)
        {
            btPersistentManifold* contactManifold = collisionDispatcher_->getManifoldByIndexInternal(i);
            int numContacts = contactManifold->getNumContacts();
            if (numContacts == 0)
                continue;

            const btCollisionObject* objectA = contactManifold->getBody0();
            const btCollisionObject* objectB = contactManifold->getBody1();
			
            std::pair<const btCollisionObject*, const btCollisionObject*> objectPair;
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
                
                CollisionSignal s;
                s.bodyA = bodyA;
                s.bodyB = bodyB;
                s.position = point.m_positionWorldOnB;
                s.normal = point.m_normalWorldOnB;
                s.distance = point.m_distance1;
                s.impulse = point.m_appliedImpulse;
                s.newCollision = newCollision;
                collisions.push_back(s);
                
                // Report newCollision = true only for the first contact, in case there are several contacts, and application does some logic depending on it
                // (for example play a sound -> avoid multiple sounds being played)
                newCollision = false;
            }
            
            currentCollisions.insert(objectPair);
        }
    }

    // Now fire all collision signals.
    {
        PROFILE(PhysicsWorld_emit_PhysicsCollisions);
        for(size_t i = 0; i < collisions.size(); ++i)
        {
            emit PhysicsCollision(collisions[i].bodyA->ParentEntity(), collisions[i].bodyB->ParentEntity(), collisions[i].position, collisions[i].normal, collisions[i].distance, collisions[i].impulse, collisions[i].newCollision);
            collisions[i].bodyA->EmitPhysicsCollision(collisions[i].bodyB->ParentEntity(), collisions[i].position, collisions[i].normal, collisions[i].distance, collisions[i].impulse, collisions[i].newCollision);
            collisions[i].bodyB->EmitPhysicsCollision(collisions[i].bodyA->ParentEntity(), collisions[i].position, collisions[i].normal, collisions[i].distance, collisions[i].impulse, collisions[i].newCollision);
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

EntityList PhysicsWorld::ObbCollisionQuery(const OBB &obb, int collisionGroup, int collisionMask)
{
    PROFILE(PhysicsWorld_ObbCollisionQuery);
    
    std::set<btCollisionObjectWrapper*> objects;
    EntityList entities;
    
    btBoxShape box(obb.HalfSize()); // Note: Bullet uses box halfsize
    float3x3 m(obb.axis[0], obb.axis[1], obb.axis[2]);
    btTransform t1(m.ToQuat(), obb.CenterPoint());
#include "DisableMemoryLeakCheck.h"
    btRigidBody* tempRigidBody = new btRigidBody(1.0f, 0, &box);
#include "EnableMemoryLeakCheck.h"
    tempRigidBody->setWorldTransform(t1);
    world_->addRigidBody(tempRigidBody, collisionGroup, collisionMask);
    tempRigidBody->activate(); // To make sure we get collision results from static sleeping rigidbodies, activate the temp rigid body
    
    ObbCallback resultCallback(objects);
    world_->contactTest(tempRigidBody, resultCallback);
    
    for (std::set<btCollisionObjectWrapper*>::iterator i = objects.begin(); i != objects.end(); ++i)
    {
        EC_RigidBody* body = static_cast<EC_RigidBody*>((*i)->getCollisionObject()->getUserPointer());
        if (body && body->ParentEntity())
            entities.push_back(body->ParentEntity()->shared_from_this());
    }
    
    world_->removeRigidBody(tempRigidBody);
    delete tempRigidBody;
    
    return entities;
}

void PhysicsWorld::SetDebugGeometryEnabled(bool enable)
{
    if (scene_.expired() || !scene_.lock()->ViewEnabled() || IsDebugGeometryEnabled() == enable)
        return;

    /// @todo Make possisble to set other debug modes too.
    setDebugMode(enable ? btIDebugDraw::DBG_DrawWireframe : btIDebugDraw::DBG_NoDebug);
}

bool PhysicsWorld::IsDebugGeometryEnabled() const
{
    return getDebugMode() != btIDebugDraw::DBG_NoDebug;
}

void PhysicsWorld::DrawDebugGeometry()
{
    if (!IsDebugGeometryEnabled())
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
    if (IsDebugGeometryEnabled() && cachedOgreWorld_)
        cachedOgreWorld_->DebugDrawLine(from, to, color.x(), color.y(), color.z());
}

} // ~Physics
