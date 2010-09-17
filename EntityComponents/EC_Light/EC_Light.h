// For conditions of distribution and use, see copyright notice in license.txt

#ifndef incl_EC_Light_EC_Light_h
#define incl_EC_Light_EC_Light_h

#include "ComponentInterface.h"
#include "AttributeInterface.h"
#include "Declare_EC.h"
#include "Vector3D.h"
#include "Color.h"

namespace Ogre
{
    class Light;
}

/// Makes the entity a light source.
/**

<table style="margin: 20px;">
<tr>
<td style="width:500px; height: 100px; border: solid 1px black; background-color: #e0d0ff; vertical-align: top; padding: 5px;">
<h2>Light</h2>
Makes the entity a light source.

Registered by RexLogic::RexLogicModule.

<b>Attributes</b>:
<ul>
<li>Vector3df: direction.
<div style="margin: 5px;">Specifies the direction vector the light is shining at.</div> 
<li>enum: light type. 
<div style="margin: 5px;">One of the values "Point", "Spot" or "Directional".</div> 
<li>Color: diffuse color.
<div style="margin: 5px;">Specifies the color the light casts.</div> 
<li>Color: specular color.
<div style="margin: 5px;">Specifies the color of the reflections the light casts.</div> 
<li>bool: cast shadows.
<div style="margin: 5px;">If true, this light casts dynamically calculated shadows on the scene.</div> 
<li>float: light range.
<div style="margin: 5px;">Specifies how far in world space units the light reaches.</div> 
<li>float: constant attenuation.
<div style="margin: 5px;">Specifies the constant term of the light attenuation equation.</div> 
<li>float: linear attenuation.
<div style="margin: 5px;">Specifies the linear term of the light attenuation equation.</div> 
<li>float: quadratic attenuation.
<div style="margin: 5px;">Specifies the quadratic term of the light attenuation equation.</div> 
<li>float: light inner angle.
<div style="margin: 5px;">Specifies inner umbra angle of the light. Only applicable for spotlights.</div> 
<li>float: light outer angle.   
<div style="margin: 5px;">Specifies outer penumbra angle of the light. Only applicable for spotlights.</div>
</ul>

<b>Exposes the following scriptable functions:</b>
<ul>
<li>...
</ul>

<b>Reacts on the following actions:</b>
<ul>
<li>"hide": Disables the light from affecting the scene.
<li>"show": Enables the light in the scene.
<li>"toggleVisibility": Toggles between the enabled and disabled states.
</ul>
</td>
</tr>

Does not emit any actions.

<b>Depends on the component OgrePlaceable</b>. The position in the OgrePlaceable component specifies the position in the world space where this light is placed at. 
</table>

*/
class EC_Light : public Foundation::ComponentInterface
{
    DECLARE_EC(EC_Light);
    
    Q_OBJECT
    
public:
    //! light type enumeration
    enum Type
    {
        LT_Point,
        LT_Spot,
        LT_Directional
    };
    
    //! Destructor.
    ~EC_Light();
    
    virtual bool IsSerializable() const { return true; }

    //! Gets placeable component
    Foundation::ComponentPtr GetPlaceable() const { return placeable_; }
    
    //! Sets placeable component
    /*! Set a null placeable (or do not set a placeable) to have a detached light
        \param placeable placeable component
     */
    void SetPlaceable(Foundation::ComponentPtr placeable);
    
    //! @return Ogre light pointer
    Ogre::Light* GetLight() const { return light_; }
    
    //! Light direction
    Attribute<Vector3df> directionAttr_;
    
    //! Light type
    Attribute<int> typeAttr_;
    
    //! Light diffuse color
    Attribute<Color> diffColorAttr_;
    
    //! Light specular color
    Attribute<Color> specColorAttr_;
    
    //! Cast shadows flag
    Attribute<bool> castShadowsAttr_;
    
    //! Light range
    Attribute<float> rangeAttr_;
    
    //! Light constant attenuation
    Attribute<float> constAttenAttr_;
    
    //! Light linear attenuation
    Attribute<float> linearAttenAttr_;
    
    //! Light quadratic attenuation
    Attribute<float> quadraAttenAttr_;
    
    //! Spotlight inner angle (degrees)
    Attribute<float> innerAngleAttr_;
    
    //! Spotlight outer angle (degrees)
    Attribute<float> outerAngleAttr_;
    

private slots:
    void UpdateOgreLight();
    
private:
    //! Constuctor.
    /*! \param module Module.
     */
    explicit EC_Light(IModule *module);
    
    //! Attaches light to placeable
    void AttachLight();
    
    //! Detaches light from placeable
    void DetachLight();
    
    //! Placeable component, optional
    Foundation::ComponentPtr placeable_;
    
    //! Ogre light
    Ogre::Light* light_;
    
    //! Attached flag
    bool attached_;


};

#endif
