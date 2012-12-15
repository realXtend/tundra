// For conditions of distribution and use, see copyright notice in LICENSE

#pragma once

#include "OgreModuleApi.h"
#include "IComponent.h"
#include "CoreTypes.h"
#include "Math/float3.h"
#include "OgreModuleFwd.h"
#include "Color.h"

/// Makes the entity a environment light.
/** <table class="header">
    <tr>
    <td>
    <h2>EnvironmentLight</h2>

    Gives an access to scene-related environment settings, such as sunlight and ambient light.

    Enviroment light component is registered by OgreRenderingModule.

    <b>Attributes</b>:
    <ul>
    <li> Color : sunColor.
    <div> @copydoc sunColor </div>
    <li> Color : ambientColor
    <div> @copydoc ambientColor </div>
    <li> float3 : sunDirection
    <div> @copydoc sunDirection </div>
    <li> bool : sunCastShadows
    <div> @copydoc sunCastShadows </div>
    <li> float : brightness
    <div> @copydoc brightness </div>
    </table> */
class OGRE_MODULE_API EC_EnvironmentLight : public IComponent
{
    Q_OBJECT
    COMPONENT_NAME("EC_EnvironmentLight", 8)

public:
    /// Do not directly allocate new components using operator new, but use the factory-based SceneAPI::CreateComponent functions instead.
    explicit EC_EnvironmentLight(Scene* scene);
    virtual ~EC_EnvironmentLight();

    /// Defines sun (diffuse) color.
    Q_PROPERTY(Color sunColor READ getsunColor WRITE setsunColor);
    DEFINE_QPROPERTY_ATTRIBUTE(Color,  sunColor);

    /// Defines ambient light color.
    Q_PROPERTY(Color ambientColor READ getambientColor WRITE setambientColor);
    DEFINE_QPROPERTY_ATTRIBUTE(Color, ambientColor);

    /// Defines sun light direction.
    Q_PROPERTY(float3 sunDirection READ getsunDirection WRITE setsunDirection);
    DEFINE_QPROPERTY_ATTRIBUTE(float3, sunDirection);

    /// Do we want the sunlight to cast shadows.
    Q_PROPERTY(bool sunCastShadows READ getsunCastShadows WRITE setsunCastShadows);
    DEFINE_QPROPERTY_ATTRIBUTE(bool, sunCastShadows);

    /// Sunlight brightness multiplier.
    Q_PROPERTY(float brightness READ getbrightness WRITE setbrightness);
    DEFINE_QPROPERTY_ATTRIBUTE(float, brightness);

public slots:
    /// Update sun state
    void UpdateSunlight();

    /// Removes sunlight.
    void RemoveSunlight();

    /// Update ambient light state.
    void UpdateAmbientLight();

private:
    void AttributesChanged();
    void CreateSunlight();

    OgreWorldWeakPtr ogreWorld;
    Ogre::Light* sunlight;
};
