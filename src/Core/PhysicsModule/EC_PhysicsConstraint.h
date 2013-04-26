// For conditions of distribution and use, see copyright notice in LICENSE

#pragma once

#include "IComponent.h"
#include "Math/float3.h"
#include "EntityReference.h"

class EC_RigidBody;

class EC_PhysicsConstraint : public IComponent
{
    Q_OBJECT
    COMPONENT_NAME("EC_PhysicsConstraint", 53)
public:
    explicit EC_PhysicsConstraint(Scene* scene);
    virtual ~EC_PhysicsConstraint();

    enum ConstraintType
    {
        Invalid = 0, ///< Invalid constraint
        Hinge, ///< Hinge constraint
        PointToPoint, ///< Point to point constraint
        Slider, ///< Slider constraint
    };

    Q_PROPERTY(bool enabled READ getenabled WRITE setenabled);
    DEFINE_QPROPERTY_ATTRIBUTE(bool, enabled);

    Q_PROPERTY(EntityReference otherEntity READ getotherEntity WRITE setotherEntity);
    DEFINE_QPROPERTY_ATTRIBUTE(EntityReference, otherEntity);

    Q_PROPERTY(uint type READ gettype WRITE settype);
    DEFINE_QPROPERTY_ATTRIBUTE(uint, type);

    Q_PROPERTY(float3 pivotPointA READ getpivotPointA WRITE setpivotPointA);
    DEFINE_QPROPERTY_ATTRIBUTE(float3, pivotPointA);

    Q_PROPERTY(float3 pivotPointB READ getpivotPointB WRITE setpivotPointB);
    DEFINE_QPROPERTY_ATTRIBUTE(float3, pivotPointB);

    Q_PROPERTY(float upperLimit READ getupperLimit WRITE setupperLimit);
    DEFINE_QPROPERTY_ATTRIBUTE(float, upperLimit);

    Q_PROPERTY(float lowerLimit READ getlowerLimit WRITE setlowerLimit);
    DEFINE_QPROPERTY_ATTRIBUTE(float, lowerLimit);

private slots:
    void UpdateSignals();
private:
    weak_ptr<EC_RigidBody> rigidBody_;
};