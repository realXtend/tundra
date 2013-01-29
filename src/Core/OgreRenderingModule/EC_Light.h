// For conditions of distribution and use, see copyright notice in LICENSE

#pragma once

#include "IComponent.h"
#include "IAttribute.h"
#include "Math/float3.h"
#include "Color.h"
#include "OgreModuleFwd.h"

/// Makes the entity a light source.
/** <table class="header">
    <tr>
    <td>
    <h2>Light</h2>
    Makes the entity a light source.

    <b>Attributes</b>:
    <ul>
    <li>enum: type
    <div> @copydoc type </div>
    <li>Color: diffColor
    <div> @copydoc diffColor </div>
    <li>Color: specColor
    <div> @copydoc specColor </div>
    <li>bool: castShadows
    <div> @copydoc castShadows </div>
    <li>float: range
    <div> @copydoc range </div>
    <li>float: brightness
    <div> @copydoc brightness </div>
    <li>float: constAtten
    <div> @copydoc constAtten </div>
    <li>float: linearAtten
    <div>@copydoc linearAtten </div>
    <li>float: quadraAtten
    <div> @copydoc quadraAtten </div>
    <li>float: innerAngle
    <div> @copydoc innerAngle </div>
    <li>float: outerAngle
    <div>@copydoc outerAngle</div>
    </ul>

    <b>Exposes the following scriptable functions:</b>
    <ul>
    <li> None.
    </ul>

    <b>Reacts on the following actions:</b>
    <ul>
    <li>"Hide": Disables the light from affecting the scene.
    <li>"Show": Enables the light in the scene.
    <li>"ToggleVisibility": Toggles between the enabled and disabled states.
    </ul>
    </td>
    </tr>

    Does not emit any actions.

    <b>Depends on the component @ref EC_Placeable "Placeable"</b>. The position in the Placeable component specifies the position in the world space where this light is placed at.
    </table> */
class EC_Light : public IComponent
{
    Q_OBJECT
    COMPONENT_NAME("EC_Light", 16)
    
public:
    /// Do not directly allocate new components using operator new, but use the factory-based SceneAPI::CreateComponent functions instead.
    explicit EC_Light(Scene* scene);

    virtual ~EC_Light();

    /// light type enumeration
    enum Type
    {
        LT_Point,
        LT_Spot,
        LT_Directional
    };
        
    /// Gets placeable component
    ComponentPtr GetPlaceable() const { return placeable_; }
    
    /// Sets placeable component
    /** Set a null placeable (or do not set a placeable) to have a detached light
        @param placeable placeable component */
    void SetPlaceable(ComponentPtr placeable);
    
    /// @return Ogre light pointer
    Ogre::Light* GetLight() const { return light_; }
    
    /// Light type, see Type.
    Q_PROPERTY(int type READ gettype WRITE settype)
    DEFINE_QPROPERTY_ATTRIBUTE(int, type);
    
    /// Light diffuse color, specifies the color the light casts.
    Q_PROPERTY(Color diffColor READ getdiffColor WRITE setdiffColor)
    DEFINE_QPROPERTY_ATTRIBUTE(Color, diffColor);
    
    /// Light specular color, specifies the color of the reflections the light casts.
    Q_PROPERTY(Color specColor READ getspecColor WRITE setspecColor)
    DEFINE_QPROPERTY_ATTRIBUTE(Color, specColor);

    /// If true, this light casts dynamically calculated shadows on the scene.
    /** @todo Unused; remove? */
    Q_PROPERTY(bool castShadows READ getcastShadows WRITE setcastShadows)
    DEFINE_QPROPERTY_ATTRIBUTE(bool, castShadows);

    /// Specifies how far in world space units the light reaches.
    Q_PROPERTY(float range READ getrange WRITE setrange)
    DEFINE_QPROPERTY_ATTRIBUTE(float, range);
    
    /// Light brightness, specifies the numerator of the light attenuation equation.
    Q_PROPERTY(float brightness READ getbrightness WRITE setbrightness)
    DEFINE_QPROPERTY_ATTRIBUTE(float, brightness);
    
    /// Light constant attenuation, specifies the constant term of the light attenuation equation.
    Q_PROPERTY(float constAtten READ getconstAtten WRITE setconstAtten)
    DEFINE_QPROPERTY_ATTRIBUTE(float, constAtten);
    
    /// Light linear attenuation, specifies the linear term of the light attenuation equation.
    Q_PROPERTY(float linearAtten READ getlinearAtten WRITE setlinearAtten)
    DEFINE_QPROPERTY_ATTRIBUTE(float, linearAtten);
    
    /// Light quadratic attenuation, specifies the quadratic term of the light attenuation equation.
    Q_PROPERTY(float quadraAtten READ getquadraAtten WRITE setquadraAtten)
    DEFINE_QPROPERTY_ATTRIBUTE(float, quadraAtten);
    
    /// Specifies inner umbra angle of the light. Only applicable for spotlights.
    Q_PROPERTY(float innerAngle READ getinnerAngle WRITE setinnerAngle)
    DEFINE_QPROPERTY_ATTRIBUTE(float, innerAngle);

    /// Specifies outer penumbra angle of the light. Only applicable for spotlights.
    Q_PROPERTY(float outerAngle READ getouterAngle WRITE setouterAngle)
    DEFINE_QPROPERTY_ATTRIBUTE(float, outerAngle);

private slots:
    /// Called when the parent entity has been set.
    void UpdateSignals();
    
    /// Handle a component being added to the parent entity, in case it is the missing Placeable we need
    void OnComponentAdded(IComponent* component, AttributeChange::Type change);
    
    /// Called when component has been removed from the parent entity. Checks if the component removed was the placeable, and autodissociates it.
    void OnComponentRemoved(IComponent* component, AttributeChange::Type change);
    
private:
    /// Update light attributes to the Ogre light object.
    void AttributesChanged();

    /// Automatically checks for placeable in same entity
    void CheckForPlaceable();
    
    /// Attaches light to placeable
    void AttachLight();
    
    /// Detaches light from placeable
    void DetachLight();
    
    ComponentPtr placeable_;
    
    /// Ogre world ptr
    OgreWorldWeakPtr world_;
    
    /// Ogre light
    Ogre::Light* light_;
    
    /// Attached to placeable -flag
    bool attached_;
};
