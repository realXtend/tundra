// For conditions of distribution and use, see copyright notice in LICENSE

#pragma once

#include "CoreDefines.h"
#include "IComponent.h"
#include "AssetReference.h"
#include "AssetFwd.h"
#include "Geometry/AABB.h"
#include "PhysicsModuleApi.h"
#include "PhysicsModuleFwd.h"

#include <LinearMath/btMotionState.h>

class EC_Placeable;
class EC_Terrain;

/// Physics rigid body entity-component
/** <table class="header">
    <tr>
    <td>
    <h2>RigidBody</h2>
    Physics rigid body entity-component

    Registered by Physics::PhysicsModule.

    <b>Attributes</b>:
    <ul>
    <li>float: mass
    <div>@copydoc mass</div>
    <li>enum: shapeType
    <div>@copydoc shapeType</div>
    <li>float3: size
    <div>@copydoc size</div>
    <li>AssetReference: collisionMeshRef
    <div>@copydoc @copydoc collisionMeshRef</div>
    <li>float: friction
    <div>@copydoc friction</div>
    <li>float: restitution
    <div>@copydoc restitution</div>
    <li>float: linearDamping
    <div>@copydoc linearDamping</div>
    <li>float: angularDamping
    <div>@copydoc angularDamping</div>
    <li>float3: linearFactor
    <div>@copydoc linearFactor</div>
    <li>float3: angularFactor
    <div>@copydoc angularFactor</div>
    <li>bool: kinematic
    <div>@copydoc kinematic</div>
    <li>bool: phantom
    <div>@copydoc phantom</div>
    <li>bool: drawDebug
    <div>@copydoc drawDebug</div>
    <li>float3: linearVelocity
    <div>@copydoc linearVelocity</div>
    <li>float3: angularVelocity
    <div>@copydoc angularVelocity</div>
    <li>int: collisionLayer
    <div>@copydoc collisionLayer</div>
    <li>int: collisionMask
    <div>@copydoc collisionMask</div>
    </ul>

    <b>Exposes the following scriptable functions:</b>
    <ul>
    <li> "SetShapeFromVisibleMesh": @copydoc SetShapeFromVisibleMesh
    <li> "SetLinearVelocity": @copydoc SetLinearVelocity
    <li> "SetAngularVelocity": @copydoc SetAngularVelocity
    <li> "ApplyForce": @copydoc ApplyForce
    <li> "ApplyTorque": @copydoc ApplyTorque
    <li> "ApplyImpulse": @copydoc ApplyImpulse
    <li> "ApplyTorqueImpulse": @copydoc ApplyTorqueImpulse
    <li> "Activate": @copydoc Activate
    <li> "IsActive": @copydoc IsActive
    <li> "KeepActive": @copydoc KeepActive
    <li> "ResetForces": @copydoc ResetForces
    <li> "SetRotation": @copydoc SetRotation
    <li> "Rotate": @copydoc Rotate
    <li> "GetLinearVelocity": @copydoc GetLinearVelocity
    <li> "GetAngularVelocity": @copydoc GetAngularVelocity
    <li> "HasAuthority": @copydoc HasAuthority
    <li> "ShapeAABB": @copydoc ShapeAABB
    </ul>

    <b>Reacts on the following actions:</b>
    <ul>
    <li>None.
    </ul>
    </td>
    </tr>

    Does not emit any actions.

    <b>Depends on the component @ref EC_Placeable "Placeable", and optionally on @ref EC_Mesh "Mesh" 
    and @ref EC_Terrain "Terrain" to copy the collision shape from them</b>.
    </table> */
class PHYSICS_MODULE_API EC_RigidBody : public IComponent, public btMotionState
{
    Q_OBJECT
    COMPONENT_NAME("EC_RigidBody", 23)
    Q_ENUMS(ShapeType)

    friend class Physics::PhysicsWorld;

public:
    /// Do not directly allocate new components using operator new, but use the factory-based SceneAPI::CreateComponent functions instead.
    explicit EC_RigidBody(Scene* scene);
    virtual ~EC_RigidBody();

