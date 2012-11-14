/**
    For conditions of distribution and use, see copyright notice in LICENSE

    @file   EC_ProximityTrigger.h
    @brief  Reports distance, each frame, of other entities that also have this same component. */

#pragma once

#include "IComponent.h"

/// Reports distance, each frame, of other entities that also have this same component.
/** <table class="header">
    <tr>
    <td>
    <h2>ProximityTrigger</h2>
    Reports distance, each frame, of other entities that also have this same component.
    The entities also need to have EC_Placeable component so that distance can be calculated.

    <b>Attributes</b>:
    <ul>
    <li>bool: active
    <div> @copydoc active </div>
    <li>float: thresholdDistance
    <div> @copydoc thresholdDistance </div>
    <li>float: interval
    <div> @copydoc interval </div>
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

    <b>Depends on @ref EC_Placeable "Placeable" component.</b>
    </table> */
class EC_ProximityTrigger : public IComponent
{
    Q_OBJECT
    COMPONENT_NAME("EC_ProximityTrigger", 33)

public:
    explicit EC_ProximityTrigger(Scene *scene);
    ~EC_ProximityTrigger();

    /// Active flag. Trigger signals are only generated when this is true. Is true by default
    /** If true (default), sends trigger signals with distance of other entities with EC_ProximityTrigger.
        The other entities' proximity triggers do not need to have 'active' set. */
    Q_PROPERTY(bool active READ getactive WRITE setactive);
    DEFINE_QPROPERTY_ATTRIBUTE(bool, active);

    /// Threshold distance.
    /** If greater than 0, entities beyond the threshold distance do not trigger the signal.
        Default is 0. The other entities' threshold values do not matter. */
    Q_PROPERTY(float thresholdDistance READ getthresholdDistance WRITE setthresholdDistance);
    DEFINE_QPROPERTY_ATTRIBUTE(float, thresholdDistance);

    /// Interval between signals in seconds. If 0, the signal is sent every frame. Default is 0
    Q_PROPERTY(float interval READ getinterval WRITE setinterval)
    DEFINE_QPROPERTY_ATTRIBUTE(float, interval);

signals:
    /// Trigger signal.
    /** When active flag is on, is sent each frame for every other entity that also has an EC_ProximityTrigger and is close enough.
        @note needs to be lowercase for QML to accept connections to it.
        @todo Make signature uppercase, QML support is deprecated. */
    void triggered(Entity* otherEntity, float distance);

private:
    /// Attribute has been updated
    void AttributesChanged();
    
private slots:
    /// Check for other triggers and emit signals
    void Update(float timeStep);

    /// Periodic update. Set up the next periodic update, then check triggers
    void PeriodicUpdate();

    /// Change update mode (periodic, or every frame)
    void SetUpdateMode();
};
