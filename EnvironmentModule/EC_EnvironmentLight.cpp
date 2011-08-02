// For conditions of distribution and use, see copyright notice in license.txt

#include "StableHeaders.h"
#define OGRE_INTEROP
#include "DebugOperatorNew.h"

#include "EC_EnvironmentLight.h"
#include "EnvironmentModule.h"

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
#include <OgreConversionUtils.h>

#include "MemoryLeakCheck.h"

EC_EnvironmentLight::EC_EnvironmentLight(Scene* scene) :
    IComponent(scene),
    sunColorAttr(this, "Sun color", Color(0.639f,0.639f,0.639f)),
    ambientColorAttr(this, "Ambient color", Color(0.364f, 0.364f, 0.364f, 1)),
    sunDiffuseColorAttr(this, "Sun diffuse color", Color(0.93f, 0.93f, 0.93f, 1)),
    sunDirectionAttr(this, "Sun direction vector", float3(-1, -1, -1)),
    fixedTimeAttr(this, "Use fixed time", false),
    currentTimeAttr(this, "Current time", 0.67f),
    sunCastShadowsAttr(this, "Sun cast shadows", true),
    useCaelumAttr(this, "Use Caelum", true),
    sunlight(0)
{
    if (scene)
        ogreWorld = scene->GetWorld<OgreWorld>();

    static AttributeMetadata currentTimeMetaData("", "0", "100", "0.1");
    currentTimeAttr.SetMetadata(&currentTimeMetaData);

    connect(this, SIGNAL(AttributeChanged(IAttribute*, AttributeChange::Type)),
        SLOT(OnAttributeUpdated(IAttribute*, AttributeChange::Type)));

    UpdateSun();
    UpdateAmbientLight();
}

EC_EnvironmentLight::~EC_EnvironmentLight()
{
    if (ogreWorld.expired())
        return;

    RemoveSun();
}

void EC_EnvironmentLight::UpdateSun()
{
    if (ogreWorld.lock() != 0)
    {
        if (sunlight != 0)
        {
            sunlight->setDiffuseColour(sunColorAttr.Get());
            sunlight->setCastShadows(sunCastShadowsAttr.Get());
            sunlight->setDirection(sunDirectionAttr.Get());
        }
        else
            CreateOgreLight();
    }
}

void EC_EnvironmentLight::RemoveSun()
{
    if (ogreWorld.expired() )
        return;

    OgreWorldPtr world = ogreWorld.lock();
    if (sunlight != 0)
    {
        Ogre::SceneManager *sceneManager = world->GetSceneManager();
        sceneManager->destroyLight(sunlight);
        sunlight = 0;
    }
}

void EC_EnvironmentLight::CreateOgreLight()
{
    if (ogreWorld.expired())
        return;

    OgreWorldPtr world = ogreWorld.lock();

    Ogre::SceneManager* sceneManager = world->GetSceneManager();
    sunlight = sceneManager->createLight(world->GetUniqueObjectName("EC_EnvironmentLightSunlight"));
    
    sunlight->setType(Ogre::Light::LT_DIRECTIONAL);
    
    sunlight->setDirection(sunDirectionAttr.Get());
    sunlight->setCastShadows(sunCastShadowsAttr.Get());
    sunlight->setDiffuseColour(sunDiffuseColorAttr.Get());
    sunlight->setSpecularColour(0.0f,0.0f,0.0f);
    ///\todo Read parameters from config file?
    //sunlight->setDiffuseColour(0.93f, 1, 0.13f);
    //sunlight->setDirection(-1, -1, -1);
    //sunlight_->setCastShadows(true);
}

void EC_EnvironmentLight::OnAttributeUpdated(IAttribute* attribute, AttributeChange::Type change)
{
    if (attribute == &sunColorAttr || attribute == &sunDirectionAttr || attribute == &sunCastShadowsAttr)
        UpdateSun();
    else if (attribute == &ambientColorAttr )
        UpdateAmbientLight();
}

void EC_EnvironmentLight::UpdateAmbientLight()
{
    if (ogreWorld.lock() != 0) 
    {
        Ogre::SceneManager *sceneMgr = ogreWorld.lock()->GetSceneManager();
        assert(sceneMgr);
        sceneMgr->setAmbientLight(ambientColorAttr.Get());
    }
}
