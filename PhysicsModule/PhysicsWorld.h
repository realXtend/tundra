// For conditions of distribution and use, see copyright notice in license.txt

#ifndef incl_Physics_PhysicsWorld_h
#define incl_Physics_PhysicsWorld_h

#include "Core.h"
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
    
    //! Step the physics world. May trigger several internal simulation substeps, according to the deltatime given.
    void Simulate(f64 frametime);
    
public slots:
    //! Set internal physics timestep (= length of each simulation step.) By default 1/60th of a second.
    void SetPhysicsUpdatePeriod(float updatePeriod);
    
    //! Return internal physics timestep
    float GetPhysicsUpdatePeriod() const { return physicsUpdatePeriod_; }
    
    //! Set gravity
    /*! Note: Bullet may behave oddly if gravity is changed after objects have already been created.
     */
    void SetGravity(const Vector3df& gravity);
    
    //! Return gravity
    Vector3df GetGravity() const;
    
    //! Return the Bullet world object
    btDynamicsWorld* GetWorld() const;
    
private:
    //! Bullet collision config
    btCollisionConfiguration* collisionConfiguration_;
    //! Bullet collision dispatcher
    btDispatcher* collisionDispatcher_;
    //! Bullet collision broadphase
    btBroadphaseInterface* broadphase_;
    //! Bullet constraint equation solver
    btConstraintSolver* solver_;
    //! Bullet physics world
    btDiscreteDynamicsWorld* world_;
    
    //! Length of internal physics timestep
    float physicsUpdatePeriod_;
};

}

#endif

