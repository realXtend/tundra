// For conditions of distribution and use, see copyright notice in license.txt

#ifndef incl_Physics_EC_RigidBody_h
#define incl_Physics_EC_RigidBody_h

#include "Core.h"
#include "IComponent.h"
#include "LinearMath/btMotionState.h"
#include "AssetReference.h"
#include "Declare_EC.h"
#include "AssetFwd.h"

#include "PhysicsModuleApi.h"

#include <QVector>

class btRigidBody;
class btCollisionShape;
class btTriangleMesh;
class btHeightfieldTerrainShape;
class EC_Placeable;

namespace Environment
{
    class EC_Terrain;
}

namespace Physics
{
    class PhysicsModule;
    class PhysicsWorld;
    struct ConvexHullSet;
}

//! Physics rigid body entity component
/**
<table class="header">
<tr>
<td>
<h2>RigidBody</h2>
Physics rigid body entity component

Registered by Physics::PhysicsModule.

<b>Attributes</b>:
<ul>
<li>float: mass
<div>Mass of the body. Set to 0 to have a static (immovable) object</div>
<li>int: shapeType
<div>Shape type. Can be box, sphere, cylinder, capsule, trimesh, or heightfield (not yet supported)</div>
<li>Vector3df: size
<div>Size (scaling) of the shape. Sphere only uses x-axis, and capsule uses only x & z axes. Shape is further scaled by Placeable scale.</div>
<li>QString: collisionMeshRef
<div>Asset ref of the collision mesh. Only effective if shapetype is TriMesh.</div>
<li>float: friction
<div>Friction coefficient between 0.0 - 1.0.</div>
<li>float: restitution
<div>Restitution coefficient between 0.0 - 1.0.</div>
<li>float: linearDamping
<div>Linear damping coefficient of the object (makes it lose velocity even when no force acts on it)</div>
<li>float: angularDamping
<div>Angular damping coefficient of the object (makes it lose angular velocity even when no force acts on it)</div>
<li>Vector3df: linearFactor
<div>Specifies the axes on which forces can act on the object, making it move</div>
<li>Vector3df: angularFactor
<div>Specifies the axes on which torques can act on the object, making it rotate.
Set to 0,0,0 to make for example an avatar capsule that does not tip over by itself.</div>
<li>bool: phantom
<div>If true, contact response is disabled, ie. there is no collision interaction between this object and others.</div>
<li>bool: drawDebug
<div>If true (default), collision shape will be visualized when physics debug drawing is enabled.</div>
</ul>

<b>Exposes the following scriptable functions:</b>
<ul>
<li> "SetShapeFromVisibleMesh": set collision mesh from visible mesh. Also sets mass 0 (static) because trimeshes cannot move in Bullet
        \return true if successful (EC_Mesh could be found and contained a mesh reference)
<li> "SetLinearVelocity": set linear velocity and activate body
        Note: sets also the attribute, signals a Default attribute change
        \param velocity New linear velocity
<li> "SetAngularVelocity": Set angular velocity and activate body
        Note: sets also the attribute, signals a Default attribute change
        \param angularVelocity New angular velocity, specified in degrees / sec
<li> "ApplyForce": apply a force to the body
        \param force Force
        \param position Object space position, by default center
<li> "ApplyTorque": apply a torque to the body
        \param torque Torque
<li> "ApplyImpulse": apply an impulse to the body
        \param impulse Impulse
        \param position Object space position, by default center
<li> "ApplyTorqueImpulse": apply a torque impulse to the body
        \param torqueImpulse Impulse
<li> "Activate": force the body to activate (wake up)
<li> "IsActive": return whether body is active
<li> "ResetForces": reset accumulated force & torque
<li> "SetRotation": forces just the rotation part of transform, while letting the position interpolate undisturbed
        \param rotation New absolute rotation vector (eulers)
<li> "Rotate": modifies just the rotation part of transform, while letting the position interpolate undisturbed
        \param rotation Delta rotation (eulers)
<li> "GetLinearVelocity": Returns linear velocity. Should be same as accessing the attribute.
<li> "GetAngularVelocity": Returns angular velocity. Should be same as accessing the attribute.
</ul>

<b>Reacts on the following actions:</b>
<ul>
<li>...
</ul>
</td>
</tr>

Does not emit any actions.

<b>Depends on the component Placeable, and optionally on Mesh & Terrain to copy the collision shape from them</b>.
</table>
*/
class PHYSICS_MODULE_API EC_RigidBody : public IComponent, public btMotionState
{
    friend class Physics::PhysicsWorld;
    
