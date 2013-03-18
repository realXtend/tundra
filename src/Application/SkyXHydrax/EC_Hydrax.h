/**
    For conditions of distribution and use, see copyright notice in LICENSE

    @file   EC_Hydrax.h
    @brief  A photorealistic water plane component using Hydrax, http://www.ogre3d.org/tikiwiki/Hydrax */

#pragma once

#include "SkyXHydraxApi.h"
#include "IComponent.h"
#include "AssetFwd.h"
#include "AssetReference.h"
#include "AssetRefListener.h"
#include "Math/float3.h"

/// A photorealistic water plane component using Hydrax, http://www.ogre3d.org/tikiwiki/Hydrax
/** This is a singleton type component and only one component per scene is allowed.
    Provides means of creating photorealistic environments together with EC_SkyX.
    @note Requires Hydrax Ogre add-on. */
class SKYX_HYDRAX_API EC_Hydrax : public IComponent
{
    Q_OBJECT
    COMPONENT_NAME("EC_Hydrax", 39)

public:
    /// Do not directly allocate new components using operator new, but use the factory-based SceneAPI::CreateComponent functions instead.
    explicit EC_Hydrax(Scene* scene);
    ~EC_Hydrax();

    /// Different noise modules supported by Hydrax.
/*
    enum NoiseModule
    {
        ProjectedGrid = 0,
        RadialGrid,
        SimpleGrid
    };

    /// Different noise types supported by Hydrax.
    enum NoiseType
    {
        Perlin = 0,
        FFT
    };

    ///
    enum NormalMode
    {
        Texture = 0,
        Vertex,
        RTT
    };
*/

    /// Config file asset reference (.hdx).
    /** Hydrax contains a vast amount of configurable options. The easiest way is to configure these options is to use
        the config file. You can edit the config file with a text editor of your choice. See /bin/media/Hydrax/Hydrax.hdx
        for the example config file. */
    DEFINE_QPROPERTY_ATTRIBUTE(AssetReference, configRef);
    Q_PROPERTY(AssetReference configRef READ getconfigRef WRITE setconfigRef);

    /// Visibility of the water.
    DEFINE_QPROPERTY_ATTRIBUTE(bool, visible);
    Q_PROPERTY(bool visible READ getvisible WRITE setvisible);

    /// Position of the water. This is always authoritative, regardless of the value in the config file.
    DEFINE_QPROPERTY_ATTRIBUTE(float3, position);
    Q_PROPERTY(float3 position READ getposition WRITE setposition);

    /// Hydrax noise module, see NoiseModule
//    DEFINE_QPROPERTY_ATTRIBUTE(int, noiseModule);
//    Q_PROPERTY(int noiseModule READ getnoiseModule WRITE setnoiseModule);

    /// Hydrax noise type, see NoiseType
//    DEFINE_QPROPERTY_ATTRIBUTE(int, noiseType);
//    Q_PROPERTY(int noiseType READ getnoiseType WRITE setnoiseType);

    /// Hydrax noise module, see NormalMode
//    DEFINE_QPROPERTY_ATTRIBUTE(int, normalMode);
//    Q_PROPERTY(int normalMode READ getnormalMode WRITE setnormalMode);

public slots:
    /// Returns the height of the simulated water at the specified world-space coordinates.
    /** @param worldPos World-space point (y omitted).
        @return Water height (world-space y) at the given position, or -1 if the point was outside of the water. */
    float HeightAt(const float3 &worldPos) const;
    /// @overload
    /** @param x X-coordinate in world-space.
        @param z Z-coordinate in world-space. */
    float HeightAt(float x, float z) const;

private:
    struct EC_HydraxImpl;
    EC_HydraxImpl *impl;
    AssetRefListener configRefListener;

    void AttributesChanged();
    void RequestConfigAsset();

private slots:
    void Create();

    /// Called when the main view active camera has changed.
    void OnActiveCameraChanged(Entity *newActiveCamera);
//    void UpdateNoiseModule();
    void Update(float frameTime);

    void ConfigLoadSucceeded(AssetPtr asset);
};
