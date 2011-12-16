/**
 *  For conditions of distribution and use, see copyright notice in license.txt
 *
 *  @file   EC_SkyX.cpp
 *  @brief  A sky component using SkyX, http://www.ogre3d.org/tikiwiki/SkyX
 */

#define MATH_OGRE_INTEROP

#include "DebugOperatorNew.h"

#include "EC_SkyX.h"

#include "Scene.h"
#include "Framework.h"
#include "FrameAPI.h"
#include "OgreWorld.h"
#include "Renderer.h"
#include "EC_Camera.h"
#include "Entity.h"
#include "LoggingFunctions.h"
#include "Math/MathFunc.h"
#include "Profiler.h"
#include "Color.h"
#include "AttributeMetadata.h"

#include <SkyX.h>
#include <QDebug>

#include "MemoryLeakCheck.h"

/// @cond PRIVATE

struct EC_SkyXImpl
{
    EC_SkyXImpl() : skyX(0), controller(0), sunlight(0), cloudLayerBottom(0), cloudLayerTop(0)
    {
        controller = new SkyX::BasicController(true);
    }

    ~EC_SkyXImpl()
    {
        if (skyX)
        {
            // Unload normal clouds
            if (skyX->getCloudsManager())
            {
                skyX->getCloudsManager()->unregisterAll();
                skyX->getCloudsManager()->removeAll();
            }

            // Unload volumetric clouds
            if (skyX->getVCloudsManager())
                skyX->getVCloudsManager()->remove();

            skyX->remove();

            Ogre::SceneManager *sm = skyX->getSceneManager();
            if (sm)
            {
                if (sunlight)
                    sm->destroyLight(sunlight);
                sm->setAmbientLight(originalAmbientColor);
            }

            SAFE_DELETE(skyX) // Deletes controller
        }

        sunlight = 0;
        controller = 0;
        cloudLayerBottom = 0;
        cloudLayerTop = 0;
    }

    operator bool () const { return skyX != 0; }

    SkyX::SkyX *skyX;
    SkyX::BasicController *controller;

    SkyX::CloudLayer *cloudLayerBottom;
    SkyX::CloudLayer *cloudLayerTop;

    Ogre::Light *sunlight;
    Color originalAmbientColor;

    float3 currentSunPosition;
};

/// @endcond

EC_SkyX::EC_SkyX(Scene* scene) :
    IComponent(scene),
    volumetricClouds(this, "Generate clouds", false),
    timeMultiplier(this, "Time multiplier", 0.0f),
    time(this, "Time [0-24]", 14.f), 
    sunriseTime(this, "Time sunrise [0-24]", 7.5f),
    sunsetTime(this, "Time sunset [0-24]", 20.5f),
    cloudCoverage(this, "Cloud coverage [0-100]", 50),
    cloudAverageSize(this, "Cloud average size [0-100]", 50),
    moonPhase(this, "Moon phase [0-100]", 50),
    windDirection(this, "Wind direction", 0.0f),
    //windSpeed(this, "Wind speed (volumetric clouds only)", 800.f),
    impl(0)
{
    static AttributeMetadata timeMetaData;
    static AttributeMetadata zeroToHundredMetadata;
    static bool metadataInitialized = false;
    if (!metadataInitialized)
    {
        timeMetaData.minimum = "0.0";
        timeMetaData.maximum = "24.0";
        timeMetaData.step = "1.0";
        zeroToHundredMetadata.minimum = "0.0";
        zeroToHundredMetadata.maximum = "100.0";
        zeroToHundredMetadata.step = "10.0";
        metadataInitialized = true;
    }
    time.SetMetadata(&timeMetaData);
    sunriseTime.SetMetadata(&timeMetaData);
    sunsetTime.SetMetadata(&timeMetaData);
    cloudCoverage.SetMetadata(&zeroToHundredMetadata);
    cloudAverageSize.SetMetadata(&zeroToHundredMetadata);
    moonPhase.SetMetadata(&zeroToHundredMetadata);

