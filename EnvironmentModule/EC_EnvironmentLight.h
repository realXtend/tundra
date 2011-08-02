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
    <li> Color : sunColor.
    <div> Defines sun color (in Caelum) </div>
    <li> Color : ambientColor.
    <div> Defines scene ambient color. </div>
    <li> Color : sunDiffuseColor.
    <div> Defines sun diffuse color.  </div>
    <li> float3 : sunDirection.
    <div> Defines sun direction vector. </div>
    <li> bool : sunCastShadows.
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
    DEFINE_QPROPERTY_ATTRIBUTE(Color,  sunColor);
    Q_PROPERTY(Color sunColor READ getsunColor WRITE setsunColor);

    /// Defines ambient light color.
    DEFINE_QPROPERTY_ATTRIBUTE(Color, ambientColor);
    Q_PROPERTY(Color ambientColor READ getambientColor WRITE setambientColor); 

    /// Defines sun diffuse light color.
    DEFINE_QPROPERTY_ATTRIBUTE(Color, sunDiffuseColor);
    Q_PROPERTY(Color sunDiffuseColor READ getsunDiffuseColor WRITE setsunDiffuseColor); 

    /// Defines sun light direction.
    DEFINE_QPROPERTY_ATTRIBUTE(float3, sunDirection);
    Q_PROPERTY(float3 sunDirection READ getsunDirection WRITE setsunDirection); 

    /// Do we want the sunlight to cast shadows.
    DEFINE_QPROPERTY_ATTRIBUTE(bool, sunCastShadows);
    Q_PROPERTY(bool sunCastShadows READ getsunCastShadows WRITE setsunCastShadows); 

public slots:
    /// Called If some of the attributes has been changed.
    void OnAttributeUpdated(IAttribute* attribute, AttributeChange::Type change);

    /// Update sun state
    void UpdateSunlight();

    /// Removes sunlight.
    void RemoveSunlight();

    /// Update ambient light state.
    void UpdateAmbientLight();

private:
    void CreateSunlight();

    OgreWorldWeakPtr ogreWorld;
    Ogre::Light* sunlight;
};
