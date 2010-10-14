// For conditions of distribution and use, see copyright notice in license.txt

#ifndef incl_EC_Light_EC_Light_h
#define incl_EC_Light_EC_Light_h

#include "IComponent.h"
#include "IAttribute.h"
#include "Declare_EC.h"
#include "Vector3D.h"
#include "Color.h"

namespace Ogre
{
    class Light;
}

/// Makes the entity a light source.
/**

<table class="header">
<tr>
<td>
<h2>Light</h2>
Makes the entity a light source.

Registered by RexLogic::RexLogicModule.

<b>Attributes</b>:
<ul>
<li>Vector3df: direction.
<div>Specifies the direction vector the light is shining at.</div> 
<li>enum: light type. 
<div>One of the values "Point", "Spot" or "Directional".</div> 
<li>Color: diffuse color.
<div>Specifies the color the light casts.</div> 
<li>Color: specular color.
<div>Specifies the color of the reflections the light casts.</div> 
<li>bool: cast shadows.
<div>If true, this light casts dynamically calculated shadows on the scene.</div> 
<li>float: light range.
<div>Specifies how far in world space units the light reaches.</div> 
<li>float: constant attenuation.
<div>Specifies the constant term of the light attenuation equation.</div> 
<li>float: linear attenuation.
<div>Specifies the linear term of the light attenuation equation.</div> 
<li>float: quadratic attenuation.
<div>Specifies the quadratic term of the light attenuation equation.</div> 
<li>float: light inner angle.
<div>Specifies inner umbra angle of the light. Only applicable for spotlights.</div> 
<li>float: light outer angle.   
<div>Specifies outer penumbra angle of the light. Only applicable for spotlights.</div>
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
class EC_Light : public IComponent
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
    ComponentPtr GetPlaceable() const { return placeable_; }
    
    //! Sets placeable component
    /*! Set a null placeable (or do not set a placeable) to have a detached light
        \param placeable placeable component
     */
    void SetPlaceable(ComponentPtr placeable);
    
    //! @return Ogre light pointer
    Ogre::Light* GetLight() const { return light_; }
    
    //! Light direction
    Q_PROPERTY(Vector3df direction READ getdirection WRITE setdirection)
    DEFINE_QPROPERTY_ATTRIBUTE(Vector3df, direction);
    
    //! Light type
    Q_PROPERTY(int type READ gettype WRITE settype)
    DEFINE_QPROPERTY_ATTRIBUTE(int, type);
    
    //! Light diffuse color
    Q_PROPERTY(Color diffColor READ getdiffColor WRITE setdiffColor)
    DEFINE_QPROPERTY_ATTRIBUTE(Color, diffColor);
    
    //! Light specular color
    Q_PROPERTY(Color specColor READ getspecColor WRITE setspecColor)
    DEFINE_QPROPERTY_ATTRIBUTE(Color, specColor);
    
    //! Cast shadows flag
    //! /todo check if this attribute can be removed cause atm it's not in use.
    Q_PROPERTY(bool castShadows READ getcastShadows WRITE setcastShadows)
    DEFINE_QPROPERTY_ATTRIBUTE(bool, castShadows);
    
    //! Light range
    Q_PROPERTY(float range READ getrange WRITE setrange)
    DEFINE_QPROPERTY_ATTRIBUTE(float, range);
    
    //! Light constant attenuation
    Q_PROPERTY(float constAtten READ getconstAtten WRITE setconstAtten)
    DEFINE_QPROPERTY_ATTRIBUTE(float, constAtten);
    
    //! Light linear attenuation
    Q_PROPERTY(float linearAtten READ getlinearAtten WRITE setlinearAtten)
    DEFINE_QPROPERTY_ATTRIBUTE(float, linearAtten);
    
    //! Light quadratic attenuation
    Q_PROPERTY(float quadraAtten READ getquadraAtten WRITE setquadraAtten)
    DEFINE_QPROPERTY_ATTRIBUTE(float, quadraAtten);
    
    //! Spotlight inner angle (degrees)
    Q_PROPERTY(float innerAngle READ getinnerAngle WRITE setinnerAngle)
    DEFINE_QPROPERTY_ATTRIBUTE(float, innerAngle);
    
    //! Spotlight outer angle (degrees)
    Q_PROPERTY(float outerAngle READ getouterAngle WRITE setouterAngle)
    DEFINE_QPROPERTY_ATTRIBUTE(float, outerAngle);
    

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
    ComponentPtr placeable_;
    
    //! Ogre light
    Ogre::Light* light_;
    
    //! Attached flag
    bool attached_;


};

#endif
