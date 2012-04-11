// For conditions of distribution and use, see copyright notice in LICENSE

#pragma once

#include "IComponent.h"
#include "CoreDefines.h"
#include "Math/float3.h"
#include "PhysicsModuleFwd.h"

class EC_RigidBody;
class EC_Placeable;

/// Physics motor component. Drives a RigidBody by impulses on each physics update, and optionally applies a damping (braking) force
/**
<table class="header">
<tr>
<td>
<h2>PhysicsMotor</h2>
Physics motor component

Registered by Physics::PhysicsModule.

<b>Attributes</b>:
<ul>
<li>float3: absoluteMoveForce
<div>World-space force applied to rigid body on each physics update.</div>
<li>float3: relativeMoveForce
<div>Local-space force applied to rigid body on each physics update.</div>
<li>float3: dampingForce
<div>Force proportional and opposite to linear velocity that will be applied to rigid body on each physics update to slow it down.</div>
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

<b>Depends on the components RigitBody and Placeable.</b>.

</table> */
class EC_PhysicsMotor : public IComponent
{
    Q_OBJECT
    COMPONENT_NAME("EC_PhysicsMotor", 43)
    
public:
    Q_PROPERTY(float3 absoluteMoveForce READ getabsoluteMoveForce WRITE setabsoluteMoveForce);
    DEFINE_QPROPERTY_ATTRIBUTE(float3, absoluteMoveForce);
    Q_PROPERTY(float3 relativeMoveForce READ getrelativeMoveForce WRITE setrelativeMoveForce);
    DEFINE_QPROPERTY_ATTRIBUTE(float3, relativeMoveForce);
    Q_PROPERTY(float3 dampingForce READ getdampingForce WRITE setdampingForce);
    DEFINE_QPROPERTY_ATTRIBUTE(float3, dampingForce);
    
    /// Do not directly allocate new components using operator new, but use the factory-based SceneAPI::CreateComponent functions instead.
    explicit EC_PhysicsMotor(Scene* scene);

    virtual ~EC_PhysicsMotor();
    
private slots:
    /// Called when the parent entity has been set.
    void UpdateSignals();

    /// Called when component has been added or removed from the parent entity. Checks the existence of the EC_Placeable component, and attaches this camera to it.
    void OnComponentStructureChanged();
    
    /// Apply forces during each physics update.
    void OnPhysicsUpdate();
    
private:
    /// Cached rigidbody pointer
    boost::weak_ptr<EC_RigidBody> rigidBody_;
    /// Cached placeable pointer
    boost::weak_ptr<EC_Placeable> placeable_;
};
