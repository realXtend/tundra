// For conditions of distribution and use, see copyright notice in LICENSE

#pragma once

#include "CoreTypes.h"
#include "SceneFwd.h"
#include "PhysicsModuleApi.h"
#include "PhysicsModuleFwd.h"
#include "Math/float3.h"
#include "Math/MathFwd.h"

#include <LinearMath/btIDebugDraw.h>

#include <set>
#include <QObject>

class OgreWorld;

/// Result of a raycast to the physical representation of a scene.
/** Other fields are valid only if entity is non-null
    @sa Physics::PhysicsWorld
    @todo Remove the QObject inheritance here, and expose as a struct to scripts. */
class PhysicsRaycastResult : public QObject
{
    Q_OBJECT
    Q_PROPERTY(Entity* entity READ getentity);
    Q_PROPERTY(float3 pos READ getpos);
    Q_PROPERTY(float3 normal READ getnormal);
    Q_PROPERTY(float distance READ getdistance);

    Entity* getentity() const { return entity; }
    float3 getpos() const { return pos; }
    float3 getnormal() const { return normal; }
    float getdistance() const { return distance; }

public:
    Entity* entity; ///< Entity that was hit, null if none
    float3 pos; ///< World coordinates of hit position
    float3 normal; ///< World face normal of hit.
    float distance; ///< Distance from ray origin to the hit point.
};

namespace Physics
{
/// A physics world that encapsulates a Bullet physics world
class PHYSICS_MODULE_API PhysicsWorld : public QObject, public btIDebugDraw, public enable_shared_from_this<PhysicsWorld>
{
    Q_OBJECT
    Q_PROPERTY(float updatePeriod READ PhysicsUpdatePeriod WRITE SetPhysicsUpdatePeriod)
    Q_PROPERTY(int maxSubSteps READ MaxSubSteps WRITE SetMaxSubSteps)
    Q_PROPERTY(float3 gravity READ Gravity WRITE SetGravity)
    Q_PROPERTY(bool drawDebugGeometry READ IsDebugGeometryEnabled WRITE SetDebugGeometryEnabled)
    Q_PROPERTY(bool running READ IsRunning WRITE SetRunning)

    friend class PhysicsModule;
    friend class ::EC_RigidBody;

public:
    /// Constructor.
    /** @param scene Scene of which this PhysicsWorld is physical representation of.
        @param isClient Whether this physics world is for a client scene i.e. only simulates local entities' motion on their own.*/
    PhysicsWorld(const ScenePtr &scene, bool isClient);
    virtual ~PhysicsWorld();
    
    /// Step the physics world. May trigger several internal simulation substeps, according to the deltatime given.
    void Simulate(f64 frametime);
    
    /// Process collision from an internal sub-step (Bullet post-tick callback)
    void ProcessPostTick(float subStepTime);
    
    /// Dynamic scene property name
    static const char* PropertyName() { return "physics"; }
    
    /// IDebugDraw override
    virtual void drawLine(const btVector3& from, const btVector3& to, const btVector3& color);
    
    /// IDebugDraw override
    virtual void reportErrorWarning(const char* warningString);
    
    /// IDebugDraw override, does nothing.
    virtual void drawContactPoint(const btVector3& /*pointOnB*/, const btVector3& /*normalOnB*/, btScalar /*distance*/, int /*lifeTime*/, const btVector3& /*color*/) {}
    
    /// IDebugDraw override, does nothing.
    virtual void draw3dText(const btVector3& /*location*/, const char* /*textString*/) {}
    
    /// IDebugDraw override
    virtual void setDebugMode(int debugMode) { debugDrawMode_ = debugMode; }
    
    /// IDebugDraw override
    virtual int getDebugMode() const { return debugDrawMode_; }
    
    /// Returns the set of collisions that occurred during the previous frame.
    /// \important Use this function only for debugging, the availability of this set data structure is not guaranteed in the future.
    const std::set<std::pair<btCollisionObject*, btCollisionObject*> > &PreviousFrameCollisions() const { return previousCollisions_; }

    /// Set physics update period (= length of each simulation step.) By default 1/60th of a second.
    /** @param updatePeriod Update period */
    void SetPhysicsUpdatePeriod(float updatePeriod);

    /// Return internal physics timestep
    float PhysicsUpdatePeriod() const { return physicsUpdatePeriod_; }

    /// Set maximum physics substeps to perform on a single frame. Once this maximum is reached, time will appear to slow down if framerate is too low.
    /** @param steps Maximum physics substeps */
    void SetMaxSubSteps(int steps);

