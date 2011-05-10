// For conditions of distribution and use, see copyright notice in license.txt

#ifndef EC_FOG_H_
#define EC_FOG_H_

#include "IComponent.h"
#include "Color.h"

namespace Ogre
{
    class ColourValue;
}

namespace Environment
{
/// Environment fog.
/**
<table class="header">
<tr>
<td>
<h2>Environment fog</h2>

Registered by Enviroment::EnvironmentModule. The EC_Fog component defines the overall fog settings for the
whole scene. This component applies whenever the camera is outside of any water cube.

<b>Attributes</b>:
<ul>
<li> float : startDistance.
<div> The start distance from the camera of fog, note this has only effect if useAttr is on (or there does not exist Caelum).  </div>
<li> float : endDistance.
<div> The end distance from the camera of fog, note this has only effect if useAttr is on (or there does not exist Caelum). </div>
<li> Color : color.
<div> Defines what is fog color, note this has only effect if useAttr is on (or there does not exist Caelum). </div>
<li> int : mode.
<div> Fog type ( none, linear, exp, exp2) this defines how fast fog density increases. </div>
<li> bool : use.
<div> Defines that are this fog values used over Caelums calculated fog values. </div>

</ul>

</table>
*/
class EC_Fog : public IComponent
{
    Q_OBJECT

public:
    /// Do not directly allocate new components using operator new, but use the factory-based SceneAPI::CreateComponent functions instead.
    explicit EC_Fog(Framework *fw);

    virtual ~EC_Fog() {}

    /// Fog start distance 
    DEFINE_QPROPERTY_ATTRIBUTE(float, startDistance);
    Q_PROPERTY(float startDistance READ getstartDistance WRITE setstartDistance);

    /// Fog end distance
    DEFINE_QPROPERTY_ATTRIBUTE(float, endDistance);
    Q_PROPERTY(float endDistance READ getendDistance WRITE setendDistance);

    /// Fog color
    DEFINE_QPROPERTY_ATTRIBUTE(Color, color);
    Q_PROPERTY(Color color READ getcolor WRITE setcolor);

    /// Fog mode, defines how Fog density increases.
    DEFINE_QPROPERTY_ATTRIBUTE(int, mode);
    Q_PROPERTY(int mode READ getmode WRITE setmode); 

    /// Use this fog over Caelum (even if Caelum is in use)
    DEFINE_QPROPERTY_ATTRIBUTE(bool, use);
    Q_PROPERTY(bool use READ getuse WRITE setuse);

    /// Returns fog color as Ogre colour value.
    Ogre::ColourValue GetColorAsOgreValue() const;

    COMPONENT_NAME("EC_Fog", 9)
};

}

#endif // EC_FOG
