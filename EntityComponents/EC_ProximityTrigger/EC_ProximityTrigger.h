/**
 *  For conditions of distribution and use, see copyright notice in license.txt
 *
 *  @file   EC_ProximityTrigger.h
 *  @brief  EC_ProximityTrigger reports distance, each frame, to other entities with EC_ProximityTrigger
 *  @note   The entity should have EC_Placeable available in advance.
 */

#ifndef incl_EC_ProximityTrigger_EC_ProximityTrigger_h
#define incl_EC_ProximityTrigger_EC_ProximityTrigger_h

#include "StableHeaders.h"
#include "IComponent.h"
#include "Declare_EC.h"

#include <QVector3D>
#include <QQuaternion>

/// EntityComponent that reports distance to other entities with EC_ProximityTrigger component
/**
<table class="header">
<tr>
<td>
<h2>ProximityTrigger</h2>
EntityComponent that reports distance to other entities with EC_ProximityTrigger component
NOTE: Assumes the the entity already has EC_Placeable. Otherwise the distance can not be calculated
<b>Attributes</b>:
<ul>
<li>bool: active
<div>If true (default), sends trigger signals that report distance to other entities with EC_ProximityTrigger.</div>
<li>float: thresholdDistance
<div>If greater than 0, entities beyond the threshold distance do not trigger the signal. Default is 0.</div>
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

    //! Set component as serializable.
    virtual bool IsSerializable() const { return true; }
    
    /// Active flag. Trigger signals are only generated when this is true. Is true by default
    Q_PROPERTY(bool active READ getactive WRITE setactive);
    
    /// Threshold distance. If greater than 0, entities beyond the threshold distance do not trigger the signal. Default is 0.
    Q_PROPERTY(float thresholdDistance READ getthresholdDistance WRITE setthresholdDistance);
    
    DEFINE_QPROPERTY_ATTRIBUTE(bool, active);
    DEFINE_QPROPERTY_ATTRIBUTE(float, thresholdDistance);
    
signals:
    /// Triggered signal. When active flag is on, is sent each frame for every other entity that has an EC_ProximityTrigger, along with the distance
    void Triggered(Scene::Entity* otherEntity, float distance);

public slots:
    void AttributeUpdated(IAttribute* attr);
    void Update(float timeStep);
    
private:
    EC_ProximityTrigger(IModule *module);
};

#endif
