/**
 *  For conditions of distribution and use, see copyright notice in license.txt
 *
 *  @file   EC_OgreEnvironment.h
 *  @brief  Ogre environment component. Gives an access to various scene-related
 *          environment settings, such as sunlight, ambient light and fog.
 *  @note   The CAELUM and HYDRAX defines are set in the root CMakeLists.txt.
 */

#ifndef incl_OgreRenderer_EC_OgreEnvironment_h
#define incl_OgreRenderer_EC_OgreEnvironment_h

#include "ComponentInterface.h"
#include "OgreModuleApi.h"

#include "Color.h"
#include "Vector3D.h"
#include "Declare_EC.h"

#include <OgreColourValue.h>
#include <OgreVector3.h>
#include <QFlags>

namespace Ogre
{
    class Light;
}

namespace Caelum
{
    class CaelumSystem;
}

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

namespace OgreRenderer
{
    class Renderer;
    class GaussianListener;
    typedef boost::shared_ptr<Renderer> RendererPtr;
    typedef boost::weak_ptr<Renderer> RendererWeakPtr;

    /** Ogre environment component.
     *  Gives an access to various scene related environment settings, such as sunlight, ambient light and fog.
     *  \ingroup OgreRenderingModuleClient EnvironmentModuleClient.
     */
    class OGRE_MODULE_API EC_OgreEnvironment : public Foundation::ComponentInterface
    {
        Q_OBJECT
        
        DECLARE_EC(EC_OgreEnvironment);
    public:
        virtual ~EC_OgreEnvironment();

        enum VisualEffectOverride
        {
            None = 1 << 1,
            AmbientLight = 1 << 3,
            SunColor = 1 << 4,
            SunDirection = 1 << 5
        };

        void SetOverride(VisualEffectOverride effect);
        void DisableOverride(VisualEffectOverride effect);

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

        Vector3df GetSunDirection() const;

        /// Whether the sunlight casts shadows or not.
        /// @param enabled Whether the light casts shadows or not.
        void SetSunCastShadows(const bool &enabled);

        /// Set the time of day
        /// @param time as float between 0-1 where 0.0 = midnight and 0.5 = midday
        void SetTime(float time);

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
        bool IsCaelumUsed() const { return useCaelum_; }

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
        EC_OgreEnvironment(IModule *module);

        /// Creates the sunlight.
        void CreateSunlight();

#ifdef CAELUM
        /// Initializes the Caelum system.
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
        /// renderer
        RendererWeakPtr renderer_;

        /// Ogre sunlight
        Ogre::Light *sunlight_;

        /// Is the camera under the water.
        bool cameraUnderWater_;

        /// Caelum system.
        Caelum::CaelumSystem *caelumSystem_;

        /// Is the Caelum system used or not.
        bool useCaelum_;

        /// Is the Hydrax system used or not.
        bool useHydrax_;

        /// Caelum Sunlight ambient color multiplier factor.
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

        /// Camera far clip distance.
        float cameraFarClip_;

        /// Bit mask of Caelum components we use.
        int caelumComponents_;
        /// Is fog color defined by caelum or user.
        bool fog_color_override_;

        QFlags<VisualEffectOverride> override_flags_;

        Ogre::ColourValue userAmbientLight_;
        Ogre::ColourValue userSunColor_;
        Ogre::Vector3 userSunDirection_;

        Hydrax::Hydrax *hydraxSystem_;
        Hydrax::Noise::Perlin *noiseModule_;
        Hydrax::Module::ProjectedGrid *module_;

    };
}

#endif