    /// Return amount of maximum physics substeps on a single frame.
    int MaxSubSteps() const { return maxSubSteps_; }

    /// Set gravity that affects all moving objects of the physics world
    /** @param gravity Gravity vector */
    void SetGravity(const float3& gravity);

    /// Return gravity
    float3 Gravity() const;

    /// Enable/disable debug geometry
    void SetDebugGeometryEnabled(bool enable);
    
    /// Get debug geometry enabled status
    bool IsDebugGeometryEnabled() const;
    
    /// Enable/disable physics simulation
    void SetRunning(bool enable) { runPhysics_ = enable; }
    
    /// Return whether simulation is on
    bool IsRunning() const { return runPhysics_; }

    /// Return the Bullet world object
    btDiscreteDynamicsWorld* BulletWorld() const;

public slots:
    /// Return whether the physics world is for a client scene. Client scenes only simulate local entities' motion on their own.
    bool IsClient() const { return isClient_; }

    /// Raycast to the world. Returns only a single (the closest) result.
    /** @param origin World origin position
        @param direction Direction to raycast to. Will be normalized automatically
        @param maxDistance Length of ray
        @param collisionGroup Collision layer. Default has all bits set.
        @param collisionMask Collision mask. Default has all bits set.
        @return result PhysicsRaycastResult structure */
    PhysicsRaycastResult* Raycast(const float3& origin, const float3& direction, float maxDistance, int collisionGroup = -1, int collisionMask = -1);

    /// Performs collision query for OBB.
    /** @param obb Oriented bounding box to test
        @param collisionGroup Collision layer of the OBB. Default has all bits set.
        @param collisionMask Collision mask of the OBB. Default has all bits set.
        @return List of entities with EC_RigidBody component intersecting the OBB */
    EntityList ObbCollisionQuery(const OBB &obb, int collisionGroup = -1, int collisionMask = -1);

signals:
    /// A physics collision has happened between two entities. 
    /** Note: both rigidbodies participating in the collision will also emit a signal separately. 
        Also, if there are several contact points, the signal will be sent multiple times for each contact.
        @param entityA The first entity
        @param entityB The second entity
        @param position World position of collision
        @param normal World normal of collision
        @param distance Contact distance
        @param impulse Impulse applied to the objects to separate them
        @param newCollision True if same collision did not happen on the previous frame.
                If collision has multiple contact points, newCollision can only be true for the first of them. */
    void PhysicsCollision(Entity* entityA, Entity* entityB, const float3& position, const float3& normal, float distance, float impulse, bool newCollision);
    
    /// Emitted before the simulation steps. Note: emitted only once per frame, not before each substep.
    /** @param frametime Length of simulation steps */
    void AboutToUpdate(float frametime);
    
    /// Emitted after each simulation step
    /** @param frametime Length of simulation step */
    void Updated(float frametime);
    
private:
    /// Bullet collision config
    btCollisionConfiguration* collisionConfiguration_;
    /// Bullet collision dispatcher
    btDispatcher* collisionDispatcher_;
    /// Bullet collision broadphase
    btBroadphaseInterface* broadphase_;
    /// Bullet constraint equation solver
    btConstraintSolver* solver_;
    /// Bullet physics world
    btDiscreteDynamicsWorld* world_;
    
    /// Length of one physics simulation step
    float physicsUpdatePeriod_;
    /// Maximum amount of physics simulation substeps to run on a frame
    int maxSubSteps_;
    
    /// Client scene flag
    bool isClient_;
    
    /// Parent scene
    SceneWeakPtr scene_;
    
    /// Previous frame's collisions. We store these to know whether the collision was new or "ongoing"
    std::set<std::pair<btCollisionObject*, btCollisionObject*> > previousCollisions_;
    
    /// Draw physics debug geometry, if debug drawing enabled
    void DrawDebugGeometry();

    /// Debug geometry manually enabled/disabled (with physicsdebug console command). If true, do not automatically enable/disable debug geometry anymore
    bool drawDebugManuallySet_;
    
    /// Whether should run physics. Default true
    bool runPhysics_;
    
    /// Variable timestep flag
    bool useVariableTimestep_;
    
    /// Bullet debug draw / debug behaviour flags
    int debugDrawMode_;
    
    /// Cached OgreWorld pointer for drawing debug geometry
    OgreWorld* cachedOgreWorld_;
    
    /// Debug draw-enabled rigidbodies. Note: these pointers are never dereferenced, it is just used for counting
    std::set<EC_RigidBody*> debugRigidBodies_;
};

}
