/**
 *  For conditions of distribution and use, see copyright notice in license.txt
 *
 *  @file   EC_OgreEnvironment.cpp
 *  @brief  Ogre environment component. Gives an access to various scene-related
 *          environment settings, such as sunlight, ambient light and fog.
 *  @note   The CAELUM and HYDRAX defines are set in the root CMakeLists.txt.
 */

#include "StableHeaders.h"
#include "DebugOperatorNew.h"

#include "EC_OgreEnvironment.h"
#include "OgreRenderingModule.h"
#include "Renderer.h"
#include "EC_OgrePlaceable.h"
#include "OgreConversionUtils.h"
#include "OgreShadowCameraSetupFocusedPSSM.h"

#include "CompositionHandler.h"


#include <QSettings>
#include <QFile>

#include <Ogre.h>

#ifdef CAELUM
#include <Caelum.h>
#endif

#ifdef HYDRAX
#include <Hydrax.h>
#include <Noise/Perlin/Perlin.h>
#include <Modules/ProjectedGrid/ProjectedGrid.h>
#endif

#include "MemoryLeakCheck.h"

const float MAX_SUNLIGHT_MULTIPLIER = 1.5f;

namespace OgreRenderer
{
/// Utility tool for clamping fog distance
void ClampFog(Real& start, Real& end, Real farclip)
{
    if (farclip < 10.0) 
        farclip = 10.0;
    if (end > farclip - 10.0)
        end = farclip - 10.0;
    if (start > farclip/3.0)
        start = farclip/3.0;
}

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
    useCaelum_(false),
    useHydrax_(false),
    sunColorMultiplier_(MAX_SUNLIGHT_MULTIPLIER),
    fogStart_(100.f),
    fogEnd_(2000.f),
    waterFogStart_(1.f),
    waterFogEnd_(50.f),
    fogColor_(),
    waterFogColor_(0.2f, 0.4f, 0.35f),
    cameraNearClip_(0.5f),
    cameraFarClip_(2000.f), 
    fog_color_override_(false),
    override_flags_(None)
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
    cameraFarClip_ = renderer_.lock()->GetViewDistance();
}

EC_OgreEnvironment::~EC_OgreEnvironment()
{
    if (renderer_.expired())
        return;
    RendererPtr renderer = renderer_.lock();
        
    SetBackgoundColor(Color(0, 0, 0));
    DisableFog();

    if (sunlight_)
    {
        Ogre::SceneManager *sceneManager = renderer->GetSceneManager();
        sceneManager->destroyLight(sunlight_);
        sunlight_ = 0;
    }
#ifdef CAELUM
        ShutdownCaelum();
#endif

#ifdef HYDRAX
        ShutdownHydrax();
#endif
    ///\todo Is compositorInstance->removeLister(listener) needed here?
    foreach(OgreRenderer::GaussianListener* listener, gaussianListeners_)
        SAFE_DELETE(listener);
}

void EC_OgreEnvironment::SetBackgoundColor(const Color &color)
{
    if (renderer_.expired())
        return;
    RendererPtr renderer = renderer_.lock();
    
    renderer->GetViewport()->setBackgroundColour(ToOgreColor(color));
}

Color EC_OgreEnvironment::GetBackgoundColor() const
{
    if (renderer_.expired())
        return Color(0.0f, 0.0f, 0.0f, 0.0f);
    RendererPtr renderer = renderer_.lock();
    
    return ToCoreColor(renderer->GetViewport()->getBackgroundColour());
}

void EC_OgreEnvironment::SetAmbientLightColor(const Color &color)
{
    if (renderer_.expired())
        return;
    RendererPtr renderer = renderer_.lock();
    
    Ogre::SceneManager* sceneManager = renderer->GetSceneManager();
    sceneManager->setAmbientLight(ToOgreColor(color));
    
    // Assure that there is "not" None-flag set. 
    if ( override_flags_.testFlag(None))
        override_flags_ &= ~None;
    
    override_flags_|=AmbientLight;
    userAmbientLight_ = ToOgreColor(color);
}

Color EC_OgreEnvironment::GetAmbientLightColor() const
{
    if (renderer_.expired())
        return Color(0.0f, 0.0f, 0.0f, 0.0f);
    RendererPtr renderer = renderer_.lock();
            
    Ogre::SceneManager *sceneManager = renderer->GetSceneManager();
    return ToCoreColor(sceneManager->getAmbientLight());
}

