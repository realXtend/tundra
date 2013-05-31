// For conditions of distribution and use, see copyright notice in LICENSE

#pragma once

#include "IComponent.h"
#include "Math/Quat.h"
#include "Math/float3.h"
#include "Math/float2.h"
#include "EntityReference.h"
#include "PhysicsWorld.h"

class btTypedConstraint;
class EC_RigidBody;

/// Physics constraint entity-component
/** <table class="header">
 <tr>
 <td>
 <h2>PhysicsConstraint</h2>
 Physics constraint entity-component
 
 Registered by Physics::PhysicsModule.
 
 <b>Attributes</b>:
 <ul>
 <li>bool: enabled
 <div>@copydoc enabled</div>
 <li>EntityReference: otherEntity
 <div>@copydoc @copydoc otherEntity</div>
 <li>enum: type
 <div>@copydoc type</div>
 <li>float3: position
 <div>@copydoc position</div>
 <li>float3: otherPosition
 <div>@copydoc otherPosition</div>
 <li>float3: rotation
 <div>@copydoc rotation</div>
 <li>float3: otherRotation
 <div>@copydoc otherRotation</div>
 <li>float2: lowerLimit
 <div>@copydoc lowerLimit</div>
 <li>float2: upperLimit
 <div>@copydoc upperLimit</div>
 </ul>
 
 <b>Exposes the following scriptable functions:</b>
 <ul>
 <li>None.
 </ul>
 
 <b>Reacts on the following actions:</b>
 <ul>
 <li>None.
 </ul>
 </td>
 </tr>
 
 Does not emit any actions.
 
 <b>Depends on the component @ref EC_RigidBody "RigidBody", and @ref EC_Placeable "Placeable"</b>.
 </table> */

class EC_PhysicsConstraint : public IComponent
{
    Q_OBJECT
    COMPONENT_NAME("EC_PhysicsConstraint", 53)
    Q_ENUMS(ConstraintType)

public:
    /// @cond PRIVATE
    /// Do not directly allocate new components using operator new, but use the factory-based SceneAPI::CreateComponent functions instead.
    explicit EC_PhysicsConstraint(Scene* scene);
    /// @endcond
    virtual ~EC_PhysicsConstraint();

    enum ConstraintType
    {
        PointToPoint = 0, ///< Point to point constraint
        Hinge, ///< Hinge constraint
        Slider, ///< Slider constraint
        ConeTwist, ///< Cone twist constraint
    };

    /// Enables / disables the constraint on this entity.
    Q_PROPERTY(bool enabled READ getenabled WRITE setenabled);
    DEFINE_QPROPERTY_ATTRIBUTE(bool, enabled);

    /// Disables collision between the bodies this constraint connects.
    Q_PROPERTY(bool disableCollision READ getdisableCollision WRITE setdisableCollision);
    DEFINE_QPROPERTY_ATTRIBUTE(bool, disableCollision);

    /// Applies this constraint on otherEntity. If this attribute is empty or invalid, the constraint will be static i.e. to an invisible fixed body.
    Q_PROPERTY(EntityReference otherEntity READ getotherEntity WRITE setotherEntity);
    DEFINE_QPROPERTY_ATTRIBUTE(EntityReference, otherEntity);

    /// Constraint type
    /** @see ConstraintType */
    Q_PROPERTY(uint type READ gettype WRITE settype);
    DEFINE_QPROPERTY_ATTRIBUTE(uint, type);

    /// Pivot point A.
    /** @note: These coordinates are in this entity's local space */
    Q_PROPERTY(float3 position READ getposition WRITE setposition);
    DEFINE_QPROPERTY_ATTRIBUTE(float3, position);

    /// Pivot point B.
    /** @note: If otherEntity points to a valid entity that has EC_RigidBody, these cordinates are in this entity's local space; otherwise they are in world space. */
    Q_PROPERTY(float3 otherPosition READ getotherPosition WRITE setotherPosition);
    DEFINE_QPROPERTY_ATTRIBUTE(float3, otherPosition);

    /// Orientation of pivot point A represented in euler angles in degrees.
    /** @note: These cordinates are in this entity's local space. This attribute has no effect on PointToPoint constraint type. */
    Q_PROPERTY(float3 rotation READ getrotation WRITE setrotation);
    DEFINE_QPROPERTY_ATTRIBUTE(float3, rotation);

    /// Orientation of pivot point B represented in euler angles in degrees.
    /** @note: If otherEntity points to a valid entity, these cordinates are in this entity's local space; otherwise they are in world space. This attribute has no effect on PointToPoint constraint type. */
    Q_PROPERTY(float3 otherRotation READ getotherRotation WRITE setotherRotation);
    DEFINE_QPROPERTY_ATTRIBUTE(float3, otherRotation);

    /// Linear limit, ranging from x to y.
    /** @note: Affects only Slider constraint type, and the 'y' component is needed for ConeTwist constraint to set the twist span.*/
    Q_PROPERTY(float2 linearLimit READ getlinearLimit WRITE setlinearLimit);
    DEFINE_QPROPERTY_ATTRIBUTE(float2, linearLimit);

    /// Angular limit, ranging from x to y. The angular limit is in degrees.
    /** @note: This attribute has no effect on PointToPoint constraint type. */
    Q_PROPERTY(float2 angularLimit READ getangularLimit WRITE setangularLimit);
    DEFINE_QPROPERTY_ATTRIBUTE(float2, angularLimit);

private slots:
    /// Creates or re-creates this constraint. The parent entity must have EC_RigidBody in advance
    void Create();
    /// Removes this constraint
    void Remove();
    /// Called when parent entity has been set
    void UpdateSignals();
    /// Called when a component had been added to the parent entity
    void OnComponentAdded(IComponent *component);
    /// Called when a component had been removed to the parent entity
    void OnComponentRemoved(IComponent *component);
    /// A helper function that checks for btRigidBody pointers in cases when EC_RigidBody is present but not yet created a btRigidBody
    void CheckForBulletRigidBody();

private:
    /// Called when some of the attributes are changed
    void AttributesChanged();
    /// Applies constraint attributes
    void ApplyAttributes();
    /// Applies constraint limits
    void ApplyLimits();

    /// Enables checking for btRigidBody pointers
    bool checkForRigidBodies;

    /// Cached pointer of this entity's rigid body
    weak_ptr<EC_RigidBody> rigidBody_;
    /// Cached pointer of the other entity's rigid body
    weak_ptr<EC_RigidBody> otherRigidBody_;
    /// Cached physics world pointer
    weak_ptr<PhysicsWorld> physicsWorld_;
    /// Constraint pointer
    btTypedConstraint *constraint_;
};