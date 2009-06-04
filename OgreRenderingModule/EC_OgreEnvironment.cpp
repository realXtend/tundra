// For conditions of distribution and use, see copyright notice in license.txt

#include "StableHeaders.h"
#include "Foundation.h"
#include "OgreRenderingModule.h"
#include "Renderer.h"
#include "EC_OgrePlaceable.h"
#include "EC_OgreEnvironment.h"
#include "OgreConversionUtils.h"

#include <Ogre.h>

#include "Caelum.h"

namespace OgreRenderer
{

EC_OgreEnvironment::EC_OgreEnvironment(Foundation::ModuleInterface* module) :
    Foundation::ComponentInterface(module->GetFramework()),
    renderer_(checked_static_cast<OgreRenderingModule*>(module)->GetRenderer()),
    sunlight_(NULL),
    caelumSystem_(NULL),
    cameraUnderWater_(false),
    attached_(false)
{
    InitShadows();
//    CreateSunlight();
//    SetAmbientLightColor(Core::Color(0, 0, 0, 1));
    InitCaelum();
}

EC_OgreEnvironment::~EC_OgreEnvironment()
{
    SetBackgoundColor(Core::Color(0, 0, 0));
    DisableFog();
    
    if (sunlight_)
    {
        DetachSunlight();
        Ogre::SceneManager *sceneManager = renderer_->GetSceneManager();        
        sceneManager->destroyLight(sunlight_);
        sunlight_ = NULL;
    }

    if (caelumSystem_)
        caelumSystem_->shutdown(true);
}

void EC_OgreEnvironment::SetPlaceable(Foundation::ComponentPtr placeable)
{
    DetachSunlight();
    placeable_ = placeable;
    AttachSunlight();
}

void EC_OgreEnvironment::SetBackgoundColor(const Core::Color &color)
{
    renderer_->GetCurrentCamera()->getViewport()->setBackgroundColour(ToOgreColor(color));
}

Core::Color EC_OgreEnvironment::GetBackgoundColor() const
{
    return ToCoreColor(renderer_->GetCurrentCamera()->getViewport()->getBackgroundColour());
}

void EC_OgreEnvironment::SetAmbientLightColor(const Core::Color &color)
{
    Ogre::SceneManager* sceneManager = renderer_->GetSceneManager();
    sceneManager->setAmbientLight(ToOgreColor(color));
}

Core::Color EC_OgreEnvironment::GetAmbientLightColor() const
{
    Ogre::SceneManager *sceneManager = renderer_->GetSceneManager();
    return ToCoreColor(sceneManager->getAmbientLight());
}

void EC_OgreEnvironment::SetSunColor(const Core::Color& color)
{
    sunlight_->setDiffuseColour(ToOgreColor(color));
}

void EC_OgreEnvironment::SetSunDirection(const Core::Vector3df& direction)
{
    sunlight_->setDirection(ToOgreVector3(direction));
}
    
void EC_OgreEnvironment::SetSunCastShadows(const bool &enabled)
{
    sunlight_->setCastShadows(enabled);
}

void EC_OgreEnvironment::SetTime(time_t time)
{
    assert(time);
    tm *ptm = gmtime(&time);
    
    // Calculate the time zone difference for hours and minutes.
    Poco::LocalDateTime *mytime = new Poco::LocalDateTime();
    int hour_diff = mytime->hour() - ptm->tm_hour;
    int min_diff = mytime->minute() - ptm->tm_min;
    SAFE_DELETE(mytime);
    
    caelumSystem_->getUniversalClock()->setGregorianDateTime(
        1900 + ptm->tm_year, 1 + ptm->tm_mon, ptm->tm_mday, ptm->tm_hour + hour_diff,
        ptm->tm_min + min_diff, ptm->tm_sec);
    
    SetTimeScale(500);
}

void EC_OgreEnvironment::UpdateVisualEffects(Core::f64 frametime)
{
    ///\todo Make this prettier (local -> member variables) when the right values are found.
    float fogStart(50.0f);
    //float fogEnd(150.0f);
    float fogEnd(200.0f);
    float waterFogStart(1.0f);
    //float waterFogEnd(15.0f);
    float waterFogEnd(45.0f);
    Ogre::ColourValue fogColor;
    Ogre::ColourValue waterFogColor(0.2, 0.4, 0.3);
    //Ogre::ColourValue waterFogColor(0.2f, 0.553f, 0.486f);
    Ogre::ColourValue backgroundColor(0.5, 0.5, 1.0);
    Ogre::ColourValue skyColor(1.0, 1.0, 1.0);
    float cameraNearClip(0.5f);
    float cameraFarClip(0); ///\todo Camera far clip must be infinite, otherwise the clouds are clipped ugly.
    
    // Get the sky/sunlight and fog colors from Caelum.
    float julDay = caelumSystem_->getUniversalClock()->getJulianDay();
    float relDayTime = fmod(julDay, 1);
    Ogre::Vector3 sunDir = caelumSystem_->getSunDirection(julDay);
    skyColor = caelumSystem_->getSunLightColour(relDayTime, sunDir);
    fogColor = caelumSystem_->getFogColour(relDayTime, sunDir);
    
    // Hide sun and moon sprites when they're below the water line (z > 0).
    // Also disable the corresponding lights.
    if (caelumSystem_->getSun()->getMainLight()->getDirection().z > 0)
    {
        caelumSystem_->getSun()->setVisibilityFlags(false);
        caelumSystem_->getSun()->getMainLight()->setVisible(false);
    }
    else
    {
        caelumSystem_->getSun()->setVisibilityFlags(true);
        caelumSystem_->getSun()->getMainLight()->setVisible(true);
    }
     
    if (caelumSystem_->getMoon()->getMainLight()->getDirection().z > 0)
    {
        caelumSystem_->getMoon()->setVisibilityFlags(false);
        caelumSystem_->getMoon()->getMainLight()->setVisible(false);
    }
    else
    {
        caelumSystem_->getMoon()->setVisibilityFlags(true);
        caelumSystem_->getMoon()->getMainLight()->setVisible(true);
    }

    // Set fogging    
    Ogre::Camera *camera = renderer_->GetCurrentCamera();
    Ogre::SceneManager *sceneManager = renderer_->GetSceneManager();
    Ogre::Entity *water = sceneManager->getEntity("WaterEntity");
    if (!water)
    {
        // No water entity. ///\todo Test.
        sceneManager->setFog(Ogre::FOG_LINEAR, fogColor * skyColor /* * backgroundColor*/, 0.001, fogStart, fogEnd);
        camera->getViewport()->setBackgroundColour(fogColor * skyColor /* * backgroundColor*/);        
    }
    else if (camera->getPosition().z >= water->getParentNode()->getPosition().z)
    {
        // We're above the water.
        caelumSystem_->forceSubcomponentVisibilityFlags(Caelum::CaelumSystem::CAELUM_COMPONENTS_ALL);  
        
        sceneManager->setFog(Ogre::FOG_LINEAR, fogColor * skyColor /* * backgroundColor*/, 0.001, fogStart, fogEnd);
        camera->getViewport()->setBackgroundColour(fogColor * skyColor /* * backgroundColor*/);

        if (cameraUnderWater_)
        {
            cameraUnderWater_ = false;
            camera->setFarClipDistance(cameraFarClip);
        }
    }
    else
    {
        // We're below the water.
        sceneManager->setFog(Ogre::FOG_LINEAR, skyColor * waterFogColor, 0.001, waterFogStart, waterFogEnd);
	    camera->getViewport()->setBackgroundColour(skyColor * waterFogColor);
	    
	    // Hide the Caleum subsystems.
	    caelumSystem_->forceSubcomponentVisibilityFlags(Caelum::CaelumSystem::CAELUM_COMPONENTS_NONE);
	    
        if (!cameraUnderWater_)
        {
            cameraUnderWater_= true;
            camera->setFarClipDistance(waterFogEnd + 10.f);
        }
    }
    
    // Update Caelum system.
    caelumSystem_->notifyCameraChanged(camera);
    caelumSystem_->updateSubcomponents(frametime);    
}

void EC_OgreEnvironment::DisableFog()
{
    Ogre::SceneManager *sceneManager = renderer_->GetSceneManager();
    sceneManager->setFog(Ogre::FOG_NONE);
}

void EC_OgreEnvironment::SetTimeScale(float value)
{
    caelumSystem_->getUniversalClock()->setTimeScale(value);
}

void EC_OgreEnvironment::CreateSunlight()
{
    Ogre::SceneManager* sceneManager = renderer_->GetSceneManager();
    sunlight_ = sceneManager->createLight(renderer_->GetUniqueObjectName());
    sunlight_->setType(Ogre::Light::LT_DIRECTIONAL);
    ///\todo Read parameters from config file?
    sunlight_->setDiffuseColour(0.93f, 1, 0.13f);
    sunlight_->setPosition(0, 0, 0);
    sunlight_->setDirection(-1, -1, -1);
    sunlight_->setCastShadows(true);

    // Set somekind of ambient light, so that the lights are visible.
    ///\todo Find a good default value.
    SetAmbientLightColor(Core::Color(0.5, 0.5, 0.5, 1));    
}

void EC_OgreEnvironment::AttachSunlight()
{
    if ((placeable_) && (!attached_))
    {
        EC_OgrePlaceable* placeable = checked_static_cast<EC_OgrePlaceable*>(placeable_.get());
        Ogre::SceneNode* node = placeable->GetSceneNode();
        node->attachObject(sunlight_);
        attached_ = true;
    }
}

void EC_OgreEnvironment::DetachSunlight()
{
    if ((placeable_) && (attached_))
    {
        EC_OgrePlaceable* placeable = checked_static_cast<EC_OgrePlaceable*>(placeable_.get());
        Ogre::SceneNode* node = placeable->GetSceneNode();
        node->detachObject(sunlight_);
        attached_ = false;
    }
}

void EC_OgreEnvironment::InitCaelum()
{
    caelumSystem_ = new Caelum::CaelumSystem(renderer_->GetRoot().get(),
        renderer_->GetSceneManager(), Caelum::CaelumSystem::CAELUM_COMPONENTS_ALL);
    
    // Flip the Caelum camera and ground node orientations 90 degrees.
    Ogre::Quaternion orientation(Ogre::Degree(90), Ogre::Vector3(1, 0, 0));
    caelumSystem_->getCaelumCameraNode()->setOrientation(orientation);
    caelumSystem_->getCaelumGroundNode()->setOrientation(orientation);
    
    // We want to manage the fog ourself.
    caelumSystem_->setManageSceneFog(false);
}
       
void EC_OgreEnvironment::InitShadows()
{
    float shadowFarDist = 50;
    unsigned short shadowTextureSize = 2048;
    size_t shadowTextureCount = 1;
    Ogre::ColourValue shadowColor(0.6f, 0.6f, 0.6f);
    
    // This is the default material to use for shadow buffer rendering pass, overridable in script.
    // Note that we use the same single material (vertex program) for each object, so we're relying on
    // that we use Ogre software skinning. Hardware skinning would require us to do different vertex programs
    // for skinned/nonskinned geometry.
    std::string ogreShadowCasterMaterial = "rex/ShadowCaster";
    
    Ogre::SceneManager* sceneManager = renderer_->GetSceneManager();
    sceneManager->setShadowColour(shadowColor);
    sceneManager->setShadowFarDistance(shadowFarDist);

    sceneManager->setShadowTextureSize(shadowTextureSize);
    sceneManager->setShadowTextureCount(shadowTextureCount);

    sceneManager->setShadowTexturePixelFormat(Ogre::PF_FLOAT16_R);
    sceneManager->setShadowTechnique(Ogre::SHADOWTYPE_TEXTURE_ADDITIVE_INTEGRATED);
    sceneManager->setShadowTextureCasterMaterial(ogreShadowCasterMaterial.c_str());
    sceneManager->setShadowTextureSelfShadow(true);
    
    Ogre::ShadowCameraSetupPtr shadowCameraSetup = Ogre::ShadowCameraSetupPtr(new Ogre::FocusedShadowCameraSetup());
    sceneManager->setShadowCameraSetup(shadowCameraSetup);
    
    // If set to true, problems with objects that clip into the ground
    sceneManager->setShadowCasterRenderBackFaces(false);
}

}
