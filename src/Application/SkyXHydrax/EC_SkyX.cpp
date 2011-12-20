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
#include "OgreRenderingModule.h"
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
    volumetricClouds(this, "Clouds [volumetric]", false),
    normalClouds(this, "Clouds [normal]", true),
    timeMultiplier(this, "Time multiplier", 0.0f),
    time(this, "Time [0-24]", 14.f), 
    sunriseTime(this, "Time sunrise [0-24]", 7.5f),
    sunsetTime(this, "Time sunset [0-24]", 20.5f),
    cloudCoverage(this, "Cloud coverage [0-100]", 50),
    cloudAverageSize(this, "Cloud average size [0-100]", 50),
    cloudHeight(this, "Cloud height", 100),
    moonPhase(this, "Moon phase [0-100]", 50),
    windDirection(this, "Wind direction", 0.0f),
    windSpeed(this, "Wind speed", 5.0f),
    sunInnerRadius(this, "Sun inner radius", 9.75f),
    sunOuterRadius(this, "Sun outer radius", 10.25f),
    impl(0)
{
    static AttributeMetadata cloudHeightMetaData;
    static AttributeMetadata timeMetaData;
    static AttributeMetadata zeroToHundredMetadata;
    static AttributeMetadata mediumStepMetadata;
    static AttributeMetadata smallStepMetadata;
    static bool metadataInitialized = false;
    if (!metadataInitialized)
    {
        timeMetaData.minimum = "0.0";
        timeMetaData.maximum = "24.0";
        timeMetaData.step = "0.5";
        zeroToHundredMetadata.minimum = "0.0";
        zeroToHundredMetadata.maximum = "100.0";
        zeroToHundredMetadata.step = "10.0";
        cloudHeightMetaData.minimum = "0.0";
        cloudHeightMetaData.maximum = "10000.0";
        cloudHeightMetaData.step = "10.0";
        mediumStepMetadata.step = "0.1";
        smallStepMetadata.step = "0.01";
        metadataInitialized = true;
    }
    time.SetMetadata(&timeMetaData);
    sunriseTime.SetMetadata(&timeMetaData);
    sunsetTime.SetMetadata(&timeMetaData);
    cloudCoverage.SetMetadata(&zeroToHundredMetadata);
    cloudAverageSize.SetMetadata(&zeroToHundredMetadata);
    cloudHeight.SetMetadata(&cloudHeightMetaData);
    moonPhase.SetMetadata(&zeroToHundredMetadata);
    timeMultiplier.SetMetadata(&smallStepMetadata);
    sunInnerRadius.SetMetadata(&mediumStepMetadata);
    sunOuterRadius.SetMetadata(&mediumStepMetadata);

    if (!framework->IsHeadless())
        connect(this, SIGNAL(ParentEntitySet()), SLOT(Create()));
}

EC_SkyX::~EC_SkyX()
{
    Remove();
}

float3 EC_SkyX::SunPosition() const
{
    if (impl)
        return impl->currentSunPosition;
    return float3();
}

void EC_SkyX::Remove()
{
    UnregisterListeners();
    SAFE_DELETE(impl);
}

