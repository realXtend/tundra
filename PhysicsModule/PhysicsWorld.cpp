// For conditions of distribution and use, see copyright notice in license.txt

#include "StableHeaders.h"
#include "PhysicsWorld.h"
#include "PhysicsUtils.h"
#include "Profiler.h"

#include "btBulletDynamicsCommon.h"

namespace Physics
{

// Assume we generate at least 10 frames per second. If less, the physics will start to slow down.
static const float cMinFps = 10.0f;

PhysicsWorld::PhysicsWorld() :
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

}