    if (framework->IsHeadless())
        return;

    connect(this, SIGNAL(ParentEntitySet()), SLOT(Create()));
}

EC_SkyX::~EC_SkyX()
{
    SAFE_DELETE(impl);
}

float3 EC_SkyX::SunPosition() const
{
    if (impl)
        return impl->currentSunPosition;
    return float3();
}

void EC_SkyX::Create()
{
    SAFE_DELETE(impl);
    if (!ParentScene())
    {
        LogError("EC_SkyX: Aborting creation, parent scene is null!");
        return;
    }

    // Return if main camera is not set
    OgreWorldPtr w = ParentScene()->GetWorld<OgreWorld>();
    if (!w->GetRenderer())
        return;
    if (!w->GetRenderer()->MainCamera())
    {
        connect(w->GetRenderer(), SIGNAL(MainCameraChanged(Entity*)), this, SLOT(Create()), Qt::UniqueConnection);
        return;
    }
    disconnect(w->GetRenderer(), SIGNAL(MainCameraChanged(Entity*)), this, SLOT(Create()));

    // SkyX is a singleton component, refuse to add multiple in a scene!
    bool sceneHasSkyX = false;
    Scene::const_iterator entIter = ParentScene()->begin();
    Scene::const_iterator entEnd = ParentScene()->end();
    while (entIter != entEnd)
    {
        EntityPtr ent = entIter->second;
        if (ent.get())
        {
            Entity::ComponentMap::const_iterator compIter = ent->Components().begin();
            Entity::ComponentMap::const_iterator compEnd = ent->Components().end();
            while(compIter != compEnd)
            {
                ComponentPtr comp = compIter->second;
                if (comp.get() && comp.get() != this && comp->TypeName() == this->TypeName())
                {
                    sceneHasSkyX = true;
                    break;
                }
                ++compIter;
            }

        }
        ++entIter;
        if (sceneHasSkyX)
            break;
    }
    if (sceneHasSkyX)
    {
        LogError("EC_SkyX: Scene already has SkyX component, refusing to create a new one.");
        return;
    }

    // Init internals
    try
    {
        Ogre::SceneManager *sm = w->GetSceneManager();
        Entity *mainCamera = w->GetRenderer()->MainCamera();
        if (!mainCamera)
        {
            LogError("Cannot create SkyX: No main camera set!");
            return;
        }
        impl = new EC_SkyXImpl();
        impl->skyX = new SkyX::SkyX(sm, impl->controller);
        impl->skyX->create();

        // A little change to default atmosphere settings.
        SkyX::AtmosphereManager::Options atOpt = impl->skyX->getAtmosphereManager()->getOptions();
        atOpt.RayleighMultiplier = 0.0045f;
        impl->skyX->getAtmosphereManager()->setOptions(atOpt);

        UpdateAttribute(&volumetricClouds, AttributeChange::Disconnected);
        UpdateAttribute(&timeMultiplier, AttributeChange::Disconnected);
        UpdateAttribute(&time, AttributeChange::Disconnected);

        connect(framework->Frame(), SIGNAL(Updated(float)), SLOT(Update(float)), Qt::UniqueConnection);
        connect(this, SIGNAL(AttributeChanged(IAttribute*, AttributeChange::Type)), SLOT(UpdateAttribute(IAttribute*, AttributeChange::Type)), Qt::UniqueConnection);

        CreateSunlight();
    }
    catch(Ogre::Exception &e)
    {
        // Currently if we try to create more than one SkyX component we end up here due to Ogre internal name collision.
        LogError("Could not create EC_SkyX: " + std::string(e.what()));
    }
}