    Q_OBJECT
    Q_ENUMS(ShapeType)
    
    DECLARE_EC(EC_RigidBody);
public:
    enum ShapeType
    {
        Shape_Box = 0,
        Shape_Sphere,
        Shape_Cylinder,
        Shape_Capsule,
        Shape_TriMesh,
        Shape_HeightField,
        Shape_ConvexHull
    };
    
    //! Mass of the body. Set to 0 for static
    Q_PROPERTY(float mass READ getmass WRITE setmass);
    DEFINE_QPROPERTY_ATTRIBUTE(float, mass);
    
    //! Shape type
    Q_PROPERTY(int shapeType READ getshapeType WRITE setshapeType)
    DEFINE_QPROPERTY_ATTRIBUTE(int, shapeType);
    
    //! Size (scaling) of the shape. Sphere only uses x-axis, and capsule uses only x & z axes. Shape is further scaled by Placeable scale.
    Q_PROPERTY(Vector3df size READ getsize WRITE setsize)
    DEFINE_QPROPERTY_ATTRIBUTE(Vector3df, size);

    //! Collision mesh asset reference.
    Q_PROPERTY(AssetReference collisionMeshRef READ getcollisionMeshRef WRITE setcollisionMeshRef);
    DEFINE_QPROPERTY_ATTRIBUTE(AssetReference, collisionMeshRef);

    //! Friction
    Q_PROPERTY(float friction READ getfriction WRITE setfriction);
    DEFINE_QPROPERTY_ATTRIBUTE(float, friction);
    
    //! Restitution
    Q_PROPERTY(float restitution READ getrestitution WRITE setrestitution);
    DEFINE_QPROPERTY_ATTRIBUTE(float, restitution);
    
    //! Linear damping
    Q_PROPERTY(float linearDamping READ getlinearDamping WRITE setlinearDamping);
    DEFINE_QPROPERTY_ATTRIBUTE(float, linearDamping);
    
    //! Angular damping
    Q_PROPERTY(float angularDamping READ getangularDamping WRITE setangularDamping);
    DEFINE_QPROPERTY_ATTRIBUTE(float, angularDamping);
    
    //! Linear factor. Defines in which dimensions the object can move
    Q_PROPERTY(Vector3df linearFactor READ getlinearFactor WRITE setlinearFactor)
    DEFINE_QPROPERTY_ATTRIBUTE(Vector3df, linearFactor);
    
    //! Angular factor. Defines in which dimensions the object can rotate
    Q_PROPERTY(Vector3df angularFactor READ getangularFactor WRITE setangularFactor)
    DEFINE_QPROPERTY_ATTRIBUTE(Vector3df, angularFactor);
    
    //! Phantom flag. If true, contact response is disabled, ie. there is no collision interaction between this object and others
    Q_PROPERTY(bool phantom READ getphantom WRITE setphantom)
    DEFINE_QPROPERTY_ATTRIBUTE(bool, phantom)
    
    //! DrawDebug flag. If true (default), collision shape will be visualized when physics debug drawing is enabled.
    Q_PROPERTY(bool drawDebug READ getdrawDebug WRITE setdrawDebug)
    DEFINE_QPROPERTY_ATTRIBUTE(bool, drawDebug)
    