    enum ShapeType
    {
        Shape_Box = 0, ///< Box
        Shape_Sphere, ///< Sphere
        Shape_Cylinder, ///< Cylinder
        Shape_Capsule, ///< Capsule
        Shape_TriMesh, ///< Triangle mesh
        Shape_HeightField, ///< Heightfield
        Shape_ConvexHull ///< Convex hull
    };

    /// Mass of the body. Set to 0 to have a static (immovable) object
    Q_PROPERTY(float mass READ getmass WRITE setmass);
    DEFINE_QPROPERTY_ATTRIBUTE(float, mass);

    /// Shape type, see ShapeType.
    Q_PROPERTY(int shapeType READ getshapeType WRITE setshapeType)
    DEFINE_QPROPERTY_ATTRIBUTE(int, shapeType);

    /// Size (scaling) of the shape.
    /** Sphere only uses x-axis, and capsule uses only x & z axes. Shape is further scaled by Placeable scale.*/
    Q_PROPERTY(float3 size READ getsize WRITE setsize)
    DEFINE_QPROPERTY_ATTRIBUTE(float3, size);

    /// Collision mesh asset reference, only effective if shapeType is Shape_TriMesh.
    Q_PROPERTY(AssetReference collisionMeshRef READ getcollisionMeshRef WRITE setcollisionMeshRef);
    DEFINE_QPROPERTY_ATTRIBUTE(AssetReference, collisionMeshRef);

    /// Friction coefficient between 0.0 - 1.0.
    Q_PROPERTY(float friction READ getfriction WRITE setfriction);
    DEFINE_QPROPERTY_ATTRIBUTE(float, friction);

    /// Restitution coefficient between 0.0 - 1.0.
    Q_PROPERTY(float restitution READ getrestitution WRITE setrestitution);
    DEFINE_QPROPERTY_ATTRIBUTE(float, restitution);

    /// Linear damping coefficient of the object (makes it lose velocity even when no force acts on it).
    Q_PROPERTY(float linearDamping READ getlinearDamping WRITE setlinearDamping);
    DEFINE_QPROPERTY_ATTRIBUTE(float, linearDamping);

    /// Angular damping coefficient of the object (makes it lose angular velocity even when no force acts on it)
    Q_PROPERTY(float angularDamping READ getangularDamping WRITE setangularDamping);
    DEFINE_QPROPERTY_ATTRIBUTE(float, angularDamping);

    /// Linear factor. Specifies the axes on which forces can act on the object, making it move.
    Q_PROPERTY(float3 linearFactor READ getlinearFactor WRITE setlinearFactor)
    DEFINE_QPROPERTY_ATTRIBUTE(float3, linearFactor);

    /// Angular factor. Defines in which dimensions the object can rotate
    Q_PROPERTY(float3 angularFactor READ getangularFactor WRITE setangularFactor)
    DEFINE_QPROPERTY_ATTRIBUTE(float3, angularFactor);

    /// Kinematic flag. If true, forces don't affect the object, but it may push other objects around.
    Q_PROPERTY(bool kinematic READ getkinematic WRITE setkinematic)
    DEFINE_QPROPERTY_ATTRIBUTE(bool, kinematic)

    /// Phantom flag. If true, contact response is disabled, ie. there is no collision interaction between this object and others
    Q_PROPERTY(bool phantom READ getphantom WRITE setphantom)
    DEFINE_QPROPERTY_ATTRIBUTE(bool, phantom)

    /// DrawDebug flag. If true, collision shape will be visualized when physics debug drawing is enabled.
    Q_PROPERTY(bool drawDebug READ getdrawDebug WRITE setdrawDebug)
    DEFINE_QPROPERTY_ATTRIBUTE(bool, drawDebug)

    /// Linear velocity
    Q_PROPERTY(float3 linearVelocity READ getlinearVelocity WRITE setlinearVelocity)
    DEFINE_QPROPERTY_ATTRIBUTE(float3, linearVelocity)

    /// Specifies the axes on which torques can act on the object, making it rotate.
    /** Set to 0,0,0 to make for example an avatar capsule that does not tip over by itself. */
    Q_PROPERTY(float3 angularVelocity READ getangularVelocity WRITE setangularVelocity)
    DEFINE_QPROPERTY_ATTRIBUTE(float3, angularVelocity)

