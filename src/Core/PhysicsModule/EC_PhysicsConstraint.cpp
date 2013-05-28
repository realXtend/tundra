// For conditions of distribution and use, see copyright notice in LICENSE

#include "StableHeaders.h"
#define MATH_BULLET_INTEROP

#include "DebugOperatorNew.h"

#include "PhysicsWorld.h"
#include "EC_PhysicsConstraint.h"
#include "EC_RigidBody.h"

#include "FrameAPI.h"
#include "Scene.h"
#include "Entity.h"
#include "Math/MathFunc.h"
#include "EC_Placeable.h"
#include "AttributeMetadata.h"

#include <BulletDynamics/ConstraintSolver/btTypedConstraint.h>
#include <BulletDynamics/ConstraintSolver/btHingeConstraint.h>
#include <BulletDynamics/ConstraintSolver/btPoint2PointConstraint.h>
#include <BulletDynamics/ConstraintSolver/btSliderConstraint.h>
#include <BulletDynamics/ConstraintSolver/btConeTwistConstraint.h>
#include <BulletDynamics/Dynamics/btDiscreteDynamicsWorld.h>

#include "MemoryLeakCheck.h"

using namespace Physics;

static Quat FromEulerDegToQuat(const float3 &degEuler)
{
    float3 radEuler = DegToRad(degEuler);
    return Quat::FromEulerXYZ(radEuler.x, radEuler.y, radEuler.z);
}

EC_PhysicsConstraint::EC_PhysicsConstraint(Scene* scene):
    IComponent(scene),
    constraint_(0),
    checkForRigidBodies(false),
    INIT_ATTRIBUTE_VALUE(enabled, "Enabled", false),
    INIT_ATTRIBUTE_VALUE(disableCollision, "Disable collision", false),
    INIT_ATTRIBUTE_VALUE(type, "Constraint type", 0),
    INIT_ATTRIBUTE_VALUE(otherEntity, "Other entity", EntityReference()),
    INIT_ATTRIBUTE_VALUE(position, "Position", float3::zero),
    INIT_ATTRIBUTE_VALUE(otherPosition, "Other position", float3::zero),
    INIT_ATTRIBUTE_VALUE(rotation, "Rotation", float3::zero),
    INIT_ATTRIBUTE_VALUE(otherRotation, "Other rotation", float3::zero), 
    INIT_ATTRIBUTE_VALUE(linearLimit, "Linear limit", float2::zero),
    INIT_ATTRIBUTE_VALUE(angularLimit, "Angular limit", float2::zero)
{
    static AttributeMetadata constraintTypeMetadata;
    static bool metadataInitialized = false;
    if(!metadataInitialized)
    {
        constraintTypeMetadata.enums[Hinge] = "Hinge";
        constraintTypeMetadata.enums[PointToPoint] = "Point to point";
        constraintTypeMetadata.enums[Slider] = "Slider";
        constraintTypeMetadata.enums[ConeTwist] = "Cone twist";
        metadataInitialized = true;
    }
    type.SetMetadata(&constraintTypeMetadata);

    connect(this, SIGNAL(ParentEntitySet()), this, SLOT(UpdateSignals()), Qt::UniqueConnection);
}

EC_PhysicsConstraint::~EC_PhysicsConstraint()
{
    Remove();
}

void EC_PhysicsConstraint::UpdateSignals()
{
    Entity *parentEntity = ParentEntity();
    if (!parentEntity)
        return;

    Scene* scene = parentEntity->ParentScene();
    physicsWorld_ = scene->GetWorld<PhysicsWorld>();

    connect(GetFramework()->Frame(), SIGNAL(Updated(float)), SLOT(CheckForBulletRigidBody()), Qt::UniqueConnection);
    connect(parentEntity, SIGNAL(ComponentAdded(IComponent*, AttributeChange::Type)), SLOT(OnComponentAdded(IComponent*)), Qt::UniqueConnection);
    connect(parentEntity, SIGNAL(ComponentRemoved(IComponent*, AttributeChange::Type)), SLOT(OnComponentRemoved(IComponent*)), Qt::UniqueConnection);
}

void EC_PhysicsConstraint::OnComponentAdded(IComponent *component)
{
    if (component->TypeId() == EC_RigidBody::TypeIdStatic())
        Create();
}

void EC_PhysicsConstraint::OnComponentRemoved(IComponent *component)
{
    if (component->TypeId() == EC_RigidBody::TypeIdStatic())
        Remove();
}