    //! Linear velocity
    Q_PROPERTY(Vector3df linearVelocity READ getlinearVelocity WRITE setlinearVelocity)
    DEFINE_QPROPERTY_ATTRIBUTE(Vector3df, linearVelocity)
    
    //! Angular velocity
    Q_PROPERTY(Vector3df angularVelocity READ getangularVelocity WRITE setangularVelocity)
    DEFINE_QPROPERTY_ATTRIBUTE(Vector3df, angularVelocity)
    
    virtual ~EC_RigidBody();
    
    //! Set component as serializable.
    virtual bool IsSerializable() const { return true; }

    //! btMotionState override. Called when Bullet wants us to tell the body's initial transform
    virtual void getWorldTransform(btTransform &worldTrans) const;

    //! btMotionState override. Called when Bullet wants to tell us the body's current transform
    virtual void setWorldTransform(const btTransform &worldTrans);

signals:
    //! A physics collision has happened between this rigid body and another entity
    /*! If there are several contact points, the signal will be sent multiple times for each contact.
        \param otherEntity The second entity
        \param position World position of collision
        \param normal World normal of collision
        \param distance Contact distance
        \param impulse Impulse applied to the objects to separate them
        \param newCollision True if same collision did not happen on the previous frame.
        If collision has multiple contact points, newCollision can only be true for the first of them.
     */
    void PhysicsCollision(Scene::Entity* otherEntity, const Vector3df& position, const Vector3df& normal, float distance, float impulse, bool newCollision);
    
public slots:
    //! Set collision mesh from visible mesh. Also sets mass 0 (static) because trimeshes cannot move in Bullet
    /*! \return true if successful (EC_Mesh could be found and contained a mesh reference)
     */
    bool SetShapeFromVisibleMesh();

    //! Set linear velocity and activate the body
    /*! Note: sets also the attribute, signals a Default attribute change
        \param velocity New linear velocity
     */
    void SetLinearVelocity(const Vector3df& velocity);
    
    //! Set angular velocity and activate the body
    /*! Note: sets also the attribute, signals a Default attribute change
        \param angularVelocity New angular velocity, specified in degrees / sec
     */
    void SetAngularVelocity(const Vector3df& angularVelocity);
    
    //! Apply a force to the body
    /*! \param force Force
        \param position Object space position, by default center
     */
    void ApplyForce(const Vector3df& force, const Vector3df& position = Vector3df::ZERO);
    
    //! Apply a torque to the body
    /*! \param torque Torque
     */
    void ApplyTorque(const Vector3df& torque);
    
    //! Apply an impulse to the body
    /*! \param impulse Impulse
        \param position Object space position, by default center
     */
    void ApplyImpulse(const Vector3df& impulse, const Vector3df& position = Vector3df::ZERO);
    
    //! Apply a torque impulse to the body
    /*! \param torqueImpulse Torque impulse
     */
    void ApplyTorqueImpulse(const Vector3df& torqueImpulse);
    
    //! Force the body to activate (wake up)
    void Activate();
    
    /// Keep the body awake. Used by VolumeTrigger to avoid bugs.
    void KeepActive();

    //! Check whether body is active
    bool IsActive();
    
    //! Reset accumulated force & torque
    void ResetForces();
    
    //! Forcibly set rotation
    /*! Use this instead of just setting the placeable's full transform to allow linear motion
        to continue uninterrupted (with proper inter-step interpolation)
        \param rotation New rotation (eulers)
     */
    void SetRotation(const Vector3df& rotation);
    
    //! Rotate the body
    /*! Use this instead of just setting the placeable's full transform to allow linear motion
        to continue uninterrupted (with proper inter-step interpolation)
        \param rotation Delta rotation (eulers)
     */
    void Rotate(const Vector3df& rotation);
    
    //! Return linear velocity
    Vector3df GetLinearVelocity();
    
    //! Return angular velocity
    Vector3df GetAngularVelocity();
    
