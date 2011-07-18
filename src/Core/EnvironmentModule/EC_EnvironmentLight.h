// For conditions of distribution and use, see copyright notice in license.txt

#pragma once

#include "IComponent.h"
#include "CoreTypes.h"
#include "Math/float3.h"
#include "OgreModuleFwd.h"
#include "Color.h"

/// Makes the entity a enviroment light.
/** <table class="header">
    <tr>
    <td>
    <h2>Environment Light</h2>

    Gives an access to scene-related environment settings, such as sunlight and ambient light.

    Enviroment light component is registered by Enviroment::EnvironmentModule. 

    <b>Attributes</b>:
    <ul>
    <li> Color : sunColorAttr.
    <div> Defines sun color (in Caelum) </div>
    <li> Color : ambientColorAttr.
    <div> Defines scene ambient color. </div>
    <li> Color : sunDiffuseColorAttr.
    <div> Defines sun diffuse color.  </div>
    <li> float3 : sunDirectionAttr.
    <div> Defines sun direction vector. </div>
    <li> bool : sunCastShadowsAttr.
    <div> Defines that does sun cast shadows (usable only if caelum is disabled) </div>
    </table>
*/
class EC_EnvironmentLight : public IComponent
{
    Q_OBJECT
    COMPONENT_NAME("EC_EnvironmentLight", 8)

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
    DEFINE_QPROPERTY_ATTRIBUTE(float3, sunDirectionAttr);
    Q_PROPERTY(float3 sunDirectionAttr READ getsunDirectionAttr WRITE setsunDirectionAttr); 

    /// \todo Remove
    DEFINE_QPROPERTY_ATTRIBUTE(bool, fixedTimeAttr);
    Q_PROPERTY(bool fixedTimeAttr READ getfixedTimeAttr WRITE setfixedTimeAttr); 

    /// \todo Remove
    DEFINE_QPROPERTY_ATTRIBUTE(float, currentTimeAttr);
    Q_PROPERTY(float currentTimeAttr READ getcurrentTimeAttr WRITE setcurrentTimeAttr); 

    /// Do we want the sunlight to cast shadows.
    DEFINE_QPROPERTY_ATTRIBUTE(bool, sunCastShadowsAttr);
    Q_PROPERTY(bool sunCastShadowsAttr READ getsunCastShadowsAttr WRITE setsunCastShadowsAttr); 

    /// \todo Remove
    DEFINE_QPROPERTY_ATTRIBUTE(bool, useCaelumAttr);
    Q_PROPERTY(bool useCaelumAttr READ getuseCaelumAttr WRITE setuseCaelumAttr); 

public slots:
    /// Called If some of the attributes has been changed.
    void OnAttributeUpdated(IAttribute* attribute, AttributeChange::Type change);

    /// Update sun state
    void UpdateSun();

    /// Removes sunlight.
    void RemoveSun();

    /// Update ambient light state.
    void UpdateAmbientLight();

private:
    void CreateOgreLight();

    OgreWorldWeakPtr ogreWorld;
    Ogre::Light* sunlight;
};