void EC_PhysicsConstraint::CheckForBulletRigidBody()
{
    if (!checkForRigidBodies)
        return;

    EC_RigidBody *rigidBody = rigidBody_.lock().get();
    EC_RigidBody *otherRigidBody = otherRigidBody_.lock().get();
    bool rigidBodyCreated = rigidBody && rigidBody->GetRigidBody();
    bool otherBodyCreated = otherRigidBody && otherRigidBody->GetRigidBody();

    if (rigidBodyCreated || otherBodyCreated)
    {
        Create();
        if (constraint_)
            checkForRigidBodies = false;
    }
}

void EC_PhysicsConstraint::AttributesChanged()
{
    bool recreate = false;
    bool applyAttributes = false;
    bool applyLimits = false;

    if (enabled.ValueChanged())
    {
        if (constraint_)
            constraint_->setEnabled(getenabled());
        else
            recreate = true;
    }

    if (disableCollision.ValueChanged())
        recreate = true;
    if (type.ValueChanged())
        recreate = true;
    if (otherEntity.ValueChanged())
        recreate = true;
    if (position.ValueChanged())
        applyAttributes = true;
    if (otherPosition.ValueChanged())
        applyAttributes = true;
    if (rotation.ValueChanged())
        applyAttributes = true;
    if (otherRotation.ValueChanged())
        applyAttributes = true;
    if (linearLimit.ValueChanged())
        applyLimits = true;
    if (angularLimit.ValueChanged())
        applyLimits = true;

    if (recreate)
        Create();
    if (!recreate && applyAttributes)
        ApplyAttributes();
    if (!recreate && applyLimits)
        ApplyLimits();
}

void EC_PhysicsConstraint::Create()
{
    if (!ParentEntity() || physicsWorld_.expired())
        return;

    Remove();

    rigidBody_ = ParentEntity()->GetComponent<EC_RigidBody>();

    Entity *otherEntity = 0;
    if (!getotherEntity().IsEmpty())
    {
        otherEntity = getotherEntity().Lookup(ParentScene()).get();
        if (otherEntity)
        {
            otherRigidBody_ = otherEntity->GetComponent<EC_RigidBody>();
            connect(otherEntity, SIGNAL(EntityRemoved(Entity*, AttributeChange::Type)), SLOT(Remove()), Qt::UniqueConnection);
            connect(otherEntity, SIGNAL(ComponentAdded(IComponent*, AttributeChange::Type)), SLOT(OnComponentAdded(IComponent*)), Qt::UniqueConnection);
            connect(otherEntity, SIGNAL(ComponentRemoved(IComponent*, AttributeChange::Type)), SLOT(OnComponentRemoved(IComponent*)), Qt::UniqueConnection);
        }
        else
            otherRigidBody_.reset();
    }
    else
        otherRigidBody_.reset();

    EC_RigidBody *rigidBodyComp = rigidBody_.lock().get();
    EC_RigidBody *otherRigidBodyComp = otherRigidBody_.lock().get();
    btRigidBody *ownBody = rigidBodyComp ? rigidBodyComp->GetRigidBody() : 0;
    btRigidBody *otherBody = otherRigidBodyComp ? otherRigidBodyComp->GetRigidBody() : 0;

    if (!ownBody && !rigidBodyComp)
        return;

    else if (!ownBody && rigidBodyComp)
    {
        checkForRigidBodies = true;
        return;
    }

    if (!otherBody && !otherRigidBodyComp)
        otherBody = &btTypedConstraint::getFixedBody();
    else if (!otherBody && otherRigidBodyComp)
    {
        checkForRigidBodies = true;
        return;
    }

    float3 worldScale(1,1,1);
    float3 otherWorldScale(1,1,1);
    
    EC_Placeable *placeable = ParentEntity()->GetComponent<EC_Placeable>().get();
    EC_Placeable *otherPlaceable = 0;
    if (otherEntity)
        otherPlaceable = otherEntity->GetComponent<EC_Placeable>().get();
    
    if (placeable)
        worldScale = placeable->WorldScale();
    if (otherPlaceable)
        otherWorldScale = otherPlaceable->WorldScale();

    btTransform ownTransform(FromEulerDegToQuat(getrotation()), getposition().Mul(worldScale));
    btTransform otherTransform(FromEulerDegToQuat(getotherRotation()), getotherPosition().Mul(otherWorldScale));

    switch(gettype())
    {
        case PointToPoint:
            constraint_ = new btPoint2PointConstraint(*ownBody, *otherBody, getposition().Mul(worldScale), getotherPosition().Mul(otherWorldScale));
            break;

        case Hinge:
            constraint_ = new btHingeConstraint(*ownBody, *otherBody, ownTransform, otherTransform);
            break;

        case Slider:
            constraint_ = new btSliderConstraint(*ownBody, *otherBody, ownTransform, otherTransform, false);
            break;

        case ConeTwist:
            constraint_ = new btConeTwistConstraint(*ownBody, *otherBody, ownTransform, otherTransform);
            break;

        default:
            break;
    }

    if (constraint_)
    {
        constraint_->setUserConstraintPtr(this);
        constraint_->setEnabled(getenabled());
        ApplyLimits();

        PhysicsWorld *world = physicsWorld_.lock().get();
        world->BulletWorld()->addConstraint(constraint_, getdisableCollision());
        world->BulletWorld()->debugDrawConstraint(constraint_);
    }
}

