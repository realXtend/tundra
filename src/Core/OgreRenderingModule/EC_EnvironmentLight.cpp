// For conditions of distribution and use, see copyright notice in LICENSE

#include "StableHeaders.h"
#define MATH_OGRE_INTEROP
#include "DebugOperatorNew.h"

#include "EC_EnvironmentLight.h"

#include "EC_Placeable.h"
#include "IAttribute.h"
#include "AttributeMetadata.h"
#include "Renderer.h"
#include "Scene/Scene.h"
#include "OgreMaterialUtils.h"
#include "LoggingFunctions.h"
#include "OgreRenderingModule.h"
#include "OgreWorld.h"

#include <Ogre.h>
#include <OgreQuaternion.h>
#include <OgreColourValue.h>

#include "MemoryLeakCheck.h"

EC_EnvironmentLight::EC_EnvironmentLight(Scene* scene) :
    IComponent(scene),
    sunColor(this, "Sunlight color", Color(0.639f,0.639f,0.639f)),
    ambientColor(this, "Ambient light color", OgreWorld::DefaultSceneAmbientLightColor()),
    sunDirection(this, "Sunlight direction vector", float3(-1.f, -1.f, -1.f)),
    sunCastShadows(this, "Sunlight cast shadows", true),
    brightness(this, "Brightness", 1.0f),
    sunlight(0)
{
    if (scene)
        ogreWorld = scene->GetWorld<OgreWorld>();

    UpdateSunlight();
    UpdateAmbientLight();
}

EC_EnvironmentLight::~EC_EnvironmentLight()
{
    RemoveSunlight();
    OgreWorldPtr world = ogreWorld.lock();
    if (world) // Restore the default ambient color so that we don't end up with a black scene.
        world->OgreSceneManager()->setAmbientLight(OgreWorld::DefaultSceneAmbientLightColor());
}

void EC_EnvironmentLight::UpdateSunlight()
{
    if (ogreWorld.expired())
        return;

    if (!sunlight)
        CreateSunlight();
    if (sunlight)
    {
        Color col = sunColor.Get();
        float b = Max(brightness.Get(), 1e-3f);
        // Manually apply brightness multiplier to the sun diffuse color
        col.r *= b;
        col.g *= b;
        col.b *= b;
        
        sunlight->setDiffuseColour(col);
        sunlight->setCastShadows(sunCastShadows.Get());
        sunlight->setDirection(sunDirection.Get());
    }
}

void EC_EnvironmentLight::RemoveSunlight()
{
    OgreWorldPtr world = ogreWorld.lock();
    if (world && sunlight)
        world->OgreSceneManager()->destroyLight(sunlight);
    sunlight = 0;
}

void EC_EnvironmentLight::CreateSunlight()
{
    OgreWorldPtr world = ogreWorld.lock();
    if (world)
    {
        sunlight = world->OgreSceneManager()->createLight(world->GetUniqueObjectName("EC_EnvironmentLight_Sunlight"));
        sunlight->setType(Ogre::Light::LT_DIRECTIONAL);
        sunlight->setSpecularColour(0.0f,0.0f,0.0f);
    }
}

void EC_EnvironmentLight::AttributesChanged()
{
    if (sunColor.ValueChanged() || brightness.ValueChanged() || sunDirection.ValueChanged() || sunCastShadows.ValueChanged())
        UpdateSunlight();
    else if (ambientColor.ValueChanged())
        UpdateAmbientLight();
}

void EC_EnvironmentLight::UpdateAmbientLight()
{
    OgreWorldPtr world = ogreWorld.lock();
    if (world)
        world->OgreSceneManager()->setAmbientLight(ambientColor.Get());
}