    /// The collision layer bitmask of this rigidbody. Several bits can be set. 0 is default (all bits set)
    Q_PROPERTY(int collisionLayer READ getcollisionLayer WRITE setcollisionLayer)
    DEFINE_QPROPERTY_ATTRIBUTE(int, collisionLayer)

    /// Tells with which collision layers this rigidbody collides with (a bitmask). 0 is default (all bits set)
    Q_PROPERTY(int collisionMask READ getcollisionMask WRITE setcollisionMask)
    DEFINE_QPROPERTY_ATTRIBUTE(int, collisionMask)

    /// btMotionState override. Called when Bullet wants us to tell the body's initial transform
    virtual void getWorldTransform(btTransform &worldTrans) const;

    /// btMotionState override. Called when Bullet wants to tell us the body's current transform
    virtual void setWorldTransform(const btTransform &worldTrans);

    void SetClientExtrapolating(bool isClientExtrapolating);

    btRigidBody* GetRigidBody() const { return body_; }

    /// Constructs axis-aligned bounding box from bullet collision shape
    /** @param outMin The minimum corner of the box
        @param outMax The maximum corner of the box */
    void GetAabbox(float3 &outAabbMin, float3 &outAabbMax);

signals:
    /// A physics collision has happened between this rigid body and another entity
    /** If there are several contact points, the signal will be sent multiple times for each contact.
        @param otherEntity The second entity
        @param position World position of collision
        @param normal World normal of collision
        @param distance Contact distance
        @param impulse Impulse applied to the objects to separate them
        @param newCollision True if same collision did not happen on the previous frame.
        If collision has multiple contact points, newCollision can only be true for the first of them. */
    void PhysicsCollision(Entity* otherEntity, const float3& position, const float3& normal, float distance, float impulse, bool newCollision);

public slots:
    /// Set collision mesh from visible mesh. Also sets mass 0 (static) because trimeshes cannot move in Bullet
    /** @return true if successful (EC_Mesh could be found and contained a mesh reference) */
    bool SetShapeFromVisibleMesh();

    /// Set linear velocity and activate the body
    /** Note: sets also the attribute, signals a Default attribute change
        @param velocity New linear velocity
        @todo Remove and use linearVelocity attribute? */
    void SetLinearVelocity(const float3& velocity);
    /// Return linear velocity. Should be same as accessing the attribute.
    /** @todo Remove and use linearVelocity attribute? */
    float3 GetLinearVelocity();

    /// Set angular velocity and activate the body
    /** Note: sets also the attribute, signals a Default attribute change
        @param angularVelocity New angular velocity, specified in degrees / sec
        @todo Remove and use angularVelocity attribute? */
    void SetAngularVelocity(const float3& angularVelocity);
    /// Return angular velocity. Should be same as accessing the attribute.
    /** @todo Remove and use angularVelocity attribute? */
    float3 GetAngularVelocity();

    /// Apply a force to the body
    /** @param force Force
        @param position Object space position, by default center */
    void ApplyForce(const float3& force, const float3& position = float3::zero);
    
    /// Apply a torque to the body
    /** @param torque Torque */
    void ApplyTorque(const float3& torque);
    
    /// Apply an impulse to the body
    /** @param impulse Impulse
        @param position Object space position, by default center */
    void ApplyImpulse(const float3& impulse, const float3& position = float3::zero);
    
    /// Apply a torque impulse to the body
    /** @param torqueImpulse Torque impulse */
    void ApplyTorqueImpulse(const float3& torqueImpulse);
    
    /// Force the body to activate (wake up)
    void Activate();
    
    /// Keep the body awake. Used by VolumeTrigger to avoid bugs.
    void KeepActive();
    
    /// Check whether body is active
    bool IsActive();
    
    /// Reset accumulated force & torque
    void ResetForces();
    
    /// Forcibly set rotation
    /** Use this instead of just setting the placeable's full transform to allow linear motion
        to continue uninterrupted (with proper inter-step interpolation)
        @param rotation New rotation (eulers) */
    void SetRotation(const float3& rotation);
    
    /// Rotate the body
    /** Use this instead of just setting the placeable's full transform to allow linear motion
        to continue uninterrupted (with proper inter-step interpolation)
        @param rotation Delta rotation (eulers) */
    void Rotate(const float3& rotation);