void EC_OgreEnvironment::SetSunColor(const Color &color)
{
#ifdef CAELUM 
    if ( caelumSystem_ != 0)
    {
        Caelum::BaseSkyLight* sun = caelumSystem_->getSun();
        if ( sun != 0)
        {          
            sun->setLightColour(ToOgreColor(color));
            
             // Assure that there is "not" None-flag set. 
            if ( override_flags_.testFlag(None))
                override_flags_ &= ~None;

            override_flags_|=SunColor;
            userSunColor_ = ToOgreColor(color); 
        } 
    }
#else
    if (sunlight_)
        sunlight_->setDiffuseColour(ToOgreColor(color));
#endif
}

Color EC_OgreEnvironment::GetSunColor() const
{
#ifdef CAELUM
    if ( caelumSystem_ != 0)
    {
        Ogre::ColourValue color;
        if ( override_flags_.testFlag(SunColor))
        {
            color = userSunColor_;
        }
        else
        {
            Vector3df sunDirection = GetSunDirection();
            color = caelumSystem_->getSunLightColour(caelumSystem_->getUniversalClock()->getJulianSecond(), ToOgreVector3(sunDirection));
        }
        return Color(color.r, color.g, color.b, color.a);
    }
#else
    if ( sunlight_ != 0)
    {
        Ogre::ColourValue color = sunlight_->getDiffuseColour();
        return Color(color.r, color.g, color.b, color.a);
    }
#endif
    return Color(0.0,0.0,0.0,0.0);
}

void EC_OgreEnvironment::SetSunDirection(const Vector3df &direction)
{
#ifdef CAELUM 
    if ( caelumSystem_ != 0)
    {
        Caelum::BaseSkyLight* sun = caelumSystem_->getSun();
        if ( sun != 0)
        {
            sun->setLightDirection(ToOgreVector3(direction));
            
            // Assure that there is "not" None-flag set. 
            if ( override_flags_.testFlag(None))
                override_flags_ &= ~None;
            
            override_flags_|=SunDirection;
            userSunDirection_ = ToOgreVector3(direction);
        }
    }
#else
    if (sunlight_)
        sunlight_->setDirection(ToOgreVector3(direction));
#endif
}

Vector3df EC_OgreEnvironment::GetSunDirection() const
{
#ifdef CAELUM
    if ( caelumSystem_ != 0)
    {
        float julDay = caelumSystem_->getUniversalClock()->getJulianDay();
        float relDayTime = fmod(julDay, 1);
        Ogre::Vector3 sunDir = caelumSystem_->getSunDirection(julDay);
        return Vector3df(sunDir.x, sunDir.y, sunDir.z);
    }
#else
    if ( sunlight_ != 0)
    {
        Ogre::Vector3 vec = sunlight_->getDirection();
        return Vector3df(vec.x, vec.y, vec.z);
    }
#endif
    return Vector3df();
}

void EC_OgreEnvironment::SetSunCastShadows(const bool &enabled)
{
    if (sunlight_)
        sunlight_->setCastShadows(enabled);
}

void EC_OgreEnvironment::SetTime(Real time)
{
    if (time < 0.0)
        time += 1.0;
    if (time >= 1.0)
        time -= 1.0;
    
    int hours = (int)(time * 24.0);
    int minutes = ((int)(time * 24.0*60.0)) % 60;
    int seconds = ((int)(time * 24.0*60.0*60.0)) % 60;
    
#ifdef CAELUM
    // Note: we actually don't use year/month/day, because then worlds would look different
    // based on real-life time of year
    caelumSystem_->getUniversalClock()->setGregorianDateTime(
        1900, 5, 1, hours,
        minutes, seconds);
    // Do not let Caelum clock proceed on its own, authoritative time comes from server
    caelumSystem_->getUniversalClock()->setTimeScale(0);        
#endif
    ///\todo Do something with the time when Caelum is not used?
}

