// For conditions of distribution and use, see copyright notice in LICENSE

#pragma once

#include "OgreModuleApi.h"
#include "IComponent.h"
#include "Color.h"
#include "OgreModuleFwd.h"

/// Defines the overall fog settings for the whole scene.
/** <table class="header">
    <tr>
    <td>
    <h2>Fog</h2>

    Defines the overall fog settings for the whole scene, hence only one component per scene is applicable.
    Sets also the background color of the viewport same as the fog color.

    Registered by OgreRenderingModule.

    <b>Attributes</b>:
    <ul>
    <li> int : mode
    <div> @copydoc mode </div>
    <li> Color : color
    <div> @copydoc color </div>
    <li> float : startDistance
    <div> @copydoc startDistance </div>
    <li> float : endDistance
    <div> @copydoc endDistance </div>
    <li> float : expDensity
    <div> @copydoc expDensity </div>
    </ul>
    </table> */
class OGRE_MODULE_API EC_Fog : public IComponent
{
    Q_OBJECT
    COMPONENT_NAME("EC_Fog", 9)

public:
    /// Do not directly allocate new components using operator new, but use the factory-based SceneAPI::CreateComponent functions instead.
    explicit EC_Fog(Scene* scene);
    /// Sets fog to None.
    ~EC_Fog();

    /// Fog modes, copied from Ogre::FogMode. Use when setting @c mode attribute.
    enum FogMode
    {
        None = 0, ///< No fog
        Exponentially, ///< Fog density increases exponentially from the camera (fog = 1/e^(distance * density))
        ExponentiallySquare, ///< Fog density increases at the square of Exponential, i.e. even quicker (fog = 1/e^(distance * density)^2)
        Linear ///< Fog density increases linearly between the start and end distances
    };

    /// Fog mode (see FogMode), defines how fast the fog density increases, Linear by default.
    Q_PROPERTY(int mode READ getmode WRITE setmode);
    DEFINE_QPROPERTY_ATTRIBUTE(int, mode);

    /// Fog color
    Q_PROPERTY(Color color READ getcolor WRITE setcolor);
    DEFINE_QPROPERTY_ATTRIBUTE(Color, color);

    /// Fog start distance, Linear only.
    Q_PROPERTY(float startDistance READ getstartDistance WRITE setstartDistance);
    DEFINE_QPROPERTY_ATTRIBUTE(float, startDistance);

    /// Fog end distance, Linear only.
    Q_PROPERTY(float endDistance READ getendDistance WRITE setendDistance);
    DEFINE_QPROPERTY_ATTRIBUTE(float, endDistance);

    /// The density of the fog in Exponentially or ExponentiallySquare mode, as a value between 0 and 1. The default is 0.001.
    Q_PROPERTY(float expDensity READ getexpDensity WRITE setexpDensity);
    DEFINE_QPROPERTY_ATTRIBUTE(float, expDensity);

private slots:
    void Update();

private:
    OgreWorldWeakPtr world;
};
