// For conditions of distribution and use, see copyright notice in license.txt

#ifndef incl_Physics_PhysicsWorld_h
#define incl_Physics_PhysicsWorld_h

#include "PhysicsModuleApi.h"

#include <QObject>

class btCollisionConfiguration;
class btBroadphaseInterface;
class btConstraintSolver;
class btDiscreteDynamicsWorld;
class btDispatcher;
class btDynamicsWorld;

namespace Physics
{

class PHYSICS_MODULE_API PhysicsWorld : public QObject
{
    Q_OBJECT
    
public:
    PhysicsWorld();
    virtual ~PhysicsWorld();
    
    void Simulate(float deltaTime, int maxSteps, float internalTimeStep);
    
    btCollisionConfiguration* collisionConfiguration_;
    btDispatcher* collisionDispatcher_;
    btBroadphaseInterface* broadphase_;
    btConstraintSolver* solver_;
    btDiscreteDynamicsWorld* world_;
};

}

#endif