void EC_OgreEnvironment::UpdateVisualEffects(f64 frametime)
{
    if (renderer_.expired())
        return;
    RendererPtr renderer = renderer_.lock();
    Ogre::Camera *camera = renderer->GetCurrentCamera();
    Ogre::Viewport *viewport = renderer->GetViewport();
    Ogre::SceneManager *sceneManager = renderer->GetSceneManager();
        
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
    
    if ( !fog_color_override_)
        fogColor_ = caelumSystem_->getGroundFog()->getColour();
#endif

#ifdef CAELUM
    // Update Caelum system.
    caelumSystem_->notifyCameraChanged(camera);
    caelumSystem_->updateSubcomponents(frametime);

    // Disable specular from the sun & moon for now, because it easily leads to too strong results
    sun->setSpecularColour(0.0f, 0.0f, 0.0f);
    moon->setSpecularColour(0.0f, 0.0f, 0.0f);
#endif

#ifdef HYDRAX
    
    // Update Hydrax system.
    hydraxSystem_->update(frametime);
    sunPos = camera->getPosition();
    sunPos -= caelumSystem_->getSun()->getLightDirection() * 80000;
    hydraxSystem_->setSunPosition(sunPos);
#endif
    Ogre::Entity* water = 0;
    
    cameraFarClip_ = renderer->GetViewDistance();
    
    if ( sceneManager->hasEntity("WaterEntity") )
        water = sceneManager->getEntity("WaterEntity");
              
    if (!water)
    {
        // No water entity, set fog value.
        Real fogStart = fogStart_;
        Real fogEnd = fogEnd_;
        ClampFog(fogStart, fogEnd, cameraFarClip_);
        
        sceneManager->setFog(Ogre::FOG_LINEAR, fogColor_, 0.001, fogStart, fogEnd);
        viewport->setBackgroundColour(fogColor_);
        camera->setFarClipDistance(cameraFarClip_);
    }
    else 
    {
        if(camera->getDerivedPosition().z >= water->getParentNode()->getPosition().z)
        {        
            // We're above the water.
            Real fogStart = fogStart_;
            Real fogEnd = fogEnd_;
            ClampFog(fogStart, fogEnd, cameraFarClip_);
        
#ifdef CAELUM
            caelumSystem_->forceSubcomponentVisibilityFlags(caelumComponents_);
#endif
            sceneManager->setFog(Ogre::FOG_LINEAR, fogColor_, 0.001, fogStart, fogEnd);
            viewport->setBackgroundColour(fogColor_);
            camera->setFarClipDistance(cameraFarClip_);
            cameraUnderWater_ = false;
        }
        else
        {
            // We're below the water.
            Real fogStart = waterFogStart_;
            Real fogEnd = waterFogEnd_;
            Real farClip = waterFogEnd_ + 10.f;
            if (farClip > cameraFarClip_)
                farClip = cameraFarClip_;            
            ClampFog(fogStart, fogEnd, farClip);            
#ifdef CAELUM
            // Hide the Caelum subsystems.
            caelumSystem_->forceSubcomponentVisibilityFlags(Caelum::CaelumSystem::CAELUM_COMPONENTS_NONE);
#endif
            sceneManager->setFog(Ogre::FOG_LINEAR, fogColor_ * waterFogColor_, 0.001, fogStart, fogEnd);
            viewport->setBackgroundColour(fogColor_ * waterFogColor_);
            camera->setFarClipDistance(farClip);
            cameraUnderWater_ = true;
        }
       
    }

#ifdef CAELUM 
    // If sun color and direction are controlled by user then their value are needed to override here. 
    // internally caelum calculates new values for those so they are needed to set again in each update loop.

    if ( override_flags_.testFlag(None) )
        return;

    if ( override_flags_.testFlag(AmbientLight))
    {   
        // Override ambient light.
        sceneManager->setAmbientLight(userAmbientLight_);
    }

    if ( override_flags_.testFlag(SunDirection) )
    {
        // Override sun direction.
        if ( sun != 0 )
            sun->setDirection(userSunDirection_);
    }

    if ( override_flags_.testFlag(SunColor) )
    {
        // Override sun color. 
        if ( sun != 0 )
            sun->setDiffuseColour(userSunColor_);
    }
#endif
}

void EC_OgreEnvironment::SetOverride(VisualEffectOverride effect)
{
    // Note: None override is a god-mode override, then caelum is used. 
    if ( effect == None )
        override_flags_ = QFlags<VisualEffectOverride>(None);
    else
    {
        // Assure that there is "not" None-flag set. 
        if ( override_flags_.testFlag(None))
            override_flags_ &= ~None;
        
        override_flags_ |= effect;   
        
    }

}

