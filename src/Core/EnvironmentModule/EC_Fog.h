// For conditions of distribution and use, see copyright notice in LICENSE

#pragma once

#include "IComponent.h"
#include "Color.h"
#include "OgreModuleFwd.h"

/// Environment fog.
/** <table class="header">
    <tr>
    <td>
    <h2>Environment fog</h2>

    Registered by EnviromentComponents plugin.
    Defines the overall fog settings for the whole scene, hence only one component per scene is applicable.
    Sets also the background color of the viewport same as the fog color.

    <b>Attributes</b>:
    <ul>
    <li> int : mode.
    <div> Fog mode (see FogMode), defines how fast the fog density increases, Linear by default. </div>
    <li> Color : color.
    <div> The color of the fog.</div>
    <li> float : startDistance.
    <div> The start distance of the fog, measured from the currently active camera, Linear mode only.</div>
    <li> float : endDistance.
    <div> The end distance of the fog, measured from the currently active camera, Linear mode only</div>
    <li> float : expDensity
    <div> The density of the fog in Exponentially or ExponentiallySquare mode, as a value between 0 and 1. The default is 0.001. </div>
    </ul>
    </table> */
class EC_Fog : public IComponent
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
    DEFINE_QPROPERTY_ATTRIBUTE(int, mode);
    Q_PROPERTY(int mode READ getmode WRITE setmode);

    /// Fog color
    DEFINE_QPROPERTY_ATTRIBUTE(Color, color);
    Q_PROPERTY(Color color READ getcolor WRITE setcolor);

    /// Fog start distance, Linear only.
    DEFINE_QPROPERTY_ATTRIBUTE(float, startDistance);
    Q_PROPERTY(float startDistance READ getstartDistance WRITE setstartDistance);

    /// Fog end distance, Linear only.
    DEFINE_QPROPERTY_ATTRIBUTE(float, endDistance);
    Q_PROPERTY(float endDistance READ getendDistance WRITE setendDistance);

    /// The density of the fog in Exponentially or ExponentiallySquare mode, as a value between 0 and 1. The default is 0.001.
    DEFINE_QPROPERTY_ATTRIBUTE(float, expDensity);
    Q_PROPERTY(float expDensity READ getexpDensity WRITE setexpDensity);

private slots:
    void Update();

private:
    OgreWorldWeakPtr world;
};
