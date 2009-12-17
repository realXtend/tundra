// For conditions of distribution and use, see copyright notice in license.txt

#ifndef incl_OgreRenderer_EC_OgreEnvironment_h
#define incl_OgreRenderer_EC_OgreEnvironment_h

#include "Foundation.h"
#include "ComponentInterface.h"
#include "OgreModuleApi.h"

#include <OgreColourValue.h>

namespace Ogre
{
    class Light;
}

#ifdef CAELUM
namespace Caelum
{
    class CaelumSystem;
}
#endif

#ifdef HYDRAX
namespace Hydrax
{
    class Hydrax;
    
    namespace Module
    {
        class ProjectedGrid;
    }
    
    namespace Noise
    {
        class Perlin;
    }
}
#endif

namespace OgreRenderer
{
    class Renderer;
    class EC_OgrePlaceable;

    typedef boost::shared_ptr<Renderer> RendererPtr;

    /** Ogre environment component.
     *  Gives an access to various scene related environment settings, such as sunlight, ambient light and fog.
     *  \ingroup OgreRenderingModuleClient
     */
    class OGRE_MODULE_API EC_OgreEnvironment : public Foundation::ComponentInterface
    {
        Q_OBJECT
        
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
        void SetBackgoundColor(const Color &color);

        /// @return Color of the viewport's background.
        Color GetBackgoundColor() const;

        /// Set ambient light color.
        /// @param color Color.
        void SetAmbientLightColor(const Color &color);

        /// @return The ambient light color.
        Color GetAmbientLightColor() const;

        /// Sets the sun color.
        /// @param color Color of the sun.
        void SetSunColor(const Color &color);

        /// @return Color of the sun.
        Color GetSunColor() const;

        /// Sets the sun direction.
        void SetSunDirection(const Vector3df &direction);

        /// Whether the sunlight casts shadows or not.
        /// @param enabled Whether the light casts shadows or not.
        void SetSunCastShadows(const bool &enabled);

        /// Set the time.
        /// @param time Time as UNIX EPOCH seconds.
        void SetTime(const time_t &time);

        /// @return Ogre light pointer
        Ogre::Light* GetSunlight() const { return sunlight_; }

        /// Updates the visual effects, e.g. the fog.
        /// @param frametime Time since last frame.
        void UpdateVisualEffects(f64 frametime);

        /// Disables the fog.
        void DisableFog();

#ifdef CAELUM
        /// Speeds up the times
        /// @param value 2 doubles etc.
        void SetTimeScale(const float &value);
#endif
        /// @return Is the Caelum system used or not.
        bool IsCaleumUsed() const { return useCaelum_; }

        /// @return Is the Hydrax system used or not.
        bool IsHydraxUsed() const { return useHydrax_; }

        void SetGroundFogStart(float fogStart) { fogStart_ = fogStart; }
        float GetGroundFogStart() const { return fogStart_; }
        
        void SetGroundFogEnd(float fogEnd) { fogEnd_ = fogEnd; }
        float GetGroundFogEnd() const { return fogEnd_; }
        
        void SetGroundFogColor(const Ogre::ColourValue& color) { fogColor_ = color; }
        Ogre::ColourValue GetGroundFogColor() const { return fogColor_; }

        void SetWaterFogStart(float fogStart) { waterFogStart_ = fogStart; }
        float GetWaterFogStart() const { return waterFogStart_; }
        
        void SetWaterFogEnd(float fogEnd) { waterFogEnd_ = fogEnd; }
        float GetWaterFogEnd() const { return waterFogEnd_; }
        
        void SetWaterFogColor(const Ogre::ColourValue& color) { waterFogColor_ = color; }
        Ogre::ColourValue GetWaterFogColor() const { return waterFogColor_; }

        void SetFogColorOverride(bool on) { fog_color_override_ = on; }
        bool GetFogColorOverride() const { return fog_color_override_; }

    private:
        /// Constructor.
        /// \param module Renderer module.
        EC_OgreEnvironment(Foundation::ModuleInterface *module);

        /// Creates the sunlight.
        void CreateSunlight();

        /// Attaches sunlight to placeable.
        /// \note Not sure if this is needed. Maybe if we want to create custom suns?
        void AttachSunlight();

        /// Detaches sunlight from placeable.
        /// \note Not sure if this is needed. Maybe if we want to create custom suns?
        void DetachSunlight();

#ifdef CAELUM
        /// Initializes the Caleum system.
        void InitCaelum();

        /// Shuts down the Caelum system.
        void ShutdownCaelum();
#endif
      
#ifdef HYDRAX
        /// Initializes the Hydrax system. 
        void InitHydrax();
        
        /// Shuts down the Hydrax system.
        void ShutdownHydrax();
#endif

        /// Initializes shadows.
        void InitShadows();

        /// placeable component, optional
        Foundation::ComponentPtr placeable_;

        /// attached to placeable -flag
        bool attached_;

        /// renderer
        RendererPtr renderer_;

        /// Ogre sunlight
        Ogre::Light *sunlight_;

        /// Is the camera under the water.
        bool cameraUnderWater_;

#ifdef CAELUM
        /// Caelum system.
        Caelum::CaelumSystem *caelumSystem_;
#endif
        /// Is the Caelum system used or not.
        bool useCaelum_;

        /// Is the Hydrax system used or not.
        bool useHydrax_;

        /// Caleum Sunlight ambient color multiplier factor.
        float sunColorMultiplier_;

        /// Fog start distance.
        float fogStart_;

        /// Fog end distance.
        float fogEnd_;

        /// Water fog start distance.
        float waterFogStart_;

        /// Water fog end distance.
        float waterFogEnd_;

        /// Fog color.
        Ogre::ColourValue fogColor_;

        /// Water fog color.
        Ogre::ColourValue waterFogColor_;

        /// Camera near clip distance.
        float cameraNearClip_;

        /// Camera far clip distance.
        float cameraFarClip_;

        /// Bit mask of Caelum components we use.
        int caelumComponents_;
        /// Is fog color defined by caelum or user.
        bool fog_color_override_;

#ifdef HYDRAX
        Hydrax::Hydrax *hydraxSystem_;
        Hydrax::Noise::Perlin *noiseModule_;
        Hydrax::Module::ProjectedGrid *module_;
#endif
   
    };
}

#endif