    /// Return physics world
    Physics::PhysicsWorld* GetPhysicsWorld() const { return world_; }

    /// Return whether have authority. On the client, returns false for non-local objects.
    bool HasAuthority() const;

    /// Returns the minimal axis-aligned bounding box that encloses the collision shape of this rigid body.
    /// Note that this function may be called even if the shape of this rigid body is not AABB.
    AABB ShapeAABB() const;

private slots:
    /// Called when the parent entity has been set.
    void UpdateSignals();
    
    /// Called when the simulation is about to be stepped
    void OnAboutToUpdate();
    
    /// Called when attributes of the placeable have changed
    void PlaceableUpdated(IAttribute *attribute);
    
    /// Called when attributes of the terrain have changed
    void TerrainUpdated(IAttribute *attribute);
    
    /// Check for placeable & terrain components and connect to their signals
    void CheckForPlaceableAndTerrain();
    
    /// Called when EC_Terrain has been regenerated
    void OnTerrainRegenerated();

    /// Called when collision mesh has been downloaded.
    void OnCollisionMeshAssetLoaded(AssetPtr asset);

private:
    /// Called when some of the attributes has been changed.
    void AttributesChanged();

    /// (Re)create the collisionshape
    void CreateCollisionShape();
    
    /// Remove the collisionshape
    void RemoveCollisionShape();
    
    /// Create a heightfield collisionshape from EC_Terrain
    void CreateHeightFieldFromTerrain();
    
    /// Create a convex hull set collisionshape
    void CreateConvexHullSetShape();
    
    /// Create the body. No-op if the scene is not associated with a physics world.
    void CreateBody();
    
    /// Destroy the body
    void RemoveBody();
    
    /// Re-add the body to the physics world because of its some properties changing
    /** (also recalculates those properties as necessary, and sets collisionshape for the body if it has changed) */
    void ReaddBody();
    
    /// Update scale from placeable & own size setting
    void UpdateScale();
    
    /// Update position & rotation from placeable
    void UpdatePosRotFromPlaceable();
    
    /// Request mesh resource (for trimesh & convexhull shapes)
    void RequestMesh();
    
    /// Calculate mass, shape & static/dynamic-classification dependant properties
    void GetProperties(btVector3& localInertia, float& m, int& collisionFlags);
    
    /// Emit a physics collision. Called from PhysicsWorld
    void EmitPhysicsCollision(Entity* otherEntity, const float3& position, const float3& normal, float distance, float impulse, bool newCollision);
    
    /// Placeable pointer
    weak_ptr<EC_Placeable> placeable_;
    
    /// Terrain pointer
    weak_ptr<EC_Terrain> terrain_;
    
    /// Internal disconnection of attribute changes. True during the time we're setting attributes ourselves due to Bullet update, to prevent endless loop
    bool disconnected_;
    
    /// On the client side, this field is used to track whether the rigid body is being interpolated from network input events (false), or extrapolated
    /// using local physics computations (true).
    /// On the server side, this flag is not used.
    bool clientExtrapolating;

    /// Bullet body
    btRigidBody* body_;
    
    /// Bullet collision shape
    btCollisionShape* shape_;
    /// Bullet collision child shape. This is needed to use btScaledBvhTriangleMeshShape
    btCollisionShape* childShape_;
    
    /// Physics world. May be 0 if the scene does not have a physics world. In that case most of EC_RigidBody's functionality is a no-op
    Physics::PhysicsWorld* world_;
    
    /// PhysicsModule pointer
    Physics::PhysicsModule* owner_;
    
    /// Cached shapetype (last created)
    int cachedShapeType_;

    /// Cached shapesize (last created)
    float3 cachedSize_;

    /// Bullet triangle mesh
    shared_ptr<btTriangleMesh> triangleMesh_;
    
    /// Convex hull set
    shared_ptr<Physics::ConvexHullSet> convexHullSet_;
    
    /// Bullet heightfield shape. Note: this is always put inside a compound shape (shape_)
    btHeightfieldTerrainShape* heightField_;
    
    /// Heightfield values, for the case the shape is a heightfield.
    std::vector<float> heightValues_;
};
