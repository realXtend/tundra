// For conditions of distribution and use, see copyright notice in license.txt

#ifndef incl_OgreRenderer_EC_OgreEnvironment_h
#define incl_OgreRenderer_EC_OgreEnvironment_h

#include "ComponentInterface.h"
#include "Foundation.h"
#include "OgreModuleApi.h"
#include <ctime>

namespace Ogre
{
    class Light;
}

namespace Caelum
{
    class CaelumSystem;
}

/*
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
*/

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
        
        /// Sets the sun direction.
        void SetSunDirection(const Core::Vector3df& direction);
        
        /// Whether the sunlight casts shadows or not.
        /// @param enabled Whether the light casts shadows or not.
        void SetSunCastShadows(const bool &enabled);
        
        /// Set the time.
        /// @param time Time as UNIX EPOCH seconds.
        void SetTime(time_t time);
        
        /// @return Ogre light pointer
        Ogre::Light* GetSunlight() const { return sunlight_; }
        
        /// Updates the visual effects, e.g. the fog.
        /// @param frametime Time since last frame.
        void UpdateVisualEffects(Core::f64 frametime);
        
        /// Disables the fog.
        void DisableFog();
        
        /// Speeds up the times
        /// @param value
        void SetTimeScale(float value);
        
    private:
        /// Constructor.
        /// \param module Renderer module.
        EC_OgreEnvironment(Foundation::ModuleInterface* module);
        
        /// Creates the sunlight.
        void CreateSunlight();
        
        /// Attaches sunlight to placeable.
        /// \note Not sure if this is needed. Maybe if we want to create custom suns?
        void AttachSunlight();
        
        /// Detaches sunlight from placeable.
        /// \note Not sure if this is needed. Maybe if we want to create custom suns?
        void DetachSunlight();
        
        /// Initializes the Caleum system.
        void InitCaelum();

		/// Shuts down the Caelum system.
		void ShutdownCaelum();
        
        /// Initializes the Hydrax system. 
        void InitHydrax();
        
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
        
        /// Caelum system.
        Caelum::CaelumSystem *caelumSystem_;
        
        /// Is the Caelum system used or not.
        bool useCaelum_;

        /// Is the Caelum system used or not.
        bool useHydrax_;
                
        /// Caleum Sunlight ambient color multiplier factor.
        float sunColorMultiplier_;

        /*Hydrax::Hydrax *hydraxSystem_;
        Hydrax::Noise::Perlin *noiseModule_;
        Hydrax::Module::ProjectedGrid *module_;*/
    };
}

#endif