    //! Return physics world
    Physics::PhysicsWorld* GetPhysicsWorld() { return world_; }

    //! Constructs axis-aligned bounding box from bullet collision shape
    /*! \param outMin The minimum corner of the box
        \param outMax The maximum corner of the box
    */
    void GetAabbox(Vector3df &outAabbMin, Vector3df &outAabbMax);

    btRigidBody* GetRigidBody() const { return body_; }
    
    //! Return whether have authority. On the client, returns false for non-local objects.
    bool HasAuthority() const;
    
private slots:
    //! Called when the parent entity has been set.
    void UpdateSignals();
    
    //! Called when some of the attributes has been changed.
    void OnAttributeUpdated(IAttribute *attribute);
    
    //! Called when attributes of the placeable have changed
    void PlaceableUpdated(IAttribute *attribute);
    
    //! Called when attributes of the terrain have changed
    void TerrainUpdated(IAttribute *attribute);
    
    //! Check for placeable & terrain components and connect to their signals
    void CheckForPlaceableAndTerrain();
    
    //! Called when EC_Terrain has been regenerated
    void OnTerrainRegenerated();

    //! Called when collision mesh has been downloaded.
    void OnCollisionMeshAssetLoaded(AssetPtr asset);

private:
    //! constructor
    /*! \param module Physics module
     */
    EC_RigidBody(IModule* module);
    
    //! (Re)create the collisionshape
    void CreateCollisionShape();
    
    //! Remove the collisionshape
    void RemoveCollisionShape();
    
    //! Create a heightfield collisionshape from EC_Terrain
    void CreateHeightFieldFromTerrain();
    
    //! Create a convex hull set collisionshape
    void CreateConvexHullSetShape();
    
    //! Create the body. No-op if the scene is not associated with a physics world.
    void CreateBody();
    
    //! Destroy the body
    void RemoveBody();
    
    //! Re-add the body to the physics world because of its some properties changing
    /*! (also recalculates those properties as necessary, and sets collisionshape for the body if it has changed)
     */
    void ReaddBody();
    
    //! Update scale from placeable & own size setting
    void UpdateScale();
    
    //! Request mesh resource (for trimesh & convexhull shapes)
    void RequestMesh();
    
    //! Calculate mass, shape & static/dynamic-classification dependant properties
    void GetProperties(btVector3& localInertia, float& m, int& collisionFlags);
    
    //! Emit a physics collision. Called from PhysicsWorld
    void EmitPhysicsCollision(Scene::Entity* otherEntity, const Vector3df& position, const Vector3df& normal, float distance, float impulse, bool newCollision);
    
    //! Placeable pointer
    boost::weak_ptr<EC_Placeable> placeable_;
    
    //! Terrain pointer
    boost::weak_ptr<Environment::EC_Terrain> terrain_;
    
    //! Internal disconnection of attribute changes. True during the time we're setting attributes ourselves due to Bullet update, to prevent endless loop
    bool disconnected_;
    
    //! Bullet body
    btRigidBody* body_;
    
    //! Bullet collision shape
    btCollisionShape* shape_;
    
    //! Physics world. May be 0 if the scene does not have a physics world. In that case most of EC_RigidBody's functionality is a no-op
    Physics::PhysicsWorld* world_;
    
    //! PhysicsModule pointer
    Physics::PhysicsModule* owner_;
    
    //! Cached shapetype (last created)
    int cachedShapeType_;

    //! Cached shapesize (last created)
    Vector3df cachedSize_;

    //! Bullet triangle mesh
    boost::shared_ptr<btTriangleMesh> triangleMesh_;
    
    //! Convex hull set
    boost::shared_ptr<Physics::ConvexHullSet> convexHullSet_;
    
    //! Bullet heightfield shape. Note: this is always put inside a compound shape (shape_)
    btHeightfieldTerrainShape* heightField_;
    
    //! Heightfield values, for the case the shape is a heightfield.
    std::vector<float> heightValues_;
};


#endif