void EC_OgreEnvironment::DisableOverride(VisualEffectOverride effect)
{
     if ( override_flags_.testFlag(effect))
        override_flags_ &= ~effect;    
}


void EC_OgreEnvironment::DisableFog()
{
    if (renderer_.expired())
        return;
    RendererPtr renderer = renderer_.lock();
    
    Ogre::SceneManager *sceneManager = renderer->GetSceneManager();
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
    if (renderer_.expired())
        return;
    RendererPtr renderer = renderer_.lock();
    
    Ogre::SceneManager* sceneManager = renderer->GetSceneManager();
    sunlight_ = sceneManager->createLight(renderer->GetUniqueObjectName());
    sunlight_->setType(Ogre::Light::LT_DIRECTIONAL);
    ///\todo Read parameters from config file?
    sunlight_->setDiffuseColour(0.93f, 1, 0.13f);
    sunlight_->setDirection(-1, -1, -1);
    sunlight_->setCastShadows(true);
    SetAmbientLightColor(Color(0.5, 0.5, 0.5, 1));
}

#ifdef CAELUM
void EC_OgreEnvironment::InitCaelum()
{
    using namespace Caelum;
    
    if (renderer_.expired())
        return;
    RendererPtr renderer = renderer_.lock();   

    caelumComponents_ = CaelumSystem::CAELUM_COMPONENTS_NONE;
    caelumComponents_ = caelumComponents_ |
        CaelumSystem::CAELUM_COMPONENT_SKY_DOME |
        CaelumSystem::CAELUM_COMPONENT_MOON |
        CaelumSystem::CAELUM_COMPONENT_SUN |
        CaelumSystem::CAELUM_COMPONENT_POINT_STARFIELD |
        CaelumSystem::CAELUM_COMPONENT_SCREEN_SPACE_FOG |
        CaelumSystem::CAELUM_COMPONENT_GROUND_FOG;
    // Caelum clouds are hidden, otherwise shadows get messed up.

    caelumSystem_ = new CaelumSystem(renderer->GetRoot().get(),
        renderer->GetSceneManager(), (CaelumSystem::CaelumComponent)caelumComponents_);

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
    if (renderer_.expired())
        return;
    RendererPtr renderer = renderer_.lock();   

    // Create Hydrax system.
    hydraxSystem_ = new Hydrax::Hydrax(renderer->GetSceneManager(), renderer->GetCurrentCamera(),
        renderer->GetViewport());

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
    if (renderer_.expired())
        return;
    RendererPtr renderer = renderer_.lock();

    bool using_directx = false;

    
    if(renderer->GetRoot()->getRenderSystem()->getName() == "Direct3D9 Rendering Subsystem")
    {
        using_directx = true;
    }

    QSettings settings(QSettings::IniFormat, QSettings::UserScope, "realXtend","configuration/OgreRenderer");
    QFile file(settings.fileName());
    if(!file.exists())
    {
        settings.beginGroup("shadow-options");
        settings.setValue("soft_shadow", "false");
        settings.endGroup();
    }
    settings.beginGroup("shadow-options");
    //unsigned short shadowTextureSize = settings.value("depthmap_size", "1024").toInt();  */
    float shadowFarDist = 50;
    unsigned short shadowTextureSize = 2048;
    if(using_directx)
    {
        shadowTextureSize = 1024;
    }

    Ogre::SceneManager* sceneManager = renderer->GetSceneManager();

    


    size_t shadowTextureCount = 1;
    if(using_directx)
    {
        shadowTextureCount = 3;
    }
    Ogre::ColourValue shadowColor(0.6f, 0.6f, 0.6f);

    // This is the default material to use for shadow buffer rendering pass, overridable in script.
    // Note that we use the same single material (vertex program) for each object, so we're relying on
    // that we use Ogre software skinning. Hardware skinning would require us to do different vertex programs
    // for skinned/nonskinned geometry.
    std::string ogreShadowCasterMaterial = "rex/ShadowCaster";
    
    

    sceneManager->setShadowColour(shadowColor);
    sceneManager->setShadowTextureCountPerLightType(Ogre::Light::LT_DIRECTIONAL, shadowTextureCount);
    sceneManager->setShadowTextureSettings(shadowTextureSize, shadowTextureCount, Ogre::PF_FLOAT32_RGB);
    sceneManager->setShadowTechnique(Ogre::SHADOWTYPE_TEXTURE_ADDITIVE_INTEGRATED);
    sceneManager->setShadowTextureCasterMaterial(ogreShadowCasterMaterial.c_str());
    sceneManager->setShadowTextureSelfShadow(true);

    Ogre::ShadowCameraSetupPtr shadowCameraSetup;
    if(using_directx)
    {
        OgreShadowCameraSetupFocusedPSSM* pssmSetup = new OgreShadowCameraSetupFocusedPSSM();

        OgreShadowCameraSetupFocusedPSSM::SplitPointList splitpoints;
        splitpoints.push_back(cameraNearClip_);
        splitpoints.push_back(3.5);
        splitpoints.push_back(11);
        splitpoints.push_back(shadowFarDist);
        pssmSetup->setSplitPoints(splitpoints);
        shadowCameraSetup = Ogre::ShadowCameraSetupPtr(pssmSetup);
    }
    else
    {
        Ogre::FocusedShadowCameraSetup* focusedSetup = new Ogre::FocusedShadowCameraSetup();
        shadowCameraSetup = Ogre::ShadowCameraSetupPtr(focusedSetup);
    }
    
    

    
    sceneManager->setShadowCameraSetup(shadowCameraSetup);
    sceneManager->setShadowFarDistance(shadowFarDist);

    // If set to true, problems with objects that clip into the ground
    sceneManager->setShadowCasterRenderBackFaces(false);

    //DEBUG
    /*if(renderer_.expired())
        return;
    Ogre::SceneManager *mngr = renderer_.lock()->GetSceneManager();
    Ogre::TexturePtr shadowTex;
    Ogre::String str("shadowDebug");
    Ogre::Overlay* debugOverlay = Ogre::OverlayManager::getSingleton().getByName(str);
    if(!debugOverlay)
        debugOverlay= Ogre::OverlayManager::getSingleton().create(str);
    for(int i = 0; i<shadowTextureCount;i++)
    {
            shadowTex = mngr->getShadowTexture(i);

            // Set up a debug panel to display the shadow
            Ogre::MaterialPtr debugMat = Ogre::MaterialManager::getSingleton().create(
                "Ogre/DebugTexture" + Ogre::StringConverter::toString(i), 
                Ogre::ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME);
            debugMat->getTechnique(0)->getPass(0)->setLightingEnabled(false);
            Ogre::TextureUnitState *t = debugMat->getTechnique(0)->getPass(0)->createTextureUnitState(shadowTex->getName());
            t->setTextureAddressingMode(Ogre::TextureUnitState::TAM_CLAMP);
            //t = debugMat->getTechnique(0)->getPass(0)->createTextureUnitState("spot_shadow_fade.png");
            //t->setTextureAddressingMode(TextureUnitState::TAM_CLAMP);
            //t->setColourOperation(LBO_ADD);

            Ogre::OverlayContainer* debugPanel = (Ogre::OverlayContainer*)
                (Ogre::OverlayManager::getSingleton().createOverlayElement("Panel", "Ogre/DebugTexPanel" + Ogre::StringConverter::toString(i)));
            debugPanel->_setPosition(0.8, i*0.25+ 0.05);
            debugPanel->_setDimensions(0.2, 0.24);
            debugPanel->setMaterialName(debugMat->getName());
            debugOverlay->add2D(debugPanel);
    }
    debugOverlay->show();*/
    if(settings.value("soft_shadow", "false") == "true")
    {
        for(int i=0;i<shadowTextureCount;i++)
        {
            OgreRenderer::GaussianListener* gaussianListener = new OgreRenderer::GaussianListener(); 
            Ogre::TexturePtr shadowTex = sceneManager->getShadowTexture(0);
            Ogre::RenderTarget* shadowRtt = shadowTex->getBuffer()->getRenderTarget();
            Ogre::Viewport* vp = shadowRtt->getViewport(0);
            Ogre::CompositorInstance *instance = Ogre::CompositorManager::getSingleton().addCompositor(vp, "Gaussian Blur");
            Ogre::CompositorManager::getSingleton().setCompositorEnabled(vp, "Gaussian Blur", true);
            instance->addListener(gaussianListener);
            gaussianListener->notifyViewportSize(vp->getActualWidth(), vp->getActualHeight());
            gaussianListeners_.push_back(gaussianListener);
        }
    }
    settings.endGroup();
}

} // namespace OgreRenderer
