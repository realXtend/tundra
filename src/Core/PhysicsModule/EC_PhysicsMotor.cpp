// For conditions of distribution and use, see copyright notice in LICENSE

#include "StableHeaders.h"
#include "DebugOperatorNew.h"

#include "EC_PhysicsMotor.h"
#include "EC_RigidBody.h"
#include "EC_Placeable.h"
#include "Entity.h"
#include "Scene/Scene.h"
#include "PhysicsModule.h"
#include "PhysicsWorld.h"
#include "PhysicsUtils.h"
#include "LoggingFunctions.h"
#include "Profiler.h"

#include "MemoryLeakCheck.h"

using namespace Physics;

EC_PhysicsMotor::EC_PhysicsMotor(Scene* scene) :
    IComponent(scene),
    absoluteMoveForce(this, "Absolute Move Force", float3::zero),
    relativeMoveForce(this, "Relative Move Force", float3::zero),
    dampingForce(this, "Damping Force", float3::zero)
{
    connect(this, SIGNAL(ParentEntitySet()), this, SLOT(UpdateSignals()), Qt::UniqueConnection);
}

EC_PhysicsMotor::~EC_PhysicsMotor()
{
}

void EC_PhysicsMotor::UpdateSignals()
{
    Entity* parent = ParentEntity();
    if (!parent)
        return;
    Scene* scene = parent->ParentScene();
    PhysicsWorld* world = scene->GetWorld<PhysicsWorld>().get();
    if (world)
        connect(world, SIGNAL(Updated(float)), this, SLOT(OnPhysicsUpdate()), Qt::UniqueConnection);
    
    connect(parent, SIGNAL(ComponentAdded(IComponent*, AttributeChange::Type)), this, SLOT(OnComponentStructureChanged()), Qt::UniqueConnection);
    connect(parent, SIGNAL(ComponentRemoved(IComponent*, AttributeChange::Type)), this, SLOT(OnComponentStructureChanged()), Qt::UniqueConnection);
    OnComponentStructureChanged(); // Check for Rigidbody & Placeable immediately
}

void EC_PhysicsMotor::OnComponentStructureChanged()
{
    Entity* parent = ParentEntity();
    if (!parent)
        return;
    rigidBody_ = parent->GetComponent<EC_RigidBody>();
    placeable_ = parent->GetComponent<EC_Placeable>();
}

void EC_PhysicsMotor::OnPhysicsUpdate()
{
    PROFILE(EC_PhysicsMotor_OnPhysicsUpdate);
    
    EC_RigidBody* body = rigidBody_.lock().get();
    EC_Placeable* placeable = placeable_.lock().get();
    
    if (body && placeable)
    {
        float3 absolute = absoluteMoveForce.Get();
        if (!absolute.Equals(float3::zero))
            body->ApplyImpulse(absolute);
        
        float3 relative = relativeMoveForce.Get();
        if (!relative.Equals(float3::zero))
        {
            float3 translate, scale;
            Quat rot;
            
            placeable->LocalToWorld().Decompose(translate, rot, scale);
            relative = rot * relative;
            body->ApplyImpulse(relative);
        }
        
        float3 damping = dampingForce.Get();
        if (!damping.Equals(float3::zero))
        {
            damping = -body->GetLinearVelocity() * damping;
            body->ApplyImpulse(damping);
        }
    }
}