void EC_SkyX::Create()
{
    if (!ParentScene())
    {
        LogError("EC_SkyX: Aborting creation, parent scene is null!");
        return;
    }

    // Return if main camera is not set
    OgreWorldPtr w = ParentScene()->GetWorld<OgreWorld>();
    if (!w || !w->GetRenderer())
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
        Remove();

        impl = new EC_SkyXImpl();
        impl->skyX = new SkyX::SkyX(w->GetSceneManager(), impl->controller);
        impl->skyX->create();

        RegisterListeners();

        ApplyAtmosphereOptions();
        UpdateAttribute(&volumetricClouds, AttributeChange::Disconnected); // Check both volumetric and normal clouds
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

    if (attr == &volumetricClouds || attr == &normalClouds)
    {
        // If both active, disable the other option so toggle works nicely in ui.
        if (volumetricClouds.Get() && normalClouds.Get())
        {
            if (attr == &volumetricClouds)
                normalClouds.Set(false, AttributeChange::Default);
            else if (attr == &normalClouds)
                volumetricClouds.Set(false, AttributeChange::Default);
            return;
        }

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
            UnregisterCamera();
            impl->skyX->getVCloudsManager()->remove();
        }
    
        // Load volumetric if enabled
        if (volumetricClouds.Get())
        {
            EC_Camera *cameraComp = GetFramework()->Renderer()->MainCameraComponent();
            Ogre::Camera *camera = cameraComp != 0 ? cameraComp->GetCamera() : 0;
            if (!camera)
            {
                LogError("EC_SkyX: Cannot create volumetric clouds without main camera!");
                return;
            }

            impl->skyX->getVCloudsManager()->getVClouds()->setDistanceFallingParams(Ogre::Vector2(1.75f, -1));
            impl->skyX->getVCloudsManager()->create(impl->skyX->getMeshManager()->getSkydomeRadius(camera));
            impl->skyX->getVCloudsManager()->setAutoupdate(false); // Don't update wind speed with time multiplier.
            RegisterCamera(camera);

            // Modify render queue group from the SkyX default 50 to 10 (RENDER_QUEUE_1). Actual sky in SkyX main entity is 5 (RENDER_QUEUE_SKIES_EARLY).
            // This will fix the issues where volumetric clouds would render before transparent materials in our normal scene rendering.
            if (impl->skyX->getVCloudsManager()->getVClouds()->getGeometryManager()->isCreated())
            {
                Ogre::SceneNode *vcloudNode = impl->skyX->getVCloudsManager()->getVClouds()->getGeometryManager()->getSceneNode();
                if (vcloudNode)
                {
                    Ogre::SceneNode::ChildNodeIterator::iterator iter = vcloudNode->getChildIterator().begin();
                    Ogre::SceneNode::ChildNodeIterator::iterator end = vcloudNode->getChildIterator().end();
                    while(iter != end)
                    {
                        Ogre::SceneNode *childNode = dynamic_cast<Ogre::SceneNode*>(iter->second);
                        if (childNode)
                        {
                            for(int i=0; i<childNode->numAttachedObjects(); i++)
                                childNode->getAttachedObject(i)->setRenderQueueGroup(Ogre::RENDER_QUEUE_1);
                        }
                        ++iter;
                    }
                }
            }

            // Update relevant attributes silently now that vclouds have been created
            UpdateAttribute(&cloudCoverage, AttributeChange::Disconnected);
            UpdateAttribute(&windDirection, AttributeChange::Disconnected);
            UpdateAttribute(&windSpeed, AttributeChange::Disconnected);
        }

        // Load normal clouds
        if (normalClouds.Get())
        {
            // Bottom layer
            SkyX::CloudLayer::Options options;
            impl->cloudLayerBottom = impl->skyX->getCloudsManager()->add(options);

            // Top layer
            options.Scale *= 2.0f;
            options.TimeMultiplier += 0.002f;
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

            // Update relevant attributes silently now that normal clouds have been created
            UpdateAttribute(&cloudHeight, AttributeChange::Disconnected);
            UpdateAttribute(&windDirection, AttributeChange::Disconnected);
            UpdateAttribute(&windSpeed, AttributeChange::Disconnected);
        }
    }
    else if (attr == &timeMultiplier)
    {
        // Make the time multiplier scale not be so steep.
        // Our minimum value in EC editor 0.01 is still quite fast.
        float skyxMultiplier = timeMultiplier.Get() / 2.0f;
        impl->skyX->setTimeMultiplier(skyxMultiplier);

        // Sometimes volumetric clouds bug out and speed up when a new time 
        // multiplier is defined. Set autoupdate again so it wont happen.
        if (volumetricClouds.Get())
        {
            impl->skyX->getVCloudsManager()->setAutoupdate(false);
            impl->skyX->getVCloudsManager()->getVClouds()->setWindSpeed(windSpeed.Get());
        }
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
        if (volumetricClouds.Get())
        {
            float skyxCoverage = cloudCoverage.Get() / 100.f; // [0,1]
            float skyxSize = cloudAverageSize.Get() / 100.f; // [0,1]
            impl->skyX->getVCloudsManager()->getVClouds()->setWheater(skyxCoverage, skyxSize, false);
        }
    }
    else if (attr == &cloudHeight)
    {
        float height = cloudHeight.Get();

        if (volumetricClouds.Get())
        {
            // Does not affect volumetric at the moment. This would
            // require to re-create the VClouds with a different radius value.
        }
        if (normalClouds.Get())
        {
            if (impl->cloudLayerBottom)
            {
                SkyX::CloudLayer::Options optionsBottom = impl->cloudLayerBottom->getOptions();
                if (optionsBottom.Height != height)
                {
                    optionsBottom.Height = height;
                    impl->cloudLayerBottom->setOptions(optionsBottom);
                }
            }
            if (impl->cloudLayerTop)
            {
                SkyX::CloudLayer::Options optionsTop = impl->cloudLayerTop->getOptions();
                if (optionsTop.Height != height + 50)
                {
                    optionsTop.Height = height + 50;
                    impl->cloudLayerTop->setOptions(optionsTop);
                }
            }
        }
    }
    else if (attr == &moonPhase)
    {
        float scaledPhase = moonPhase.Get() - 50.f; // [-50,50]
        float skyxPhase = (float)scaledPhase / 50.0f; // [-1,1]
        impl->controller->setMoonPhase(skyxPhase);
    }
    else if (attr == &windDirection)
    {
        if (volumetricClouds.Get())
        {
            impl->skyX->getVCloudsManager()->getVClouds()->setWindDirection(Ogre::Radian(DegToRad(windDirection.Get())));
        }
        if (normalClouds.Get())
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
    else if (attr == &windSpeed)
    {
        if (volumetricClouds.Get())
        {
            impl->skyX->getVCloudsManager()->setAutoupdate(false);
            impl->skyX->getVCloudsManager()->getVClouds()->setWindSpeed(windSpeed.Get());
        }
        if (normalClouds.Get())
        {
            if (!impl->cloudLayerBottom || !impl->cloudLayerTop)
                return;

            SkyX::CloudLayer::Options optionsBottom = impl->cloudLayerBottom->getOptions();
            SkyX::CloudLayer::Options optionsTop = impl->cloudLayerTop->getOptions();

            float speedMultiplier = windSpeed.Get() / 2.0f;
            optionsBottom.TimeMultiplier = speedMultiplier;
            optionsTop.TimeMultiplier = speedMultiplier + 0.5f;

            impl->cloudLayerBottom->setOptions(optionsBottom);
            impl->cloudLayerTop->setOptions(optionsBottom);   
        }
    }
    else if (attr == &sunInnerRadius || attr == &sunOuterRadius)
    {
        ApplyAtmosphereOptions();
    }
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
    if (impl->skyX->getCamera() != camera)
    {
        if (impl->skyX->getCamera() != 0) 
            UnregisterCamera(impl->skyX->getCamera());
        RegisterCamera(camera);
    }
        
    // Update our sunlight
    impl->currentSunPosition = camera->getDerivedPosition() + impl->controller->getSunDirection() * impl->skyX->getMeshManager()->getSkydomeRadius(camera);
    if (impl->sunlight)
    {
        // Sun light looks ugly when coming beneath the water (night time), disable it.
        /// @todo Animate dim the light down and up
        if (impl->currentSunPosition.y < 0 && impl->sunlight->isVisible())
            impl->sunlight->setVisible(false);
        else if (impl->currentSunPosition.y > 0 && !impl->sunlight->isVisible())
            impl->sunlight->setVisible(true);
        impl->sunlight->setDirection(-impl->controller->getSunDirection()); // -(Earth-to-Sun direction)
    }

    // Do not replicate constant time attribute updates as SkyX internals are authoritative for it.
    time.Set(impl->controller->getTime().x, AttributeChange::LocalOnly);
}

