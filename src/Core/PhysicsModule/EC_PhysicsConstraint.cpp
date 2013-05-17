// For conditions of distribution and use, see copyright notice in LICENSE

#include "StableHeaders.h"
#include "DebugOperatorNew.h"

#include "EC_PhysicsConstraint.h"
#include "EC_RigidBody.h"

#include "AttributeMetadata.h"

using namespace Physics;

EC_PhysicsConstraint::EC_PhysicsConstraint(Scene* scene):
    IComponent(scene),
    INIT_ATTRIBUTE_VALUE(enabled, "Enabled", false),
    INIT_ATTRIBUTE_VALUE(type, "Constraint type", 0),
    INIT_ATTRIBUTE_VALUE(otherEntity, "Other entity", EntityReference()),
    INIT_ATTRIBUTE_VALUE(pivotPointA, "Pivot point A", float3::zero),
    INIT_ATTRIBUTE_VALUE(pivotPointB, "Pivot point B", float3::zero),
    INIT_ATTRIBUTE_VALUE(upperLimit, "Upper limit", 0.f),
    INIT_ATTRIBUTE_VALUE(lowerLimit, "Lower limit", 0.f)
{
    static AttributeMetadata constraintTypeMetadata;
    static bool metadataInitialized = false;
    if(!metadataInitialized)
    {
        constraintTypeMetadata.enums[Invalid] = "Invalid";
        constraintTypeMetadata.enums[Hinge] = "Hinge";
        constraintTypeMetadata.enums[PointToPoint] = "Point to point";
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