void EC_PhysicsConstraint::Remove()
{
    if (constraint_)
    {
        EC_RigidBody *ownRigidComp = rigidBody_.lock().get();
        EC_RigidBody *otherRigidComp = otherRigidBody_.lock().get();
        if (otherRigidComp && otherRigidComp->GetRigidBody())
            otherRigidComp->GetRigidBody()->removeConstraintRef(constraint_);
        if (ownRigidComp && ownRigidComp->GetRigidBody())
            ownRigidComp->GetRigidBody()->removeConstraintRef(constraint_);

        PhysicsWorld *world = physicsWorld_.lock().get();
        if (world && world->BulletWorld())
            world->BulletWorld()->removeConstraint(constraint_);

        rigidBody_.reset();
        otherRigidBody_.reset();
        delete constraint_;
        constraint_ = 0;
    }
}

void EC_PhysicsConstraint::ApplyAttributes()
{
    if (!constraint_)
        return;

    float3 worldScale(1,1,1);
    float3 otherWorldScale(1,1,1);

    EC_Placeable *placeable = ParentEntity()->GetComponent<EC_Placeable>().get();
    Entity *entity = getotherEntity().Lookup(ParentScene()).get();
    EC_Placeable *otherPlaceable = 0;
    if (entity)
        otherPlaceable = entity->GetComponent<EC_Placeable>().get();

    if (placeable)
        worldScale = placeable->WorldScale();
    if (otherPlaceable)
        otherWorldScale = otherPlaceable->WorldScale();

    btTransform ownTransform(FromEulerDegToQuat(getrotation()), getposition().Mul(worldScale));
    btTransform otherTransform(FromEulerDegToQuat(getotherRotation()), getotherPosition().Mul(otherWorldScale));

    switch (constraint_->getConstraintType())
    {
        case POINT2POINT_CONSTRAINT_TYPE:
        {
            btPoint2PointConstraint* pointConstraint = static_cast<btPoint2PointConstraint*>(constraint_);
            pointConstraint->setPivotA(getposition().Mul(worldScale));
            pointConstraint->setPivotB(getotherPosition().Mul(otherWorldScale));
        }
            break;
            
        case HINGE_CONSTRAINT_TYPE:
        {
            btHingeConstraint* hingeConstraint = static_cast<btHingeConstraint*>(constraint_);
            hingeConstraint->setFrames(ownTransform, otherTransform);
        }
            break;
            
        case SLIDER_CONSTRAINT_TYPE:
        {
            btSliderConstraint* sliderConstraint = static_cast<btSliderConstraint*>(constraint_);
            sliderConstraint->setFrames(ownTransform, otherTransform);
        }
            break;
            
        case CONETWIST_CONSTRAINT_TYPE:
        {
            btConeTwistConstraint* coneTwistConstraint = static_cast<btConeTwistConstraint*>(constraint_);
            coneTwistConstraint->setFrames(ownTransform, otherTransform);
        }
            break;

        default:
            break;
    }
}

void EC_PhysicsConstraint::ApplyLimits()
{
    if (!constraint_)
        return;

    switch (constraint_->getConstraintType())
    {
        case HINGE_CONSTRAINT_TYPE:
        {
            btHingeConstraint* hingeConstraint = static_cast<btHingeConstraint*>(constraint_);
            hingeConstraint->setLimit(DegToRad(getangularLimit().x), DegToRad(getangularLimit().y));
        }
            break;
            
        case SLIDER_CONSTRAINT_TYPE:
        {
            btSliderConstraint* sliderConstraint = static_cast<btSliderConstraint*>(constraint_);
            
            sliderConstraint->setLowerLinLimit(getlinearLimit().x);
            sliderConstraint->setUpperLinLimit(getlinearLimit().y);

            sliderConstraint->setLowerAngLimit(DegToRad(getangularLimit().x));
            sliderConstraint->setUpperAngLimit(DegToRad(getangularLimit().y));
        }
            break;
            
        case CONETWIST_CONSTRAINT_TYPE:
        {
            btConeTwistConstraint* coneTwistConstraint = static_cast<btConeTwistConstraint*>(constraint_);
            coneTwistConstraint->setLimit(DegToRad(getangularLimit().y), DegToRad(getangularLimit().y), DegToRad(getlinearLimit().y));
        }
            break;
            
        default:
            break;
    }
}