void EC_SkyX::CreateSunlight()
{
    if (impl)
    {
        // Ambient and sun diffuse color copied from EC_EnvironmentLight
        OgreWorldPtr w = ParentScene()->GetWorld<OgreWorld>();
        Ogre::SceneManager *sm = w->GetSceneManager();
        impl->originalAmbientColor = sm->getAmbientLight();
        sm->setAmbientLight(Color(0.364f, 0.364f, 0.364f, 1.f));

        impl->sunlight = sm->createLight(w->GetRenderer()->GetUniqueObjectName("SkyXSunlight"));
        impl->sunlight->setType(Ogre::Light::LT_DIRECTIONAL);
        impl->sunlight->setDiffuseColour(Color(0.639f,0.639f,0.639f));
        impl->sunlight->setSpecularColour(0.f,0.f,0.f);
        impl->sunlight->setDirection(impl->controller->getSunDirection());
        impl->sunlight->setCastShadows(true);
    }
}

void EC_SkyX::UpdateAttribute(IAttribute *attr, AttributeChange::Type change)
{
    if (!impl)
        return;

    if (attr == &volumetricClouds)
    {
        EC_Camera *cameraComp = GetFramework()->Renderer()->MainCameraComponent();
        Ogre::Camera *camera = cameraComp != 0 ? cameraComp->GetCamera() : 0;

        // Unload normal clouds
        if (impl->skyX->getCloudsManager())
        {
            impl->skyX->getCloudsManager()->unregisterAll();
            impl->skyX->getCloudsManager()->removeAll();

            impl->cloudLayerBottom = 0;
            impl->cloudLayerTop = 0;
        }

        // Unload volumetric clouds
        if (impl->skyX->getVCloudsManager())
        {
            if (camera && impl->skyX->getVCloudsManager()->getVClouds())
                impl->skyX->getVCloudsManager()->getVClouds()->unregisterCamera(camera);
            impl->skyX->getVCloudsManager()->remove();
        }
    
        if (volumetricClouds.Get())
        {
            impl->skyX->getVCloudsManager()->create();
            if (camera && impl->skyX->getVCloudsManager()->getVClouds())
                impl->skyX->getVCloudsManager()->getVClouds()->registerCamera(camera);
        }
        else
        {
            // Default options layer
            SkyX::CloudLayer::Options options;
            impl->cloudLayerBottom = impl->skyX->getCloudsManager()->add(options);

            // Modified options layer;
            options.Height += 50;
            options.Scale *= 2.0f;
            options.TimeMultiplier += 0.002f;

            // Add top layer
            impl->cloudLayerTop = impl->skyX->getCloudsManager()->add(options);

            // Change the color a bit from the default
            SkyX::ColorGradient ambientGradient;
            ambientGradient.addCFrame(SkyX::ColorGradient::ColorFrame(Ogre::Vector3(1,1,1)*0.95, 1.0f));
            ambientGradient.addCFrame(SkyX::ColorGradient::ColorFrame(Ogre::Vector3(0.7,0.7,0.65), 0.625f)); 
            ambientGradient.addCFrame(SkyX::ColorGradient::ColorFrame(Ogre::Vector3(0.6,0.55,0.4), 0.5625f));
            ambientGradient.addCFrame(SkyX::ColorGradient::ColorFrame(Ogre::Vector3(0.6,0.45,0.3)*0.2, 0.5f));
            ambientGradient.addCFrame(SkyX::ColorGradient::ColorFrame(Ogre::Vector3(0.5,0.25,0.25)*0.4, 0.45f));
            ambientGradient.addCFrame(SkyX::ColorGradient::ColorFrame(Ogre::Vector3(0.2,0.2,0.3)*0.2, 0.35f));
            ambientGradient.addCFrame(SkyX::ColorGradient::ColorFrame(Ogre::Vector3(0.2,0.2,0.5)*0.2, 0));
            impl->cloudLayerTop->setAmbientGradient(ambientGradient);
        }
    }
    else if (attr == &timeMultiplier)
    {
        impl->skyX->setTimeMultiplier((Ogre::Real)timeMultiplier.Get());
    }
    else if (attr == &time || attr == &sunsetTime || attr == &sunriseTime)
    {
        // Ignore local changes as its updated constantly 
        // from SkyX when a time multiplier in > 0.
        if (change == AttributeChange::Replicate)
            impl->controller->setTime(Ogre::Vector3(time.Get(), sunriseTime.Get(), sunsetTime.Get()));

        // Update moon phase if time changed
        if (attr == &time && change == AttributeChange::LocalOnly)
        {
            /// @todo Do smarter logic that takes sunrise and sunset into account
            moonPhase.Set(time.Get() * (100.f / 24.f), AttributeChange::LocalOnly); // [0,24] -> [0,100]
        }
    }
    else if (attr == &cloudCoverage || attr == &cloudAverageSize)
    {
        if (!volumetricClouds.Get())
            return;
        
        float skyxCoverage = cloudCoverage.Get() / 100.f; // [0,1]
        float skyxSize = cloudAverageSize.Get() / 100.f; // [0,1]
        impl->skyX->getVCloudsManager()->getVClouds()->setWheater(skyxCoverage, skyxSize, false);
    }
    else if (attr == &moonPhase)
    {
        float realScalePhase = moonPhase.Get() - 50.f; // [-50,50]
        float skyxPhase = (float)realScalePhase / 50.0f; // [-1,1]
        impl->controller->setMoonPhase(skyxPhase);
    }
    else if (attr == &windDirection)
    {
        if (volumetricClouds.Get())
        {
            impl->skyX->getVCloudsManager()->getVClouds()->setWindDirection(Ogre::Radian(DegToRad(windDirection.Get())));
        }
        else
        {
            if (!impl->cloudLayerBottom || !impl->cloudLayerTop)
                return;

            Ogre::Radian r1(Ogre::Degree(windDirection.Get())), r2(Ogre::Degree(windDirection.Get()));
            Ogre::Vector2 ogreWindDirection = Ogre::Vector2(Ogre::Math::Cos(r1), Ogre::Math::Sin(r2));
            
            SkyX::CloudLayer::Options optionsBottom = impl->cloudLayerBottom->getOptions();
            SkyX::CloudLayer::Options optionsTop = impl->cloudLayerTop->getOptions();

            optionsBottom.WindDirection = ogreWindDirection;
            optionsTop.WindDirection = ogreWindDirection;

            impl->cloudLayerBottom->setOptions(optionsBottom);
            impl->cloudLayerTop->setOptions(optionsBottom);            
        }
    }
/*    
    else if (attr == &windSpeed)
    {
       if (volumetricClouds.Get())
           impl->skyX->getVCloudsManager()->setWindSpeed(windSpeed.Get());
    }
*/ 
}

