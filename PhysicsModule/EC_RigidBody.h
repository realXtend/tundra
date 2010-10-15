// For conditions of distribution and use, see copyright notice in license.txt

#ifndef incl_Physics_EC_RigidBody_h
#define incl_Physics_EC_RigidBody_h

#include "Core.h"
#include "IComponent.h"
#include "LinearMath/btMotionState.h"
#include "Declare_EC.h"

class btRigidBody;
class btCollisionShape;
class btTriangleMesh;
class EC_Placeable;

namespace Physics
{
    class PhysicsModule;
    class PhysicsWorld;
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
<li>QString: collisionMeshId
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
<div>Specifies the axes on which torques can act on the object, making it rotate. Set to 0,0,0 to make for example an avatar capsule that does not tip over by itself.</div>
</ul>

<b>Exposes the following scriptable functions:</b>
<ul>
</ul>

<b>Reacts on the following actions:</b>
<ul>
<li>...
</ul>
</td>
</tr>

Does not emit any actions.

<b>Depends on the component Placeable</b>.
</table>
*/
class EC_RigidBody : public IComponent, public btMotionState
{
    Q_OBJECT
    Q_ENUMS(EventType)
    
    DECLARE_EC(EC_RigidBody);
public:
    enum ShapeType
    {
        Shape_Box = 0,
        Shape_Sphere,
        Shape_Cylinder,
        Shape_Capsule,
        Shape_TriMesh,
        Shape_HeightField
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
    
    //! Collision mesh asset ID
    Q_PROPERTY(QString collisionMeshId READ getcollisionMeshId WRITE setcollisionMeshId);
    DEFINE_QPROPERTY_ATTRIBUTE(QString, collisionMeshId);
    
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
    
    virtual ~EC_RigidBody();
    
    //! Set component as serializable.
    virtual bool IsSerializable() const { return true; }
    
    bool HandleEvent(event_category_id_t category_id, event_id_t event_id, IEventData *data);
    
    //! btMotionState override. Called when Bullet wants us to tell the body's initial transform
    virtual void getWorldTransform(btTransform &worldTrans) const;
    //! btMotionState override. Called when Bullet wants to tell us the body's current transform
    virtual void setWorldTransform(const btTransform &worldTrans);
    
public slots:
    
private slots:
    //! Called when the parent entity has been set.
    void UpdateSignals();
    
    //! Called when some of the attributes has been changed.
    void AttributeUpdated(IAttribute *attribute);
    
    //! Called when attributes of the placeable have changed
    void PlaceableUpdated(IAttribute *attribute);
    
    //! Check for placeable component and connect to its signals
    void CheckForPlaceable();
    
private:
    //! constructor
    /*! \param module Physics module
     */
    EC_RigidBody(IModule* module);
    
    //! (Re)create the collisionshape
    void CreateCollisionShape();
    
    //! Remove the collisionshape
    void RemoveCollisionShape();
    
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
    
    //! Placeable found-flag
    boost::weak_ptr<EC_Placeable> placeable_;
    
    //! Placeable change disconnected-flag. Set when we're setting placeable ourselves due to Bullet update, to prevent endless loop
    bool placeableDisconnected_; 
    
    //! Bullet body
    btRigidBody* body_;
    
    //! Bullet collision shape
    btCollisionShape* shape_;
    
    //! Physics world
    Physics::PhysicsWorld* world_;
    
    //! PhysicsModule pointer
    Physics::PhysicsModule* owner_;
    
    //! Cached shapetype (last created)
    int cachedShapeType_;
    //! Cached shapesize (last created)
    Vector3df cachedSize_;
    
    //! Request tag for the collision mesh
    request_tag_t collision_mesh_tag_;
    //! Resource event category
    event_category_id_t resource_event_category_;
    
    //! Bullet triangle mesh
    boost::shared_ptr<btTriangleMesh> triangleMesh_;
};


#endif
