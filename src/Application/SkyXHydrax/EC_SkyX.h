/**
    For conditions of distribution and use, see copyright notice in LICENSE

    @file   EC_SkyX.h
    @brief  A sky component using SkyX, http://www.ogre3d.org/tikiwiki/SkyX */

#pragma once

#include "SkyXHydraxApi.h"
#include "IComponent.h"
#include "Math/float3.h"
#include "Color.h"

namespace Ogre { class Camera; }

/// A Sky component using SkyX, http://www.ogre3d.org/tikiwiki/SkyX
/** This is a singleton type component and only one component per scene is allowed.
    Provides means of creating photorealistic environments together with EC_Hydrax.
    @note Requires SkyX Ogre add-on.
    @note SkyX and EnvironmentLight components can be considered somewhat mutually exclusive. */
class SKYX_HYDRAX_API EC_SkyX : public IComponent
{
    Q_OBJECT
    COMPONENT_NAME("EC_SkyX", 38)

public:
    /// Do not directly allocate new components using operator new, but use the factory-based SceneAPI::CreateComponent functions instead.
    explicit EC_SkyX(Scene* scene);
    ~EC_SkyX();

    /// Different cloud types supported by SkyX
    enum CloudType
    {
        None, ///< Disabled.
        Normal, ///< Cloud layer at fixed height above camera.
        Volumetric, ///< Volumetric clouds.
    };

    /// Used cloud type, see CloudType.
    DEFINE_QPROPERTY_ATTRIBUTE(int, cloudType);
    Q_PROPERTY(int cloudType READ getcloudType WRITE setcloudType);

    /// The time multiplier can be a also a negative number, 0 will disable auto-updating.
    /** @note If time multiplier is altered on a headless Tundra, the changes to the time attribute
              are *not* made accordingly. If you want to guarantee the same time between all participants,
              do not use timeMultiplier, but instead drive the time changes manually from a script. */
    DEFINE_QPROPERTY_ATTRIBUTE(float, timeMultiplier);
    Q_PROPERTY(float timeMultiplier READ gettimeMultiplier WRITE settimeMultiplier);

    /// Time of day in [0,24]h range.
    DEFINE_QPROPERTY_ATTRIBUTE(float, time);
    Q_PROPERTY(float time READ gettime WRITE settime);

    /// Sunrise time in [0,24]h range.
    DEFINE_QPROPERTY_ATTRIBUTE(float, sunriseTime);
    Q_PROPERTY(float sunriseTime READ getsunriseTime WRITE setsunriseTime);

    /// Sunset time in [0,24]h range.
    DEFINE_QPROPERTY_ATTRIBUTE(float, sunsetTime);
    Q_PROPERTY(float sunsetTime READ getsunsetTime WRITE setsunsetTime);

    /// Cloud coverage with range [0,100] (Volumetric clouds only).
    DEFINE_QPROPERTY_ATTRIBUTE(float, cloudCoverage);
    Q_PROPERTY(float cloudCoverage READ getcloudCoverage WRITE setcloudCoverage);

    /// Average cloud size with range [0,100] (Volumetric clouds only).
    DEFINE_QPROPERTY_ATTRIBUTE(float, cloudAverageSize);
    Q_PROPERTY(float cloudAverageSize READ getcloudAverageSize WRITE setcloudAverageSize);

    /// The height at the clouds will reside.
    DEFINE_QPROPERTY_ATTRIBUTE(float, cloudHeight);
    Q_PROPERTY(float cloudHeight READ getcloudHeight WRITE setcloudHeight);

    /// Moon phase with range [0,100] where 0 means fully covered moon, 50 clear moon and 100 fully covered moon.
    DEFINE_QPROPERTY_ATTRIBUTE(float, moonPhase);
    Q_PROPERTY(float moonPhase READ getmoonPhase WRITE setmoonPhase);

    /// Sun inner radius.
    DEFINE_QPROPERTY_ATTRIBUTE(float, sunInnerRadius);
    Q_PROPERTY(float sunInnerRadius READ getsunInnerRadius WRITE setsunInnerRadius);

