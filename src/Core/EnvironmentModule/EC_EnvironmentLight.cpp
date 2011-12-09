// For conditions of distribution and use, see copyright notice in LICENSE

#include "StableHeaders.h"
#define MATH_OGRE_INTEROP
#include "DebugOperatorNew.h"
#include "EC_EnvironmentLight.h"
#include "EC_Placeable.h"
#include "IAttribute.h"
#include "AttributeMetadata.h"
#include "Renderer.h"
#include "Scene.h"
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
    ambientColor(this, "Ambient light color", Color(0.364f, 0.364f, 0.364f, 1.f)),
    sunDirection(this, "Sunlight direction vector", float3(-1.f, -1.f, -1.f)),
    sunCastShadows(this, "Sunlight cast shadows", true),
    brightness(this, "Brightness", 1.0f),
    sunlight(0)
{
    if (scene)
        ogreWorld = scene->GetWorld<OgreWorld>();

    connect(this, SIGNAL(AttributeChanged(IAttribute*, AttributeChange::Type)),
        SLOT(OnAttributeUpdated(IAttribute*, AttributeChange::Type)));

    UpdateSunlight();
    UpdateAmbientLight();
}

EC_EnvironmentLight::~EC_EnvironmentLight()
{
    if (ogreWorld.expired())
        return;

    RemoveSunlight();
}

void EC_EnvironmentLight::UpdateSunlight()
{
    if (ogreWorld.lock() != 0)
    {
        if (!sunlight)
            CreateSunlight();
        
        if (sunlight)
        {
            Color col = sunColor.Get();
            float b = std::max(brightness.Get(), 1e-3f);
            // Manually apply brightness multiplier to the sun diffuse color
            col.r *= b;
            col.g *= b;
            col.b *= b;
            
            sunlight->setDiffuseColour(col);
            sunlight->setCastShadows(sunCastShadows.Get());
            sunlight->setDirection(sunDirection.Get());
        }
    }
}

void EC_EnvironmentLight::RemoveSunlight()
{
    if (ogreWorld.expired())
        return;

    OgreWorldPtr world = ogreWorld.lock();
    if (sunlight != 0)
    {
        Ogre::SceneManager *sceneManager = world->OgreSceneManager();
        sceneManager->destroyLight(sunlight);
        sunlight = 0;
    }
}

void EC_EnvironmentLight::CreateSunlight()
{
    if (ogreWorld.expired())
        return;

    OgreWorldPtr world = ogreWorld.lock();

    Ogre::SceneManager* sceneManager = world->OgreSceneManager();
    sunlight = sceneManager->createLight(world->GetUniqueObjectName("EC_EnvironmentLight_Sunlight"));

    sunlight->setType(Ogre::Light::LT_DIRECTIONAL);
    sunlight->setSpecularColour(0.0f,0.0f,0.0f);
}

void EC_EnvironmentLight::OnAttributeUpdated(IAttribute* attribute, AttributeChange::Type change)
{
    if (attribute == &sunColor || attribute == &brightness || attribute == &sunDirection || attribute == &sunCastShadows)
        UpdateSunlight();
    else if (attribute == &ambientColor )
        UpdateAmbientLight();
}

void EC_EnvironmentLight::UpdateAmbientLight()
{
    if (ogreWorld.lock() != 0) 
    {
        Ogre::SceneManager *sceneMgr = ogreWorld.lock()->OgreSceneManager();
        assert(sceneMgr);
        sceneMgr->setAmbientLight(ambientColor.Get());
    }
}
