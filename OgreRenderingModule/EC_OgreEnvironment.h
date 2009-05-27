// For conditions of distribution and use, see copyright notice in license.txt

#ifndef incl_OgreRenderer_EC_OgreEnvironment_h
#define incl_OgreRenderer_EC_OgreEnvironment_h

#include "ComponentInterface.h"
#include "Foundation.h"
#include "OgreModuleApi.h"

namespace Ogre
{

class Light;

}


namespace OgreRenderer
{

class Renderer;
class EC_OgrePlaceable;

typedef boost::shared_ptr<Renderer> RendererPtr;

/** Ogre environment component.
 *  Gives an access to various scene related environment settings, sucs as sunlight, ambient light and fog.
 *  \ingroup OgreRenderingModuleClient
 */
class OGRE_MODULE_API EC_OgreEnvironment : public Foundation::ComponentInterface
{
    DECLARE_EC(EC_OgreEnvironment);
public:
    virtual ~EC_OgreEnvironment();
    
    /// Gets the placeable component.
    Foundation::ComponentPtr GetSunPlaceable() const { return placeable_; }
    
    /// Sets placeable component.
    /// set a null placeable (or do not set a placeable) to have a detached light.
    /// @param placeable Placeable component.
    void SetPlaceable(Foundation::ComponentPtr placeable);
    
    /// Sets the viewport's background color.
    /// @param color Color.
    void SetBackgoundColor(const Core::Color &color);
    
    /// @return Color of the viewport's background.
    Core::Color GetBackgoundColor() const;
    
    /// Set ambient light color.
    /// @param color Color.
    void SetAmbientLightColor(const Core::Color &color);
    
    /// @return The ambient light color.
    Core::Color GetAmbientLightColor() const;
    
    /// Sets the sun color.
    /// @param color Color of the sun.
    void SetSunColor(const Core::Color &color);

    /// @return Color of the sun.
    Core::Color GetSunColor() const;
    
    /// Sets the sunlight attenuation.
    void SetSunAttenuation(float range, float constant, float linear, float quad);
    
    /// Sets the sun direction.
    void SetSunDirection(const Core::Vector3df& direction);
    
    /// Whether the sunlight casts shadows or not.
    /// @param enabled Whether the light casts shadows or not.
    void SetSunCastShadows(const bool &enabled);

    /// @return Ogre light pointer
    Ogre::Light* GetSunlight() const { return sunlight_; }
    
private:
    /// Constructor.
    /// \param module Renderer module.
    EC_OgreEnvironment(Foundation::ModuleInterface* module);
    
    /// Creates the sunlight.
    void CreateSunlight();
    
    /// Attaches sunlight to placeable.
    void AttachSunlight();
    
    /// Detaches sunlight from placeable.
    void DetachSunlight();
    
    /// Initializes shadows.
    void InitShadows();
    
    /// placeable component, optional
    Foundation::ComponentPtr placeable_;
    
    /// renderer
    RendererPtr renderer_;
    
    /// Ogre sunlight
    Ogre::Light *sunlight_;
};

}

#endif