void EC_SkyX::Update(float frameTime)
{
    if (!impl)
        return;
   
    EC_Camera *cameraComp = GetFramework()->Renderer()->MainCameraComponent();
    Ogre::Camera *camera = cameraComp != 0 ? cameraComp->GetCamera() : 0;
    if (!camera)
        return;

    PROFILE(EC_SkyX_Update);

    // This seems like utter nonsense as impl->skyX->notifyCameraRender(camera); will work
    // nicely without doing this manually but you'll get ugly log prints to console.
    if (impl->skyX->getCamera() != camera && impl->skyX->getVCloudsManager() && impl->skyX->getVCloudsManager()->getVClouds())
    {
        SkyX::VClouds::VClouds *vClouds = impl->skyX->getVCloudsManager()->getVClouds();
        if (impl->skyX->getCamera() != 0) 
            vClouds->unregisterCamera(impl->skyX->getCamera());
        vClouds->registerCamera(camera);
    }

    impl->skyX->update(frameTime);
    impl->skyX->notifyCameraRender(camera);
    
    // Update our sunlight
    impl->currentSunPosition = camera->getDerivedPosition() - impl->controller->getSunDirection() * impl->skyX->getMeshManager()->getSkydomeRadius(camera);
    if (impl->sunlight)
        impl->sunlight->setDirection(impl->controller->getSunDirection());

    // Do not replicate constant time attribute updates as SkyX internals are authoritative for it.
    time.Set(impl->controller->getTime().x, AttributeChange::LocalOnly);
}
