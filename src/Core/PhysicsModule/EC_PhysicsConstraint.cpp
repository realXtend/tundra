// For conditions of distribution and use, see copyright notice in LICENSE

#include "StableHeaders.h"
#include "DebugOperatorNew.h"

#include "EC_PhysicsConstraint.h"
#include "EC_RigidBody.h"

#include "AttributeMetadata.h"

using namespace Physics;

EC_PhysicsConstraint::EC_PhysicsConstraint(Scene* scene):
    IComponent(scene),
    enabled(this, "Enabled", false),
    type(this, "Constraint type", 0),
    otherEntity(this, "Other entity", EntityReference()),
    pivotPointA(this, "Pivot point A", float3::zero),
    pivotPointB(this, "Pivot point B", float3::zero),
    upperLimit(this, "Upper limit", 0.f),
    lowerLimit(this, "Lower limit", 0.f)

{
    static AttributeMetadata constraintTypeMetadata;
    static bool metadataInitialized = false;
    if(!metadataInitialized)
    {
        constraintTypeMetadata.enums[Invalid] = "Invalid";
        constraintTypeMetadata.enums[Hinge] = "Hinge";
        constraintTypeMetadata.enums[Point2Point] = "Point to point";
        constraintTypeMetadata.enums[Slider] = "Slider";
        metadataInitialized = true;
    }
    type.SetMetadata(&constraintTypeMetadata);

    connect(this, SIGNAL(ParentEntitySet()), this, SLOT(UpdateSignals()), Qt::UniqueConnection);
}

EC_PhysicsConstraint::~EC_PhysicsConstraint()
{
}

void EC_PhysicsConstraint::UpdateSignals()
{
}