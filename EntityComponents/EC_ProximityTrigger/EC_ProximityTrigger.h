/**
 *  For conditions of distribution and use, see copyright notice in license.txt
 *
 *  @file   EC_ProximityTrigger.h
 *  @brief  EC_ProximityTrigger reports distance, each frame, of other entities that also have EC_ProximityTrigger component
 */

#ifndef incl_EC_ProximityTrigger_EC_ProximityTrigger_h
#define incl_EC_ProximityTrigger_EC_ProximityTrigger_h

#include "IComponent.h"
#include "Declare_EC.h"

#include <QVector3D>
#include <QQuaternion>

/// EntityComponent that reports distance of other entities that also have an EC_ProximityTrigger component
/**
<table class="header">
<tr>
<td>
<h2>ProximityTrigger</h2>
EntityComponent that reports distance to other entities that also have EC_ProximityTrigger component. The entities
also need to have EC_Placeable component so that distance can be calculated.

<b>Attributes</b>:
<ul>
<li>bool: active
<div>If true (default), sends trigger signals with distance of other entities with EC_ProximityTrigger. The other entities' proximity triggers do not need to have 'active' set.</div>
<li>float: thresholdDistance
<div>If greater than 0, entities beyond the threshold distance do not trigger the signal. Default is 0. The other entities' threshold values do not matter.</div>
<li>float: interval
<div>Interval of trigger signals in seconds. If 0, the signal is sent every frame. Default is 0.</div>
</ul>

<b>Exposes the following scriptable functions:</b>
<ul>
<li>...
</ul>

<b>Reacts on the following actions:</b>
<ul>
<li>...
</ul>
</td>
</tr>

Does not emit any actions.

<b>Depends on EC_Placeable.</b>
</table>
*/
class EC_ProximityTrigger : public IComponent
{
    Q_OBJECT
    DECLARE_EC(EC_ProximityTrigger);

public:
    ~EC_ProximityTrigger();

    /// Active flag. Trigger signals are only generated when this is true. Is true by default
    Q_PROPERTY(bool active READ getactive WRITE setactive);

    /// Threshold distance. If greater than 0, entities beyond the threshold distance do not trigger the signal. Default is 0, which means distance does not matter.
    Q_PROPERTY(float thresholdDistance READ getthresholdDistance WRITE setthresholdDistance);
    
    /// Interval between signals in seconds. If 0, the signal is sent every frame. Default is 0
    Q_PROPERTY(float interval READ getinterval WRITE setinterval)

    DEFINE_QPROPERTY_ATTRIBUTE(bool, active);
    DEFINE_QPROPERTY_ATTRIBUTE(float, thresholdDistance);
    DEFINE_QPROPERTY_ATTRIBUTE(float, interval);
    
signals:
    /// Trigger signal. When active flag is on, is sent each frame for every other entity that also has an EC_ProximityTrigger and is close enough.
    /// Note: needs to be lowercase for QML to accept connections to it
    void triggered(Scene::Entity* otherEntity, float distance);

public slots:
    /// Attribute has been updated
    void OnAttributeUpdated(IAttribute* attr);
    
private slots:
    /// Check for other triggers and emit signals
    void Update(float timeStep);
    /// Periodic update. Set up the next periodic update, then check triggers
    void PeriodicUpdate();
    /// Change update mode (periodic, or every frame)
    void SetUpdateMode();
    
private:
    EC_ProximityTrigger(IModule *module);
};

#endif