    /// Sun outer radius.
    DEFINE_QPROPERTY_ATTRIBUTE(float, sunOuterRadius);
    Q_PROPERTY(float sunOuterRadius READ getsunOuterRadius WRITE setsunOuterRadius);

    /// Wind direction, in degrees.
    DEFINE_QPROPERTY_ATTRIBUTE(float, windDirection);
    Q_PROPERTY(float windDirection READ getwindDirection WRITE setwindDirection);

    /// Wind speed. Might need different value with normal versus volumetric clouds to actually get same speed.
    DEFINE_QPROPERTY_ATTRIBUTE(float, windSpeed);
    Q_PROPERTY(float windSpeed READ getwindSpeed WRITE setwindSpeed);

    /// Diffuse color of the sunlight.
    DEFINE_QPROPERTY_ATTRIBUTE(Color, sunlightDiffuseColor);
    Q_PROPERTY(Color sunlightDiffuseColor READ getsunlightDiffuseColor WRITE setsunlightDiffuseColor);

    /// Specular color of the sunlight.
    DEFINE_QPROPERTY_ATTRIBUTE(Color, sunlightSpecularColor);
    Q_PROPERTY(Color sunlightSpecularColor READ getsunlightSpecularColor WRITE setsunlightSpecularColor);

    /// Diffuse color of the moonlight.
    DEFINE_QPROPERTY_ATTRIBUTE(Color, moonlightDiffuseColor);
    Q_PROPERTY(Color moonlightDiffuseColor READ getmoonlightDiffuseColor WRITE setmoonlightDiffuseColor);

    /// Specular color of the moonlight.
    DEFINE_QPROPERTY_ATTRIBUTE(Color, moonlightSpecularColor);
    Q_PROPERTY(Color moonlightSpecularColor READ getmoonlightSpecularColor WRITE setmoonlightSpecularColor);

    /// Scene's ambient light color.
    DEFINE_QPROPERTY_ATTRIBUTE(Color, ambientLightColor);
    Q_PROPERTY(Color ambientLightColor READ getambientLightColor WRITE setambientLightColor);

public slots:
    /// Returns whether or not the sun is visible (above horizon).
    bool IsSunVisible() const;

    /// Returns position of the sun, or nan if not applicable.
    float3 SunPosition() const;

    /// Returns Earth-to-Sun direction of the sun, or nan if not applicable.
    float3 SunDirection() const;

    /// Returns whether or not the moon is visible (above horizon).
    bool IsMoonVisible() const;

    /// Returns position of the moon, or nan if not applicable.
    float3 MoonPosition() const;

    /// Returns Earth-to-Moon direction of the moon, or nan if not applicable.
    float3 MoonDirection() const;

private slots:
    void Create();
    void OnActiveCameraChanged(Entity *camEntity);
    void UpdateAttribute(IAttribute *attr, AttributeChange::Type change);
    void Update(float frameTime);

private:
    struct Impl;
    Impl *impl;

    void Remove();
    void CreateLights();
    void RegisterListeners();
    void UnregisterListeners();
    void UpdateLightsAndPositions();
    
    // VCloudManager register/unregister functions.
    // If input camera is null, Tundra's active camera is used.
    void RegisterCamera(Ogre::Camera *camera = 0); /**< @todo SkyX internal logic, move to Impl. */
    void UnregisterCamera(Ogre::Camera *camera = 0); /**< @todo SkyX internal logic, move to Impl. */
    void HandleVCloudsCamera(Ogre::Camera *camera, bool registerCamera); /**< @todo SkyX internal logic, move to Impl. */
    void ApplyAtmosphereOptions();
    void UnloadNormalClouds(); /**< @todo SkyX internal logic, move to Impl. */
    void UnloadVolumetricClouds(); /**< @todo SkyX internal logic, move to Impl. */

    Ogre::Camera *FindOgreCamera(Entity *cameraEntity) const;
};
