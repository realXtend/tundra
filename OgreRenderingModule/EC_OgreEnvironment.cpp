// For conditions of distribution and use, see copyright notice in license.txt

#include "StableHeaders.h"
#include "Foundation.h"
#include "OgreRenderingModule.h"
#include "Renderer.h"
#include "EC_OgrePlaceable.h"
#include "OgreConversionUtils.h"
#include "EC_OgreEnvironment.h"

#include <Ogre.h>

///\note The CAELUM and HYDRAX defines are set in the root CMakeLists.txt.

#ifdef CAELUM
#include <Caelum.h>
#endif

#ifdef HYDRAX
#include <Hydrax.h>
#include <Noise/Perlin/Perlin.h>
#include <Modules/ProjectedGrid/ProjectedGrid.h>
#endif

const float MAX_SUNLIGHT_MULTIPLIER = 1.5f;

namespace OgreRenderer
{

EC_OgreEnvironment::EC_OgreEnvironment(Foundation::ModuleInterface *module) :
    Foundation::ComponentInterface(module->GetFramework()),
    renderer_(checked_static_cast<OgreRenderingModule*>(module)->GetRenderer()),
    sunlight_(0),
//#ifdef CAELUM
//    caelumSystem_(0),
//#endif
#ifdef HYDRAX
    hydraxSystem_(0),
    noiseModule_(0),
    module_(0),
#endif
    cameraUnderWater_(false),
    attached_(false),
    useCaelum_(false),
    useHydrax_(false),
    sunColorMultiplier_(MAX_SUNLIGHT_MULTIPLIER),
    fogStart_(100.f),
    fogEnd_(500.f),
    waterFogStart_(1.f),
    waterFogEnd_(50.f),
    fogColor_(),
    waterFogColor_(0.2f, 0.4f, 0.35f),
    cameraNearClip_(0.5f),
    cameraFarClip_(500.f), 
    fog_color_override_(false)
{
#ifdef CAELUM
    InitCaelum();
    useCaelum_ = true;
#else
    CreateSunlight();
#endif

#ifdef HYDRAX
    InitHydrax();
    useHydrax_ = true;
#endif

    InitShadows();
}

EC_OgreEnvironment::~EC_OgreEnvironment()
{
    SetBackgoundColor(Color(0, 0, 0));
    DisableFog();

    if (sunlight_)
    {
        DetachSunlight();
        Ogre::SceneManager *sceneManager = renderer_->GetSceneManager();
        sceneManager->destroyLight(sunlight_);
        sunlight_ = 0;
    }

#ifdef CAELUM
        ShutdownCaelum();
#endif

#ifdef HYDRAX
        ShutdownHydrax();
#endif
}

void EC_OgreEnvironment::SetPlaceable(Foundation::ComponentPtr placeable)
{
    DetachSunlight();
    placeable_ = placeable;
    AttachSunlight();
}

void EC_OgreEnvironment::SetBackgoundColor(const Color &color)
{
    renderer_->GetCurrentCamera()->getViewport()->setBackgroundColour(ToOgreColor(color));
}

Color EC_OgreEnvironment::GetBackgoundColor() const
{
    return ToCoreColor(renderer_->GetCurrentCamera()->getViewport()->getBackgroundColour());
}

void EC_OgreEnvironment::SetAmbientLightColor(const Color &color)
{
    Ogre::SceneManager* sceneManager = renderer_->GetSceneManager();
    sceneManager->setAmbientLight(ToOgreColor(color));
}

Color EC_OgreEnvironment::GetAmbientLightColor() const
{
    Ogre::SceneManager *sceneManager = renderer_->GetSceneManager();
    return ToCoreColor(sceneManager->getAmbientLight());
}

void EC_OgreEnvironment::SetSunColor(const Color &color)
{
    if (sunlight_)
        sunlight_->setDiffuseColour(ToOgreColor(color));
}

Color EC_OgreEnvironment::GetSunColor() const
{
    if ( sunlight_ != 0)
    {
        Ogre::ColourValue color = sunlight_->getDiffuseColour();
        return Color(color.r, color.g, color.b, color.a);
    }
    return Color(0.0,0.0,0.0,0.0);
}

void EC_OgreEnvironment::SetSunDirection(const Vector3df &direction)
{
    if (sunlight_)
        sunlight_->setDirection(ToOgreVector3(direction));
}

Vector3df EC_OgreEnvironment::GetSunDirection() const
{
    if ( sunlight_ != 0)
    {
        Ogre::Vector3 vec = sunlight_->getDirection();
        return Vector3df(vec.x, vec.y, vec.z);
    }
    return Vector3df();
}

void EC_OgreEnvironment::SetSunCastShadows(const bool &enabled)
{
    if (sunlight_)
        sunlight_->setCastShadows(enabled);
}

void EC_OgreEnvironment::SetTime(const time_t &time)
{
    assert(time);
    tm *ptm = gmtime(&time);

    // Calculate the time zone difference for hours and minutes.
    Poco::LocalDateTime *mytime = new Poco::LocalDateTime();
    int hour_diff = mytime->hour() - ptm->tm_hour;
    int min_diff = mytime->minute() - ptm->tm_min;
    SAFE_DELETE(mytime);

#ifdef CAELUM
    caelumSystem_->getUniversalClock()->setGregorianDateTime(
        1900 + ptm->tm_year, 1 + ptm->tm_mon, ptm->tm_mday, ptm->tm_hour + hour_diff,
        ptm->tm_min + min_diff, ptm->tm_sec);
#endif
    ///\todo Do something with the time when Caelum is not used?
}

void EC_OgreEnvironment::UpdateVisualEffects(f64 frametime)
{
#ifdef CAELUM
    // Set sunlight attenuation using diffuse multiplier.
    // Seems to be working ok, but feel free to fix if you find better logic and/or values.
    Ogre::ColourValue diffuseMultiplier(sunColorMultiplier_, sunColorMultiplier_, sunColorMultiplier_, 1);
    caelumSystem_->getSun()->setDiffuseMultiplier(diffuseMultiplier);

    Ogre::Light* sun = caelumSystem_->getSun()->getMainLight();
    Ogre::Light* moon = caelumSystem_->getMoon()->getMainLight();

    float sunDirZaxis = caelumSystem_->getSun()->getMainLight()->getDirection().z;
    if (sunDirZaxis > 0)
    {
        sunColorMultiplier_ -= 0.005f;
        if (sunColorMultiplier_ <= 0.05f)
            sunColorMultiplier_ = 0.05f;
    }
    else if(sunDirZaxis < 0)
    {
        sunColorMultiplier_ += 0.010f;
        if (sunColorMultiplier_ >= MAX_SUNLIGHT_MULTIPLIER)
            sunColorMultiplier_ = MAX_SUNLIGHT_MULTIPLIER;
    }

    // Get the sky/sunlight and fog colors from Caelum.
    float julDay = caelumSystem_->getUniversalClock()->getJulianDay();
    float relDayTime = fmod(julDay, 1);
    Ogre::Vector3 sunDir = caelumSystem_->getSunDirection(julDay);
    
    if ( !fog_color_override_)
        fogColor_ = caelumSystem_->getGroundFog()->getColour();
#endif

    // Set fogging
    Ogre::Camera *camera = renderer_->GetCurrentCamera();
    Ogre::SceneManager *sceneManager = renderer_->GetSceneManager();
    Ogre::Entity* water = 0;
    
    if ( sceneManager->hasEntity("WaterEntity") )
        water = sceneManager->getEntity("WaterEntity");
              
    if (!water)
    {
        // No water entity. ///\todo Test. Prolly crashes here.
        sceneManager->setFog(Ogre::FOG_LINEAR, fogColor_, 0.001, fogStart_, fogEnd_);
        camera->getViewport()->setBackgroundColour(fogColor_);
        camera->setFarClipDistance(cameraFarClip_);
    }
    else if(camera->getPosition().z >= water->getParentNode()->getPosition().z)
    {
        // We're above the water.
#ifdef CAELUM
        caelumSystem_->forceSubcomponentVisibilityFlags(caelumComponents_);
#endif
        sceneManager->setFog(Ogre::FOG_LINEAR, fogColor_, 0.001, fogStart_, fogEnd_);
        camera->getViewport()->setBackgroundColour(fogColor_);
        camera->setFarClipDistance(cameraFarClip_);
        cameraUnderWater_ = false;
    }
    else
    {
        // We're below the water.
#ifdef CAELUM
        // Hide the Caelum subsystems.
        caelumSystem_->forceSubcomponentVisibilityFlags(Caelum::CaelumSystem::CAELUM_COMPONENTS_NONE);
#endif
        sceneManager->setFog(Ogre::FOG_LINEAR, fogColor_ * waterFogColor_, 0.001, waterFogStart_, waterFogEnd_);
        camera->getViewport()->setBackgroundColour(fogColor_ * waterFogColor_);
        camera->setFarClipDistance(waterFogEnd_ + 10.f);
        cameraUnderWater_ = true;
    }

#ifdef CAELUM
    // Update Caelum system.
    caelumSystem_->notifyCameraChanged(camera);
    caelumSystem_->updateSubcomponents(frametime);

    // Disable specular from the sun & moon for now, because it easily leads to too strong results
    sun->setSpecularColour(0.0f, 0.0f, 0.0f);
    moon->setSpecularColour(0.0f, 0.0f, 0.0f);
    
    // Get the sun's position. The magic number 80000 is from "Nature" demo app, found from OGRE forum.
    // This would be used for Hydrax.
//    Ogre::Vector3 sunPos = camera->getPosition();
//    sunPos -= caelumSystem_->getSun()->getLightDirection() * 80000;
#endif

#ifdef HYDRAX
    // Update Hydrax system.
    hydraxSystem_->update(frametime);

    //Ogre::Vector3 origPos(-5000, -5000, 20);
    //hydraxSystem_->setPosition(origPos);

    sunPos = camera->getPosition();
    sunPos -= caelumSystem_->getSun()->getLightDirection() * 80000;

    //Ogre::Vector3 flippedSunPos(sunPos.y, sunPos.z, sunPos.x);
    hydraxSystem_->setSunPosition(sunPos);
    //hydraxSystem_->setPosition(Ogre::Vector3(-5000, 20, -5000));
    //hydraxSystem_->setVisible(true);

//        Ogre::Vector3 cam_pos = hydraxSystem_->getCamera()->getPosition();
//        hydraxSystem_->getCamera()->setPosition(cam_pos);
//    std::cout << "C " << hydraxSystem_->getMesh()->getSceneNode()->getPosition() << std::endl;
//        std::cout << "D " << hydraxSystem_->getRttManager()->getPlanesSceneNode()->getPosition()<< std::endl;
#endif
}

void EC_OgreEnvironment::DisableFog()
{
    Ogre::SceneManager *sceneManager = renderer_->GetSceneManager();
    sceneManager->setFog(Ogre::FOG_NONE);
}

#ifdef CAELUM
void EC_OgreEnvironment::SetTimeScale(const float &value)
{
    caelumSystem_->getUniversalClock()->setTimeScale(value);
}
#endif

void EC_OgreEnvironment::CreateSunlight()
{
    Ogre::SceneManager* sceneManager = renderer_->GetSceneManager();
    sunlight_ = sceneManager->createLight(renderer_->GetUniqueObjectName());
    sunlight_->setType(Ogre::Light::LT_DIRECTIONAL);
    ///\todo Read parameters from config file?
    sunlight_->setDiffuseColour(0.93f, 1, 0.13f);
    sunlight_->setDirection(-1, -1, -1);
    sunlight_->setCastShadows(true);
    SetAmbientLightColor(Color(0.5, 0.5, 0.5, 1));
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

#ifdef CAELUM
void EC_OgreEnvironment::InitCaelum()
{
    using namespace Caelum;

    caelumComponents_ = CaelumSystem::CAELUM_COMPONENTS_NONE;
    caelumComponents_ = caelumComponents_ |
        CaelumSystem::CAELUM_COMPONENT_SKY_DOME |
        CaelumSystem::CAELUM_COMPONENT_MOON |
        CaelumSystem::CAELUM_COMPONENT_SUN |
        CaelumSystem::CAELUM_COMPONENT_POINT_STARFIELD |
        CaelumSystem::CAELUM_COMPONENT_SCREEN_SPACE_FOG |
        CaelumSystem::CAELUM_COMPONENT_GROUND_FOG;
    // Caelum clouds are hidden, otherwise shadows get messed up.

    caelumSystem_ = new CaelumSystem(renderer_->GetRoot().get(),
        renderer_->GetSceneManager(), (CaelumSystem::CaelumComponent)caelumComponents_);

    // Flip the Caelum camera and ground node orientations 90 degrees.
    Ogre::Quaternion orientation(Ogre::Degree(90), Ogre::Vector3(1, 0, 0));
    caelumSystem_->getCaelumCameraNode()->setOrientation(orientation);
    caelumSystem_->getCaelumGroundNode()->setOrientation(orientation);

    // We want to manage the fog ourself.
    caelumSystem_->setManageSceneFog(false);

    // Use just one light (the brightest one) at a time.
    caelumSystem_->setEnsureSingleLightSource(true);
    caelumSystem_->setEnsureSingleShadowSource(true);

    caelumSystem_->getMoon()->setDiffuseMultiplier(Ogre::ColourValue(0.25f, 0.25f, 0.25f));
}

void EC_OgreEnvironment::ShutdownCaelum()
{
    caelumSystem_->shutdown(true);
}
#endif

#ifdef HYDRAX
void EC_OgreEnvironment::InitHydrax()
{
    // Create Hydrax system.
    hydraxSystem_ = new Hydrax::Hydrax(renderer_->GetSceneManager(), renderer_->GetCurrentCamera(),
        renderer_->GetCurrentCamera()->getViewport());

    // Create noise module. 
    noiseModule_ = new Hydrax::Noise::Perlin(Hydrax::Noise::Perlin::Options(8, 1.15f, 0.49f, 1.14f, 1.27f));

    /*Ogre::Plane(Ogre::Vector3::UNIT_Z, 0)*/

    // Create water plane
//    Ogre::Plane plane(Ogre::Vector3::NEGATIVE_UNIT_Z, 0);
/*Ogre::MeshManager::getSingleton().createPlane("HydraxWaterMesh",
        Ogre::ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME, plane,
        5000, 5000, 10, 10, true, 1, 1, 1, Ogre::Vector3::UNIT_X);*/
   
    // Create our projected grid module  (Rush)
    /*module_ = new Hydrax::Module::ProjectedGrid(hydraxSystem_,  // Hydrax parent pointer
        noiseModule_,                                            // Noise module
       Ogre::Plane(Ogre::Vector3(0,1,0), Ogre::Vector3(0,0,0)), // Base plane
        Hydrax::MaterialManager::NM_VERTEX,                        // Normal mode
        Hydrax::Module::ProjectedGrid::Options(150));            // Projected grid options
    */

    // Nature
    module_ = new Hydrax::Module::ProjectedGrid(
        hydraxSystem_, noiseModule_,
        Ogre::Plane(Ogre::Vector3(0, 1, 0), Ogre::Vector3(0, 0, 0)),
        Hydrax::MaterialManager::NM_VERTEX,
        Hydrax::Module::ProjectedGrid::Options(256, 3.25f, 0.035f, true));
    
    hydraxSystem_->setModule(static_cast<Hydrax::Module::Module*>(module_));

//    hydraxSystem_->loadCfg("HydraxDemo.hdx");
    hydraxSystem_->loadCfg("goodPreset.hdx"); // from "Rush" demo app

//    hydraxSystem_->setPosition(Ogre::Vector3(0,0,0));
    hydraxSystem_->create();

    // Rotate the water plane.
    Ogre::Quaternion orientation(Ogre::Degree(90), Ogre::Vector3(1, 0, 0));
    hydraxSystem_->rotate(orientation);
//    hydraxSystem_->getMesh()->getSceneNode()->setOrientation(orientation);
//    hydraxSystem_->getRttManager()->getPlanesSceneNode()->setOrientation(orientation);
        
//    hydraxSystem_->setPosition(Ogre::Vector3(-5000, -5000, 20));
    
//    hydraxSystem_->setPolygonMode(Ogre::PM_WIREFRAME);
    
//    hydraxSystem_->getMaterialManager()->getMaterial(Hydrax::MaterialManager::MAT_WATER)->getTechnique(0)->getPass(0)->setFog(true, Ogre::FOG_NONE);
    
    /*hydraxSystem_->setPlanesError(4);
    hydraxSystem_->setDepthLimit(5.2);
    hydraxSystem_->setNormalDistortion(0.035);
    //hydraxSystem_->setDepthColor(Ogre::Vector3(0.04,0.185,0.265));
    hydraxSystem_->setSmoothPower(2.5);
    hydraxSystem_->setCausticsScale(8);
    hydraxSystem_->setCausticsEnd(0.65);
    hydraxSystem_->setGlobalTransparency(0);
    hydraxSystem_->setFullReflectionDistance(99999997952.0);
    hydraxSystem_->setPolygonMode(Ogre::PM_SOLID/*0*//*);
    hydraxSystem_->setFoamScale(0.1);

    hydraxSystem_->getMaterialManager()->getMaterial(Hydrax::MaterialManager::MAT_WATER)->getTechnique(0)->getPass(0)->setFog(true, Ogre::FOG_NONE);
    */
}

void EC_OgreEnvironment::ShutdownHydrax()
{
    hydraxSystem_->remove();
}
#endif

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

} // namespace OgreRenderer
