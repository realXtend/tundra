// For conditions of distribution and use, see copyright notice in license.txt

#pragma once

#include "IComponent.h"
#include "CoreTypes.h"
#include "Vector3D.h"
#include "OgreModuleFwd.h"

#include <Color.h>
#include <QString>

#ifdef CAELUM
namespace Caelum
{
    class CaelumSystem;
}
#endif

/// Makes the entity a enviroment light.
/**
<table class="header">
<tr>
<td>
<h2>Environment Light</h2>

Enviroment light component is registered by Enviroment::EnvironmentModule. Component gives an access to various scene related environment settings, 
such as sunlight, ambient light. On default component uses Caelum and because of that changes to some of attributes (sun light color, direction ambient color) 
does not work until user disables Caelum (by setting useCaelum attribute to false). Use can also override server time by setting useFixedTime true, if that and useCaelum is set 
false user can totally control scene "global" lights.   

<b>Attributes</b>:
<ul>
<li> Color : sunColorAttr.
<div> Defines sun color (in Caelum) </div>
<li> Color : ambientColorAttr.
<div> Defines scene ambient color. </div>
<li> Color : sunDiffuseColorAttr.
<div> Defines sun diffuse color.  </div>
<li> Vector3df : sunDirectionAttr.
<div> Defines sun direction vector (note caelum must be disabled). </div>
<li> bool : fixedTimeAttr.
<div> Defines that do we use time from server or do we use given fixed time. </div>
<li> float : currentTimeAttr.
<div> Current time which is used in scene </div>
<li> bool : sunCastShadowsAttr.
<div> Defines that does sun cast shadows (usable only if caelum is disabled) </div>
<li> bool : useCaelumAttr.
<div> Defines that do we use Caelum or not. </div>
</table>
*/
class EC_EnvironmentLight : public IComponent
{
    Q_OBJECT

public:
    /// Do not directly allocate new components using operator new, but use the factory-based SceneAPI::CreateComponent functions instead.
    explicit EC_EnvironmentLight(Scene* scene);

    virtual ~EC_EnvironmentLight();

    /// Defines sun color.
    DEFINE_QPROPERTY_ATTRIBUTE(Color,  sunColorAttr);
    Q_PROPERTY(Color sunColorAttr READ getsunColorAttr WRITE setsunColorAttr); 

    /// Defines ambient light color.
    DEFINE_QPROPERTY_ATTRIBUTE(Color, ambientColorAttr);
    Q_PROPERTY(Color ambientColorAttr READ getambientColorAttr WRITE setambientColorAttr); 

    /// Defines sun diffuse light color.
    DEFINE_QPROPERTY_ATTRIBUTE(Color, sunDiffuseColorAttr);
    Q_PROPERTY(Color sunDiffuseColorAttr READ getsunDiffuseColorAttr WRITE setsunDiffuseColorAttr); 

    /// Defines sun light direction.
    DEFINE_QPROPERTY_ATTRIBUTE(Vector3df, sunDirectionAttr);
    Q_PROPERTY(Vector3df sunDirectionAttr READ getsunDirectionAttr WRITE setsunDirectionAttr); 

    /// Defines that is fixed time used.
    DEFINE_QPROPERTY_ATTRIBUTE(bool, fixedTimeAttr);
    Q_PROPERTY(bool fixedTimeAttr READ getfixedTimeAttr WRITE setfixedTimeAttr); 

    /// Defines current time
    DEFINE_QPROPERTY_ATTRIBUTE(float, currentTimeAttr);
    Q_PROPERTY(float currentTimeAttr READ getcurrentTimeAttr WRITE setcurrentTimeAttr); 

    /// Defines time which is used currently
    DEFINE_QPROPERTY_ATTRIBUTE(bool, sunCastShadowsAttr);
    Q_PROPERTY(bool sunCastShadowsAttr READ getsunCastShadowsAttr WRITE setsunCastShadowsAttr); 

     /// Defines that is caelum to used to define sun color & direction vector and ambient light.
    DEFINE_QPROPERTY_ATTRIBUTE(bool, useCaelumAttr);
    Q_PROPERTY(bool useCaelumAttr READ getuseCaelumAttr WRITE setuseCaelumAttr); 
   
    COMPONENT_NAME("EC_EnvironmentLight", 8)
public slots:

    /// Called If some of the attributes has been changed.
    void OnAttributeUpdated(IAttribute* attribute, AttributeChange::Type change);

    /// Update sun state
    void UpdateSun();

    /// Remove sun.
    void RemoveSun();

    /// Update ambient light state.
    void UpdateAmbientLight();

    /// This is update loop, which will update caelum systems this is called in Environment-class for each frame.
    void Update(float frameTime);

    /// Update time.
    void UpdateTime();

private:
    void CreateOgreLight();

    /**
     * Helper function which is used to update environment light. 
     */
    void ChangeEnvironmentLight(IAttribute* attribute);
    OgreWorldWeakPtr world_;

     /// Sunlight which is used if there does not exist caelum
    Ogre::Light* sunLight_;

#ifdef CAELUM
    /// Caelum system.
    Caelum::CaelumSystem *caelumSystem_;
#endif
};
