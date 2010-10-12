// For conditions of distribution and use, see copyright notice in license.txt

#include "StableHeaders.h"
#include "PhysicsWorld.h"

#include "btBulletDynamicsCommon.h"

namespace Physics
{

PhysicsWorld::PhysicsWorld() :
    collisionConfiguration_(0),
    collisionDispatcher_(0),
    broadphase_(0),
    solver_(0),
    world_(0)
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

void PhysicsWorld::Simulate(float deltaTime, int maxSubSteps, float internalTimeStep)
{
    world_->stepSimulation((float)deltaTime, maxSubSteps, internalTimeStep);
}

}