// Private

void EC_SkyX::RegisterListeners()
{
    if (GetFramework()->IsHeadless())
        return;
    if (!impl || !impl->skyX)
        return;

    // Register SkyX listeners. This is the proper way to do rendering.
    // If we do our own calls to impl->skyX->update() and impl->skyX->notifyCameraRender()
    // with FrameAPI::Updated() there will be rendering artifact when camera is being moved!
    Ogre::Root::getSingleton().addFrameListener(impl->skyX);

    OgreRenderer::OgreRenderingModule *ogreRenderingModule = GetFramework()->GetModule<OgreRenderer::OgreRenderingModule>();
    OgreRenderer::Renderer *renderer = ogreRenderingModule != 0 ? ogreRenderingModule->GetRenderer().get() : 0;
    Ogre::RenderWindow *window = renderer != 0 ? renderer->GetCurrentRenderWindow() : 0;
    if (window)
        window->addListener(impl->skyX);
    else
        LogError("EC_SkyX: Failed to register listener to render window.");
}

void EC_SkyX::UnregisterListeners()
{
    if (GetFramework()->IsHeadless())
        return;
    if (!impl || !impl->skyX)
        return;

    Ogre::Root::getSingleton().removeFrameListener(impl->skyX);

    // Cant use OgreWorld from parent scene as it would fail in the dtor.
    OgreRenderer::OgreRenderingModule *ogreRenderingModule = GetFramework()->GetModule<OgreRenderer::OgreRenderingModule>();
    OgreRenderer::Renderer *renderer = ogreRenderingModule != 0 ? ogreRenderingModule->GetRenderer().get() : 0;
    Ogre::RenderWindow *window = renderer != 0 ? renderer->GetCurrentRenderWindow() : 0;
    if (window)
        window->removeListener(impl->skyX);
    else
        LogError("EC_SkyX: Failed to unregister listener from render window.");
}

void EC_SkyX::RegisterCamera(Ogre::Camera *camera)
{
    HandleVCloudsCamera(camera, true);
}

void EC_SkyX::UnregisterCamera(Ogre::Camera *camera)
{
    HandleVCloudsCamera(camera, false);
}

void EC_SkyX::HandleVCloudsCamera(Ogre::Camera *camera, bool registerCamera)
{
    if (!impl || !impl->skyX->getVCloudsManager() || !impl->skyX->getVCloudsManager()->getVClouds())
        return;
    if (!camera)
    {
        EC_Camera *cameraComp = GetFramework()->Renderer()->MainCameraComponent();
        camera = cameraComp != 0 ? cameraComp->GetCamera() : 0;
    }
    if (camera)
    {
        if (registerCamera)
            impl->skyX->getVCloudsManager()->getVClouds()->registerCamera(camera);
        else
            impl->skyX->getVCloudsManager()->getVClouds()->unregisterCamera(camera);
    }
}

void EC_SkyX::ApplyAtmosphereOptions()
{
    if (!impl)
        return;

    SkyX::AtmosphereManager::Options options = impl->skyX->getAtmosphereManager()->getOptions();
    if (options.InnerRadius != sunInnerRadius.Get())
        options.InnerRadius = sunInnerRadius.Get();
    if (options.OuterRadius != sunOuterRadius.Get())
        options.OuterRadius = sunOuterRadius.Get();
    impl->skyX->getAtmosphereManager()->setOptions(options);
}
