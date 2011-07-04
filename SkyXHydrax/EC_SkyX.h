/**
 *  For conditions of distribution and use, see copyright notice in license.txt
 *
 *  @file   EC_SkyX.h
 *  @brief  A sky component using SkyX, http://www.ogre3d.org/tikiwiki/SkyX
 */

#pragma once

#include "IComponent.h"
#include "Math/float3.h"

class EC_Camera;
struct EC_SkyXImpl;

/// A Sky component using SkyX, http://www.ogre3d.org/tikiwiki/SkyX
/** This is a singleton type component and only one component per scene is allowed.
    Provides means of creating photorealistic environments together with EC_Hydrax.
    @note Requires SkyX Ogre add-on. */
class EC_SkyX : public IComponent
{
    Q_OBJECT
    COMPONENT_NAME("EC_SkyX", 666)

public:
    /// Do not directly allocate new components using operator new, but use the factory-based SceneAPI::CreateComponent functions instead.
    explicit EC_SkyX(Scene* scene);
    ~EC_SkyX();

    /// Do we want to use volumetric clouds.
    DEFINE_QPROPERTY_ATTRIBUTE(bool, volumetricClouds);
    Q_PROPERTY(bool volumetricClouds READ getvolumetricClouds WRITE setvolumetricClouds);

    /// The time multiplier can be a also a negative number, 0 will disable auto-updating.
    DEFINE_QPROPERTY_ATTRIBUTE(float, timeMultiplier);
    Q_PROPERTY(float timeMultiplier READ gettimeMultiplier WRITE settimeMultiplier);

    /// Time of the day: x = time in [0, 24]h range, y = sunrise hour in [0, 24]h range, z = sunset hour in [0, 24] range.
    DEFINE_QPROPERTY_ATTRIBUTE(float3, time);
    Q_PROPERTY(float3 time READ gettime WRITE settime);

public slots:
    /// Returns position of the sun.
    float3 SunPosition() const;

    void CreateSunlight();

private:
    EC_SkyXImpl *impl;

private slots:
    void Create();

    /// Called when the main view active camera has changed.
    void OnActiveCameraChanged(EC_Camera *newActiveCamera);
    void UpdateAttribute(IAttribute *attr);
    void Update(float frameTime);
};
